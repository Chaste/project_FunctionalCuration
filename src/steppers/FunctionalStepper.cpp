/*

Copyright (c) 2005-2012, University of Oxford.
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

#include "FunctionalStepper.hpp"

#include "BacktraceException.hpp"
#include "ProtoHelperMacros.hpp"

FunctionalStepper::FunctionalStepper(const std::string& rName,
                                     const std::string& rUnits,
                                     AbstractExpressionPtr pExpr)
    : AbstractStepper(rName, rUnits),
      mpExpr(pExpr)
{
    SetCurrentOutputPoint(DOUBLE_UNSET); // Done by base class, but here for clarity
}


bool FunctionalStepper::AtEnd()
{
    return false;
}


unsigned FunctionalStepper::GetNumberOfOutputPoints() const
{
    PROTO_EXCEPTION("The number of points in a functionalRange cannot be determined.");
}


bool FunctionalStepper::IsEndFixed() const
{
    return false;
}


void FunctionalStepper::Initialise()
{
    Reset();
}


void FunctionalStepper::Reset()
{
    mCurrentStep = (unsigned)(-1);
    Step();
}


double FunctionalStepper::Step()
{
    EXCEPT_IF_NOT(mpEnvironment);
    AbstractValuePtr p_value = (*mpExpr)(*mpEnvironment);
    PROTO_ASSERT(p_value->IsDouble(), "The functionalRange definition must evaluate to a real number.");
    mCurrentStep++;
    SetCurrentOutputPoint(GET_SIMPLE_VALUE(p_value));
    return GetCurrentOutputPoint();
}
