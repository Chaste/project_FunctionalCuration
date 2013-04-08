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

NestedSimulation::NestedSimulation(boost::shared_ptr<AbstractSimulation> pNestedSimulation,
                                   boost::shared_ptr<AbstractStepper> pStepper,
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
        std::cout << "Nested simulation " << mpStepper->GetIndexName() << " step "
                  << mpStepper->GetCurrentOutputNumber() << " value " << mpStepper->GetCurrentOutputPoint()
                  << "..." << std::endl;
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
