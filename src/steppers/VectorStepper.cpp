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

#include "VectorStepper.hpp"

#include "BacktraceException.hpp"

VectorStepper::VectorStepper(const std::string& rIndexName,
                             const std::string& rIndexUnits,
                             const std::vector<double>& rValues)
    : AbstractStepper(rIndexName, rIndexUnits),
      mValues(rValues)
{
    PROTO_ASSERT(!rValues.empty(), "A VectorStepper must be given a non-empty vector.");
    SetCurrentOutputPoint(mValues.front());
}


VectorStepper::VectorStepper(const std::string& rIndexName,
                             const std::string& rIndexUnits,
                             const std::vector<AbstractExpressionPtr>& rValues)
    : AbstractStepper(rIndexName, rIndexUnits)
{
    PROTO_ASSERT(!rValues.empty(), "A VectorStepper must be given a non-empty vector.");
    mExpressions = rValues;
}


void VectorStepper::Initialise()
{
    if (!mExpressions.empty())
    {
        mValues = EvaluateParameters(true);
        SetCurrentOutputPoint(mValues.front());
    }
}


unsigned VectorStepper::GetNumberOfOutputPoints() const
{
    return mValues.size();
}


bool VectorStepper::IsEndFixed() const
{
    return true;
}


void VectorStepper::Reset()
{
    PROTO_ASSERT(!mValues.empty(), "Un-initialised stepper!");
    mCurrentStep = 0;
    SetCurrentOutputPoint(mValues.front());
}


double VectorStepper::Step()
{
    ++mCurrentStep;
    SetCurrentOutputPoint(mCurrentStep < mValues.size() ? mValues[mCurrentStep] : DOUBLE_UNSET);
    return GetCurrentOutputPoint();
}
