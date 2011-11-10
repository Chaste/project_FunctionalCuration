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
#include "Environment.hpp"
#include "LocatableConstruct.hpp"

class AbstractSimulation;
typedef boost::shared_ptr<AbstractSimulation> AbstractSimulationPtr;

/**
 * Base class for the simulation execution logic in a protocol.
 */
class AbstractSimulation : public LocatableConstruct, boost::noncopyable
{
public:
    /** For readability - it's a long type! */
    typedef boost::shared_ptr<std::vector<AbstractStepperPtr> > StepperCollection;

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
                       AbstractStepperPtr pStepper,
                       boost::shared_ptr<ModifierCollection> pModifiers=boost::shared_ptr<ModifierCollection>(),
                       StepperCollection pSteppers=StepperCollection());

    /** Virtual destructor. */
    virtual ~AbstractSimulation();

    /**
     * Run a simulation, returning an Environment containing the results.  This is the method
     * external callers should use to run an entire simulation.
     */
    EnvironmentPtr Run();

    /**
     * Run a simulation, filling in the results if requested.
     *
     * This method must be provided by concrete subclasses.  If the supplied pointer is
     * NULL then no results are being stored for this simulation, and the subclass should
     * just run the simulation.  Otherwise, the supplied Environment will have arrays
     * defined for each simulation output, but their contents will be undefined and must
     * be filled in.
     *
     * @param pResults  an Environment containing results arrays to fill in, or an empty pointer
     */
    virtual void Run(EnvironmentPtr pResults)=0;

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

    /** Get method for what #mpSteppers points to. */
    std::vector<AbstractStepperPtr>& rGetSteppers()
    {
        return *mpSteppers;
    }

    /**
     * Set method for #mpCell used by the initial parser implementation.
     *
     * \todo Store AbstractUntemplatedSystemWithOutputs instead?
     *
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
     * Initialise a simulation results environment, creating all the output arrays
     * defined from the model.
     *
     * @param pResults  the results Environment
     */
    void CreateOutputArrays(EnvironmentPtr pResults);

    /**
     * If this simulation is controlled by a while loop, then we might need to resize the
     * output arrays whenever they exceed the current allocation, and shrink them to the
     * final extent of the loop at the end of the simulation.
     *
     * @param pResults  the results Environment
     */
    void ResizeOutputs(EnvironmentPtr pResults);

    /**
     * Create a delegatee of our environment containing views of the simulation results
     * thus far.
     * Should only be called if this simulation is controlled by a while loop and has a
     * results prefix.
     *
     * @param pResults  the Environment containing the full simulation results
     */
    void CreateResultViews(EnvironmentPtr pResults);

    /**
     * Add the model outputs at the current simulation step to the results environment.
     * This method will exit early if we're not storing results for the current
     * simulation - there's no need to test before calling.
     *
     * @param pResults  the results Environment
     */
    void AddOutputData(EnvironmentPtr pResults);

    /**
     * The cell model the protocol is being run on.
     */
    boost::shared_ptr<AbstractCardiacCellInterface> mpCell;

    /**
     * The stepper controlling this simulation's loop.
     */
    AbstractStepperPtr mpStepper;

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
    EnvironmentPtr mpEnvironment;

private:
    /**
     * Add model outputs for the current simulation step.  Called by AddOutputData
     * to do the actual work.
     *
     * @param pResults  the results Environment
     */
    template<typename VECTOR>
    void AddOutputDataTemplated(EnvironmentPtr pResults);

    /** The namespace prefix to use for outputs from this simulation. */
    std::string mOutputsPrefix;
};

#endif /*ABSTRACTSIMULATION_HPP_*/
