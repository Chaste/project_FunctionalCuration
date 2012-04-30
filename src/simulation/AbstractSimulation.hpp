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
#include "OutputFileHandler.hpp"
#include "FileFinder.hpp"

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

    /**
     * Set where to write any debug tracing to, if desired.
     *
     * @param pHandler  handler for the output folder
     */
    virtual void SetOutputFolder(boost::shared_ptr<OutputFileHandler> pHandler);

    /**
     * Get the folder to which to write any debug tracing, if it has been set.
     * (If not, an unset FileFinder will be returned.)
     */
    FileFinder GetOutputFolder() const;

protected:
    /**
     * This method must be called by subclasses as the first thing within their simulation loop.
     *
     * @param pResults  the results Environment
     */
    void LoopBodyStartHook(EnvironmentPtr pResults);

    /**
     * This method must be called by subclasses as the last thing within their simulation loop,
     * just before their stepper is incremented.
     *
     * @param pResults  the results Environment
     */
    void LoopBodyEndHook(EnvironmentPtr pResults);

    /**
     * This method must be called by subclasses after their simulation loop has completed.
     *
     * @param pResults  the results Environment
     */
    void LoopEndHook(EnvironmentPtr pResults);

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

    /** Where to write tracing/debug output, if desired. */
    boost::shared_ptr<OutputFileHandler> mpOutputHandler;

private:
    /** The namespace prefix to use for outputs from this simulation. */
    std::string mOutputsPrefix;
};

#endif /*ABSTRACTSIMULATION_HPP_*/
