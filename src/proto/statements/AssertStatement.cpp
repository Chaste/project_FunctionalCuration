/*

Copyright (c) 2005-2012, University of Oxford.
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

#include "AssertStatement.hpp"

#include <boost/make_shared.hpp>
#include "ValueTypes.hpp"
#include "BacktraceException.hpp"
#include "ProtoHelperMacros.hpp"

AssertStatement::AssertStatement(const AbstractExpressionPtr pAssertion)
    : mpAssertion(pAssertion)
{}

AbstractValuePtr AssertStatement::operator()(Environment& rEnv) const
{
    AbstractValuePtr p_assertion_value = (*mpAssertion)(rEnv);
    PROTO_ASSERT(p_assertion_value->IsDouble() || p_assertion_value->IsNull(),
                 "Assertion did not yield a simple value or null.");
    PROTO_ASSERT(p_assertion_value->IsDouble(), "Assertion failed: result is Null.");
    PROTO_ASSERT(GET_SIMPLE_VALUE(p_assertion_value), "Assertion failed: result is zero.");
    return boost::make_shared<NullValue>();
}
