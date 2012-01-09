/*

Copyright (C) University of Oxford, 2005-2012

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
