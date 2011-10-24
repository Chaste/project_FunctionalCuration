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

#include "AbstractSimulation.hpp"

#include <boost/foreach.hpp>

#include "BacktraceException.hpp"

AbstractSimulation::AbstractSimulation(boost::shared_ptr<AbstractCardiacCellInterface> pCell,
                                       boost::shared_ptr<AbstractStepper> pStepper,
                                       boost::shared_ptr<ModifierCollection> pModifiers,
                                       boost::shared_ptr<std::vector<boost::shared_ptr<AbstractStepper> > > pSteppers)
    : mpCell(pCell),
      mpStepper(pStepper),
      mpModifiers(pModifiers),
      mpSteppers(pSteppers)
{
    if (!mpSteppers)
    {
        // Create the stepper collection vector
        mpSteppers.reset(new std::vector<boost::shared_ptr<AbstractStepper> >);
    }
    PROTO_ASSERT(mpSteppers->empty() || mpSteppers->front()->IsEndFixed(),
                 "A while loop may only be the outermost loop for a simulation.");
    // Add our stepper to the front of the shared collection (innermost is last)
    mpSteppers->insert(mpSteppers->begin(), mpStepper);
    mpStepper->SetEnvironment(mEnvironment);
}


AbstractSimulation::~AbstractSimulation()
{
}


void AbstractSimulation::InitialiseSteppers()
{
    BOOST_FOREACH(boost::shared_ptr<AbstractStepper> p_stepper, *mpSteppers)
    {
        p_stepper->Initialise();
    }
}


Environment& AbstractSimulation::rGetEnvironment()
{
    return mEnvironment;
}


void AbstractSimulation::SetOutputsPrefix(const std::string& rPrefix)
{
    mOutputsPrefix = rPrefix;
    if (!mpStepper->IsEndFixed())
    {
        // Create an environment to contain views of the simulation outputs thus far, for
        // use in the loop condition test if needed.
        EnvironmentPtr p_view_env(new Environment(true/* allow overwrite */));
        mEnvironment.SetDelegateeEnvironment(p_view_env, rPrefix);
    }
}


std::string AbstractSimulation::GetOutputsPrefix() const
{
    return mOutputsPrefix;
}
