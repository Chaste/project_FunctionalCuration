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

#include "MultipleStepper.hpp"

#include <cassert>
#include <boost/foreach.hpp>

#include "BacktraceException.hpp"

MultipleStepper::MultipleStepper(const std::vector<AbstractStepperPtr>& rSteppers)
    : AbstractStepper(rSteppers.front()->GetIndexName(), rSteppers.front()->GetUnits()),
      mSteppers(rSteppers)
{
    assert(!mSteppers.empty()); // We'll have seen undefined behaviour already if this fails!
    SetCurrentOutputPoint(mSteppers.front()->GetCurrentOutputPoint());
}


bool MultipleStepper::AtEnd()
{
    return mSteppers.front()->AtEnd();
}


unsigned MultipleStepper::GetNumberOfOutputPoints() const
{
    return mSteppers.front()->GetNumberOfOutputPoints();
}


bool MultipleStepper::IsEndFixed() const
{
    return mSteppers.front()->IsEndFixed();
}


void MultipleStepper::Initialise()
{
    BOOST_FOREACH(AbstractStepperPtr p_stepper, mSteppers)
    {
        p_stepper->Initialise();
    }
}


void MultipleStepper::Reset()
{
    BOOST_FOREACH(AbstractStepperPtr p_stepper, mSteppers)
    {
        p_stepper->Reset();
    }
    mCurrentStep = 0u;
    SetCurrentOutputPoint(mSteppers.front()->GetCurrentOutputPoint());
}

/**
 * Increment the primary stepper to its next output point, and return the corresponding
 * value of the stepper variable.  The value is undefined if AtEnd returns true
 * after this call.
 *
 * Throws if any stepper in the collection has already reached its end.
 */
double MultipleStepper::Step()
{
    BOOST_FOREACH(AbstractStepperPtr p_stepper, mSteppers)
    {
        PROTO_ASSERT(!p_stepper->AtEnd(), "A subsidiary range for this task has been exhausted.");
    }
    BOOST_FOREACH(AbstractStepperPtr p_stepper, mSteppers)
    {
        p_stepper->Step();
    }
    mCurrentStep++;
    SetCurrentOutputPoint(mSteppers.front()->GetCurrentOutputPoint());
    return GetCurrentOutputPoint();
}


void MultipleStepper::SetEnvironment(EnvironmentPtr pEnv)
{
    mpEnvironment = pEnv;
    BOOST_FOREACH(AbstractStepperPtr p_stepper, mSteppers)
    {
        p_stepper->SetEnvironment(pEnv);
    }
}
