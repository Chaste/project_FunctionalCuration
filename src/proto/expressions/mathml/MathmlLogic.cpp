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

#include "MathmlLogic.hpp"

#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include "BacktraceException.hpp"
#include "ValueTypes.hpp"
#include "ProtoHelperMacros.hpp"

MathmlAnd::MathmlAnd(const std::vector<AbstractExpressionPtr>& rOperands)
    : MathmlOperator("and", rOperands)
{
    PROTO_ASSERT(!mChildren.empty(), "Boolean 'and' operator requires operands.");
}

AbstractValuePtr MathmlAnd::operator()(const Environment& rEnv) const
{
    bool result = true;
    for (std::vector<AbstractExpressionPtr>::const_iterator it=mChildren.begin();
         result && it != mChildren.end(); // Short-circuit
         ++it)
    {
        AbstractValuePtr p_value = (**it)(rEnv);
        PROTO_ASSERT(p_value->IsDouble(), "Boolean 'and' operator requires its operands to be simple values.");
        result = result && GET_SIMPLE_VALUE(p_value);
    }
    return TraceResult(boost::make_shared<SimpleValue>(result));
}



MathmlOr::MathmlOr(const std::vector<AbstractExpressionPtr>& rOperands)
    : MathmlOperator("or", rOperands)
{
    PROTO_ASSERT(!mChildren.empty(), "Boolean 'or' operator requires operands.");
}

AbstractValuePtr MathmlOr::operator()(const Environment& rEnv) const
{
    bool result = false;
    for (std::vector<AbstractExpressionPtr>::const_iterator it=mChildren.begin();
         !result && it != mChildren.end(); // Short-circuit
         ++it)
    {
        AbstractValuePtr p_value = (**it)(rEnv);
        PROTO_ASSERT(p_value->IsDouble(), "Boolean 'or' operator requires its operands to be simple values.");
        result = result || GET_SIMPLE_VALUE(p_value);
    }
    return TraceResult(boost::make_shared<SimpleValue>(result));
}



MathmlXor::MathmlXor(const std::vector<AbstractExpressionPtr>& rOperands)
    : MathmlOperator("xor", rOperands)
{
    PROTO_ASSERT(!mChildren.empty(), "Boolean 'xor' operator requires operands.");
}

AbstractValuePtr MathmlXor::operator()(const Environment& rEnv) const
{
    bool result = false;
    std::vector<AbstractValuePtr> operands = EvaluateChildren(rEnv);
    BOOST_FOREACH(AbstractValuePtr p_operand, operands)
    {
        PROTO_ASSERT(p_operand->IsDouble(), "Boolean 'xor' operator requires its operands to be simple values.");
        result ^= bool(GET_SIMPLE_VALUE(p_operand));
    }
    return TraceResult(boost::make_shared<SimpleValue>(result));
}



MathmlNot::MathmlNot(const std::vector<AbstractExpressionPtr>& rOperands)
    : MathmlOperator("not", rOperands)
{
    PROTO_ASSERT(mChildren.size() == 1, "Boolean 'not' operator requires 1 operand.");
}

AbstractValuePtr MathmlNot::operator()(const Environment& rEnv) const
{
    AbstractValuePtr p_operand = (*mChildren.front())(rEnv);
    PROTO_ASSERT(p_operand->IsDouble(), "Boolean 'not' operator requires its operand to be a simple value.");
    bool result = !(GET_SIMPLE_VALUE(p_operand));
    return TraceResult(boost::make_shared<SimpleValue>(result));
}
