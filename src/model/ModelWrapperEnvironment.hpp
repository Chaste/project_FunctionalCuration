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

#ifndef MODELWRAPPERENVIRONMENT_HPP_
#define MODELWRAPPERENVIRONMENT_HPP_

#include "Environment.hpp"
#include "AbstractParameterisedSystem.hpp"

/**
 * An Environment subclass that wraps a model, exposing its parameters, state variables, and derived
 * quantities as simple values.
 *
 * \todo Do derived quantities need to be handled more efficiently?
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
