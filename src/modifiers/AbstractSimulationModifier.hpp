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

#ifndef ABSTRACTSIMULATIONMODIFIER_HPP_
#define ABSTRACTSIMULATIONMODIFIER_HPP_

#include <boost/shared_ptr.hpp>

#include "AbstractStepper.hpp"
#include "AbstractCardiacCellInterface.hpp"
#include "LocatableConstruct.hpp"

/**
 * Base class for modifications that can be made to a cell model or simulation
 * environment as the simulation progresses.
 */
class AbstractSimulationModifier : public LocatableConstruct
{
public:
    /**
     * Specification of when to apply this modification.
     */
    enum ApplyWhen {
        AT_START_ONLY, /**< Apply just at the start of the loop */
        EVERY_LOOP,    /**< Apply at the beginning of each time round the loop */
        AT_END         /**< Only after the final iteration round the loop */
    };

    /**
     * Create a modifier.
     *
     * @param when  when this modifier should be applied
     */
    AbstractSimulationModifier(ApplyWhen when);

    /** Virtual destructor. */
    virtual ~AbstractSimulationModifier();

    /**
     * Apply this modification to the cell model.
     *
     * This method is called by the simulation engine, checks the stepper's state against
     * our #mWhen specification, and calls ReallyApply if the conditions match.  It may
     * be overridden by subclasses if they need to do something completely different.
     *
     * @param pCell  the cell to modify
     * @param pStepper  controls the current loop of the simulation
     */
    virtual void operator()(boost::shared_ptr<AbstractCardiacCellInterface> pCell,
                            boost::shared_ptr<AbstractStepper> pStepper);

    /**
     * Apply this modification to the cell model if it takes effect at the end of a loop.
     *
     * @param pCell  the cell to modify
     * @param pStepper  controls the current loop of the simulation
     */
    virtual void ApplyAtEnd(boost::shared_ptr<AbstractCardiacCellInterface> pCell,
                            boost::shared_ptr<AbstractStepper> pStepper);

protected:
    /**
     * Method subclasses must supply which really implements the modification.
     *
     * @param pCell  the cell to modify
     * @param pStepper  controls the current loop of the simulation
     */
    virtual void ReallyApply(boost::shared_ptr<AbstractCardiacCellInterface> pCell,
                             boost::shared_ptr<AbstractStepper> pStepper) =0;

    /** When to apply this modification. */
    ApplyWhen mWhen;
};


#endif // ABSTRACTSIMULATIONMODIFIER_HPP_
