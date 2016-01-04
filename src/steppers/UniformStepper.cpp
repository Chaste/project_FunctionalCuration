/*

Copyright (c) 2005-2016, University of Oxford.
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
