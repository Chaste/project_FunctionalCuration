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

class AbstractModifier(object):
    """Base class for modifiers in the protocol language."""
    START_ONLY = 0
    EACH_LOOP = 1
    END_ONLY = 2
    def Apply(self):
        raise NotImplementedError
    
class SetVariable(AbstractModifier):
        def __init__(self, when, variableName, valueExpr):
            self.when = when
            self.variableName = variableName
            self.valueExpr = valueExpr
            if variableName == 'oxmeta:membrane_voltage':
                self.variableName = 'y'
            elif variableName == 'oxmeta:leakage_current':
                self.variableName = 'a'
        
        def Apply(self, simul):
            value = self.valueExpr.Evaluate(simul.env).value
            simul.model.SetVariable(self.when, simul.env, self.variableName, value)
        
class SaveState(AbstractModifier):
        def __init__(self, when, stateName):
            self.when = when
            self.stateName = stateName
        
        def Apply(self, simul):
            simul.model.SaveState(self.when, self.stateName)
    
class ResetState(AbstractModifier):
        def __init__(self, when, stateName=None):
            self.when = when
            self.stateName = stateName
            
        def Apply(self, simul):
            simul.model.ResetState(self.when, self.stateName)
                
                #all classes call methods of model to do work

        