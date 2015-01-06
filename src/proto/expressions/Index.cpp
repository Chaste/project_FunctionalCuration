/*

Copyright (c) 2005-2015, University of Oxford.
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

#include "Index.hpp"

#include <algorithm>
#include <boost/assign/list_of.hpp>
#include <boost/make_shared.hpp>

#include "BacktraceException.hpp"
#include "NdArray.hpp"
#include "ValueTypes.hpp"

Index::Index(const std::vector<AbstractExpressionPtr>& rOperands)
    : FunctionCall("~index", rOperands)
{}

Index::Index(const AbstractExpressionPtr pOperand,
             const AbstractExpressionPtr pIndices,
             const AbstractExpressionPtr pDim,
             const AbstractExpressionPtr pShrink,
             const AbstractExpressionPtr pPad,
             const AbstractExpressionPtr pPadValue)
    : FunctionCall("~index", boost::assign::list_of(pOperand)(pIndices)(pDim)(pShrink)(pPad)(pPadValue))
{}

AbstractValuePtr Index::operator()(const Environment& rEnv) const
{
    // Get & check arguments
    const unsigned num_args = mChildren.size();
    PROTO_ASSERT(num_args <= 6 && num_args >= 2,
                 "Index requires 2-6 operands; " << mChildren.size() << " received.");
    std::vector<AbstractValuePtr> actual_params = EvaluateChildren(rEnv);
    const AbstractValuePtr p_operand = actual_params[0];
    const AbstractValuePtr p_indices = actual_params[1];
    AbstractValuePtr p_dim = num_args > 2 ? actual_params[2] : boost::make_shared<DefaultParameter>();
    AbstractValuePtr p_shrink = num_args > 3 ? actual_params[3] : boost::make_shared<DefaultParameter>();
    AbstractValuePtr p_pad = num_args > 4 ? actual_params[4] : boost::make_shared<DefaultParameter>();
    AbstractValuePtr p_pad_value = num_args > 5 ? actual_params[5] : boost::make_shared<DefaultParameter>();
    PROTO_ASSERT(p_operand->IsArray(), "First argument to index should be an array.");
    PROTO_ASSERT(p_indices->IsArray(), "Second argument to index should be an array.");
    PROTO_ASSERT(p_dim->IsDouble() || p_dim->IsDefault(),
                 "Third argument to index, if given, should be an integer.");
    PROTO_ASSERT(p_shrink->IsDouble() || p_shrink->IsDefault(),
                 "Fourth argument to index, if given, should be a simple value.");
    PROTO_ASSERT(p_pad->IsDouble() || p_pad->IsDefault(),
                 "Fifth argument to index, if given, should be a simple value.");
    PROTO_ASSERT(p_pad_value->IsDouble() || p_pad_value->IsDefault(),
                 "Sixth argument to index, if given, should be a simple value.");
    NdArray<double> operand = GET_ARRAY(p_operand);
    NdArray<double> indices = GET_ARRAY(p_indices);
    PROTO_ASSERT(indices.GetNumDimensions() == 2u,
                 "The indices array passed to index must have dimension 2, not " << indices.GetNumDimensions());
    if (p_dim->IsDefault())
    {
        p_dim = boost::make_shared<SimpleValue>(operand.GetNumDimensions() - 1);
    }
    if (p_shrink->IsDefault())
    {
        p_shrink = boost::make_shared<SimpleValue>(0.0);
    }
    if (p_pad->IsDefault())
    {
        p_pad = boost::make_shared<SimpleValue>(0.0);
    }
    if (p_pad_value->IsDefault())
    {
        p_pad_value = boost::make_shared<SimpleValue>(DBL_MAX);
    }
    // Get & check simple value arguments
    const NdArray<double>::Index dimension = (NdArray<double>::Index)(GET_SIMPLE_VALUE(p_dim));
    const NdArray<double>::Index operand_dimensions = operand.GetNumDimensions();
    PROTO_ASSERT(dimension < operand_dimensions,
                 "The operand to index has " << operand_dimensions
                 << " dimensions, and so cannot be compressed along dimension " << dimension << '.');
    // Check indices shape & determine result shape
    const double pad = GET_SIMPLE_VALUE(p_pad);
    const double shrink = GET_SIMPLE_VALUE(p_shrink);
    PROTO_ASSERT(pad == 0 || shrink == 0, "You cannot both pad and shrink!");
    const unsigned num_entries = indices.GetShape()[0];
    PROTO_ASSERT(indices.GetShape()[1] == operand_dimensions,
                 "Indices are the wrong size (" << indices.GetShape()[1]
                 << ") for this operand of dimension " << operand_dimensions << '.');
    NdArray<double>::Extents shape = operand.GetShape();

    /* Create an array of the same shape as operand except that it has extent 1 along dimension,
     * and fill it with the number of entries in the result array along that strip of dimension.
     *
     * We can then find the min & max of this array, and either throw if they don't match (shrink=0)
     * or create the result with extent given by the minimum.  However, if the min is 0 we always
     * throw.
     */
    shape[dimension] = 1;
    NdArray<double> result_shape(shape);
    std::fill(result_shape.Begin(), result_shape.End(), 0.0);
    NdArray<double>::Indices idxs = result_shape.GetIndices();
    for (unsigned i=0; i<num_entries; ++i)
    {
        // Get the indices of the next operand element
        for (unsigned j=0; j<operand_dimensions; ++j)
        {
            NdArray<double>::Indices ij = boost::assign::list_of(i)(j);
            idxs[j] = (NdArray<double>::Index)indices[ij];
        }
        idxs[dimension] = 0;
        result_shape[idxs]++;
    }
    // Find max & min
    double max_extent_d = *std::max_element(result_shape.Begin(), result_shape.End());
    double min_extent_d = *std::min_element(result_shape.Begin(), result_shape.End());
    if ((min_extent_d == 0 && pad == 0) || (min_extent_d != max_extent_d && shrink == 0 && pad == 0))
    {
        PROTO_EXCEPTION("Cannot index if the result is irregular (extent ranges from " << min_extent_d
                        << " to " << max_extent_d << ").");
    }
    unsigned extent = (pad == 0) ? (unsigned)min_extent_d : (unsigned)max_extent_d;

    // Create result array
    shape[dimension] = extent;
    NdArray<double> result(shape);
    if (pad != 0)
    {
        // Fill with the pad value to start with
        std::fill(result.Begin(), result.End(), GET_SIMPLE_VALUE(p_pad_value));
    }

    // Re-use extent array to keep track of how far we've got for each non-compressable dimension
    NdArray<double> next_index = result_shape;
    std::fill(next_index.Begin(), next_index.End(), 0);

    // Now fill in the result
    int begin, end, move;
    if (shrink+pad < 0)
    {
        begin = num_entries-1;
        end = -1;
        move = -1;
    }
    else
    {
        begin = 0;
        end = num_entries;
        move = 1;
    }
    for (int i = begin; i != end; i += move)
    {
        // Get the indices of the next operand element
        for (unsigned j=0; j<operand_dimensions; ++j)
        {
            NdArray<double>::Indices ij = boost::assign::list_of(i)(j);
            idxs[j] = (NdArray<double>::Index)indices[ij];
        }
        double value = operand[idxs];
        // Now figure out where to put it
        idxs[dimension] = 0;
        double& r_next_index = next_index[idxs];
        unsigned this_next_index = (unsigned) r_next_index;
        // Only add if we're within the extent of the result
        if (this_next_index < extent)
        {
            idxs[dimension] = (shrink+pad < 0) ? extent-this_next_index-1 : this_next_index;
            result[idxs] = value;
            r_next_index++;
        }
    }

    return boost::make_shared<ArrayValue>(result);
}
