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

#include "MathmlRelations.hpp"

#include <boost/make_shared.hpp>
#include "BacktraceException.hpp"
#include "ValueTypes.hpp"
#include "ProtoHelperMacros.hpp"

MathmlEq::MathmlEq(const std::vector<AbstractExpressionPtr>& rOperands)
    : MathmlOperator("eq", rOperands)
{
    PROTO_ASSERT(mChildren.size() == 2, "Equality operator requires 2 operands.");
}

AbstractValuePtr MathmlEq::operator()(const Environment& rEnv) const
{
    std::vector<AbstractValuePtr> operands = EvaluateChildren(rEnv);
    PROTO_ASSERT(operands[0]->IsDouble(), "Equality operator requires its operands to be simple values.");
    PROTO_ASSERT(operands[1]->IsDouble(), "Equality operator requires its operands to be simple values.");
    bool result = GET_SIMPLE_VALUE(operands[0]) == GET_SIMPLE_VALUE(operands[1]);
    return boost::make_shared<SimpleValue>(result);
}


MathmlNeq::MathmlNeq(const std::vector<AbstractExpressionPtr>& rOperands)
    : MathmlOperator("neq", rOperands)
{
    PROTO_ASSERT(mChildren.size() == 2, "Not-equal operator requires 2 operands.");
}

AbstractValuePtr MathmlNeq::operator()(const Environment& rEnv) const
{
    std::vector<AbstractValuePtr> operands = EvaluateChildren(rEnv);
    PROTO_ASSERT(operands[0]->IsDouble(), "Not-equal operator requires its operands to be simple values.");
    PROTO_ASSERT(operands[1]->IsDouble(), "Not-equal operator requires its operands to be simple values.");
    bool result = GET_SIMPLE_VALUE(operands[0]) != GET_SIMPLE_VALUE(operands[1]);
    return boost::make_shared<SimpleValue>(result);
}


MathmlLt::MathmlLt(const std::vector<AbstractExpressionPtr>& rOperands)
    : MathmlOperator("lt", rOperands)
{
    PROTO_ASSERT(mChildren.size() == 2, "Less-than operator requires 2 operands.");
}

AbstractValuePtr MathmlLt::operator()(const Environment& rEnv) const
{
    std::vector<AbstractValuePtr> operands = EvaluateChildren(rEnv);
    PROTO_ASSERT(operands[0]->IsDouble(), "Less-than operator requires its operands to be simple values.");
    PROTO_ASSERT(operands[1]->IsDouble(), "Less-than operator requires its operands to be simple values.");
    bool result = GET_SIMPLE_VALUE(operands[0]) < GET_SIMPLE_VALUE(operands[1]);
    return boost::make_shared<SimpleValue>(result);
}


MathmlGt::MathmlGt(const std::vector<AbstractExpressionPtr>& rOperands)
    : MathmlOperator("gt", rOperands)
{
    PROTO_ASSERT(mChildren.size() == 2, "Greater-than operator requires 2 operands.");
}

AbstractValuePtr MathmlGt::operator()(const Environment& rEnv) const
{
    std::vector<AbstractValuePtr> operands = EvaluateChildren(rEnv);
    PROTO_ASSERT(operands[0]->IsDouble(), "Greater-than operator requires its operands to be simple values.");
    PROTO_ASSERT(operands[1]->IsDouble(), "Greater-than operator requires its operands to be simple values.");
    bool result = GET_SIMPLE_VALUE(operands[0]) > GET_SIMPLE_VALUE(operands[1]);
    return boost::make_shared<SimpleValue>(result);
}


MathmlLeq::MathmlLeq(const std::vector<AbstractExpressionPtr>& rOperands)
    : MathmlOperator("leq", rOperands)
{
    PROTO_ASSERT(mChildren.size() == 2, "Less-than-or-equals operator requires 2 operands.");
}

AbstractValuePtr MathmlLeq::operator()(const Environment& rEnv) const
{
    std::vector<AbstractValuePtr> operands = EvaluateChildren(rEnv);
    PROTO_ASSERT(operands[0]->IsDouble(), "Less-than-or-equals operator requires its operands to be simple values.");
    PROTO_ASSERT(operands[1]->IsDouble(), "Less-than-or-equals operator requires its operands to be simple values.");
    bool result = GET_SIMPLE_VALUE(operands[0]) <= GET_SIMPLE_VALUE(operands[1]);
    return boost::make_shared<SimpleValue>(result);
}


MathmlGeq::MathmlGeq(const std::vector<AbstractExpressionPtr>& rOperands)
    : MathmlOperator("geq", rOperands)
{
    PROTO_ASSERT(mChildren.size() == 2, "Greater-than-or-equals operator requires 2 operands.");
}

AbstractValuePtr MathmlGeq::operator()(const Environment& rEnv) const
{
    std::vector<AbstractValuePtr> operands = EvaluateChildren(rEnv);
    PROTO_ASSERT(operands[0]->IsDouble(), "Greater-than-or-equals operator requires its operands to be simple values.");
    PROTO_ASSERT(operands[1]->IsDouble(), "Greater-than-or-equals operator requires its operands to be simple values.");
    bool result = GET_SIMPLE_VALUE(operands[0]) >= GET_SIMPLE_VALUE(operands[1]);
    return boost::make_shared<SimpleValue>(result);
}
