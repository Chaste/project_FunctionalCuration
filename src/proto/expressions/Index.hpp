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

#ifndef INDEX_HPP_
#define INDEX_HPP_

#include "FunctionCall.hpp"

#include "ProtoHelperMacros.hpp"

/**
 * The fundamental "index" construct in the post-processing language.
 *
 * Index an array using a vector of indices as given by Find, returning a new array consisting of
 * only those entries of the input.  You can choose which way entries are moved (i.e. which
 * dimension to move them along) to avoid gaps appearing in the result array.
 *
 * If the indices would produce an irregular array, this is normally treated as an error condition.
 * In other words, in every dimension each subarray should have the same size.  However, shorter
 * rows may be padded or longer rows shrunk if appropriate options are passed.  It is always an
 * error if the result array is empty.
 */
class Index : public FunctionCall
{
public:
    /**
     * Construct an index expression.
     *
     * @param rOperands  the operands
     */
    Index(const std::vector<AbstractExpressionPtr>& rOperands);

    /**
     * Construct an index expression listing each argument separately.  Useful for testing.
     *
     * @param pOperand  the operand array
     * @param pIndices  the array to index rOperand with
     * @param pDim  which dimension to compress the array along
     * @param pShrink  how to deal with an irregular output.  If shrink and pad are zero it is an error.
     *   Otherwise if shrinking the output along dimension would give a regular result, do so.
     *   If shrink is positive, remove entries from the end; if it is negative, remove them
     *   from the start.
     * @param pPad  how to deal with an irregular output.  Rather than shrinking, pad shorter
     *   rows along dimension to the length of the longest row.  If pad is positive, add padding
     *   on the end; if it is negative, add padding at the start.
     *   You cannot both pad and shrink.
     * @param pPadValue  the value to pad with
     */
    Index(const AbstractExpressionPtr pOperand,
          const AbstractExpressionPtr pIndices,
          const AbstractExpressionPtr pDim=DEFAULT_EXPR,
          const AbstractExpressionPtr pShrink=DEFAULT_EXPR,
          const AbstractExpressionPtr pPad=DEFAULT_EXPR,
          const AbstractExpressionPtr pPadValue=DEFAULT_EXPR);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;
};

#endif // INDEX_HPP_
