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

#include "Fold.hpp"

#include <vector>
#include <boost/assign/list_of.hpp>
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
    : FunctionCall("~fold", boost::assign::list_of(pFunc)(pArray)(pInit)(pDim))
{}

AbstractValuePtr Fold::operator()(const Environment& rEnv) const
{
    // Get & check arguments
    PROTO_ASSERT(mChildren.size() == 4, "A fold requires 4 arguments.");
    std::vector<AbstractValuePtr> actual_params = EvaluateChildren(rEnv);
    const AbstractValuePtr p_func = actual_params[0];
    const AbstractValuePtr p_array = actual_params[1];
    AbstractValuePtr p_init = actual_params[2];
    AbstractValuePtr p_dim = actual_params[3];
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
    const NdArray<double>::Index dimension = static_cast<NdArray<double>::Index>(GET_SIMPLE_VALUE(p_dim));
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
    return boost::make_shared<ArrayValue>(result);
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
