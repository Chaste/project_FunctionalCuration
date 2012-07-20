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

#include "AbstractTemplatedSystemWithOutputs.hpp"

#include <algorithm>
#include <boost/foreach.hpp>

#include "ModelWrapperEnvironment.hpp"
#include "NullDeleter.hpp"
#include "ValueTypes.hpp"
#include "NdArray.hpp"

#include "AbstractParameterisedSystem.hpp"
#include "AbstractCardiacCellInterface.hpp"
#include "VectorHelperFunctions.hpp"
#include "Warnings.hpp"
#include "Exception.hpp"

#ifdef CHASTE_CVODE
// CVODE headers
#include <nvector/nvector_serial.h>
#endif


template<typename VECTOR>
void AbstractTemplatedSystemWithOutputs<VECTOR>::SolveModel(double endPoint)
{
    dynamic_cast<AbstractCardiacCellInterface*>(this)->SolveAndUpdateState(this->mFreeVariable, endPoint);
    this->mFreeVariable = endPoint;
}


template<typename VECTOR>
void AbstractTemplatedSystemWithOutputs<VECTOR>::ProcessOutputsInfo()
{
    const AbstractUntemplatedParameterisedSystem * const p_this = dynamic_cast<const AbstractUntemplatedParameterisedSystem * const>(this);
    assert(p_this);

    this->mOutputNames.reserve(mOutputsInfo.size() + mVectorOutputsInfo.size());
    this->mOutputUnits.reserve(mOutputsInfo.size() + mVectorOutputsInfo.size());

    // Fill in info for 'normal' outputs (single values per output step)
    for (unsigned i=0; i<mOutputsInfo.size(); i++)
    {
        switch (mOutputsInfo[i].second)
        {
        case FREE:
            this->mOutputNames.push_back(p_this->GetSystemInformation()->GetFreeVariableName());
            this->mOutputUnits.push_back(p_this->GetSystemInformation()->GetFreeVariableUnits());
            break;
        case STATE:
            this->mOutputNames.push_back(p_this->rGetStateVariableNames()[mOutputsInfo[i].first]);
            this->mOutputUnits.push_back(p_this->rGetStateVariableUnits()[mOutputsInfo[i].first]);
            break;
        case PARAMETER:
            this->mOutputNames.push_back(p_this->rGetParameterNames()[mOutputsInfo[i].first]);
            this->mOutputUnits.push_back(p_this->rGetParameterUnits()[mOutputsInfo[i].first]);
            break;
        case DERIVED:
            this->mOutputNames.push_back(p_this->rGetDerivedQuantityNames()[mOutputsInfo[i].first]);
            this->mOutputUnits.push_back(p_this->rGetDerivedQuantityUnits()[mOutputsInfo[i].first]);
            break;
        }
    }

    // Fill in info for vector outputs (vector of variables per output step)
    this->mOutputNames.resize(mOutputsInfo.size() + mVectorOutputsInfo.size());
    this->mOutputUnits.resize(mOutputsInfo.size() + mVectorOutputsInfo.size());
    std::copy(mVectorOutputNames.begin(), mVectorOutputNames.end(),
              this->mOutputNames.begin() + mOutputsInfo.size());
    std::fill_n(this->mOutputUnits.begin() + mOutputsInfo.size(), mVectorOutputsInfo.size(), "unspecified");
}


template<typename VECTOR>
EnvironmentCPtr AbstractTemplatedSystemWithOutputs<VECTOR>::GetOutputs()
{
    AbstractParameterisedSystem<VECTOR>* p_this = dynamic_cast<AbstractParameterisedSystem<VECTOR>*>(this);
    assert(p_this);

    EnvironmentPtr p_outputs(new Environment);

    bool computed_derived_quantities = false;
    VECTOR derived_quantities;
    const std::string loc_info("Model " + p_this->GetSystemName());
    const unsigned num_normal_outputs = mOutputsInfo.size();

    // Add 'normal' outputs to the environment (single values per output step)
    for (unsigned i=0; i<num_normal_outputs; i++)
    {
        double value;
        switch (mOutputsInfo[i].second)
        {
            case FREE:
                value = this->mFreeVariable;
                break;
            case STATE:
                value = GetVectorComponent(p_this->rGetStateVariables(), mOutputsInfo[i].first);
                break;
            case PARAMETER:
                value = p_this->GetParameter(mOutputsInfo[i].first);
                break;
            case DERIVED:
                if (!computed_derived_quantities)
                {
                    derived_quantities = p_this->ComputeDerivedQuantities(this->mFreeVariable,
                                                                          p_this->rGetStateVariables());
                    computed_derived_quantities = true;
                }
                value = GetVectorComponent(derived_quantities, mOutputsInfo[i].first);
                break;
        }
        AbstractValuePtr p_value(new SimpleValue(value));
        p_value->SetUnits(this->mOutputUnits[i]);
        p_outputs->DefineName(this->mOutputNames[i], p_value, loc_info);
    }

    // Add vector outputs to the environment (vector of variables per output step)
    const unsigned num_vector_outputs = mVectorOutputsInfo.size();
    for (unsigned i=0; i<num_vector_outputs; i++)
    {
        const unsigned output_length = mVectorOutputsInfo[i].size();
        const NdArray<double>::Extents shape(1u, output_length);
        NdArray<double> value(shape);
        for (NdArray<double>::Iterator iter = value.Begin(); iter != value.End(); ++iter)
        {
            const unsigned j = iter.rGetIndices()[0];
            switch (mVectorOutputsInfo[i][j].second)
            {
            case FREE:
                *iter = this->mFreeVariable;
                break;
            case STATE:
                *iter = GetVectorComponent(p_this->rGetStateVariables(), mVectorOutputsInfo[i][j].first);
                break;
            case PARAMETER:
                *iter = p_this->GetParameter(mVectorOutputsInfo[i][j].first);
                break;
            case DERIVED:
                if (!computed_derived_quantities)
                {
                    derived_quantities = p_this->ComputeDerivedQuantities(this->mFreeVariable,
                                                                          p_this->rGetStateVariables());
                    computed_derived_quantities = true;
                }
                *iter = GetVectorComponent(derived_quantities, mVectorOutputsInfo[i][j].first);
                break;
            }
        }
        AbstractValuePtr p_value(new ArrayValue(value));
        p_value->SetUnits(this->mOutputUnits[num_normal_outputs + i]);
        p_outputs->DefineName(this->mOutputNames[num_normal_outputs + i], p_value, loc_info);
    }

    if (computed_derived_quantities)
    {
        DeleteVector(derived_quantities);
    }

    return p_outputs;
}


template<typename VECTOR>
void AbstractTemplatedSystemWithOutputs<VECTOR>::SetNamespaceBindings(const std::map<std::string, std::string>& rNamespaceBindings)
{
    ///\todo Go back to the assert when nested protocols are done properly
//    assert(mEnvironmentMap.empty());
    mEnvironmentMap.clear();
    // Create the wrapper Environment(s)
    boost::shared_ptr<AbstractParameterisedSystem<VECTOR> > p_system(dynamic_cast<AbstractParameterisedSystem<VECTOR>*>(this),
                                                                     NullDeleter());
    EnvironmentPtr p_model_env(new ModelWrapperEnvironment<VECTOR>(p_system));
    typedef std::pair<std::string, std::string> StringPair;
    BOOST_FOREACH(StringPair binding, rNamespaceBindings)
    {
        if (binding.second != "https://chaste.comlab.ox.ac.uk/cellml/ns/oxford-metadata#" &&
            binding.second != "http://www.cellml.org/cellml/1.0#" &&
            binding.second != "https://chaste.cs.ox.ac.uk/nss/protocol/0.1#")
        {
            WARNING("This implementation currently only supports using the oxmeta annotations to access model variables."
                    " The namespace '" << binding.second << "' is unsupported.");
        }
        mEnvironmentMap[binding.first] = p_model_env;
    }
}


////////////////////////////////////////////////////////////////////////////////////
// Explicit instantiation
////////////////////////////////////////////////////////////////////////////////////

template class AbstractTemplatedSystemWithOutputs<std::vector<double> >;
#ifdef CHASTE_CVODE
template class AbstractTemplatedSystemWithOutputs<N_Vector>;
#endif
