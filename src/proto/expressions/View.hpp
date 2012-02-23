/*

Copyright (c) 2005-2012, University of Oxford.
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
