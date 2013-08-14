 
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
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGEnv.
"""

import numpy as np
import unittest

import fc.language.expressions as E
import fc.language.statements as S
import fc.language.values as V
import fc.utility.environment as Env

N = E.N

class TestSpeed(unittest.TestCase):
    """Test speed of python implementation using simple expressions involving very large arrays."""
    def TestAddingLargeArrays(self):
        # 1-d array
        env = Env.Environment()
        parameters = ['large_array1', 'large_array2']
        body = [S.Return(E.Plus(E.NameLookUp('large_array1'), E.NameLookUp('large_array2')))]
        add = E.LambdaExpression(parameters, body)
        large_array1 = E.NewArray(E.NameLookUp("i"), E.TupleExpression(N(0), N(0), N(1), N(10000000), E.Const(V.String("i"))), comprehension=True)
        large_array2 = E.NewArray(E.NameLookUp("i"), E.TupleExpression(N(0), N(0), N(1), N(10000000), E.Const(V.String("i"))), comprehension=True)
        result = E.Map(add, large_array1, large_array2)
        predicted = 2*np.arange(10000000)
        np.testing.assert_array_almost_equal(result.Evaluate(env).array, predicted)
    
    def TestMultipleOperationExpressionWithLargeArrays(self):
        env = Env.Environment()
        parameters = ['a', 'b', 'c']
        body = [S.Return(E.Times(E.Plus(E.NameLookUp('a'), E.NameLookUp('b')), E.NameLookUp('c')))]
        add_times = E.LambdaExpression(parameters, body)
        a = E.NewArray(E.NameLookUp("i"), E.TupleExpression(N(0), N(0), N(1), N(10000000), E.Const(V.String("i"))), comprehension=True)
        b = E.NewArray(E.NameLookUp("i"), E.TupleExpression(N(0), N(0), N(1), N(10000000), E.Const(V.String("i"))), comprehension=True)
        c = E.NewArray(E.NameLookUp("i"), E.TupleExpression(N(0), N(0), N(1), N(10000000), E.Const(V.String("i"))), comprehension=True)
        result = E.Map(add_times, a, b, c)
        predicted = np.arange(10000000)*(2*np.arange(10000000))
        np.testing.assert_array_almost_equal(result.Evaluate(env).array, predicted)
