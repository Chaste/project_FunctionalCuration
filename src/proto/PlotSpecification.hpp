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

#ifndef PLOTSPECIFICATION_HPP_
#define PLOTSPECIFICATION_HPP_

#include <string>
#include <vector>
#include "LocatableConstruct.hpp"

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

    /**
     * Store the names of the variable units.
     * @param rVariableUnits  the unit names
     */
    void SetVariableUnits(const std::vector<std::string>& rVariableUnits);

    /** Get the names of the variable units to plot. */
    const std::vector<std::string>& rGetVariableUnits() const;

private:
    /** The graph title. */
    std::string mTitle;

    /** The names of the variables to plot. */
    std::vector<std::string> mVariableNames;

    /** The units of the variables to plot. */
    std::vector<std::string> mVariableUnits;
};

#endif // PLOTSPECIFICATION_HPP_
