/*

Copyright (C) University of Oxford, 2005-2012

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
     * @param pInit  the initial value for the fold; defaults to the first element of the input
     * @param pDim  the dimension along which to perform the fold
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
