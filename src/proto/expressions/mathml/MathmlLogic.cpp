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
