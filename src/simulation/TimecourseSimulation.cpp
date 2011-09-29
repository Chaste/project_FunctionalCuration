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


TimecourseSimulation::TimecourseSimulation(boost::shared_ptr<AbstractCardiacCellInterface> pCell,
                                           boost::shared_ptr<AbstractStepper> pStepper,
                                           boost::shared_ptr<ModifierCollection> pModifiers)
    : AbstractSimulation(pCell, pStepper, pModifiers)
{
}


void TimecourseSimulation::Run(AbstractProtocolOutputs& rOutputs)
{
    // Loop over time
    for (mpStepper->Reset(); !mpStepper->AtEnd(); /* step done in loop body */)
    {
        /// \todo  Move modifiers invocation to base class method?
        if (mpModifiers)
        {
            (*mpModifiers)(mpCell, mpStepper);
        }
        // Compute outputs here, so we get the initial state
        rOutputs.AddOutputData(rGetSteppers());
        // Simulate until the next output point, if there is one
        const double curr_time = mpStepper->GetCurrentOutputPoint();
        const double next_time = mpStepper->Step();
        if (!mpStepper->AtEnd())
        {
            mpCell->SolveAndUpdateState(curr_time, next_time);
        }
    }
    if (mpModifiers)
    {
        mpModifiers->ApplyAtEnd(mpCell, mpStepper);
    }
}

//// Alternative implementation with UniformStepper (untested):
//#include <boost/pointer_cast.hpp> // NB: Not available on Boost 1.33.1
//void TimecourseSimulation::Run(AbstractProtocolOutputs& rOutputs)
//{
//    boost::shared_ptr<UniformStepper> p_stepper = boost::dynamic_pointer_cast<UniformStepper>(mpStepper);
//    assert(p_stepper);
//    // Can only apply modifiers at start
//    if (mpModifiers)
//    {
//        (*mpModifiers)(mpCell, mpStepper);
//    }
//    // Solve for whole time period
//    OdeSolution states = p_cell->Compute(p_stepper->GetStartPoint(), p_stepper->GetEndPoint(), p_stepper->GetInterval());
//    // Compute and fill in outputs
//    const unsigned num_points = 1 + states.GetNumberOfTimeSteps();
//    assert(num_points == p_stepper->GetNumberOfOutputPoints());
//    p_stepper->Reset();
//    for (unsigned i=0; i<num_points; ++i)
//    {
//        rOutputs.AddOutputData(rGetSteppers(), states.rGetSolutions()[i]);
//        p_stepper->Step();
//    }
//}
