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

#include "ModelWrapperEnvironment.hpp"

#include <boost/assign/list_inserter.hpp> // for 'push_back()'
#include <boost/pointer_cast.hpp>

#ifdef CHASTE_CVODE
#include <nvector/nvector_serial.h>
#endif // CHASTE_CVODE

#include "BacktraceException.hpp"
#include "ValueTypes.hpp"
#include "ProtoHelperMacros.hpp"
#include "AbstractSystemWithOutputs.hpp"

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
        double free_var = boost::dynamic_pointer_cast<AbstractUntemplatedSystemWithOutputs>(mpModel)->GetFreeVariable();
        double value = DOUBLE_UNSET;
        std::string units;
        if (rName == mpModel->GetSystemInformation()->GetFreeVariableName())
        {
            value = free_var;
            units = mpModel->GetSystemInformation()->GetFreeVariableUnits();
        }
        else
        {
            unsigned idx = mpModel->GetAnyVariableIndex(rName);
            value = mpModel->GetAnyVariable(idx, free_var);
            units = mpModel->GetAnyVariableUnits(idx);
        }
        p_result.reset(new SimpleValue(value));
        p_result->SetUnits(units);
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
        if (rName == mpModel->GetSystemInformation()->GetFreeVariableName())
        {
            boost::dynamic_pointer_cast<AbstractUntemplatedSystemWithOutputs>(mpModel)->SetFreeVariable(value);
        }
        else
        {
            mpModel->SetAnyVariable(rName, value);
        }
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
            + mpModel->GetNumberOfDerivedQuantities()
            + 1 /* free variable */;
}


template<typename VECTOR>
std::vector<std::string> ModelWrapperEnvironment<VECTOR>::GetDefinedNames() const
{
    std::vector<std::string> names;
    boost::assign::push_back(names).range(mpModel->rGetStateVariableNames())
                                   .range(mpModel->rGetParameterNames())
                                   .range(mpModel->rGetDerivedQuantityNames());
    names.push_back(mpModel->GetSystemInformation()->GetFreeVariableName());
    return names;
}



//////////////////////////////////////////////////////////////////////
// Explicit instantiation
//////////////////////////////////////////////////////////////////////

#ifdef CHASTE_CVODE
template class ModelWrapperEnvironment<N_Vector>;
#endif
template class ModelWrapperEnvironment<std::vector<double> >;
