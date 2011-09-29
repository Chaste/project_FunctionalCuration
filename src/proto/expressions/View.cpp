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

#include "View.hpp"

#include <map>
#include <boost/assign/list_of.hpp>
#include <boost/make_shared.hpp>
#include "BacktraceException.hpp"
#include "ProtoHelperMacros.hpp"
#include "RangeHelperFunctions.hpp"
//#include "VectorStreaming.hpp"

// Save typing...
typedef NdArray<double>::Range R;
typedef NdArray<double>::Index Index;

View::View(const AbstractExpressionPtr pArray, const std::vector<AbstractExpressionPtr>& rRanges)
    : FunctionCall("~view", boost::assign::list_of(pArray).range(rRanges))
{}

AbstractValuePtr View::operator()(const Environment& rEnv) const
{
    // Evaluate & check operands
    std::vector<AbstractValuePtr> actual_params = EvaluateChildren(rEnv);
    const AbstractValuePtr p_array = actual_params[0];
    PROTO_ASSERT(p_array->IsArray(), "First argument to view should be an array.");
    NdArray<double> array = GET_ARRAY(p_array);
    const Index N = array.GetNumDimensions();
    std::vector<AbstractValuePtr> tuples(++actual_params.begin(), actual_params.end());

    // Extract the range specifications
    std::map<Index, R> range_specs;
    ExtractRangeSpecs(range_specs, tuples, GetLocationInfo());
    const Index max_dimension = range_specs.rbegin()->first;
    PROTO_ASSERT(max_dimension == R::END || max_dimension < N,
                 "Range specifications must be for a dimension that exists; a specification is given for dimension "
                 << max_dimension << " but the array only has " << N << " dimensions.");

    // Check that every dimension is specified
    const Index num_specs = range_specs.size();
    std::map<Index, R>::iterator default_range = range_specs.find(R::END);
    PROTO_ASSERT(default_range != range_specs.end() || num_specs == N,
                 "To create a view a range must be specified for every dimension in the array;"
                 << " the array has " << N << " dimensions but " << num_specs
                 << " ranges have been supplied.");

    // Construct the view
    std::vector<R> view_indices;
    view_indices.reserve(N);
//    std::cout << "View indices for input size " << N << ":" << std::endl;
    for (Index dim=0; dim<N; ++dim)
    {
        std::map<Index, R>::iterator this_dim = range_specs.find(dim);
        if (this_dim == range_specs.end())
        {
//            std::cout << "  Default for dim " << dim << std::endl;
            view_indices.push_back(default_range->second);
        }
        else
        {
//            std::cout << "  For dim " << dim << ": " << this_dim->second.mBegin << ":" << this_dim->second.mStep << ":" << this_dim->second.mEnd << std::endl;
            view_indices.push_back(this_dim->second);
        }
    }
    NdArray<double> view;
    try
    {
        view = array[view_indices];
    }
    catch (const Exception& e)
    {
        PROTO_EXCEPTION("Error constructing array view: " + e.GetMessage());
    }
//    std::cout << "View: Result shape: " << view.GetShape() << std::endl;
//    if (view.GetNumDimensions() == 0) std::cout << "View: Result=" << *view.Begin() << std::endl;
    return boost::make_shared<ArrayValue>(view);
}
