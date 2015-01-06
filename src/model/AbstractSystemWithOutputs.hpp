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
#ifndef ABSTRACTSYSTEMWITHOUTPUTS_HPP_
#define ABSTRACTSYSTEMWITHOUTPUTS_HPP_

#include <vector>
#include <string>
#include <map>

#include "Environment.hpp"
#include "OutputFileHandler.hpp"
#include "FileFinder.hpp"

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
     * @return  the names of this system's inputs.
     */
    const std::vector<std::string>& rGetInputNames() const;

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
     * @return whether the model doesn't maintain internal state between successive calls to SolveModel,
     * and thus implicitly has a reset modifier between calls.
     */
    bool HasImplicitReset() const;


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
     * Set where to write any debug/raw outputs to, if desired.
     *
     * @param pHandler  handler for the output folder
     */
    void SetOutputFolder(boost::shared_ptr<OutputFileHandler> pHandler);

    /**
     * @return the folder to which to write any debug/raw outputs, if it has been set.
     * (If not, an unset FileFinder will be returned.)
     */
    FileFinder GetOutputFolder() const;


    /** Default constructor sets the free variable to an 'unset' value. */
    AbstractSystemWithOutputs();

    /** Virtual destructor since we have virtual methods. */
    virtual ~AbstractSystemWithOutputs();

protected:
    /** Names of system outputs. */
    std::vector<std::string> mOutputNames;

    /** Units of system outputs. */
    std::vector<std::string> mOutputUnits;

    /** Names of system inputs. */
    std::vector<std::string> mInputNames;

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

    /** Where to write any debug/raw output to, if required. */
    boost::shared_ptr<OutputFileHandler> mpOutputHandler;

    /** Whether there is an implicit reset modifier between calls to SolveModel. */
    bool mHasImplicitReset;
};

#endif /*ABSTRACTSYSTEMWITHOUTPUTS_HPP_*/
