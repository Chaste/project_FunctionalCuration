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

#include "ModelWrapperEnvironment.hpp"

#include <boost/assign/list_inserter.hpp> // for 'push_back()'

#ifdef CHASTE_CVODE
#include <nvector/nvector_serial.h>
#endif // CHASTE_CVODE

#include "BacktraceException.hpp"
#include "ValueTypes.hpp"
#include "ProtoHelperMacros.hpp"

template<typename VECTOR>
ModelWrapperEnvironment<VECTOR>::ModelWrapperEnvironment(boost::shared_ptr<AbstractParameterisedSystem<VECTOR> > pModel)
    : mpModel(pModel)
{}


template<typename VECTOR>
AbstractValuePtr ModelWrapperEnvironment<VECTOR>::Lookup(const std::string& rName, const std::string& rCallerLocation) const
{
    AbstractValuePtr p_result;
    try
    {
        unsigned idx = mpModel->GetAnyVariableIndex(rName);
        double value = mpModel->GetAnyVariable(idx);
        p_result.reset(new SimpleValue(value));
        p_result->SetUnits(mpModel->GetAnyVariableUnits(idx));
    }
    catch (const Exception& e)
    {
    }
    if (!p_result)
    {
        PROTO_EXCEPTION2("Name " << rName << " is not defined in the model.", rCallerLocation);
    }
    return p_result;
}


template<typename VECTOR>
void ModelWrapperEnvironment<VECTOR>::DefineName(const std::string& rName, const AbstractValuePtr pValue,
                                                 const std::string& rCallerLocation)
{
    PROTO_EXCEPTION2("Defining new names in a model is not allowed.", rCallerLocation);
}


template<typename VECTOR>
void ModelWrapperEnvironment<VECTOR>::OverwriteDefinition(const std::string& rName,
                                                          const AbstractValuePtr pValue,
                                                          const std::string& rCallerLocation)
{
    PROTO_ASSERT2(pValue->IsDouble(), "Only real numbers can be assigned to model variables.",
                  rCallerLocation);
    double value = GET_SIMPLE_VALUE(pValue);
    try
    {
        mpModel->SetAnyVariable(rName, value);
    }
    catch (const Exception& e)
    {
        PROTO_EXCEPTION2("Name " << rName << " is not defined in the model.", rCallerLocation);
    }
}


template<typename VECTOR>
unsigned ModelWrapperEnvironment<VECTOR>::GetNumberOfDefinitions() const
{
    return mpModel->GetNumberOfStateVariables()
            + mpModel->GetNumberOfParameters()
            + mpModel->GetNumberOfDerivedQuantities();
}


template<typename VECTOR>
std::vector<std::string> ModelWrapperEnvironment<VECTOR>::GetDefinedNames() const
{
    std::vector<std::string> names;
    boost::assign::push_back(names).range(mpModel->rGetStateVariableNames())
                                   .range(mpModel->rGetParameterNames())
                                   .range(mpModel->rGetDerivedQuantityNames());
    return names;
}



//////////////////////////////////////////////////////////////////////
// Explicit instantiation
//////////////////////////////////////////////////////////////////////

#ifdef CHASTE_CVODE
template class ModelWrapperEnvironment<N_Vector>;
#endif
template class ModelWrapperEnvironment<std::vector<double> >;
