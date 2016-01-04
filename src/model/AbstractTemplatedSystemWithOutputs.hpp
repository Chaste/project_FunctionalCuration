/*

Copyright (c) 2005-2016, University of Oxford.
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

#ifndef ABSTRACTTEMPLATEDSYSTEMWITHOUTPUTS_HPP_
#define ABSTRACTTEMPLATEDSYSTEMWITHOUTPUTS_HPP_

#include "AbstractSystemWithOutputs.hpp"

#include <vector>
#include <map>
#include <string>

#include "Environment.hpp"

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

    /**
     * Solve the system from the current state up to the given end point.
     */
    void SolveModel(double endPoint);

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

#endif // ABSTRACTTEMPLATEDSYSTEMWITHOUTPUTS_HPP_
