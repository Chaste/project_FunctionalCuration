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
import Environment as Env
import scipy.integrate
import Values as V

class AbstractRange(V.Simple):
    """Base class for ranges in the protocol language."""
    def Initialise(self, env):
        pass
    
    @property
    def value(self):
        return self.current  
    
class UniformRange(AbstractRange):
    def __init__(self, name, start, end, step):
        self.name = name
        self.start = start.value
        self.end = end.value
        self.step = step.value
        self.current = self.start
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
        
        
    def GetNumberOfOutputPoints(self):
        return (round(self.end-self.start)/self.step) + 1
    
    def GetCurrentOutputPoint(self):
        return self.current
    
    def GetCurrentOutputNumber(self):
        return self.count - 1
    
class VectorRange(AbstractRange):
    def __init__(self, name, arrOrExpr):
        self.name = name
        if isinstance(arrOrExpr, V.Array):
            self.expr = None
            self.arrRange = arrOrExpr
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
    
    def GetCurrentOutputPoint(self):
        return self.current
    
    def GetCurrentOutputNumber(self):
        return self.count - 1
    
class While(AbstractRange):
    def __init__(self, name, condition):
        self.name = name
        self.condition = condition
        self.count = 0
        self.numberOfOutputs
        
    def __iter__(self):
        self.count = 0
        return self
    
    # initialize class saves the environment locally which is used to evaluate the condition expression in next
    # in simulation run, once the simulation is complete, you resize the outputs to make sure
    #that the results arrays are the proper size instead of the multiple of 1000
    # check in loopstarthook to see if the 0 dimension of anything in the results array is greater than
    # or equal to Getnumberofoutputpoints and if it is then it resizes the 0th dimension 
    # of each thing in the results array by increasing by 1000. this resizing happens in hook, changing
    #self.numberofoutputs happens in the next method
    
    
    def next(self):
        if not self.condition:
            self.count = 0
            raise StopIteration     
        else:
            self.count += 1
            return self.count
        
    def GetNumberOfOutputPoints(self):
        return self.count
    
    def GetCurrentOutputPoint(self):
        return self.count
    
    def GetCurrentOutputNumber(self):
        return self.count - 1
        
    
    
    