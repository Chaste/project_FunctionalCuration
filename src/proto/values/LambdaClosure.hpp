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

#ifndef LAMBDACLOSURE_HPP_
#define LAMBDACLOSURE_HPP_

#include <string>
#include <vector>
#include <boost/weak_ptr.hpp>

#include "AbstractValue.hpp"
#include "LocatableConstruct.hpp"

#include "AbstractStatement.hpp"
#include "Environment.hpp"

/**
 * A function definition storable in an Environment.
 */
class LambdaClosure : public AbstractValue, public LocatableConstruct
{
public:
    /**
     * Create a function closure.
     *
     * @param pDefiningEnv  the environment in which the lambda was defined
     * @param rFormalParameters  the names of the function's parameters
     * @param rBody  the body of the function - the statements to execute when the function is called
     * @param rDefaultParameters  default values for parameters, if any are defined
     */
    LambdaClosure(EnvironmentCPtr pDefiningEnv,
                  const std::vector<std::string>& rFormalParameters,
                  const std::vector<AbstractStatementPtr>& rBody,
                  const std::vector<AbstractValuePtr>& rDefaultParameters);

    /**
     * Call the function with the given parameter values in the given environment.
     *
     * @param rCallersEnv  the environment of the call
     * @param rActualParameters  the parameter values
     */
    AbstractValuePtr operator()(const Environment& rCallersEnv,
                                const std::vector<AbstractValuePtr>& rActualParameters) const;

    /** Used for testing that this is a LambdaClosure. */
    bool IsLambda() const;

private:
    /** The environment in which this lambda was defined. */
    boost::weak_ptr<const Environment> mpDefiningEnv;

    /** The names of the function's parameters. */
    std::vector<std::string> mFormalParameters;

    /** The body of the function - the statements to execute when the function is called. */
    std::vector<AbstractStatementPtr> mBody;

    /** Default values for parameters, if any are defined. */
    std::vector<AbstractValuePtr> mDefaultParameters;
};


#endif /* LAMBDACLOSURE_HPP_ */
