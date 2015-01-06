/*

Copyright (c) 2005-2015, University of Oxford.
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

#include "Environment.hpp"

#include <sstream>
#include <cassert>
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
    // This should normally be caught at a higher level, but just in case...
    PROTO_ASSERT2(prefix.empty() || !mpDelegateeEnvs[prefix] || mpDelegateeEnvs[prefix] == pDelegateeEnv,
                  "Delegatee environment prefix '" << prefix << "' is already in use.",
                  "Environment::SetDelegateeEnvironment");
    mpDelegateeEnvs[prefix] = pDelegateeEnv;
}


void Environment::AddSubEnvironment(const EnvironmentCPtr pSubEnv, std::string prefix)
{
    PROTO_ASSERT2(!prefix.empty(),
                  "A sub-environment must be given a prefix.",
                  "Environment::AddSubEnvironment");
    SetDelegateeEnvironment(pSubEnv, prefix);
    mSubEnvironmentNames.push_back(prefix);
}


const std::vector<std::string>& Environment::rGetSubEnvironmentNames() const
{
    return mSubEnvironmentNames;
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


void Environment::Clear()
{
    mBindings.clear();
    BOOST_FOREACH(const std::string& r_prefix, rGetSubEnvironmentNames())
    {
        EnvironmentPtr p_sub_env
            = boost::const_pointer_cast<Environment>(mpDelegateeEnvs[r_prefix]);
        p_sub_env->Clear();
    }
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


void Environment::RemoveDefinition(const std::string& rName, const std::string& rCallerLocation)
{
    if (mAllowOverwrite)
    {
        std::map<std::string, AbstractValuePtr>::iterator it = mBindings.find(rName);
        if (it == mBindings.end())
        {
            PROTO_EXCEPTION2("Name " << rName << " is not defined and may not be removed.", rCallerLocation);
        }
        else
        {
            mBindings.erase(it);
        }
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

