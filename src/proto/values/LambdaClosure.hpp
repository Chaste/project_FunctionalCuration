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
