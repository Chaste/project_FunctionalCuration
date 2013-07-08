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

#include "LambdaClosure.hpp"

#include <cassert>

#include "BacktraceException.hpp"
#include "DebugProto.hpp"

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
    PROPAGATE_BACKTRACE_ENV(p_result = p_local_env->ExecuteStatements(mBody, true /* says return is allowed */), *p_local_env);
    return p_result;
}

bool LambdaClosure::IsLambda() const
{
    return true;
}
