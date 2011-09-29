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
    PROTO_ASSERT(mChildren.size() == 6, "Index requires 6 operands; " << mChildren.size() << " received.");
    std::vector<AbstractValuePtr> actual_params = EvaluateChildren(rEnv);
    const AbstractValuePtr p_operand = actual_params[0];
    const AbstractValuePtr p_indices = actual_params[1];
    AbstractValuePtr p_dim = actual_params[2];
    AbstractValuePtr p_shrink = actual_params[3];
    AbstractValuePtr p_pad = actual_params[4];
    AbstractValuePtr p_pad_value = actual_params[5];
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
    const NdArray<double>::Index dimension = static_cast<NdArray<double>::Index>(GET_SIMPLE_VALUE(p_dim));
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
            idxs[j] = indices[ij];
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
            idxs[j] = indices[ij];
        }
        double value = operand[idxs];
        // Now figure out where to put it
        idxs[dimension] = 0;
        double& r_next_index = next_index[idxs];
        unsigned next_index = (unsigned) r_next_index;
        // Only add if we're within the extent of the result
        if (next_index < extent)
        {
            idxs[dimension] = (shrink+pad < 0) ? extent-next_index-1 : next_index;
            result[idxs] = value;
            r_next_index++;
        }
    }

    return boost::make_shared<ArrayValue>(result);
}
