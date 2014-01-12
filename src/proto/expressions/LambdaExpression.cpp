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

#include "LambdaExpression.hpp"

#include "ReturnStatement.hpp"
#include "LambdaClosure.hpp"
#include "BacktraceException.hpp"

LambdaExpression::LambdaExpression(const std::vector<std::string>& rFormalParameters,
                 const std::vector<AbstractStatementPtr>& rBody,
                 const std::vector<AbstractValuePtr>& rDefaults)
    : mFormalParameters(rFormalParameters),
      mBody(rBody),
      mDefaultParameters(rDefaults)
{
    CheckLengths();
}

LambdaExpression::LambdaExpression(const std::vector<std::string>& rFormalParameters,
                 const AbstractExpressionPtr pBodyExpr,
                 const std::vector<AbstractValuePtr>& rDefaults)
    : mFormalParameters(rFormalParameters),
      mDefaultParameters(rDefaults)
{
    CheckLengths();
    mBody.push_back(boost::make_shared<ReturnStatement>(pBodyExpr));
    mBody.back()->SetLocationInfo("(Implicit return statement)");
}

AbstractValuePtr LambdaExpression::operator()(const Environment& rEnv) const
{
    boost::shared_ptr<LambdaClosure> p_closure(new LambdaClosure(rEnv.GetAsDelegatee(),
                                                                 mFormalParameters, mBody, mDefaultParameters));
    p_closure->SetLocationInfo(GetLocationInfo());
    return TraceResult(p_closure);
}

void LambdaExpression::CheckLengths() const
{
    PROTO_ASSERT(mDefaultParameters.empty() || mDefaultParameters.size() == mFormalParameters.size(),
                 "If default values are given, the vector should have the same size as the number of parameters (" << mFormalParameters.size() << "); length was " << mDefaultParameters.size() << ".");
}
