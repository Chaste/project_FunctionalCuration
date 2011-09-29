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
    boost::shared_ptr<LambdaClosure> p_closure(new LambdaClosure(rEnv, mFormalParameters, mBody, mDefaultParameters));
    p_closure->SetLocationInfo(GetLocationInfo());
    return p_closure;
}

void LambdaExpression::CheckLengths() const
{
    PROTO_ASSERT(mDefaultParameters.empty() || mDefaultParameters.size() == mFormalParameters.size(),
                 "If default values are given, the vector should have the same size as the number of parameters (" << mFormalParameters.size() << "); length was " << mDefaultParameters.size() << ".");
}
