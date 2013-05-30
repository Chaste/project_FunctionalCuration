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

import unittest
import sys

# Import the module to test
sys.path[0:0] = ['python/pycml', 'projects/FunctionalCuration/src/python']
import MathRelations as M
import Values as V

class TestRelations(unittest.TestCase):
    def TestEq(self):
        self.assertFalse(M.Eq(M.Const(V.Simple(1)), M.Const(V.Simple(0))).Evaluate({}).value)
        self.assertTrue(M.Eq(M.Const(V.Simple(1)), M.Const(V.Simple(1))).Evaluate({}).value)
        
    def TestNeq(self):
        self.assertTrue(M.Neq(M.Const(V.Simple(1)), M.Const(V.Simple(0))).Evaluate({}).value)
        self.assertFalse(M.Neq(M.Const(V.Simple(1)), M.Const(V.Simple(1))).Evaluate({}).value)
        
    def TestLt(self):
        self.assertFalse(M.Lt(M.Const(V.Simple(1)), M.Const(V.Simple(0))).Evaluate({}).value)
        self.assertTrue(M.Lt(M.Const(V.Simple(0)), M.Const(V.Simple(1))).Evaluate({}).value)
        self.assertFalse(M.Lt(M.Const(V.Simple(1)), M.Const(V.Simple(1))).Evaluate({}).value)
        
    def TestGt(self):
        self.assertTrue(M.Gt(M.Const(V.Simple(1)), M.Const(V.Simple(0))).Evaluate({}).value)
        self.assertFalse(M.Gt(M.Const(V.Simple(0)), M.Const(V.Simple(1))).Evaluate({}).value)
        self.assertFalse(M.Gt(M.Const(V.Simple(1)), M.Const(V.Simple(1))).Evaluate({}).value)
        
    def TestLeq(self):
        self.assertFalse(M.Leq(M.Const(V.Simple(1)), M.Const(V.Simple(0))).Evaluate({}).value)
        self.assertTrue(M.Leq(M.Const(V.Simple(0)), M.Const(V.Simple(1))).Evaluate({}).value)
        self.assertTrue(M.Leq(M.Const(V.Simple(1)), M.Const(V.Simple(1))).Evaluate({}).value)
        
    def TestGeq(self):
        self.assertTrue(M.Geq(M.Const(V.Simple(1)), M.Const(V.Simple(0))).Evaluate({}).value)
        self.assertFalse(M.Geq(M.Const(V.Simple(0)), M.Const(V.Simple(1))).Evaluate({}).value)
        self.assertTrue(M.Geq(M.Const(V.Simple(1)), M.Const(V.Simple(1))).Evaluate({}).value)
        
        