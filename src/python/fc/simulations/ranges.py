"""Copyright (c) 2005-2013, University of Oxford.
All rights reserved.

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.

This file is part of Chaste.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
 * Neither the name of the University of Oxford nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
"""

from ..language import values as V

class AbstractRange(V.Simple):
    """Base class for ranges in the protocol language."""
    def Initialise(self, env):
        pass

    @property
    def value(self):
        return self.current

    def GetCurrentOutputPoint(self):
        return self.current

    def GetCurrentOutputNumber(self):
        return self.count - 1


class UniformRange(AbstractRange):
    def __init__(self, name, startExpr, endExpr, stepExpr):
        self.name = name
        self.startExpr = startExpr
        self.endExpr = endExpr
        self.stepExpr = stepExpr
        self.current = float('nan')
        self.count = 0
        
    def __iter__(self):
        self.count = 0
        self.current = self.start
        return self
    
    def next(self):
        if self.current >= self.end:
            self.count = 0
            raise StopIteration
        else:
            self.count += 1
            self.current = self.start + self.step * (self.count - 1)
            return self.current
        
    def Initialise(self, env):
        self.env = env
        self.start = self.startExpr.Evaluate(self.env).value
        self.step = self.stepExpr.Evaluate(self.env).value
        self.end = self.endExpr.Evaluate(self.env).value
        self.current = self.start
        
    def GetNumberOfOutputPoints(self):
        return (round(self.end-self.start)/self.step) + 1


class VectorRange(AbstractRange):
    def __init__(self, name, arrOrExpr):
        self.name = name
        if isinstance(arrOrExpr, V.Array):
            self.expr = None
            self.arrRange = arrOrExpr.array
            self.current = self.arrRange[0]
        else:
            self.expr = arrOrExpr
            self.current = float('nan')
        self.count = 0
    
    def Initialise(self, env):
        self.env = env
        if self.expr:
            self.arrRange = self.expr.Evaluate(env).array
            self.current = self.arrRange[0]
        
    def __iter__(self):
        self.count = 0
        self.current = 0
        return self
    
    def next(self):
        if self.count >= len(self.arrRange):
            self.count = 0
            raise StopIteration
        else:
            self.current = self.arrRange[self.count]
            self.env.unwrappedBindings[self.name] = self.current
            self.count += 1
            return self.current
        
    def GetNumberOfOutputPoints(self):
        return len(self.arrRange)


class While(AbstractRange):
    def __init__(self, name, condition):
        self.name = name
        self.condition = condition
        self.count = 0
        self.numberOfOutputs = 1000

    @property
    def current(self):
        return self.count - 1
    
    def __iter__(self):
        self.count = 0
        return self
    
    def Initialise(self, env):
        self.env = env
        self.env.bindings[self.name] = V.Simple(self.current)
        self.env.unwrappedBindings[self.name] = self.current

    def next(self):
        self.count += 1
        self.env.bindings[self.name] = self
        self.env.unwrappedBindings[self.name] = self.current
        if self.count >= self.numberOfOutputs:
            self.numberOfOutputs += 1000
        if self.count > 1 and not self.condition.Evaluate(self.env).value:
            self.numberOfOutputs = self.count - 1
            raise StopIteration
        else:
            return self.current

    def GetNumberOfOutputPoints(self):
        return self.numberOfOutputs