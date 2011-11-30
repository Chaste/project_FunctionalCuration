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

#include "LambdaClosure.hpp"

#include "BacktraceException.hpp"

LambdaClosure::LambdaClosure(EnvironmentCPtr pDefiningEnv,
                             const std::vector<std::string>& rFormalParameters,
                             const std::vector<AbstractStatementPtr>& rBody,
                             const std::vector<AbstractValuePtr>& rDefaultParameters)
    : mpDefiningEnv(pDefiningEnv),
      mFormalParameters(rFormalParameters),
      mBody(rBody),
      mDefaultParameters(rDefaultParameters)
{
    // This should be checked by the defining LambdaExpression
    assert(mDefaultParameters.empty() || mDefaultParameters.size() == mFormalParameters.size());
}

AbstractValuePtr LambdaClosure::operator()(const Environment& rCallersEnv,
                                           const std::vector<AbstractValuePtr>& rActualParameters) const
{
    const unsigned num_params = mFormalParameters.size();
    PROTO_ASSERT(num_params == rActualParameters.size()
                 || (!mDefaultParameters.empty() && num_params > rActualParameters.size()),
                 "Function expected " << num_params << " parameters, but received " << rActualParameters.size() << ".");
    std::vector<AbstractValuePtr> params = rActualParameters;
    params.resize(num_params);
    // Default parameters (if any)
    if (!mDefaultParameters.empty())
    {
        assert(num_params == mDefaultParameters.size());
        for (unsigned i=0; i<num_params; ++i)
        {
            if (!params[i] || params[i]->IsDefault())
            {
                PROTO_ASSERT(mDefaultParameters[i],
                             "Default value requested for parameter " << i << ", but no default supplied.");
                params[i] = mDefaultParameters[i];
            }
        }
    }
    // Create local environment and execute function body
    EnvironmentPtr p_local_env(new Environment(mpDefiningEnv.lock()->GetAsDelegatee()));
    p_local_env->DefineNames(mFormalParameters, params, GetLocationInfo());
    AbstractValuePtr p_result;
    PROPAGATE_BACKTRACE(p_result = p_local_env->ExecuteStatements(mBody, true /* says return is allowed */));
    return p_result;
}

bool LambdaClosure::IsLambda() const
{
    return true;
}
