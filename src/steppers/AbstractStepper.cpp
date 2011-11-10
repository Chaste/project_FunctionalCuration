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

#include "AbstractStepper.hpp"
#include "BacktraceException.hpp"
#include "NullDeleter.hpp"
#include "ProtoHelperMacros.hpp"

AbstractStepper::AbstractStepper(const std::string& rIndexName,
                                 const std::string& rIndexUnits)
    : SimpleValue(DOUBLE_UNSET),
      mIndexName(rIndexName),
      mIndexUnits(rIndexUnits),
      mCurrentStep(0u)
{
}


AbstractStepper::~AbstractStepper()
{
}


bool AbstractStepper::AtEnd()
{
    return GetCurrentOutputNumber() == GetNumberOfOutputPoints();
}


std::string AbstractStepper::GetIndexName() const
{
    return mIndexName;
}

std::string AbstractStepper::GetUnits() const
{
    return mIndexUnits;
}


void AbstractStepper::SetEnvironment(EnvironmentPtr pEnv)
{
    mpEnvironment = pEnv;
    AbstractValuePtr p_this(this, NullDeleter());
    pEnv->DefineName(mIndexName, p_this, GetLocationInfo());
}

Environment& AbstractStepper::rGetEnvironment() const
{
    return *mpEnvironment;
}


std::vector<double> AbstractStepper::EvaluateParameters(bool allowArray)
{
    EXCEPT_IF_NOT(mpEnvironment);
    std::vector<double> values;
    for (std::vector<AbstractExpressionPtr>::const_iterator it = mExpressions.begin();
         it != mExpressions.end();
         ++it)
    {
        AbstractValuePtr p_value = (**it)(*mpEnvironment);
        if (allowArray)
        {
            PROTO_ASSERT(p_value->IsArray(), "A stepper parameter must be a real number or array.");
            NdArray<double> array = GET_ARRAY(p_value);
            PROTO_ASSERT(array.GetNumDimensions() <= 1,
                         "An array of stepper parameters must have only 1 dimension.");
            for (NdArray<double>::ConstIterator it=array.Begin(); it!=array.End(); ++it)
            {
                values.push_back(*it);
            }
        }
        else
        {
            PROTO_ASSERT(p_value->IsDouble(), "A stepper parameter must be a real number.");
            values.push_back(GET_SIMPLE_VALUE(p_value));
        }
    }
    return values;
}


unsigned AbstractStepper::GetCurrentOutputNumber() const
{
    return mCurrentStep;
}

void AbstractStepper::SetCurrentOutputPoint(double value)
{
    *mValue.Begin() = value;
}

double AbstractStepper::GetCurrentOutputPoint() const
{
    return GetValue();
}
