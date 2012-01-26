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
#ifndef ABSTRACTSYSTEMWITHOUTPUTS_HPP_
#define ABSTRACTSYSTEMWITHOUTPUTS_HPP_

#include <vector>
#include <string>
#include <map>

#include "Environment.hpp"

/**
 * Base class for AbstractSystemWithOutputs which contains all the data and functionality
 * that doesn't depend on the underlying vector type.  In particular, this class allows
 * querying what outputs are available, but not actually computing them.
 */
class AbstractUntemplatedSystemWithOutputs
{
public:
    /**
     * Get the number of system outputs.
     */
    unsigned GetNumberOfOutputs() const;

    /**
     * Get the vector of output names.
     */
    std::vector<std::string> GetOutputNames() const;

    /**
     * Get the vector of output units.
     */
    std::vector<std::string> GetOutputUnits() const;

    /**
     * Get the index of an output, given its name.
     *
     * @param rName  the name of an output.
     */
    unsigned GetOutputIndex(const std::string& rName) const;

    /**
     * Get the names of system outputs which are vectors of variables.  Currently these names
     * are oxmeta names, but eventually they'll be arbitrary URIs.
     */
    const std::vector<std::string>& rGetVectorOutputNames() const;

    /**
     * Get the lengths of the system outputs which are vectors of variables.
     */
    std::vector<unsigned> GetVectorOutputLengths() const;

    /**
     * Set the bindings from prefix to namespace URI used by the protocol for accessing model
     * variables.  The Environment wrappers around this model can then be created and
     * retrieved with rGetEnvironmentMap.
     *
     * @param rNamespaceBindings  the prefix->URI map
     */
    virtual void SetNamespaceBindings(const std::map<std::string, std::string>& rNamespaceBindings) =0;

    /**
     * Get the environments to use for easy access to (annotated) model variables.
     * The returned map associates each Environment wrapping some portion of the model
     * variables with the prefix to use for accessing it.  SetNamespaceBindings must
     * have been called before using this method.
     */
    const std::map<std::string, EnvironmentPtr>& rGetEnvironmentMap() const;

    /**
     * Get the short name used in the model code for the given variable reference.
     * @param rVariableReference  the value of the 'target' attribute on a SED-ML variable reference.
     * @return a name that can be looked up in the model environment
     */
    const std::string& rGetShortName(const std::string& rVariableReference) const;

    /**
     * What types of variable can be system outputs.
     */
    enum OutputTypes
    {
        FREE,      ///< The free variable (usually time)
        STATE,     ///< A state variable
        PARAMETER, ///< A modifiable parameter
        DERIVED    ///< A derived quantity
    };

    /** Virtual destructor to force this class to be polymorphic. */
    virtual ~AbstractUntemplatedSystemWithOutputs();

protected:
    /**
     * Information encoding which variables in the system are outputs.
     * Their order matters.
     * This must be set up by subclass constructors, if the system has outputs.
     */
    std::vector<std::pair<unsigned, OutputTypes> > mOutputsInfo;

    /**
     * Information encoding which vectors of variables in the system form single outputs.
     * Their order matters.
     * This must be set up by subclass constructors, if the system has such outputs.
     */
    std::vector<std::vector<std::pair<unsigned, OutputTypes> > > mVectorOutputsInfo;

    /** Names of system outputs that are vectors of variables. */
    std::vector<std::string> mVectorOutputNames;

    /** Environments wrapping model variables, with their prefixes. */
    std::map<std::string, EnvironmentPtr> mEnvironmentMap;

    /**
     * For SED-ML support, a map from the variable reference target used in the SED-ML document
     * to the variable name in the model, which can be used to look up the variable in the (only,
     * in this case) wrapper Environment.
     */
    std::map<std::string, std::string> mNameMap;
};

/**
 * An additional base class for cells that have outputs specified via a protocol.
 *
 * The template parameter is the type used by the cell for vectors of real numbers.
 * It will be either std::vector<double> (for AbstractCardiacCell subclasses) or
 * N_Vector (for AbstractCvodeCell subclasses).
 */
template<typename VECTOR>
class AbstractSystemWithOutputs : public AbstractUntemplatedSystemWithOutputs
{
public:
    /**
     * Compute the system outputs from the given system state.
     * Uses the current values for the parameters.
     *
     * \todo  Is using current param values safe for all protocols?  Will need to watch out for this.
     *
     * @param time  the time at which to compute the outputs
     * @param rState  values for the state variables
     */
    VECTOR ComputeOutputs(double time,
                          const VECTOR& rState);

    /**
     * Compute the system outputs which are vectors of variables from the given system state.
     * Uses the current values for the parameters.
     *
     * @param time  the time at which to compute the outputs
     * @param rState  values for the state variables
     */
    std::vector<VECTOR> ComputeVectorOutputs(double time, const VECTOR& rState);

    /**
     * Set the bindings from prefix to namespace URI used by the protocol for accessing model
     * variables.  The Environment wrappers around this model can then be created and
     * retrieved with rGetEnvironmentMap.
     *
     * @param rNamespaceBindings  the prefix->URI map
     */
    void SetNamespaceBindings(const std::map<std::string, std::string>& rNamespaceBindings);
};

#endif /*ABSTRACTSYSTEMWITHOUTPUTS_HPP_*/
