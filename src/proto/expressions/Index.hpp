/*

Copyright (C) University of Oxford, 2005-2011

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.

This file is part of Chaste.

Chaste is free software: you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Chaste is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details. The offer of Chaste under the terms of the
License is subject to the License being interpreted in accordance with
English Law and subject to any action against the University of Oxford
being under the jurisdiction of the English Courts.

You should have received a copy of the GNU Lesser General Public License
along with Chaste. If not, see <http://www.gnu.org/licenses/>.

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
