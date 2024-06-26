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

#include "MathmlMinus.hpp"

#include <boost/make_shared.hpp>
#include "BacktraceException.hpp"
#include "ValueTypes.hpp"
#include "ProtoHelperMacros.hpp"

MathmlMinus::MathmlMinus(const std::vector<AbstractExpressionPtr>& rOperands)
    : MathmlOperator("minus", rOperands)
{
    PROTO_ASSERT(mChildren.size() == 1u || mChildren.size() == 2u,
                 "Minus operator requires one or two operands.");
}

AbstractValuePtr MathmlMinus::operator()(const Environment& rEnv) const
{
    std::vector<AbstractValuePtr> operands = EvaluateChildren(rEnv);
    for (std::vector<AbstractValuePtr>::const_iterator it = operands.begin();
         it != operands.end();
         ++it)
    {
        PROTO_ASSERT((*it)->IsDouble(), "Minus operator requires its operands to be simple values.");
    }
    double result;
    if (operands.size() == 1u)
    {
        result = -GET_SIMPLE_VALUE(operands[0]);
    }
    else
    {
        result = GET_SIMPLE_VALUE(operands[0]) - GET_SIMPLE_VALUE(operands[1]);
    }
    return TraceResult(boost::make_shared<SimpleValue>(result));
}
