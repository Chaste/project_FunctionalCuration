/*

Copyright (c) 2005-2015, University of Oxford.
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

#include "WhileStepper.hpp"

#include "BacktraceException.hpp"
#include "ProtoHelperMacros.hpp"

WhileStepper::WhileStepper(const std::string& rIndexName,
                           const std::string& rIndexUnits,
                           const AbstractExpressionPtr pStoppingCondition)
    : AbstractStepper(rIndexName, rIndexUnits),
      mNumberOfOutputPoints(1000u),
      mpCondition(pStoppingCondition),
      mAtEnd(false)
{
    assert(mpCondition);
    SetCurrentOutputPoint(0.0);
}


void WhileStepper::Initialise()
{
    assert(mpEnvironment);
}


unsigned WhileStepper::GetNumberOfOutputPoints() const
{
    return mNumberOfOutputPoints;
}


bool WhileStepper::IsEndFixed() const
{
    return false;
}


void WhileStepper::Reset()
{
    mCurrentStep = 0;
    mAtEnd = false;
    SetCurrentOutputPoint(0.0);
}


bool WhileStepper::AtEnd()
{
    return mAtEnd;
}


double WhileStepper::Step()
{
    ++mCurrentStep;
    SetCurrentOutputPoint(mCurrentStep);

    // Check the loop condition
    AbstractValuePtr p_result = (*mpCondition)(*mpEnvironment);
    PROTO_ASSERT(p_result->IsDouble(), "A while loop condition must evaluate to a number.");
    mAtEnd = !GET_SIMPLE_VALUE(p_result);
    if (mAtEnd)
    {
        mNumberOfOutputPoints = mCurrentStep;
    }
    else if (mCurrentStep == mNumberOfOutputPoints)
    {
        mNumberOfOutputPoints += 1000u;
    }

    return GetCurrentOutputPoint();
}
