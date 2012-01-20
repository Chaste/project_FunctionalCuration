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

#include "Map.hpp"

#include <boost/make_shared.hpp>

#include "BacktraceException.hpp"
#include "VectorStreaming.hpp"
#include "LambdaClosure.hpp"
#include "NdArray.hpp"
#include "ProtoHelperMacros.hpp"

Map::Map(const std::vector<AbstractExpressionPtr>& rParameters)
    : FunctionCall("~map", rParameters)
{
    PROTO_ASSERT(rParameters.size() > 1, "Map requires more than 1 parameter.");
}

AbstractValuePtr Map::operator()(const Environment& rEnv) const
{
    std::vector<AbstractValuePtr> actual_params = EvaluateChildren(rEnv);
    AbstractValuePtr p_lambda = actual_params.front();
    PROTO_ASSERT(p_lambda->IsLambda(), "First argument to map is not a function.");
    LambdaClosure& func = *static_cast<LambdaClosure*>(p_lambda.get());
    // Check that remaining args are arrays of the same size
    std::vector<NdArray<double> > arg_arrays;
    for (std::vector<AbstractValuePtr>::const_iterator it = ++actual_params.begin();
         it != actual_params.end();
         ++it)
    {
        PROTO_ASSERT((*it)->IsArray(),
                     "Except for the first, all arguments to map should be arrays; argument "
                     << (it - actual_params.begin()) << " is not.");
        arg_arrays.push_back(GET_ARRAY(*it));
    }
    NdArray<double>::Extents shape = arg_arrays[0].GetShape();
    for (unsigned i=1; i<arg_arrays.size(); ++i)
    {
        PROTO_ASSERT(arg_arrays[i].GetShape() == shape,
                     "The shapes of the arrays passed to map must match; argument " << i << " of shape "
                     << arg_arrays[i].GetShape() << " does not match argument 0 of shape " << shape << ".");
    }
    // Create result array
    NdArray<double> result = NdArray<double>(shape);
    // Apply fn
    NdArray<double>::Indices indices = result.GetIndices();
    const NdArray<double>::Index num_elts = result.GetNumElements();
    for (NdArray<double>::Index i=0; i<num_elts; ++i)
    {
        std::vector<AbstractValuePtr> fn_args;
        for (unsigned j=0; j<arg_arrays.size(); ++j)
        {
            fn_args.push_back(boost::make_shared<SimpleValue>(arg_arrays[j][indices]));
        }
        AbstractValuePtr p_result_value = func(rEnv, fn_args);
        PROTO_ASSERT(p_result_value->IsDouble(), "The function passed to map must only return simple values.");
        result[indices] = GET_SIMPLE_VALUE(p_result_value);
        result.IncrementIndices(indices);
    }
    return TraceResult(boost::make_shared<ArrayValue>(result));
}
