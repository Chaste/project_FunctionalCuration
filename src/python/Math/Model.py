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
from ErrorHandling import ProtocolError

class AbstractModel(object):
    """Base class for statements in the protocol language."""
    def Simulate(self):
        raise NotImplementedError    
    
class TestOdeModel(AbstractModel):
    def __init__(self, a):
        self.savedStates = {} # key is name, value is numpy array of saved state
        self.a = a
        self.r = scipy.integrate.ode(self.f)
        self.r.set_initial_value(0, 0)
        self.modifiers = []
        
    def f(self, t, y):
        return self.a

    def SetInitialTime(self, t):
        self.r.set_initial_value(self.r.y, 0)
        
    def SetVariable(self, when, env, variableName, value):
        if not hasattr(self, variableName):
            raise ProtocolError("Type", type(self), "does not have a variable named", variableName)
        setattr(self, variableName, value)
    
    def SaveState(self, when, name):
        self.savedStates[name] = self.r.y
    
    def ResetState(self, when, name):
        if name is None:
            self.r.set_initial_value(0, 0)
        else:
            self.r.set_initial_value(self.savedStates[name], 0)
                
    def Simulate(self, endPoint):
        self.y = self.r.integrate(endPoint)
        assert self.r.successful()
        #self.y = scipy.integrate.odeint(self.f, 0, self.r.t)
    
    def GetOutputs(self):
        env = Env.Environment()
        env.DefineName('a', V.Simple(self.a))
        env.DefineName('y', V.Simple(self.r.y))
        return env

    
    
# for simulate time always starts from 0 and returns the integration of the differential equation 
#at the point given
# get outputs returns an environment where the inputs 
# testodemodel constructor initializes self.y to zero and 
# dydt = a
# define y as V.Simple(self.y) in environment for getoutputs
# def f(self, t, a): as method of testodemodel
# return self.a
# testodemodel sets up scipy.integrate.ode and simulate will call testodemodel.solver.integrate 
#or something
# constructor will call like r = scipy.integrate.ode(f)
# r.setinitialvalue for y=0 in constructor for test as well
# simulate will call integrate on r 