/*

Copyright (c) 2005-2014, University of Oxford.
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
#include <boost/make_shared.hpp>
#include <iostream>
#include <sstream>
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


void NestedSimulation::Run(EnvironmentPtr pResults)
{
    for (mpStepper->Reset(); !mpStepper->AtEnd(); mpStepper->Step())
    {
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
        }
        std::cout << "Nested simulation " << mpStepper->GetIndexName() << " step "
                  << mpStepper->GetCurrentOutputNumber() << " (value " << mpStepper->GetCurrentOutputPoint()
                  << ") on process " << PetscTools::GetMyRank() << "..." << std::endl;
        if (GetTrace() && mpOutputHandler)
        {
            // Set a run-specific subfolder for the nested simulation to save debug results in
            std::stringstream run_dir;
            run_dir << "run_" << mpStepper->GetCurrentOutputNumber();
            boost::shared_ptr<OutputFileHandler> p_this_run = boost::make_shared<OutputFileHandler>(mpOutputHandler->FindFile(run_dir.str()), false);
            mpNestedSimulation->SetOutputFolder(p_this_run);
            // Get it to trace too, for saner semantics
            mpNestedSimulation->SetTrace();
        }
        LoopBodyStartHook();
        // Run the nested simulation, which will add any outputs produced
        mpNestedSimulation->Run(pResults);
        LoopBodyEndHook();
    }
    LoopEndHook();
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


bool NestedSimulation::CanParallelise()
{
    // The outermost loop checks if we are able to parallelise this nested simulation
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
            if (p_sim->CanParalleliseHere(state_names))
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
        }
    }
    return p_parallel_sim;
}


bool NestedSimulation::CanParalleliseHere(std::set<std::string>& rStatesSaved) const
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


void NestedSimulation::ZeroInitialiseResults()
{
    AbstractSimulation::ZeroInitialiseResults();
    mpNestedSimulation->ZeroInitialiseResults();
}
