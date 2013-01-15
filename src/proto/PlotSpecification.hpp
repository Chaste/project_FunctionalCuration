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

#ifndef PLOTSPECIFICATION_HPP_
#define PLOTSPECIFICATION_HPP_

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "LocatableConstruct.hpp"

class PlotSpecification;
typedef boost::shared_ptr<PlotSpecification> PlotSpecificationPtr;

/**
 * A minimal description of a graph to plot.
 */
class PlotSpecification : public LocatableConstruct
{
public:
    /**
     * Define a graph plotting two variables against each other.
     * This only really makes sense if both are 1d arrays.
     * @param rTitle  the graph title
     * @param rXVariable  the name of the variable to plot on the x axis
     * @param rYVariable  the name of the variable to plot on the y axis
     */
    PlotSpecification(const std::string& rTitle,
                      const std::string& rXVariable,
                      const std::string& rYVariable);

    /**
     * Define a graph plotting a single variable against the relevant stepper values.
     * The intended result is for multiple traces showing variation with the last dimension.
     * This is suitable e.g. for plotting timecourses of raw model outputs.
     *
     * @param rTitle  the graph title
     * @param rDataVariable  the name of the variable to plot
     */
    PlotSpecification(const std::string& rTitle,
                      const std::string& rDataVariable);

    /** Get the graph title. */
    const std::string& rGetTitle() const;

    /** Get the names of the variables to plot. */
    const std::vector<std::string>& rGetVariableNames() const;

    /** Get the descriptions of the variables to plot - used for axis labels. */
    const std::vector<std::string>& rGetVariableDescriptions() const;

    /** Get the names of the variable units to plot. */
    const std::vector<std::string>& rGetVariableUnits() const;

    /** Get the graph display title. */
    const std::string& rGetDisplayTitle() const;

    /**
     * Set the plot display title.
     * @param rTitle  the new title
     */
    void SetDisplayTitle(const std::string& rTitle);

    /**
     * Store the variable descriptions.
     * @param rVariableDescriptions  the descriptions of the variables for axis labels
     */
    void SetVariableDescriptions(const std::vector<std::string>& rVariableDescriptions);

    /**
     * Store the names of the variable units.
     * @param rVariableUnits  the unit names
     */
    void SetVariableUnits(const std::vector<std::string>& rVariableUnits);

private:
    /** The graph title; also used for the plot file name. */
    std::string mTitle;

    /** The title to actually display on the plot. */
    std::string mDisplayTitle;

    /** The names of the variables to plot. */
    std::vector<std::string> mVariableNames;

    /** The descriptions of the variables to plot. */
    std::vector<std::string> mVariableDescriptions;

    /** The units of the variables to plot. */
    std::vector<std::string> mVariableUnits;
};

#endif // PLOTSPECIFICATION_HPP_
