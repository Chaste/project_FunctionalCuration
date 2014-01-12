/*

Copyright (c) 2005-2014, University of Oxford.
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

*/

#ifndef FOLD_HPP_
#define FOLD_HPP_

#include "FunctionCall.hpp"

#include "AbstractExpression.hpp"
#include "ProtoHelperMacros.hpp"

/**
 * The fundamental "fold" concept in the post-processing language.
 *
 * This collapses an array along one dimension by iteratively applying a binary function to the
 * current result and the next element.  An initial value may be supplied, or the first element used.
 * The result array has extent one for the folded dimension.
 *
 * Technical note: this performs a left fold.
 */
class Fold : public FunctionCall
{
public:
    /**
     * Construct a fold expression.
     *
     * @param rOperands  the 4 operands (the number will not be checked until evaluation)
     */
    Fold(const std::vector<AbstractExpressionPtr>& rOperands);

    /**
     * Constructor useful for tests.
     *
     * @param pFunc  the function to be folded
     * @param pArray  the operand array
     * @param pInit  the initial value for the fold (defaults to the first element of the input)
     * @param pDim  the dimension along which to perform the fold (defaults to last)
     */
    Fold(const AbstractExpressionPtr pFunc,
         const AbstractExpressionPtr pArray,
         const AbstractExpressionPtr pInit=DEFAULT_EXPR,
         const AbstractExpressionPtr pDim=DEFAULT_EXPR);

    /**
     * Perform the fold operation.
     *
     * @param rEnv  the environment in which to evaluate the fold call
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;

private:
    /**
     * The recursive foldl call.  We use foldl so that it can be implemented more efficiently
     * with iteration, since we don't need to deal with infinite lists!
     *
     * @param rEnv  the environment in which to evaluate the fold call
     * @param rFunc  the function to be folded
     * @param pInit  the initial value for the fold; defaults to the first element of the input
     * @param rOperand  the operand array
     * @param rIndices  indices into rOperand
     * @param rJ  reference to the element of rIndices for the dimension being folded over
     * @param length  original length of the dimension being folded
     */
    double Foldl(const Environment& rEnv,
                 const LambdaClosure& rFunc,
                 const AbstractValuePtr pInit,
                 const NdArray<double>& rOperand,
                 NdArray<double>::Indices& rIndices,
                 NdArray<double>::Index& rJ,
                 NdArray<double>::Index length) const;
};

#endif // FOLD_HPP_
