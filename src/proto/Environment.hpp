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

#ifndef ENVIRONMENT_HPP_
#define ENVIRONMENT_HPP_

#include <string>
#include <vector>
#include <map>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include "AbstractStatement.hpp"
#include "AbstractValue.hpp"

class Environment;
typedef boost::shared_ptr<const Environment> EnvironmentCPtr;
typedef boost::shared_ptr<Environment> EnvironmentPtr;

/**
 * A mapping of names to values in the protocol language.
 */
class Environment : boost::noncopyable
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
     * Modify a name-value mapping in the environment.
     * This is provided to support the definition of protocol inputs, and is only permitted if the
     * environment has been created with the allowOverwrite flag on.
     * @param rName
     * @param pValue
     * @param rCallerLocation  location information to use in error backtrace if name is not already defined
     */
    virtual void OverwriteDefinition(const std::string& rName, const AbstractValuePtr pValue,
                                     const std::string& rCallerLocation);

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

private:
    /** The actual name-value bindings. */
    std::map<std::string, AbstractValuePtr> mBindings;

    /**
     * Environments to delegate to if we are asked to look up a name that isn't defined here.
     */
    std::map<std::string, EnvironmentCPtr> mpDelegateeEnvs;

    /** Whether to allow OverwriteDefinition to be called. */
    bool mAllowOverwrite;

    /** For use in FreshIdent. */
    static unsigned mNextFreshIdent;
};

#endif // ENVIRONMENT_HPP_
