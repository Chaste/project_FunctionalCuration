/*

Copyright (c) 2005-2016, University of Oxford.
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

#include "Fold.hpp"

#include <vector>
#include <boost/make_shared.hpp>

#include "LambdaClosure.hpp"
#include "ValueTypes.hpp"
#include "NdArray.hpp"
#include "BacktraceException.hpp"

Fold::Fold(const std::vector<AbstractExpressionPtr>& rOperands)
    : FunctionCall("~fold", rOperands)
{}

Fold::Fold(const AbstractExpressionPtr pFunc,
           const AbstractExpressionPtr pArray,
           const AbstractExpressionPtr pInit,
           const AbstractExpressionPtr pDim)
    : FunctionCall("~fold", {pFunc, pArray, pInit, pDim})
{}

AbstractValuePtr Fold::operator()(const Environment& rEnv) const
{
    // Get & check arguments
    const unsigned num_args = mChildren.size();
    PROTO_ASSERT(num_args <= 4 && num_args >= 2, "A fold requires 2-4 arguments.");
    std::vector<AbstractValuePtr> actual_params = EvaluateChildren(rEnv);
    const AbstractValuePtr p_func = actual_params[0];
    const AbstractValuePtr p_array = actual_params[1];
    AbstractValuePtr p_init = num_args > 2 ? actual_params[2] : boost::make_shared<DefaultParameter>();
    AbstractValuePtr p_dim = num_args > 3 ? actual_params[3] : boost::make_shared<DefaultParameter>();
    PROTO_ASSERT(p_func->IsLambda(), "First argument to fold should be a function.");
    PROTO_ASSERT(p_array->IsArray(), "Second argument to fold should be an array.");
    PROTO_ASSERT(p_init->IsNull() || p_init->IsDouble() || p_init->IsDefault(),
                 "Third argument to fold, if given, should be a simple value or Null.");
    PROTO_ASSERT(p_dim->IsDouble() || p_dim->IsDefault(),
                 "Fourth argument to fold, if given, should be an integer.");
    LambdaClosure& func = *static_cast<LambdaClosure*>(p_func.get());
    NdArray<double> operand = GET_ARRAY(p_array);
    if (p_init->IsDefault())
    {
        p_init = boost::make_shared<NullValue>();
    }
    if (p_dim->IsDefault())
    {
        p_dim = boost::make_shared<SimpleValue>(operand.GetNumDimensions() - 1);
    }
    const NdArray<double>::Index dimension = (NdArray<double>::Index)(GET_SIMPLE_VALUE(p_dim));
    PROTO_ASSERT(dimension < operand.GetNumDimensions(),
                 "Cannot fold over dimension " << dimension << " as the operand array only has "
                 << operand.GetNumDimensions() << " dimensions.");

    // Create result array
    NdArray<double>::Extents shape = operand.GetShape();
    const NdArray<double>::Index original_length = shape[dimension];
    if (p_init->IsNull() && original_length == 0)
    {
        PROTO_EXCEPTION("Cannot foldr1 over empty dimension" << dimension << ".");
    }
    shape[dimension] = 1;
    NdArray<double> result(shape);

    // Fill it in
    const NdArray<double>::Index size = result.GetNumElements();
    NdArray<double>::Indices indices = result.GetIndices();
    for (NdArray<double>::Index i=0; i<size; ++i)
    {
        NdArray<double>::Index& r_j = indices[dimension];
        double result_item = Foldl(rEnv, func, p_init, operand, indices, r_j, original_length);
        r_j = 0;
        result[indices] = result_item;
        result.IncrementIndices(indices);
    }
    return TraceResult(boost::make_shared<ArrayValue>(result));
}

double Fold::Foldl(const Environment& rEnv,
                   const LambdaClosure& rFunc,
                   const AbstractValuePtr pInit,
                   const NdArray<double>& rOperand,
                   NdArray<double>::Indices& rIndices,
                   NdArray<double>::Index& rJ,
                   NdArray<double>::Index length) const
{
    double init;
    if (pInit->IsNull())
    {
        init = rOperand[rIndices];
        ++rJ;
    }
    else
    {
        init = GET_SIMPLE_VALUE(pInit);
    }
    for (; rJ<length; ++rJ)
    {
        std::vector<AbstractValuePtr> args;
        args.push_back(boost::make_shared<SimpleValue>(init));
        args.push_back(boost::make_shared<SimpleValue>(rOperand[rIndices]));
        AbstractValuePtr p_result_value = rFunc(rEnv, args);
        PROTO_ASSERT(p_result_value->IsDouble(),
                     "The function supplied to fold should only return simple values.");
        init = GET_SIMPLE_VALUE(p_result_value);
    }
    return init;
}
