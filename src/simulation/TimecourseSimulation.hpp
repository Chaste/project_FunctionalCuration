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

#ifndef TIMECOURSESIMULATION_HPP_
#define TIMECOURSESIMULATION_HPP_

#include "AbstractSimulation.hpp"

/**
 * Simulate the cell against time.
 */
class TimecourseSimulation : public AbstractSimulation
{
public:
    /**
     * Constructor.
     *
     * @param pCell  the cell model the protocol is being run on
     * @param pStepper  controls the iteration around this simulation's loop
     * @param pModifiers  details any modifications to be made to the cell or
     *     simulation parameters as the simulation progresses
     */
    TimecourseSimulation(boost::shared_ptr<AbstractCardiacCellInterface> pCell,
                         boost::shared_ptr<AbstractStepper> pStepper,
                         boost::shared_ptr<ModifierCollection> pModifiers=boost::shared_ptr<ModifierCollection>());

protected:
    /**
     * Run a simulation, filling in the results.
     *
     * @param pResults  an Environment containing results arrays to fill in
     */
    void Run(EnvironmentPtr pResults);
};

#endif /*TIMECOURSESIMULATION_HPP_*/
