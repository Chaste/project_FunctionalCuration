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

#ifndef WHILESTEPPER_HPP_
#define WHILESTEPPER_HPP_

#include "AbstractStepper.hpp"

/**
 * A stepper that implements a while loop, continuing to increment the loop counter
 * variable (which starts at 0) until a stopping condition evaluates as true.
 */
class WhileStepper : public AbstractStepper
{
public:
    /**
     * Create a while stepper with the given loop condition.
     *
     * @param rIndexName  the name of the index variable iterated over by this stepper
     * @param rIndexUnits  the units of the index variable
     * @param pStoppingCondition  the loop condition expression
     */
    WhileStepper(const std::string& rIndexName,
                 const std::string& rIndexUnits,
                 const AbstractExpressionPtr pCondition);

    /**
     * Return the number of output points that will be generated by this stepper.
     * This is guaranteed to be at least as large as the current output point, but may
     * not be the final value until AtEnd evaluates as true.
     */
    unsigned GetNumberOfOutputPoints() const;

    /**
     * The end point is not fixed, and hence the number of output points is not known at the start.
     */
    bool IsEndFixed() const;

    /**
     * This merely checks that the stepper's environment has been set with SetEnvironment, since
     * this is the environment in which the loop condition will be evaluated.
     */
    void Initialise();

    /**
     * Reset this stepper to its starting point.
     */
    void Reset();

    /**
     * Test whether this stepper has reached the end, by evaluating the loop condition.
     */
    bool AtEnd();

    /**
     * Increment this stepper to its next output point, and return the corresponding
     * value of the index variable.
     */
    double Step();

private:
    /** The current estimate of the number of output points. */
    unsigned mNumberOfOutputPoints;

    /** The loop condition. */
    AbstractExpressionPtr mpCondition;
};

#endif // WHILESTEPPER_HPP_
