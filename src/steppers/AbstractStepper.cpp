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

#include "AbstractStepper.hpp"

#include <cassert>
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
    assert(mpEnvironment);
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
            for (NdArray<double>::ConstIterator array_it=array.Begin(); array_it!=array.End(); ++array_it)
            {
                values.push_back(*array_it);
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
