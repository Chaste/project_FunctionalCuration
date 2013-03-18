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

#include "PlotSpecification.hpp"

#include <boost/assign/list_of.hpp>

PlotSpecification::PlotSpecification(const std::string& rTitle,
                                     const std::string& rXVariable,
                                     const std::string& rYVariable)
    : mTitle(rTitle),
      mDisplayTitle(rTitle)
{
    mVariableNames = boost::assign::list_of(rXVariable)(rYVariable);
}

PlotSpecification::PlotSpecification(const std::string& rTitle,
                                     const std::string& rDataVariable)
    : mTitle(rTitle),
      mDisplayTitle(rTitle),
      mVariableNames(1, rDataVariable)
{}

const std::string& PlotSpecification::rGetTitle() const
{
    return mTitle;
}

const std::vector<std::string>& PlotSpecification::rGetVariableNames() const
{
    return mVariableNames;
}

void PlotSpecification::SetVariableDescriptions(const std::vector<std::string>& rVariableDescriptions)
{
    mVariableDescriptions = rVariableDescriptions;
}

const std::vector<std::string>& PlotSpecification::rGetVariableDescriptions() const
{
    return mVariableDescriptions;
}

void PlotSpecification::SetVariableUnits(const std::vector<std::string>& rVariableUnits)
{
    mVariableUnits = rVariableUnits;
}

const std::vector<std::string>& PlotSpecification::rGetVariableUnits() const
{
    return mVariableUnits;
}

void PlotSpecification::SetDisplayTitle(const std::string& rTitle)
{
    mDisplayTitle = rTitle;
}

const std::string& PlotSpecification::rGetDisplayTitle() const
{
    return mDisplayTitle;
}

const std::string& PlotSpecification::rGetKeyVariableName() const
{
    return mKeyVariableName;
}

void PlotSpecification::SetKeyVariableName(const std::string& rName)
{
    mKeyVariableName = rName;
}

const std::string& PlotSpecification::rGetGnuplotTerminal() const
{
    return mGnuplotTerminal;
}

void PlotSpecification::SetGnuplotTerminal(const std::string& rTerminal)
{
    mGnuplotTerminal = rTerminal;
}

const std::vector<std::string>& PlotSpecification::rGetGnuplotExtraCommands() const
{
    return mGnuplotExtraSettings;
}

const std::string& PlotSpecification::rGetStyle() const
{
    return mStyle;
}

void PlotSpecification::SetGnuplotExtraCommands(const std::vector<std::string>& rCommands)
{
    mGnuplotExtraSettings = rCommands;
}

void PlotSpecification::SetStyle(const std::string& rStyle)
{
    mStyle = rStyle;
}
