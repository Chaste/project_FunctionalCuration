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

#include "TimecourseSimulation.hpp"

#include "AbstractCardiacCellInterface.hpp"
#include "AbstractParameterisedSystem.hpp"
#include "BacktraceException.hpp"
#include "NdArray.hpp"
#include "ProtoHelperMacros.hpp"

#include "VectorHelperFunctions.hpp"

TimecourseSimulation::TimecourseSimulation(boost::shared_ptr<AbstractUntemplatedSystemWithOutputs> pModel,
                                           boost::shared_ptr<AbstractStepper> pStepper,
                                           boost::shared_ptr<ModifierCollection> pModifiers)
    : AbstractSimulation(pModel, pStepper, pModifiers)
{
}


/**
 * Function called by AddOutputData to do the actual work.
 *
 * @param pResults  the results environment
 * @param pModel  the model being simulated
 * @param pSim  the simulation
 */
template<typename VECTOR>
void AddOutputDataTemplated(EnvironmentPtr pResults,
                            boost::shared_ptr<AbstractSystemWithOutputs<VECTOR> > pModel,
                            AbstractSimulation* pSim)
{
    // Get the model information
    assert(pModel);
    std::vector<boost::shared_ptr<AbstractStepper> >& r_steppers = pSim->rGetSteppers();

    // Figure out which part of the output arrays to fill
    const unsigned nesting_depth = r_steppers.size();
    NdArray<double>::Indices indices(nesting_depth);
    for (unsigned dim=0; dim<nesting_depth; ++dim)
    {
        indices[dim] = r_steppers[dim]->GetCurrentOutputNumber();
    }

    // Fill in outputs
    const std::vector<std::string> output_names = pModel->GetOutputNames();
    const unsigned num_outputs = output_names.size();
    VECTOR outputs = pModel->ComputeOutputs();
    assert(GetVectorSize(outputs) == num_outputs);
    for (unsigned i=0; i<num_outputs; ++i)
    {
        NdArray<double> array = GET_ARRAY(pResults->Lookup(output_names[i], pSim->GetLocationInfo()));
        array[indices] = GetVectorComponent(outputs, i);
    }
    DeleteVector(outputs);

    // Fill in vector outputs
    const std::vector<std::string>& r_vector_output_names = pModel->rGetVectorOutputNames();
    const unsigned num_vector_outputs = r_vector_output_names.size();
    std::vector<VECTOR> vector_outputs = pModel->ComputeVectorOutputs();
    assert(vector_outputs.size() == num_vector_outputs);
    for (unsigned i=0; i<num_vector_outputs; ++i)
    {
        NdArray<double>::Indices our_indices(indices);
        our_indices.push_back(0u);
        NdArray<double> array = GET_ARRAY(pResults->Lookup(r_vector_output_names[i], pSim->GetLocationInfo()));
        // Fill in the vector
        const unsigned vector_length = GetVectorSize(vector_outputs[i]);
        assert(vector_length == array.GetShape().back());
        for (unsigned j=0; j<vector_length; ++j)
        {
            array[our_indices] = GetVectorComponent(vector_outputs[i], j);
            our_indices.back()++;
        }
        // Free memory
        DeleteVector(vector_outputs[i]);
    }
}


/**
 * Add the model outputs at the current simulation step to the results environment.
 * This method will exit early if we're not storing results for the current
 * simulation - there's no need to test before calling.
 *
 * @param pResults  the results environment
 * @param pModel  the model being simulated
 * @param pSim  the simulation
 */
void AddOutputData(EnvironmentPtr pResults,
                   boost::shared_ptr<AbstractUntemplatedSystemWithOutputs> pModel,
                   AbstractSimulation* pSim)
{
    if (!pResults)
    {
        // Not storing output
        return;
    }
#ifdef CHASTE_CVODE
    boost::shared_ptr<AbstractSystemWithOutputs<N_Vector> > p_system
        = boost::dynamic_pointer_cast<AbstractSystemWithOutputs<N_Vector> >(pModel);
    if (p_system)
    {
        AddOutputDataTemplated<N_Vector>(pResults, p_system, pSim);
    }
    else
    {
#endif // CHASTE_CVODE
        boost::shared_ptr<AbstractSystemWithOutputs<std::vector<double> > > p_system2
            = boost::dynamic_pointer_cast<AbstractSystemWithOutputs<std::vector<double> > >(pModel);
        if (p_system2)
        {
            AddOutputDataTemplated<std::vector<double> >(pResults, p_system2, pSim);
        }
        else
        {
            PROTO_EXCEPTION2("Unexpected template parameter for model.", pSim->GetLocationInfo());
        }
#ifdef CHASTE_CVODE
    }
#endif // CHASTE_CVODE
}


void TimecourseSimulation::Run(EnvironmentPtr pResults)
{
    // Loop over time
    mpStepper->Reset();
    mpModel->SetFreeVariable(mpStepper->GetCurrentOutputPoint());
    while (!mpStepper->AtEnd())
    {
        LoopBodyStartHook(pResults);
        // Compute outputs here, so we get the initial state
        AddOutputData(pResults, mpModel, this);
        LoopBodyEndHook(pResults);
        // Simulate until the next output point, if there is one
        const double next_time = mpStepper->Step();
        if (!mpStepper->AtEnd())
        {
            mpModel->SolveModel(next_time);
        }
    }
    LoopEndHook(pResults);
}
