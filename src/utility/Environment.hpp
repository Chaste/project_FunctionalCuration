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

#ifndef ENVIRONMENT_HPP_
#define ENVIRONMENT_HPP_

#include <string>
#include <vector>
#include <map>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "AbstractStatement.hpp"
#include "AbstractValue.hpp"

class Environment;
typedef boost::shared_ptr<const Environment> EnvironmentCPtr;
typedef boost::shared_ptr<Environment> EnvironmentPtr;

// For use with BOOST_FOREACH and std::map
typedef std::pair<std::string, EnvironmentPtr> StringEnvPair;

/**
 * A mapping of names to values in the protocol language.
 *
 * Note that the class requires that all instances are managed by shared pointers.  The
 * delegatee functionality will produce tr1::bad_weak_ptr exceptions if this is not done.
 */
class Environment : boost::noncopyable, public boost::enable_shared_from_this<Environment>
{
public:
    /**
     * Create an empty root environment.
     * @param allowOverwrite  whether to allow the method OverwriteDefinition to be called
     */
    Environment(bool allowOverwrite=false);

    /**
     * The delegatee environment is the mechanism used to have multiple scopes, e.g. function,
     * module, global.  Scopes are always nested, so the delegatee will delegate in turn if the
     * name isn't found.  This constructor creates us with a default delegatee.
     *
     * Additional delegatees can also be bound to namespace prefixes with the SetDelegateeEnvironment
     * method; this mechanism is used for looking up ontology terms in a model, dealing with
     * imported protocols, and accessing simulation results.
     *
     * @param pDelegateeEnv  the default delegatee environment
     */
    Environment(const EnvironmentCPtr pDelegateeEnv);

    /** We have virtual methods. */
    virtual ~Environment();

    /**
     * Get the delegatee for this environment corresponding to the given prefix.
     * Defaults to the default delegatee.
     * Returns an empty pointer if no delegatee exists for that prefix.
     *
     * @param prefix  the namespace prefix to get the delegatee for
     */
    EnvironmentCPtr GetDelegateeEnvironment(std::string prefix="") const;

    /**
     * Set the delegatee environment to use for a given prefix.
     *
     * @param pDelegateeEnv  the delegatee environment
     * @param prefix  the namespace prefix to set the delegatee for
     */
    void SetDelegateeEnvironment(const EnvironmentCPtr pDelegateeEnv,
                                 std::string prefix="");

    /**
     * Get a pointer to this environment so that we can pass it as a delegatee.
     */
    EnvironmentCPtr GetAsDelegatee() const;

    /**
     * Add a new sub-environment.  This is just a delegatee environment, but its
     * prefix (which must be non-empty) is recorded in a separate list, which may
     * be retrieved using rGetSubEnvironmentNames().
     *
     * @param pSubEnv  the delegatee environment
     * @param prefix  the non-empty prefix to use
     */
    void AddSubEnvironment(const EnvironmentCPtr pSubEnv, std::string prefix);

    /**
     * Get the prefixes of our sub-environments, if any.
     */
    const std::vector<std::string>& rGetSubEnvironmentNames() const;

    /**
     * Remove all definitions from this environment, and from any sub-environments (but not other delegatees).
     */
    void Clear();

    /**
     * Test whether a name is defined in this environment, or its delegatees (if a prefixed name is given).
     *
     * @param rName  the name to check
     * @param rCallerLocation  location information to use in error backtrace if a prefix doesn't match any delegatee
     */
    bool HasName(const std::string& rName, const std::string& rCallerLocation="<anon>") const;

    /**
     * Look up a name in the environment and return the mapped value.
     * @param rName  the name to look up
     * @param rCallerLocation  location information to use in error backtrace if name isn't defined
     */
    virtual AbstractValuePtr Lookup(const std::string& rName, const std::string& rCallerLocation="<anon>") const;

    /**
     * Add a new name-value mapping to the environment.
     *
     * @param rName
     * @param pValue
     * @param rCallerLocation  location information to use in error backtrace if name is already defined
     */
    virtual void DefineName(const std::string& rName, const AbstractValuePtr pValue,
                            const std::string& rCallerLocation);

    /**
     * Add a set of name-value mappings to the environment.
     * This is used to define function arguments in the local environment, for instance.
     *
     * @param rNames
     * @param rValues
     * @param rCallerLocation  location information to use in error backtrace if name is already defined
     */
    void DefineNames(const std::vector<std::string>& rNames, const std::vector<AbstractValuePtr>& rValues,
                     const std::string& rCallerLocation);

    /**
     * Merge two environments - add all the definitions from rEnv to this environment.
     *
     * @param rEnv  environment to merge into this one
     * @param rCallerLocation  location information to use in error backtrace if a name is already defined
     */
    void Merge(const Environment& rEnv, const std::string& rCallerLocation);

    /**
     * Modify a name-value mapping in the environment.
     * This is provided to support the definition of protocol inputs, and is only permitted if the
     * environment has been created with the allowOverwrite flag on.
     * @param rName
     * @param pValue
     * @param rCallerLocation  location information to use in error backtrace if name is not already defined
     */
    virtual void OverwriteDefinition(const std::string& rName, const AbstractValuePtr pValue,
                                     const std::string& rCallerLocation);

    /**
     * Iff this environment supports overwriting definitions, this method will remove an existing
     * definition of the given name.
     * Delegatees are not consulted by this method: the name must be defined in this environment.
     *
     * @param rName
     * @param rCallerLocation  location information to use in error backtrace if name is not already defined
     */
    virtual void RemoveDefinition(const std::string& rName, const std::string& rCallerLocation);

    /** Get the number of definitions in this environment. */
    virtual unsigned GetNumberOfDefinitions() const;

    /** Get the names defined in this environment. */
    virtual std::vector<std::string> GetDefinedNames() const;

    /**
     * Execute a protocol language statement within the context of this environment.
     *
     * @param pStatement  the statement
     * @param returnAllowed  whether we are in a function so a return statement is allowed
     */
    AbstractValuePtr ExecuteStatement(const AbstractStatementPtr pStatement,
                                      bool returnAllowed=false);

    /**
     * Execute a sequence of protocol language statements within the context of this environment.
     *
     * @param rStatements  the statements
     * @param returnAllowed  whether we are in a function so a return statement is allowed
     */
    AbstractValuePtr ExecuteStatements(const std::vector<AbstractStatementPtr>& rStatements,
                                       bool returnAllowed=false);

    /**
     * Get a name that is guaranteed not to be defined in this environment.
     */
    static std::string FreshIdent();

protected:
    /** The actual name-value bindings. */
    std::map<std::string, AbstractValuePtr> mBindings;

private:
    /**
     * Environments to delegate to if we are asked to look up a name that isn't defined here.
     */
    std::map<std::string, EnvironmentCPtr> mpDelegateeEnvs;

    /** Names of our sub-environments. */
    std::vector<std::string> mSubEnvironmentNames;

    /** Whether to allow OverwriteDefinition to be called. */
    bool mAllowOverwrite;

    /** For use in FreshIdent. */
    static unsigned mNextFreshIdent;
};

#endif // ENVIRONMENT_HPP_
