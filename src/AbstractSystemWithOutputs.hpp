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
#ifndef ABSTRACTSYSTEMWITHOUTPUTS_HPP_
#define ABSTRACTSYSTEMWITHOUTPUTS_HPP_

#include <vector>
#include <string>
#include <map>

#include "Environment.hpp"

/**
 * Base class for models in the protocol system.   It allows querying what outputs are available,
 * declares the GetOutputs method for accessing the current values of the outputs, and provides
 * access to the model's wrapper environment(s) for reading/setting model variables.
 *
 * In addition, it keeps track of the value of the model's free variable and declares a SolveModel
 * method.  These support the case where a model is a system of ODEs.
 */
class AbstractSystemWithOutputs
{
public:
    /**
     * Get the number of system outputs.
     */
    unsigned GetNumberOfOutputs() const;

    /**
     * Get the names of this system's outputs.
     */
    const std::vector<std::string>& rGetOutputNames() const;

    /**
     * Get the units of this system's outputs.
     */
    const std::vector<std::string>& rGetOutputUnits() const;

    /**
     * Get the current values of this system's outputs.
     */
    virtual EnvironmentCPtr GetOutputs() =0;


    /**
     * Set the current value of the free variable (typically time).
     *
     * @param freeVariable  the new value
     */
    void SetFreeVariable(double freeVariable);

    /**
     * Get the current value of the free variable (typically time).
     */
    double GetFreeVariable() const;

    /**
     * Solve the system from the current state up to the given point.
     *
     * @param endPoint  the final value of the free variable
     */
    virtual void SolveModel(double endPoint) =0;


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


    /** Default constructor sets the free variable to an 'unset' value. */
    AbstractSystemWithOutputs();

    /** Virtual destructor since we have virtual methods. */
    virtual ~AbstractSystemWithOutputs();

protected:
    /** Names of system outputs. */
    std::vector<std::string> mOutputNames;

    /** Units of system outputs. */
    std::vector<std::string> mOutputUnits;

    /** Stores the current value of the free variable; used for solving the system. */
    double mFreeVariable;

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
 * An intermediate base class for models created from CellML by PyCml.
 *
 * The template parameter is the type used by the model for vectors of real numbers.
 * It will be either std::vector<double> (for AbstractCardiacCell subclasses) or
 * N_Vector (for AbstractCvodeCell subclasses).
 */
template<typename VECTOR>
class AbstractTemplatedSystemWithOutputs : public AbstractSystemWithOutputs
{
public:
    /**
     * Get the current values of this system's outputs.
     */
    EnvironmentCPtr GetOutputs();

    /**
     * Set the bindings from prefix to namespace URI used by the protocol for accessing model
     * variables.  The Environment wrappers around this model can then be created and
     * retrieved with rGetEnvironmentMap.
     *
     * @param rNamespaceBindings  the prefix->URI map
     */
    void SetNamespaceBindings(const std::map<std::string, std::string>& rNamespaceBindings);

protected:
    /**
     * Must be called by subclasses after they have set up #mOutputsInfo, #mVectorOutputsInfo
     * and #mVectorOutputNames.  This method fills in #mOutputNames and #mOutputUnits from the
     * information therein.
     */
    void ProcessOutputsInfo();

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

    /**
     * Names of the system outputs that are vectors of variables.  The order must match
     * #mVectorOutputsInfo.  This must be set up by subclass constructors, if the system
     * has such outputs.
     */
    std::vector<std::string> mVectorOutputNames;
};

#endif /*ABSTRACTSYSTEMWITHOUTPUTS_HPP_*/
