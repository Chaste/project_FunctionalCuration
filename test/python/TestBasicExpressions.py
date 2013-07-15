
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
import MathExpressions as M
import Expressions as E
import Values as V
import Environment as Env
import numpy as np
import ArrayExpressions as A
from ErrorHandling import ProtocolError

def N(number):
    return M.Const(V.Simple(number))

class TestBasicExpressions(unittest.TestCase):
    def TestAddition(self):
        env = Env.Environment()
        self.assertAlmostEqual(M.Plus(M.Const(V.Simple(1)), M.Const(V.Simple(2))).Evaluate(env).value, 3)
        self.assertAlmostEqual(M.Plus(M.Const(V.Simple(1)), M.Const(V.Simple(2)), M.Const(V.Simple(4))).Evaluate(env).value, 7)
    
    def TestWith0dArray(self):
        env = Env.Environment()
        one = V.Array(np.array(1))
        self.assertEqual(one.value, 1)
        two = V.Array(np.array(2))
        four = V.Array(np.array(4))
        self.assertAlmostEqual(M.Plus(M.Const(one), M.Const(two)).Evaluate(env).value, 3)
        self.assertAlmostEqual(M.Plus(M.Const(one), M.Const(two), M.Const(four)).Evaluate(env).value, 7)
        self.assertAlmostEqual(M.Minus(M.Const(one), M.Const(two)).Evaluate(env).value, -1)
        self.assertAlmostEqual(M.Power(M.Const(two), M.Const(four)).Evaluate(env).value, 16)
        self.assertAlmostEqual(M.Times(M.Const(four), M.Const(two)).Evaluate(env).value, 8)
        self.assertAlmostEqual(M.Root(M.Const(four)).Evaluate(env).value, 2)
                
    def TestMinus(self):
        env = Env.Environment()
        self.assertAlmostEqual(M.Minus(M.Const(V.Simple(1)), M.Const(V.Simple(2))).Evaluate(env).value, -1)
        self.assertAlmostEqual(M.Minus(M.Const(V.Simple(4))).Evaluate(env).value, -4)
        
    def TestTimes(self):
        env = Env.Environment()
        self.assertAlmostEqual(M.Times(M.Const(V.Simple(6)), M.Const(V.Simple(2))).Evaluate(env).value, 12)
        self.assertAlmostEqual(M.Times(M.Const(V.Simple(6)), M.Const(V.Simple(2)), M.Const(V.Simple(3))).Evaluate(env).value, 36)
        
    def TestDivide(self):
        env = Env.Environment()
        self.assertAlmostEqual(M.Divide(M.Const(V.Simple(1)), M.Const(V.Simple(2))).Evaluate(env).value, .5)
        
    def TestMax(self):
        env = Env.Environment()
        self.assertAlmostEqual(M.Max(M.Const(V.Simple(6)), M.Const(V.Simple(12)), M.Const(V.Simple(2))).Evaluate(env).value, 12)

    def TestMin(self):
        env = Env.Environment()
        self.assertAlmostEqual(M.Min(M.Const(V.Simple(6)), M.Const(V.Simple(2)), M.Const(V.Simple(12))).Evaluate(env).value, 2)
        
    def TestRem(self):
        env = Env.Environment()
        self.assertAlmostEqual(M.Rem(M.Const(V.Simple(6)), M.Const(V.Simple(4))).Evaluate(env).value, 2)
        
    def TestPower(self):
        env = Env.Environment()
        self.assertAlmostEqual(M.Power(M.Const(V.Simple(2)), M.Const(V.Simple(3))).Evaluate(env).value, 8)
        
    def TestRoot(self):
        env = Env.Environment()
        self.assertAlmostEqual(M.Root(M.Const(V.Simple(16))).Evaluate(env).value, 4)
        self.assertAlmostEqual(M.Root(M.Const(V.Simple(3)), M.Const(V.Simple(8))).Evaluate(env).value, 2)
        
    def TestAbs(self):
        env = Env.Environment()
        self.assertAlmostEqual(M.Abs(M.Const(V.Simple(-4))).Evaluate(env).value, 4)
        self.assertAlmostEqual(M.Abs(M.Const(V.Simple(4))).Evaluate(env).value, 4)
        
    def TestFloor(self):
        env = Env.Environment()
        self.assertAlmostEqual(M.Floor(M.Const(V.Simple(1.8))).Evaluate(env).value, 1)
        
    def TestCeiling(self):
        env = Env.Environment()
        self.assertAlmostEqual(M.Ceiling(M.Const(V.Simple(1.2))).Evaluate(env).value, 2)
    
    def TestExp(self):
        env = Env.Environment()
        self.assertAlmostEqual(M.Exp(M.Const(V.Simple(3))).Evaluate(env).value, 20.0855369231)
    
    def TestLn(self):
        env = Env.Environment()
        self.assertAlmostEqual(M.Ln(M.Const(V.Simple(3))).Evaluate(env).value, 1.0986122886) 
        
    def TestLog(self):
        env = Env.Environment()
        self.assertAlmostEqual(M.Log(M.Const(V.Simple(3))).Evaluate(env).value, 0.4771212547196)
        self.assertAlmostEqual(M.Log(M.Const(V.Simple(4)), M.Const(V.Simple(3))).Evaluate(env).value, 0.79248125036)
    
    def TestNameLookUp(self):
        env = Env.Environment()
        one = V.Simple(1)
        env.DefineName("one", one)
        # Note that Evaluate will try to optimise using numexpr, and so always returns a V.Array (0d in this case)
        self.assertEqual(E.NameLookUp("one").Interpret(env).value, 1)
        np.testing.assert_array_equal(E.NameLookUp("one").Evaluate(env).array, np.array(1))

    def TestIf(self):
        # test is true
        env = Env.Environment()
        result = E.If(N(1), M.Plus(N(1), N(2)), M.Minus(N(1), N(2))).Evaluate(env)
        self.assertEqual(3, result.value)
        
        # test is false
        result = E.If(N(0), M.Plus(N(1), N(2)), M.Minus(N(1), N(2))).Evaluate(env)
        self.assertEqual(-1, result.value)
        
    def TestAccessor(self):
        env = Env.Environment()
        
        # test simple value
        simple = N(1)
        result = E.Accessor(simple, E.Accessor.IS_SIMPLE_VALUE).Interpret(env) 
        self.assertEqual(1, result.value)
        
        # test array
        array = A.NewArray(A.NewArray(N(1), N(2)), A.NewArray(N(3), N(4)))
        result = E.Accessor(array, E.Accessor.IS_ARRAY).Interpret(env)
        self.assertEqual(1, result.value)
        result = E.Accessor(array, E.Accessor.NUM_DIMS).Interpret(env)
        self.assertEqual(2, result.value)
        result = E.Accessor(array, E.Accessor.NUM_ELEMENTS).Interpret(env)
        self.assertEqual(4, result.value)
        result = E.Accessor(array, E.Accessor.SHAPE).Interpret(env)
        np.testing.assert_array_almost_equal(result.array, np.array([2,2]))

        # test string
        string_test = M.Const(V.String("hi"))
        result = E.Accessor(string_test, E.Accessor.IS_STRING).Interpret(env)
        self.assertEqual(1, result.value)
        result = E.Accessor(array, E.Accessor.IS_STRING).Interpret(env)
        self.assertEqual(0, result.value)
        
        # test function
        function = E.LambdaExpression.Wrap(M.Plus, 3)
        result = E.Accessor(function, E.Accessor.IS_FUNCTION).Interpret(env)
        self.assertEqual(1, result.value)
        result = E.Accessor(string_test, E.Accessor.IS_FUNCTION).Interpret(env)
        self.assertEqual(0, result.value)
        
        # test tuple
        tuple_test = E.TupleExpression(N(1), N(2))
        result = E.Accessor(tuple_test, E.Accessor.IS_TUPLE).Interpret(env)
        self.assertEqual(1, result.value)
        
        # test null
        null_test = M.Const(V.Null())
        result = E.Accessor(null_test, E.Accessor.IS_NULL).Interpret(env)
        self.assertEqual(1, result.value)
        
        # test default
        default_test = M.Const(V.DefaultParameter())
        result = E.Accessor(default_test, E.Accessor.IS_DEFAULT).Interpret(env)
        self.assertEqual(1, result.value)
        result = E.Accessor(null_test, E.Accessor.IS_DEFAULT).Interpret(env)
        self.assertEqual(0, result.value)
        
        # test if non-array variables have array attributes, should raise errors
        self.assertRaises(ProtocolError, E.Accessor(default_test, E.Accessor.NUM_DIMS).Interpret, env)
        self.assertRaises(ProtocolError, E.Accessor(function, E.Accessor.SHAPE).Interpret, env)
        self.assertRaises(ProtocolError, E.Accessor(string_test, E.Accessor.NUM_ELEMENTS).Interpret, env)

    