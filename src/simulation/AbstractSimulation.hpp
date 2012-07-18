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

#include <string>
#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include "AbstractSystemWithOutputs.hpp"
#include "AbstractStepper.hpp"
#include "ModifierCollection.hpp"
#include "Environment.hpp"
#include "LocatableConstruct.hpp"
#include "NdArray.hpp"

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
     * @param pModel  the model the protocol is being run on
     * @param pStepper  controls the iteration around this simulation's loop
     * @param pModifiers  details any modifications to be made to the cell or
     *     simulation parameters as the simulation progresses
     * @param pSteppers  if this is part of a nested simulation, the shared
     *     collection of steppers.  pStepper will be added by this constructor.
     */
    AbstractSimulation(boost::shared_ptr<AbstractSystemWithOutputs> pModel,
                       AbstractStepperPtr pStepper,
                       boost::shared_ptr<ModifierCollection> pModifiers=boost::shared_ptr<ModifierCollection>(),
                       StepperCollection pSteppers=StepperCollection());

    /** Virtual destructor. */
    virtual ~AbstractSimulation();

    /**
     * Run a simulation, returning an Environment containing the results (the same Environment
     * given by GetResultsEnvironment).  This is the method external callers should use to run
     * an entire simulation.
     */
    EnvironmentPtr Run();

    /** Get method for #mpModel. */
    boost::shared_ptr<AbstractSystemWithOutputs> GetModel()
    {
        return mpModel;
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
     * Set the model being simulated, and add its wrapper environments as delegatees from our
     * environments.
     *
     * @param pModel  the model the protocol is being run on
     */
    virtual void SetModel(boost::shared_ptr<AbstractSystemWithOutputs> pModel);

    /** Call Initialise on all the steppers in this simulation. */
    void InitialiseSteppers();

    /** Get the environment in which this simulation is run. */
    Environment& rGetEnvironment();

    /**
     * Get the environment that will contain this simulation's results (when it has been run).
     * This environment is still useful prior to running the simulation, as it delegates by prefix
     * to the model this simulation runs, and thus can be used to look up model variables.
     */
    EnvironmentPtr GetResultsEnvironment();

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

    /**
     * Run a simulation, filling in the results if requested.
     *
     * This method must be provided by concrete subclasses.  If the supplied pointer is
     * NULL then no results are being stored for this simulation, and the subclass should
     * just run the simulation.  Otherwise, the AddIterationOutputs method should be called
     * by the innermost simulation object to fill in the results.
     *
     * @param pResults  an Environment to be filled in with results, or an empty pointer
     */
    virtual void Run(EnvironmentPtr pResults)=0;

protected:
    /**
     * This method must be called by subclasses as the first thing within their simulation loop.
     */
    void LoopBodyStartHook();

    /**
     * This method must be called by subclasses as the last thing within their simulation loop,
     * just before their stepper is incremented.
     */
    void LoopBodyEndHook();

    /**
     * This method must be called by subclasses after their simulation loop has completed.
     */
    void LoopEndHook();

    /**
     * This method adds outputs from the current iteration to the overall simulation outputs.
     * It should be called by the innermost loop of the simulation at each iteration, prior to
     * calling LoopBodyEndHook.
     *
     * @param pResults  the environment in which to record the whole simulation's results
     *     (or an empty pointer if not recording)
     * @param pIterationOutputs  the outputs from the current iteration
     */
    void AddIterationOutputs(EnvironmentPtr pResults, EnvironmentCPtr pIterationOutputs);

    /**
     * If this simulation is controlled by a while loop, then we might need to resize the
     * output arrays whenever they exceed the current allocation, and shrink them to the
     * final extent of the loop at the end of the simulation.
     */
    void ResizeOutputs();

    /**
     * Create a delegatee of #mpEnvironment containing views of the simulation results thus far.
     * Should only be called if this simulation is controlled by a while loop and has a results prefix.
     *
     * Implementation note: #mpResultsEnvironment contains the full-size (as far as is known at present)
     * results arrays, only partially filled in.  Hence these are not suitable for access in the while
     * loop condition, which needs to see the results up to this point as complete arrays.  Creating a
     * delegatee of #mpEnvironment with our results prefix will mean that that new environment is used in
     * preference to #mpResultsEnvironment (which is a delegatee of the protocol library environment)
     * when looking up names with our prefix.
     */
    void CreateResultViews();

    /**
     * The model the protocol is being run on.
     */
    boost::shared_ptr<AbstractSystemWithOutputs> mpModel;

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

    /** The environment in which to store this simulation's results. */
    EnvironmentPtr mpResultsEnvironment;

    /** The shapes of the model outputs on the first iteration. */
    std::map<std::string, NdArray<double>::Extents> mModelOutputShapes;
};

#endif /*ABSTRACTSIMULATION_HPP_*/
