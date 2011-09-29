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

#ifndef VIEW_HPP_
#define VIEW_HPP_

#include "FunctionCall.hpp"

#include "NdArray.hpp"
#include "ValueTypes.hpp"

/**
 * The fundamental "view" construct in the post-processing language.
 *
 * This operation generates a view of an array, i.e. a sub-array.  Each dimension can
 * be indexed separately, specifying start & end positions, and a stride (to skip elements).
 * The stride can also be negative to produce a reverse view (in which case start should be
 * greater than end).
 */
class View : public FunctionCall
{
public:
    /**
     * Create a view generating expression, specifying the ranges over each dimension.
     *
     * Ranges should either be all 3-tuples or all 4-tuples.  If the former syntax is used, then
     * the number of ranges should be equal to the number of dimensions in the input array, and
     * each tuple specifies (start, stride, end) over the corresponding dimension.  If 4-tuples
     * are used, then each specifies (start, stride, end, dimension).  One tuple may use the Null
     * value for dimension; this specification is then used as a default for any dimensions not
     * given explicitly.
     *
     * If stride is zero, then start & end must be equal, and the given dimension will not appear
     * in the view (the view will have lower dimensionality than the input); the value of start
     * specifies which slice of the input to use along that dimension.
     *
     * Both start and/or end may be the Null value to indicate the start/end of the dimension's
     * extent, respectively, provided that stride is not zero.
     *
     * @param pArray  an expression evaluating to the array to obtain a view of
     * @param rRanges  expressions specifying how each dimension should be viewed.
     */
    View(const AbstractExpressionPtr pArray, const std::vector<AbstractExpressionPtr>& rRanges);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;
};

#endif // VIEW_HPP_
