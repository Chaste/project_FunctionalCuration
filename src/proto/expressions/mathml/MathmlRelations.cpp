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
    return TraceResult(boost::make_shared<SimpleValue>(result));
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
    return TraceResult(boost::make_shared<SimpleValue>(result));
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
    return TraceResult(boost::make_shared<SimpleValue>(result));
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
    return TraceResult(boost::make_shared<SimpleValue>(result));
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
    return TraceResult(boost::make_shared<SimpleValue>(result));
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
    return TraceResult(boost::make_shared<SimpleValue>(result));
}
