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

#include "CombinedSimulation.hpp"

#include <string>
#include <iostream>
#include <boost/foreach.hpp>

CombinedSimulation::CombinedSimulation(const std::vector<AbstractSimulationPtr>& rChildSims,
                                       Scheduling scheduling)
    : AbstractSimulation(boost::shared_ptr<AbstractSystemWithOutputs>(), AbstractStepperPtr()),
      mChildSims(rChildSims),
      mScheduling(scheduling)
{
    BOOST_FOREACH(AbstractSimulationPtr p_child_sim, mChildSims)
    {
        // Make the child's main environment delegate to ours
        p_child_sim->rGetEnvironment().SetDelegateeEnvironment(this->mpEnvironment, "");
        // Add child's results as a sub-environment of ours
        std::string child_prefix = p_child_sim->GetOutputsPrefix();
        if (!child_prefix.empty())
        {
            GetResultsEnvironment()->AddSubEnvironment(p_child_sim->GetResultsEnvironment(), child_prefix);
        }
    }
}


void CombinedSimulation::Run(EnvironmentPtr pResults)
{
    // Since we have no parallelisation yet, we execute in forward order for sequential,
    // and reverse order for parallel to catch likely bugs!
    if (mScheduling == SEQUENTIAL)
    {
        BOOST_FOREACH(AbstractSimulationPtr p_child_sim, mChildSims)
        {
            std::cout << "Running child simulation " << p_child_sim->GetOutputsPrefix() << "..." << std::endl;
            p_child_sim->GetResultsEnvironment()->Clear();
            p_child_sim->InitialiseSteppers();
            p_child_sim->Run();
        }
    }
    else
    {
        BOOST_REVERSE_FOREACH(AbstractSimulationPtr p_child_sim, mChildSims)
        {
            std::cout << "Running child simulation " << p_child_sim->GetOutputsPrefix() << "..." << std::endl;
            p_child_sim->GetResultsEnvironment()->Clear();
            p_child_sim->InitialiseSteppers();
            p_child_sim->Run();
        }
    }
    AddIterationOutputs(pResults, GetResultsEnvironment());
}


void CombinedSimulation::SetModel(boost::shared_ptr<AbstractSystemWithOutputs> pModel)
{
    AbstractSimulation::SetModel(pModel);
    BOOST_FOREACH(AbstractSimulationPtr p_child_sim, mChildSims)
    {
        p_child_sim->SetModel(pModel);
    }
}


void CombinedSimulation::SetOutputFolder(boost::shared_ptr<OutputFileHandler> pHandler)
{
    AbstractSimulation::SetOutputFolder(pHandler);
    BOOST_FOREACH(AbstractSimulationPtr p_child_sim, mChildSims)
    {
        p_child_sim->SetOutputFolder(pHandler);
    }
}
