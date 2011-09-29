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

#ifndef MODELWRAPPERENVIRONMENT_HPP_
#define MODELWRAPPERENVIRONMENT_HPP_

#include "Environment.hpp"
#include "AbstractParameterisedSystem.hpp"

/**
 * An Environment subclass that wraps a model, exposing its parameters, state variables, and derived
 * quantities as simple values.
 *
 * \todo How do we pass the right time to GetAnyVariable?
 * \todo How do we handle derived quantities?  If time is OK, it should work, albeit inefficiently.
 */
template<typename VECTOR>
class ModelWrapperEnvironment : public Environment
{
public:
    /**
     * Construct a new wrapper environment.
     * @param pModel  the model to wrap
     */
    ModelWrapperEnvironment(boost::shared_ptr<AbstractParameterisedSystem<VECTOR> > pModel);

    /**
     * Look up a name in the environment and return the mapped value.
     * @param rName  the name to look up
     * @param rCallerLocation  location information to use in error backtrace if name isn't defined
     */
    AbstractValuePtr Lookup(const std::string& rName, const std::string& rCallerLocation="<anon>") const;

    /**
     * Adding to this kind of environment isn't allowed.
     * @param rName
     * @param pValue
     * @param rCallerLocation
     */
    void DefineName(const std::string& rName, const AbstractValuePtr pValue,
                    const std::string& rCallerLocation);

    /**
     * Modify a name-value mapping in the environment, in other words, set a model variable.
     * This is used for the SetValue modifier.
     * @param rName
     * @param pValue
     * @param rCallerLocation  location information to use in error backtrace if name is not already defined
     */
    void OverwriteDefinition(const std::string& rName, const AbstractValuePtr pValue,
                             const std::string& rCallerLocation);

    /** Get the number of definitions in this environment. */
    unsigned GetNumberOfDefinitions() const;

    /** Get the names defined in this environment. */
    std::vector<std::string> GetDefinedNames() const;

private:
    /** The encapsulated model. */
    boost::shared_ptr<AbstractParameterisedSystem<VECTOR> > mpModel;
};

#endif // MODELWRAPPERENVIRONMENT_HPP_
