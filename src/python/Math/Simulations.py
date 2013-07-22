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
import numpy as np

class AbstractSimulation(object):
    """Base class for simulations in the protocol language."""
    def __init__(self, prefix=None):
        self.prefix = prefix
        self.ranges = range
#         if prefix:
        self.results = Env.Environment()
#         else:
#             self.results = None
    
    def InternalRun(self):
        raise NotImplementedError 
    
    def SetModel(self, model):
        self.model = model   
    
    def Run(self):
        self.InternalRun()
        return self.results
        # run results of internal run
    
    def AddIterationOutputs(self, env):
        if self.results is not None and not self.results:  
            range_dims = tuple([r.GetNumberOfOutputPoints() for r in self.ranges])         
            for name in env.DefinedNames():
                output = env.LookUp(name)
                results = np.empty(range_dims + output.array.shape)
                self.results.DefineName(name, V.Array(results))
        if self.results:
            range_indices = tuple([r.GetCurrentOutputNumber() for r in self.ranges])
            for name in env.DefinedNames():
                result = self.results.LookUp(name).array
                result[range_indices] = env.LookUp(name).array
        
class Timecourse(AbstractSimulation):   
    def __init__(self, range_):
        super(Timecourse, self).__init__()
        self.model = None
        self.range_ = range_
        self.ranges = [self.range_]     
        
    def InternalRun(self):
        for t in self.range_:
            if self.range_.count == 1:
                self.model.SetInitialTime(t)
                self.AddIterationOutputs(self.model.GetOutputs())
            else:
                self.model.Simulate(t)
                self.AddIterationOutputs(self.model.GetOutputs())
    
class Nested(AbstractSimulation):
    def __init__(self, nestedSim, range_, modifiers=[]):
        super(Nested, self).__init__()
        self.nestedSim = nestedSim
        self.range_ = range_
        self.modifiers = modifiers
        self.ranges = self.nestedSim.ranges
        self.ranges.insert(0, self.range_)
        self.results = self.nestedSim.results
    
    def ZeroInitialiseResults(self):
        pass
    
    def InternalRun(self):
        for t in self.range_:
            self.nestedSim.Run()
    
    
    