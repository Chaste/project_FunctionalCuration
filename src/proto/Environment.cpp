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

#include "Environment.hpp"

#include <sstream>
#include <boost/foreach.hpp>

#include "BacktraceException.hpp"
#include "NullDeleter.hpp"

Environment::Environment(bool allowOverwrite)
    : mAllowOverwrite(allowOverwrite)
{}


Environment::Environment(const EnvironmentCPtr pDelegateeEnv)
    : mAllowOverwrite(false)
{
    mpDelegateeEnvs[""] = pDelegateeEnv;
}


Environment::~Environment()
{}


EnvironmentCPtr Environment::GetAsDelegatee() const
{
    return shared_from_this();
}


void Environment::SetDelegateeEnvironment(const EnvironmentCPtr pDelegateeEnv,
                                          std::string prefix)
{
    mpDelegateeEnvs[prefix] = pDelegateeEnv;
}


EnvironmentCPtr Environment::GetDelegateeEnvironment(std::string prefix) const
{
    std::map<std::string, EnvironmentCPtr>::const_iterator it = mpDelegateeEnvs.find(prefix);
    EnvironmentCPtr delegatee;
    if (it != mpDelegateeEnvs.end())
    {
        delegatee = it->second;
    }
    return delegatee;
}


/**
 * Find a suitable environment to delegate to when looking up the given name.
 * @param rEnv  the current environment
 * @param rName  the name to look up; the first prefix if present will be removed
 * @param rCallerLocation  location information to use in any error backtrace
 */
EnvironmentCPtr FindDelegatee(const Environment& rEnv, std::string& rName, const std::string& rCallerLocation)
{
    EnvironmentCPtr delegatee;
    // Check for a prefixed name first
    size_t colon = rName.find(':');
    if (colon != std::string::npos)
    {
        std::string prefix = rName.substr(0, colon);
        delegatee = rEnv.GetDelegateeEnvironment(prefix);
        if (delegatee)
        {
            rName = rName.substr(colon+1);
        }
    }
    if (!delegatee)
    {
        // Try the default delegatee if it exists
        delegatee = rEnv.GetDelegateeEnvironment();
    }
    if (!delegatee && colon != std::string::npos)
    {
        PROTO_EXCEPTION2("No environment associated with the prefix '" << rName.substr(0, colon) << "'.",
                         rCallerLocation);
    }
    return delegatee;
}


AbstractValuePtr Environment::Lookup(const std::string& rName, const std::string& rCallerLocation) const
{
    AbstractValuePtr p_result;
    std::map<std::string, AbstractValuePtr>::const_iterator it = mBindings.find(rName);
    if (it != mBindings.end())
    {
        p_result = it->second;
    }
    else if (!mpDelegateeEnvs.empty())
    {
        std::string name = rName;
        EnvironmentCPtr delegatee = FindDelegatee(*this, name, rCallerLocation);
        if (delegatee)
        {
            p_result = delegatee->Lookup(name, rCallerLocation);
        }
    }
    if (!p_result)
    {
        PROTO_EXCEPTION2("Name " << rName << " is not defined in this environment.", rCallerLocation);
    }
    return p_result;
}


void Environment::DefineName(const std::string& rName, const AbstractValuePtr pValue,
                             const std::string& rCallerLocation)
{
    if (rName.find(':') != std::string::npos)
    {
        PROTO_EXCEPTION2("Names such as '" << rName << "' containing a colon are not allowed.",
                         rCallerLocation);
    }
    std::map<std::string, AbstractValuePtr>::const_iterator it = mBindings.find(rName);
    if (it != mBindings.end())
    {
        PROTO_EXCEPTION2("Name " << rName << " is already defined and may not be re-bound.", rCallerLocation);
    }
    mBindings[rName] = pValue;
}


void Environment::DefineNames(const std::vector<std::string>& rNames,
                              const std::vector<AbstractValuePtr>& rValues,
                              const std::string& rCallerLocation)
{
    assert(rNames.size() == rValues.size());
    for (unsigned i=0; i<rNames.size(); ++i)
    {
        DefineName(rNames[i], rValues[i], rCallerLocation);
    }
}


void Environment::Merge(const Environment& rEnv, const std::string& rCallerLocation)
{
    BOOST_FOREACH(const std::string& r_name, rEnv.GetDefinedNames())
    {
        DefineName(r_name, rEnv.Lookup(r_name, rCallerLocation), rCallerLocation);
    }
}


void Environment::OverwriteDefinition(const std::string& rName, const AbstractValuePtr pValue,
                                      const std::string& rCallerLocation)
{
    std::map<std::string, AbstractValuePtr>::iterator it = mBindings.find(rName);
    if (it == mBindings.end())
    {
        std::string name(rName);
        EnvironmentPtr delegatee = boost::const_pointer_cast<Environment>(FindDelegatee(*this, name, rCallerLocation));
        if (delegatee)
        {
            delegatee->OverwriteDefinition(name, pValue, rCallerLocation);
        }
        else if (mAllowOverwrite)
        {
            PROTO_EXCEPTION2("Name " << rName << " is not defined and may not be overwritten.", rCallerLocation);
        }
        else
        {
            PROTO_EXCEPTION2("This environment does not support overwriting mappings.", rCallerLocation);
        }
    }
    else if (mAllowOverwrite)
    {
        it->second = pValue;
    }
    else
    {
        PROTO_EXCEPTION2("This environment does not support overwriting mappings.", rCallerLocation);
    }
}


unsigned Environment::GetNumberOfDefinitions() const
{
    return mBindings.size();
}


std::vector<std::string> Environment::GetDefinedNames() const
{
    std::vector<std::string> names;
    names.reserve(mBindings.size());
    for (std::map<std::string, AbstractValuePtr>::const_iterator it = mBindings.begin();
         it != mBindings.end(); ++it)
    {
        names.push_back(it->first);
    }
    return names;
}


AbstractValuePtr Environment::ExecuteStatement(const AbstractStatementPtr pStatement,
                                               bool returnAllowed)
{
    AbstractValuePtr p_result;
    PROPAGATE_BACKTRACE2(p_result = (*pStatement)(*this), pStatement->GetLocationInfo());
    if (!p_result->IsNull() && !returnAllowed)
    {
        PROTO_EXCEPTION2("Return statement found in non-function.", pStatement->GetLocationInfo());
    }
    return p_result;
}


AbstractValuePtr Environment::ExecuteStatements(const std::vector<AbstractStatementPtr>& rStatements,
                                                bool returnAllowed)
{
    AbstractValuePtr p_result;
    for (std::vector<AbstractStatementPtr>::const_iterator it = rStatements.begin();
         it != rStatements.end();
         ++it)
    {
        p_result = ExecuteStatement(*it, returnAllowed);
        if (!p_result->IsNull()) break;
    }
    return p_result;
}


std::string Environment::FreshIdent()
{
    std::stringstream ident;
    ident << "~" << mNextFreshIdent++;
    return ident.str();
}


unsigned Environment::mNextFreshIdent = 0u;

