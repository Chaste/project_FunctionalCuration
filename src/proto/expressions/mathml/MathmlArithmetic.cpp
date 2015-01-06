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

#include "MathmlArithmetic.hpp"

#include <algorithm>
#include <cmath>
#include <boost/numeric/conversion/bounds.hpp>
#include <boost/make_shared.hpp>
#include "BacktraceException.hpp"
#include "ValueTypes.hpp"
#include "ProtoHelperMacros.hpp"

MathmlMax::MathmlMax(const std::vector<AbstractExpressionPtr>& rOperands)
    : MathmlOperator("max", rOperands)
{
    PROTO_ASSERT(!mChildren.empty(), "Max operator requires operands.");
}

AbstractValuePtr MathmlMax::operator()(const Environment& rEnv) const
{
    double result = boost::numeric::bounds<double>::lowest();
    std::vector<AbstractValuePtr> operands = EvaluateChildren(rEnv);
    for (std::vector<AbstractValuePtr>::const_iterator it = operands.begin();
         it != operands.end();
         ++it)
    {
        PROTO_ASSERT((*it)->IsDouble(), "Max operator requires its operands to be simple values.");
        result = std::max(result, GET_SIMPLE_VALUE(*it));
    }
    return TraceResult(boost::make_shared<SimpleValue>(result));
}



MathmlMin::MathmlMin(const std::vector<AbstractExpressionPtr>& rOperands)
    : MathmlOperator("min", rOperands)
{
    PROTO_ASSERT(!mChildren.empty(), "Min operator requires operands.");
}

AbstractValuePtr MathmlMin::operator()(const Environment& rEnv) const
{
    double result = boost::numeric::bounds<double>::highest();
    std::vector<AbstractValuePtr> operands = EvaluateChildren(rEnv);
    for (std::vector<AbstractValuePtr>::const_iterator it = operands.begin();
         it != operands.end();
         ++it)
    {
        PROTO_ASSERT((*it)->IsDouble(), "Min operator requires its operands to be simple values.");
        result = std::min(result, GET_SIMPLE_VALUE(*it));
    }
    return TraceResult(boost::make_shared<SimpleValue>(result));
}



MathmlRem::MathmlRem(const std::vector<AbstractExpressionPtr>& rOperands)
    : MathmlOperator("rem", rOperands)
{
    PROTO_ASSERT(mChildren.size() == 2, "Remainder operator requires 2 operands.");
}

AbstractValuePtr MathmlRem::operator()(const Environment& rEnv) const
{
    std::vector<AbstractValuePtr> operands = EvaluateChildren(rEnv);
    PROTO_ASSERT(operands[0]->IsDouble(), "Remainder operator requires its operands to be simple values.");
    PROTO_ASSERT(operands[1]->IsDouble(), "Remainder operator requires its operands to be simple values.");
    double result = fmod(GET_SIMPLE_VALUE(operands[0]), GET_SIMPLE_VALUE(operands[1]));
    return TraceResult(boost::make_shared<SimpleValue>(result));
}



MathmlQuotient::MathmlQuotient(const std::vector<AbstractExpressionPtr>& rOperands)
    : MathmlOperator("quotient", rOperands)
{
    PROTO_ASSERT(mChildren.size() == 2, "Quotient operator requires 2 operands.");
}

AbstractValuePtr MathmlQuotient::operator()(const Environment& rEnv) const
{
    std::vector<AbstractValuePtr> operands = EvaluateChildren(rEnv);
    PROTO_ASSERT(operands[0]->IsDouble(), "Quotient operator requires its operands to be simple values.");
    PROTO_ASSERT(operands[1]->IsDouble(), "Quotient operator requires its operands to be simple values.");
    double result;
    modf(GET_SIMPLE_VALUE(operands[0]) / GET_SIMPLE_VALUE(operands[1]), &result);
    return TraceResult(boost::make_shared<SimpleValue>(result));
}



MathmlPower::MathmlPower(const std::vector<AbstractExpressionPtr>& rOperands)
    : MathmlOperator("power", rOperands)
{
    PROTO_ASSERT(mChildren.size() == 2, "Power operator requires 2 operands.");
}

AbstractValuePtr MathmlPower::operator()(const Environment& rEnv) const
{
    std::vector<AbstractValuePtr> operands = EvaluateChildren(rEnv);
    PROTO_ASSERT(operands[0]->IsDouble(), "Power operator requires its operands to be simple values.");
    PROTO_ASSERT(operands[1]->IsDouble(), "Power operator requires its operands to be simple values.");
    double result = pow(GET_SIMPLE_VALUE(operands[0]), GET_SIMPLE_VALUE(operands[1]));
    return TraceResult(boost::make_shared<SimpleValue>(result));
}



MathmlRoot::MathmlRoot(const std::vector<AbstractExpressionPtr>& rOperands)
    : MathmlOperator("root", rOperands)
{
    PROTO_ASSERT(mChildren.size() == 1 || mChildren.size() == 2,
                 "Root operator requires 1 operand, optionally with a degree qualifier.");
}

AbstractValuePtr MathmlRoot::operator()(const Environment& rEnv) const
{
    std::vector<AbstractValuePtr> operands = EvaluateChildren(rEnv);
    PROTO_ASSERT(operands.back()->IsDouble(), "Root operator requires its operand to be a simple value.");
    double operand = GET_SIMPLE_VALUE(operands.back());
    double degree = 2;
    if (operands.size() == 2)
    {
        PROTO_ASSERT(operands.front()->IsDouble(), "Root operator requires its qualifier to be a simple value.");
        degree = GET_SIMPLE_VALUE(operands.front());
    }
    double result;
    if (degree == 2)
    {
        result = sqrt(operand);
    }
    else
    {
        result = pow(operand, 1/degree);
    }
    return TraceResult(boost::make_shared<SimpleValue>(result));
}



MathmlAbs::MathmlAbs(const std::vector<AbstractExpressionPtr>& rOperands)
    : MathmlOperator("abs", rOperands)
{
    PROTO_ASSERT(mChildren.size() == 1, "Absolute value operator requires 1 operand.");
}

AbstractValuePtr MathmlAbs::operator()(const Environment& rEnv) const
{
    std::vector<AbstractValuePtr> operands = EvaluateChildren(rEnv);
    PROTO_ASSERT(operands[0]->IsDouble(), "Absolute value operator requires its operand to be a simple value.");
    double result = fabs(GET_SIMPLE_VALUE(operands[0]));
    return TraceResult(boost::make_shared<SimpleValue>(result));
}



MathmlFloor::MathmlFloor(const std::vector<AbstractExpressionPtr>& rOperands)
    : MathmlOperator("floor", rOperands)
{
    PROTO_ASSERT(mChildren.size() == 1, "Floor operator requires 1 operand.");
}

AbstractValuePtr MathmlFloor::operator()(const Environment& rEnv) const
{
    std::vector<AbstractValuePtr> operands = EvaluateChildren(rEnv);
    PROTO_ASSERT(operands[0]->IsDouble(), "Floor operator requires its operand to be a simple value.");
    double result = floor(GET_SIMPLE_VALUE(operands[0]));
    return TraceResult(boost::make_shared<SimpleValue>(result));
}



MathmlCeiling::MathmlCeiling(const std::vector<AbstractExpressionPtr>& rOperands)
    : MathmlOperator("ceiling", rOperands)
{
    PROTO_ASSERT(mChildren.size() == 1, "Ceiling operator requires 1 operand.");
}

AbstractValuePtr MathmlCeiling::operator()(const Environment& rEnv) const
{
    std::vector<AbstractValuePtr> operands = EvaluateChildren(rEnv);
    PROTO_ASSERT(operands[0]->IsDouble(), "Ceiling operator requires its operand to be a simple value.");
    double result = ceil(GET_SIMPLE_VALUE(operands[0]));
    return TraceResult(boost::make_shared<SimpleValue>(result));
}
