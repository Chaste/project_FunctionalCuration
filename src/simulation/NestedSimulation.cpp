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

#include "NestedSimulation.hpp"

#include <boost/foreach.hpp>
#include <iostream>
#include <sstream>
#include <cstring> // For memcpy
#include "PetscTools.hpp"
#include "NullDeleter.hpp"
#include "BacktraceException.hpp"
#include "ProtoHelperMacros.hpp"

NestedSimulation::NestedSimulation(AbstractSimulationPtr pNestedSimulation,
                                   AbstractStepperPtr pStepper,
                                   boost::shared_ptr<ModifierCollection> pModifiers)
    : AbstractSimulation(pNestedSimulation->GetModel(), pStepper, pModifiers, pNestedSimulation->GetSteppers()),
      mpNestedSimulation(pNestedSimulation)
{
    mpNestedSimulation->rGetEnvironment().SetDelegateeEnvironment(mpEnvironment->GetAsDelegatee());
}


/**
 * Replicate distributed results arrays so each process has the full data.
 *
 * @param pResults  the results environment
 * @param rLoc  the location information of the caller, for use if an error occurs
 */
void ReplicateResults(EnvironmentPtr pResults, const std::string& rLoc)
{
    BOOST_FOREACH(const std::string& r_output_name, pResults->GetDefinedNames())
    {
        AbstractValuePtr p_output = pResults->Lookup(r_output_name, rLoc);
        PROTO_ASSERT2(p_output->IsArray(), "Model produced non-array output " << r_output_name << ".", rLoc);
        NdArray<double> array = GET_ARRAY(p_output);
        double* p_data = &(*array.Begin());
        double* p_result = new double[array.GetNumElements()];
        int mpi_ret = MPI_Allreduce(p_data, p_result, array.GetNumElements(), MPI_DOUBLE, MPI_SUM, PETSC_COMM_WORLD);
        assert(mpi_ret == MPI_SUCCESS);
        memcpy(p_data, p_result, array.GetNumElements() * sizeof(double));
    }
    // Check for any results sub-environments, and replicate them too, recursively.
    BOOST_FOREACH(const std::string& r_sub_prefix, pResults->rGetSubEnvironmentNames())
    {
        EnvironmentPtr p_sub_results
            = boost::const_pointer_cast<Environment>(pResults->GetDelegateeEnvironment(r_sub_prefix));
        assert(p_sub_results);
        ReplicateResults(p_sub_results, rLoc);
    }
}


void NestedSimulation::Run(EnvironmentPtr pResults)
{
    // The outermost loop checks if we are able to parallelise this nested simulation
    ///\todo #2341 consider checking if we're already split by a test isolating processes
    boost::shared_ptr<NestedSimulation> p_parallel_sim;
    if (mParalleliseLoops && mpStepper->IsEndFixed() && mpStepper == rGetSteppers().front())
    {
        unsigned num_levels = 0u;
        unsigned parallised_level = 0u;
        std::set<std::string> state_names;
        boost::shared_ptr<NestedSimulation> p_sim(this, NullDeleter());
        // Determine the nesting level at which we'll split processing across processes.
        // We split at the innermost allowable level, leading to the smallest possible chunks of work for load balancing.
        while (p_sim)
        {
            ++num_levels;
            if (p_sim->CanParallelise(state_names))
            {
                p_parallel_sim = p_sim;
                parallised_level = num_levels;
            }
            p_sim = boost::dynamic_pointer_cast<NestedSimulation>(p_sim->mpNestedSimulation);
        }
        if (p_parallel_sim)
        {
            // Tell the level which can parallelise how to compute which process does what
            std::vector<unsigned> loop_index_multipliers(parallised_level, 1u);
            for (unsigned i=parallised_level-1; i != 0u; --i)
            {
                loop_index_multipliers[i-1] = loop_index_multipliers[i] * rGetSteppers()[i]->GetNumberOfOutputPoints();
            }
            p_parallel_sim->SetParallelMultipliers(loop_index_multipliers);
            // We need to initialise results arrays with zeros so we can easily replicate results at the end
            while (p_sim)
            {
                p_sim->mpNestedSimulation->mZeroInitialiseArrays = true;
                p_sim = boost::dynamic_pointer_cast<NestedSimulation>(p_sim->mpNestedSimulation);
            }
            // Use process isolation to parallelise in case the model contains communication barriers
            PetscTools::IsolateProcesses();
        }
    }

    for (mpStepper->Reset(); !mpStepper->AtEnd(); mpStepper->Step())
    {
        std::string proc_info;
        if (!mParallelMultipliers.empty())
        {
            // Figure out if we should do this iteration
            unsigned iteration_count = 0u;
            for (unsigned i=0; i<mParallelMultipliers.size(); ++i)
            {
                iteration_count += mParallelMultipliers[i] * rGetSteppers()[i]->GetCurrentOutputNumber();
            }
            if (iteration_count % PetscTools::GetNumProcs() != PetscTools::GetMyRank())
            {
                continue; // Someone else will do it
            }
            std::stringstream proc_info_stream;
            proc_info_stream << " on process " << PetscTools::GetMyRank();
            proc_info = proc_info_stream.str();
        }
        std::cout << "Nested simulation " << mpStepper->GetIndexName() << " step "
                  << mpStepper->GetCurrentOutputNumber() << " value " << mpStepper->GetCurrentOutputPoint()
                  << proc_info << "..." << std::endl;
        LoopBodyStartHook();
        // Run the nested simulation, which will add any outputs produced
        mpNestedSimulation->Run(pResults);
        LoopBodyEndHook();
    }
    LoopEndHook();

    if (p_parallel_sim)
    {
        // Stop isolating processes
        PetscTools::IsolateProcesses(false);
        PetscTools::Barrier("NestedSimulation::Run");
        // Replicate the results so each process has a full set
        if (pResults)
        {
            ReplicateResults(pResults, GetLocationInfo());
        }
    }
}


void NestedSimulation::SetModel(boost::shared_ptr<AbstractSystemWithOutputs> pModel)
{
    AbstractSimulation::SetModel(pModel);
    mpNestedSimulation->SetModel(pModel);
}


void NestedSimulation::SetOutputFolder(boost::shared_ptr<OutputFileHandler> pHandler)
{
    AbstractSimulation::SetOutputFolder(pHandler);
    mpNestedSimulation->SetOutputFolder(pHandler);
}


bool NestedSimulation::CanParallelise(std::set<std::string>& rStatesSaved) const
{
    bool has_reset = mpModel->HasImplicitReset();
    // Check whether our own modifiers make it safe
    if (!has_reset)
    {
        for (unsigned i=0; i<mpModifiers->GetNumModifiers(); ++i)
        {
            AbstractSimulationModifierPtr p_modifier = (*mpModifiers)[i];
            if (p_modifier->IsReset())
            {
                if (p_modifier->GetWhenApplied() == AbstractSimulationModifier::EVERY_LOOP)
                {
                    if (rStatesSaved.find(p_modifier->GetStateName()) == rStatesSaved.end())
                    {
                        has_reset = true;
                    }
                }
            }
            else if (!p_modifier->GetStateName().empty())
            {
                // This is a save state modifier, so record the state name used for when testing inner loops
                // and/or subsequent modifiers in this loop.
                rStatesSaved.insert(p_modifier->GetStateName());
            }
        }
    }
    // Check whether our nested simulation's modifiers make it OK
    if (!has_reset)
    {
        assert(mpNestedSimulation);
        boost::shared_ptr<ModifierCollection> p_inner_modifers = mpNestedSimulation->mpModifiers;
        for (unsigned i=0; i<p_inner_modifers->GetNumModifiers(); ++i)
        {
            AbstractSimulationModifierPtr p_modifier = (*p_inner_modifers)[i];
            if (p_modifier->IsReset())
            {
                if (p_modifier->GetWhenApplied() == AbstractSimulationModifier::AT_START_ONLY
                    || p_modifier->GetWhenApplied() == AbstractSimulationModifier::EVERY_LOOP)
                {
                    if (rStatesSaved.find(p_modifier->GetStateName()) == rStatesSaved.end())
                    {
                        has_reset = true;
                    }
                }
            }
            else if (!p_modifier->GetStateName().empty())
            {
                // This is a save state modifier, so record the state name used for when testing inner loops
                // and/or subsequent modifiers in this loop.
                rStatesSaved.insert(p_modifier->GetStateName());
            }
        }
    }
    return has_reset;
}


void NestedSimulation::SetParallelMultipliers(const std::vector<unsigned>& rMultipliers)
{
    mParallelMultipliers = rMultipliers;
}
