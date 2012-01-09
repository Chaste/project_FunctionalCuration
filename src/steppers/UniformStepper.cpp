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

#include "UniformStepper.hpp"

#include <cmath>
#include <boost/assign/list_of.hpp>

#include "BacktraceException.hpp"
#include "ProtoHelperMacros.hpp"

const double SMIDGE = 1e-10;

UniformStepper::UniformStepper(const std::string& rIndexName,
                               const std::string& rIndexUnits,
                               double startPoint,
                               double endPoint,
                               double stepInterval)
    : AbstractStepper(rIndexName, rIndexUnits),
      mStartPoint(startPoint),
      mEndPoint(endPoint),
      mInterval(stepInterval)
{
    CheckParameters();
}


UniformStepper::UniformStepper(const std::string& rIndexName,
                               const std::string& rIndexUnits,
                               AbstractExpressionPtr pStartPoint,
                               AbstractExpressionPtr pEndPoint,
                               AbstractExpressionPtr pStepInterval)
    : AbstractStepper(rIndexName, rIndexUnits),
      mStartPoint(DOUBLE_UNSET),
      mEndPoint(DOUBLE_UNSET),
      mInterval(DOUBLE_UNSET)

{
    mExpressions = boost::assign::list_of(pStartPoint)(pEndPoint)(pStepInterval);
    mNumSteps = (unsigned)(-1); // Unknown as yet
    SetCurrentOutputPoint(mStartPoint);
}


void UniformStepper::Initialise()
{
    if (!mExpressions.empty())
    {
        std::vector<double> params = EvaluateParameters();
        assert(params.size() == 3);
        mStartPoint = params[0];
        mEndPoint = params[1];
        mInterval = params[2];
        CheckParameters();
    }
}


void UniformStepper::CheckParameters()
{
    const double range = mEndPoint - mStartPoint;

    // If end<start, interval better be negative
    PROTO_ASSERT(range * mInterval >= 0,
                 "If and only if endPoint is before startPoint, stepInterval must be negative.");

    mNumSteps = (unsigned) floor(range/mInterval + 0.5);

    // Check interval divides range
    PROTO_ASSERT(fabs(mNumSteps*mInterval - range) <= SMIDGE, "The step interval must divide the range.");

    SetCurrentOutputPoint(mStartPoint);
}


unsigned UniformStepper::GetNumberOfOutputPoints() const
{
    return mNumSteps + 1;
}


bool UniformStepper::IsEndFixed() const
{
    return true;
}


void UniformStepper::Reset()
{
    mCurrentStep = 0;
    SetCurrentOutputPoint(mStartPoint);
}


double UniformStepper::Step()
{
    // Check for overflow?
    SetCurrentOutputPoint(mStartPoint + mInterval * (++mCurrentStep));
    return GetCurrentOutputPoint();
}


double UniformStepper::GetStartPoint() const
{
    return mStartPoint;
}


double UniformStepper::GetEndPoint() const
{
    return mEndPoint;
}


double UniformStepper::GetInterval() const
{
    return mInterval;
}
