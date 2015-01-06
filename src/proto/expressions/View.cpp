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
    PROTO_ASSERT(max_dimension == (Index)R::END || max_dimension < N,
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
    return TraceResult(boost::make_shared<ArrayValue>(view));
}
