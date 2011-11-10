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

#include "TimecourseSimulation.hpp"

#include "AbstractSystemWithOutputs.hpp"
#include "AbstractParameterisedSystem.hpp"
#include "BacktraceException.hpp"
#include "NdArray.hpp"
#include "ProtoHelperMacros.hpp"

#include "VectorHelperFunctions.hpp"

TimecourseSimulation::TimecourseSimulation(boost::shared_ptr<AbstractCardiacCellInterface> pCell,
                                           boost::shared_ptr<AbstractStepper> pStepper,
                                           boost::shared_ptr<ModifierCollection> pModifiers)
    : AbstractSimulation(pCell, pStepper, pModifiers)
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
    const VECTOR& r_state = boost::dynamic_pointer_cast<AbstractParameterisedSystem<VECTOR> >(pModel)->rGetStateVariables();
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
    const double time = r_steppers.back()->GetCurrentOutputPoint();
    VECTOR outputs = pModel->ComputeOutputs(time, r_state);
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
    std::vector<VECTOR> vector_outputs = pModel->ComputeVectorOutputs(time, r_state);
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
    }
}


/**
 * Add the model outputs at the current simulation step to the results environment.
 * This method will exit early if we're not storing results for the current
 * simulation - there's no need to test before calling.
 *
 * @param pResults  the results environment
 * @param pCell  the model being simulated
 * @param pSim  the simulation
 */
void AddOutputData(EnvironmentPtr pResults,
                   boost::shared_ptr<AbstractCardiacCellInterface> pCell,
                   AbstractSimulation* pSim)
{
    if (!pResults)
    {
        // Not storing output
        return;
    }
    boost::shared_ptr<AbstractSystemWithOutputs<N_Vector> > p_system
        = boost::dynamic_pointer_cast<AbstractSystemWithOutputs<N_Vector> >(pCell);
    if (p_system)
    {
        AddOutputDataTemplated<N_Vector>(pResults, p_system, pSim);
    }
    else
    {
        boost::shared_ptr<AbstractSystemWithOutputs<std::vector<double> > > p_system2
            = boost::dynamic_pointer_cast<AbstractSystemWithOutputs<std::vector<double> > >(pCell);
        if (p_system2)
        {
            AddOutputDataTemplated<std::vector<double> >(pResults, p_system2, pSim);
        }
        else
        {
            PROTO_EXCEPTION2("Unexpected template parameter for model.", pSim->GetLocationInfo());
        }
    }
}


void TimecourseSimulation::Run(EnvironmentPtr pResults)
{
    // Loop over time
    for (mpStepper->Reset(); !mpStepper->AtEnd(); /* step done in loop body */)
    {
        LoopBodyStartHook(pResults);
        // Compute outputs here, so we get the initial state
        AddOutputData(pResults, mpCell, this);
        LoopBodyEndHook(pResults);
        // Simulate until the next output point, if there is one
        const double curr_time = mpStepper->GetCurrentOutputPoint();
        const double next_time = mpStepper->Step();
        if (!mpStepper->AtEnd())
        {
            mpCell->SolveAndUpdateState(curr_time, next_time);
        }
    }
    LoopEndHook(pResults);
}
