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

#include "Map.hpp"

#include <boost/make_shared.hpp>

#include "BacktraceException.hpp"
#include "VectorStreaming.hpp"
#include "LambdaClosure.hpp"
#include "NdArray.hpp"
#include "ProtoHelperMacros.hpp"

Map::Map(const std::vector<AbstractExpressionPtr>& rParameters, bool allowImplicitArrays)
    : FunctionCall("~map", rParameters),
      mAllowImplicitArrays(allowImplicitArrays)
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
    unsigned ref_i = 0u;
    for (unsigned i=1; i<arg_arrays.size(); ++i)
    {
        if (mAllowImplicitArrays)
        {
            if (shape.empty() && arg_arrays[i].GetShape() != shape)
            {
                shape = arg_arrays[i].GetShape();
                ref_i = i;
            }
        }
        PROTO_ASSERT(arg_arrays[i].GetShape() == shape || (mAllowImplicitArrays && arg_arrays[i].GetShape().empty()),
                     "The shapes of the arrays passed to map must match; argument " << i << " of shape "
                     << arg_arrays[i].GetShape() << " does not match argument " << ref_i << " of shape " << shape << ".");
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
            if (mAllowImplicitArrays && arg_arrays[j].GetShape().empty())
            {
                fn_args.push_back(boost::make_shared<SimpleValue>(*arg_arrays[j].Begin()));
            }
            else
            {
                fn_args.push_back(boost::make_shared<SimpleValue>(arg_arrays[j][indices]));
            }
        }
        AbstractValuePtr p_result_value = func(rEnv, fn_args);
        PROTO_ASSERT(p_result_value->IsDouble(), "The function passed to map must only return simple values.");
        result[indices] = GET_SIMPLE_VALUE(p_result_value);
        result.IncrementIndices(indices);
    }
    return TraceResult(boost::make_shared<ArrayValue>(result));
}
