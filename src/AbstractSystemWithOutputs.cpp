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

#include "AbstractSystemWithOutputs.hpp"

#include <boost/foreach.hpp>

#include "ModelWrapperEnvironment.hpp"
#include "NullDeleter.hpp"

#include "AbstractParameterisedSystem.hpp"
#include "AbstractCardiacCell.hpp"
#include "AbstractCvodeCell.hpp"
#include "VectorHelperFunctions.hpp"
#include "Exception.hpp"
#include "Warnings.hpp"

#ifdef CHASTE_CVODE
// CVODE headers
#include <nvector/nvector_serial.h>
#endif


unsigned AbstractUntemplatedSystemWithOutputs::GetNumberOfOutputs() const
{
    return mOutputsInfo.size();
}


std::vector<unsigned> AbstractUntemplatedSystemWithOutputs::GetVectorOutputLengths() const
{
    std::vector<unsigned> lengths(mVectorOutputsInfo.size());
    for (unsigned i=0; i<lengths.size(); ++i)
    {
        lengths[i] = mVectorOutputsInfo[i].size();
    }
    return lengths;
}


const std::vector<std::string>& AbstractUntemplatedSystemWithOutputs::rGetVectorOutputNames() const
{
    return mVectorOutputNames;
}


std::vector<std::string> AbstractUntemplatedSystemWithOutputs::GetOutputNames() const
{
    const AbstractUntemplatedParameterisedSystem * const p_this = dynamic_cast<const AbstractUntemplatedParameterisedSystem * const>(this);
    assert(p_this);

    std::vector<std::string> names;
    names.reserve(GetNumberOfOutputs());
    for (unsigned i=0; i<GetNumberOfOutputs(); i++)
    {
        switch (mOutputsInfo[i].second)
        {
        case FREE:
            // Special-case for the free variable, assuming it is time
            names.push_back("time");
            break;
        case STATE:
            names.push_back(p_this->rGetStateVariableNames()[mOutputsInfo[i].first]);
            break;
        case PARAMETER:
            names.push_back(p_this->rGetParameterNames()[mOutputsInfo[i].first]);
            break;
        case DERIVED:
            names.push_back(p_this->rGetDerivedQuantityNames()[mOutputsInfo[i].first]);
            break;
        }
    }
    return names;
}


std::vector<std::string> AbstractUntemplatedSystemWithOutputs::GetOutputUnits() const
{
    const AbstractUntemplatedParameterisedSystem * const p_this = dynamic_cast<const AbstractUntemplatedParameterisedSystem * const>(this);

    std::vector<std::string> units;
    units.reserve(GetNumberOfOutputs());
    for (unsigned i=0; i<GetNumberOfOutputs(); i++)
    {
        switch (mOutputsInfo[i].second)
        {
        case FREE:
            // Special-case for the free variable, assuming it is time (which is always ms in Chaste)
            units.push_back("milliseconds");
            break;
        case STATE:
            units.push_back(p_this->rGetStateVariableUnits()[mOutputsInfo[i].first]);
            break;
        case PARAMETER:
            units.push_back(p_this->rGetParameterUnits()[mOutputsInfo[i].first]);
            break;
        case DERIVED:
            units.push_back(p_this->rGetDerivedQuantityUnits()[mOutputsInfo[i].first]);
            break;
        }
    }
    return units;
}


unsigned AbstractUntemplatedSystemWithOutputs::GetOutputIndex(const std::string& rName) const
{
    const AbstractUntemplatedParameterisedSystem * const p_this = dynamic_cast<const AbstractUntemplatedParameterisedSystem * const>(this);

    unsigned index = UNSIGNED_UNSET;

    // Check each output in turn
    for (unsigned i=0; i<GetNumberOfOutputs(); i++)
    {
        switch (mOutputsInfo[i].second)
        {
        case FREE:
            // Special-case for the free variable.  There's probably a better way to do this, as the model might call it something else...
            if (rName == "time")
            {
                index = i;
            }
            break;
        case STATE:
            if (p_this->rGetStateVariableNames()[mOutputsInfo[i].first] == rName)
            {
                index = i;
            }
            break;
        case PARAMETER:
            if (p_this->rGetParameterNames()[mOutputsInfo[i].first] == rName)
            {
                index = i;
            }
            break;
        case DERIVED:
            if (p_this->rGetDerivedQuantityNames()[mOutputsInfo[i].first] == rName)
            {
                index = i;
            }
            break;
        }
        if (index != UNSIGNED_UNSET)
        {
            break; // Found it, so stop searching
        }
    }

    if (index == UNSIGNED_UNSET)
    {
        EXCEPTION("No output named '" + rName + "'.");
    }
    return index;
}


const std::map<std::string, EnvironmentPtr>& AbstractUntemplatedSystemWithOutputs::rGetEnvironmentMap() const
{
    return mEnvironmentMap;
}


const std::string& AbstractUntemplatedSystemWithOutputs::rGetShortName(const std::string& rVariableReference) const
{
    std::map<std::string, std::string>::const_iterator it = mNameMap.find(rVariableReference);
    if (it == mNameMap.end())
    {
        EXCEPTION("Variable reference '" << rVariableReference << "' not found in this model.");
    }
    return it->second;
}


AbstractUntemplatedSystemWithOutputs::~AbstractUntemplatedSystemWithOutputs()
{
}


//////////////////////////////////////////////////////////////////////
//
// The templated sub-class with compute methods, and helper functions
//
//////////////////////////////////////////////////////////////////////


/**
 * Helper function to create a new vector with given size.  All entries
 * will be initialised to zero.
 *
 * This isn't a member so that we can specialise it without having to
 * specialise the whole class.
 *
 * @param rVec  the vector to create
 * @param size  the size of the vector
 */
template<typename VECTOR>
inline void CreateNewVector(VECTOR& rVec, unsigned size);


/**
 * Specialisation for std::vector<double>.
 * @param rVec
 * @param size
 */
template<>
inline void CreateNewVector(std::vector<double>& rVec, unsigned size)
{
    rVec.resize(size);
}


#ifdef CHASTE_CVODE
/**
 * Specialisation for N_Vector.
 * @param rVec
 * @param size
 */
template<>
inline void CreateNewVector(N_Vector& rVec, unsigned size)
{
    rVec = N_VNew_Serial(size);
    for (unsigned i=0; i<size; i++)
    {
        NV_Ith_S(rVec, i) = 0.0;
    }
}
#endif // CHASTE_CVODE


template<typename VECTOR>
VECTOR AbstractSystemWithOutputs<VECTOR>::ComputeOutputs(
        double time,
        const VECTOR& rState)
{
    AbstractParameterisedSystem<VECTOR>* p_this = dynamic_cast<AbstractParameterisedSystem<VECTOR>*>(this);
    assert(p_this);

    VECTOR outputs;
    CreateNewVector(outputs, GetNumberOfOutputs());

    bool computed_derived_quantities = false;
    VECTOR derived_quantities;

    for (unsigned i=0; i<GetNumberOfOutputs(); i++)
    {
        switch (mOutputsInfo[i].second)
        {
        case FREE:
            // Special-case for the free variable, assuming it is time
            SetVectorComponent(outputs, i, time);
            break;
        case STATE:
            SetVectorComponent(outputs, i, GetVectorComponent(rState, mOutputsInfo[i].first));
            break;
        case PARAMETER:
            SetVectorComponent(outputs, i, p_this->GetParameter(mOutputsInfo[i].first));
            break;
        case DERIVED:
            if (!computed_derived_quantities)
            {
                derived_quantities = p_this->ComputeDerivedQuantities(time, rState);
                computed_derived_quantities = true;
            }
            SetVectorComponent(outputs, i, GetVectorComponent(derived_quantities, mOutputsInfo[i].first));
            break;
        }
    }

    if (computed_derived_quantities)
    {
        DeleteVector(derived_quantities);
    }

    return outputs;
}


template<typename VECTOR>
std::vector<VECTOR> AbstractSystemWithOutputs<VECTOR>::ComputeVectorOutputs(double time, const VECTOR& rState)
{
    AbstractParameterisedSystem<VECTOR>* p_this = dynamic_cast<AbstractParameterisedSystem<VECTOR>*>(this);
    assert(p_this);

    bool computed_derived_quantities = false;
    VECTOR derived_quantities;

    const unsigned num_vector_outputs = mVectorOutputsInfo.size();
    std::vector<VECTOR> outputs(num_vector_outputs);
    for (unsigned i=0; i<num_vector_outputs; i++)
    {
        const unsigned output_length = mVectorOutputsInfo[i].size();
        VECTOR& r_output = outputs[i];
        CreateNewVector(r_output, output_length);
        for (unsigned j=0; j<output_length; j++)
        {
            switch (mVectorOutputsInfo[i][j].second)
            {
            case FREE:
                // Special-case for the free variable, assuming it is time
                SetVectorComponent(r_output, j, time);
                break;
            case STATE:
                SetVectorComponent(r_output, j, GetVectorComponent(rState, mVectorOutputsInfo[i][j].first));
                break;
            case PARAMETER:
                SetVectorComponent(r_output, j, p_this->GetParameter(mVectorOutputsInfo[i][j].first));
                break;
            case DERIVED:
                if (!computed_derived_quantities)
                {
                    derived_quantities = p_this->ComputeDerivedQuantities(time, rState);
                    computed_derived_quantities = true;
                }
                SetVectorComponent(r_output, j, GetVectorComponent(derived_quantities, mVectorOutputsInfo[i][j].first));
                break;
            }
        }
    }

    if (computed_derived_quantities)
    {
        DeleteVector(derived_quantities);
    }

    return outputs;
}


template<typename VECTOR>
void AbstractSystemWithOutputs<VECTOR>::SetNamespaceBindings(const std::map<std::string, std::string>& rNamespaceBindings)
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
            binding.second != "http://www.cellml.org/cellml/1.0#")
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

template class AbstractSystemWithOutputs<std::vector<double> >;
#ifdef CHASTE_CVODE
template class AbstractSystemWithOutputs<N_Vector>;
#endif
