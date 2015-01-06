"""Copyright (c) 2005-2015, University of Oxford.
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

import fc.language.expressions as E
import fc.language.values as V

class TestLogicOperators(unittest.TestCase):
    """Tests logic using simple values. Tests and, or, xor, not."""
    def TestAnd(self):
        self.assertEqual(E.And(E.Const(V.Simple(1)), E.Const(V.Simple(0))).Evaluate({}).value, 0)
        self.assertEqual(E.And(E.Const(V.Simple(1)), E.Const(V.Simple(1)), E.Const(V.Simple(1))).Evaluate({}).value, 1)
    
    def TestOr(self):
        self.assertEqual(E.Or(E.Const(V.Simple(1)), E.Const(V.Simple(0))).Evaluate({}).value, 1)
        self.assertEqual(E.Or(E.Const(V.Simple(0)), E.Const(V.Simple(0)), E.Const(V.Simple(0))).Evaluate({}).value, 0)
        
    def TestXor(self):
        self.assertEqual(E.Xor(E.Const(V.Simple(1)), E.Const(V.Simple(0))).Evaluate({}).value, 1)
        self.assertEqual(E.Xor(E.Const(V.Simple(0)), E.Const(V.Simple(0))).Evaluate({}).value, 0)
        self.assertEqual(E.Xor(E.Const(V.Simple(1)), E.Const(V.Simple(1))).Evaluate({}).value, 0)
        
    def TestNot(self):
        self.assertEqual(E.Not(E.Const(V.Simple(1))).Evaluate({}).value, 0)
        self.assertEqual(E.Not(E.Const(V.Simple(3))).Evaluate({}).value, 0)
        self.assertEqual(E.Not(E.Const(V.Simple(0))).Evaluate({}).value, 1)
