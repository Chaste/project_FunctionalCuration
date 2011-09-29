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

#ifndef ABSTRACTSIMULATION_HPP_
#define ABSTRACTSIMULATION_HPP_

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include "AbstractCardiacCellInterface.hpp"
#include "AbstractStepper.hpp"
#include "ModifierCollection.hpp"
#include "AbstractProtocolOutputs.hpp"
#include "Environment.hpp"
#include "LocatableConstruct.hpp"

/**
 * Base class for the simulation execution logic in a protocol.
 */
class AbstractSimulation : public LocatableConstruct, boost::noncopyable
{
public:
    /** For readability - it's a long type! */
    typedef boost::shared_ptr<std::vector<boost::shared_ptr<AbstractStepper> > > StepperCollection;

    /**
     * Constructor.
     *
     * @param pCell  the cell model the protocol is being run on
     * @param pStepper  controls the iteration around this simulation's loop
     * @param pModifiers  details any modifications to be made to the cell or
     *     simulation parameters as the simulation progresses
     * @param pSteppers  if this is part of a nested simulation, the shared
     *     collection of steppers.  pStepper will be added by this constructor.
     */
    AbstractSimulation(boost::shared_ptr<AbstractCardiacCellInterface> pCell,
                       boost::shared_ptr<AbstractStepper> pStepper,
                       boost::shared_ptr<ModifierCollection> pModifiers=boost::shared_ptr<ModifierCollection>(),
                       StepperCollection pSteppers=StepperCollection());

    /** Virtual destructor. */
    virtual ~AbstractSimulation();

    /**
     * Run a simulation, filling in the given outputs collection.
     *
     * @param rOutputs  will be filled with the simulation outputs.
     */
    virtual void Run(AbstractProtocolOutputs& rOutputs) =0;

    /** Get method for #mpCell. */
    boost::shared_ptr<AbstractCardiacCellInterface> GetCell()
    {
        return mpCell;
    }

    /** Get method for #mpSteppers. */
    StepperCollection GetSteppers()
    {
        return mpSteppers;
    }

    /** Get method for #mpSteppers used by AbstractProtocolOutputs. */
    std::vector<boost::shared_ptr<AbstractStepper> >& rGetSteppers()
    {
        return *mpSteppers;
    }

    /**
     * Set method for #mpCell used by the initial parser implementation.
     * @param pCell  the cell model the protocol is being run on
     */
    virtual void SetCell(boost::shared_ptr<AbstractCardiacCellInterface> pCell)
    {
        mpCell = pCell;
    }

    /** Call Initialise on all the steppers in this simulation. */
    void InitialiseSteppers();

    /** Get the environment in which this simulation is run. */
    Environment& rGetEnvironment();

    /**
     * Set the namespace prefix to use for outputs from this simulation.
     *
     * @param rPrefix
     */
    void SetOutputsPrefix(const std::string& rPrefix);

    /** Get the namespace prefix to use for outputs from this simulation. */
    std::string GetOutputsPrefix() const;

protected:
    /**
     * The cell model the protocol is being run on.
     */
    boost::shared_ptr<AbstractCardiacCellInterface> mpCell;

    /**
     * The stepper controlling this simulation's loop.
     */
    boost::shared_ptr<AbstractStepper> mpStepper;

    /**
     * Collection of modifiers which can alter the cell or simulation settings
     * as the simulation progresses.
     */
    boost::shared_ptr<ModifierCollection> mpModifiers;

    /**
     * All the steppers for an entire nested simulation.  Used to index the
     * AbstractProtocolOutputs object when adding data.
     */
    StepperCollection mpSteppers;

    /** The environment in which this simulation is run. */
    Environment mEnvironment;

private:
    /** The namespace prefix to use for outputs from this simulation. */
    std::string mOutputsPrefix;
};

#endif /*ABSTRACTSIMULATION_HPP_*/
