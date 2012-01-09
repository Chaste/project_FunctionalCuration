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

#ifndef UNIFORMSTEPPER_HPP_
#define UNIFORMSTEPPER_HPP_

#include <string>

#include "AbstractStepper.hpp"

/**
 * A simulation protocol stepper that generates a uniform spread of output points
 * over a defined range.
 */
class UniformStepper : public AbstractStepper
{
public:
    /**
     * Create a uniform stepper.
     *
     * Note that both start and end points will be included in the range, and that
     * stepInterval must divide endPoint - startPoint.
     *
     * @param rIndexName  the name of the index variable iterated over by this stepper
     * @param rIndexUnits  the units of the index variable
     * @param startPoint  the start of the range to iterate over
     * @param endPoint  the end of the range to iterate over
     * @param stepInterval  the size of the steps the range is divided into
     */
    UniformStepper(const std::string& rIndexName,
                   const std::string& rIndexUnits,
                   double startPoint,
                   double endPoint,
                   double stepInterval);

    /**
     * Create a uniform stepper.
     *
     * Note that both start and end points will be included in the range, and that
     * stepInterval must divide endPoint - startPoint.
     *
     * @param rIndexName  the name of the index variable iterated over by this stepper
     * @param rIndexUnits  the units of the index variable
     * @param pStartPoint  the start of the range to iterate over
     * @param pEndPoint  the end of the range to iterate over
     * @param pStepInterval  the size of the steps the range is divided into
     */
    UniformStepper(const std::string& rIndexName,
                   const std::string& rIndexUnits,
                   AbstractExpressionPtr pStartPoint,
                   AbstractExpressionPtr pEndPoint,
                   AbstractExpressionPtr pStepInterval);

    /**
     * Return the number of output points that will be generated by this stepper.
     * This allows us to create the correct size of output array before filling it
     * with data.
     */
    unsigned GetNumberOfOutputPoints() const;

    /**
     * The end point is fixed, and hence the number of output points is known at the start.
     */
    bool IsEndFixed() const;

    /**
     * If this stepper was constructed with expressions rather than values, evaluate the
     * expressions in the stepper's environment (which must have been set with SetEnvironment)
     * to convert them to the parameters for this stepper.
     */
    void Initialise();

    /**
     * Reset this stepper to its starting point.
     */
    void Reset();

    /**
     * Increment this stepper to its next output point, and return the corresponding
     * value of the index variable.
     */
    double Step();

    /** Get method for #mStartPoint */
    double GetStartPoint() const;

    /** Get method for #mEndPoint */
    double GetEndPoint() const;

    /** Get method for #mInterval */
    double GetInterval() const;

private:
    /** The start of the range to iterate over. */
    double mStartPoint;

    /** The end of the range to iterate over. */
    double mEndPoint;

    /** The size of the steps the range is divided into. */
    double mInterval;

    /** How many steps there are in total. */
    unsigned mNumSteps;

    /**
     * Check that the stepper's parameters are valid.
     */
    void CheckParameters();
};

#endif // UNIFORMSTEPPER_HPP_
