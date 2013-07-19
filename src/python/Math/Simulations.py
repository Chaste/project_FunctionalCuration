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
    """Base class for statements in the protocol language."""
    def InternalRun(self, env=None):
        raise NotImplementedError    
    
    def Run(self):
        return env
    
    def AddIterationOutputs(self):
        pass
        
class Timecourse(AbstractSimulation):
    def __init__(self, model, range_):
        self.model = model
        self.range_ = range_
        
    def Run(self):
        results = {}
        env = None
        for t in self.range_:
            if self.range_.count == 1:
                env = self.model.GetOutputs()
                for name in env.DefinedNames():
                    results[name] = [env.LookUp(name).value]
            else:
                self.model.Simulate(t)
                for name in env.DefinedNames():
                    results[name].append(self.model.GetOutputs().LookUp(name).value)
        env_results = Env.Environment()
        for name in results:
            env_results.DefineName(name, V.Array(np.array(results[name])))
        return env_results
    
    
    # abstract simulation class has two run methods, raise notimplemented error and 
    #the other has no arguments and returns an environment
    # internalRun(env=None) is notimplementederror so its implemented in timecoursesimulation
    # addIterationOutputs
    # in constructor for abstract simulation is model and range (range is an instance of 
    #abstract range) uniformrange is subclass of abstractrange
    # implement iterator method for abstract range  (method iter returns self) and you need
    #a method __next__
    #uniformrange takes start, end, step size
    #get number of output points just returns number of steps
    #timecoursesimulation just loops through the uniformrange values and simulates for each
    # save outputs of each point along the way
    # additerationoutputs is method for abstractsimulation- end up with array with results of
    #each time step
    # getcurrentoutput number in abstractrange class returns current place in range so that you can do
    #result of that index and assign it for that time step
    # timecoursesimulation is a subclass of simulation
    
    
    