/*

Copyright (c) 2005-2014, University of Oxford.
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

#ifndef NESTEDSIMULATION_HPP_
#define NESTEDSIMULATION_HPP_

#include <set>
#include "AbstractSimulation.hpp"

/**
 * A nested simulation that contains another simulation.  Each time round this
 * simulation's loop, we run the contained simulation.
 */
class NestedSimulation : public AbstractSimulation
{
public:
    /**
     * Create a new nested simulation.
     *
     * @param pNestedSimulation  the simulation contained within this one
     * @param pStepper  controls the iteration around this simulation's loop
     * @param pModifiers  details any modifications to be made to the cell or
     *     simulation parameters as the simulation progresses
     */
    NestedSimulation(AbstractSimulationPtr pNestedSimulation,
                     AbstractStepperPtr pStepper,
                     boost::shared_ptr<ModifierCollection> pModifiers=boost::shared_ptr<ModifierCollection>());

    /**
     * Set method for #mpModel used by the initial parser implementation.
     * @param pModel  the model the protocol is being run on
     */
    virtual void SetModel(boost::shared_ptr<AbstractSystemWithOutputs> pModel);

    /**
     * Set where to write any debug tracing to, if desired.
     *
     * @param pHandler  handler for the output folder
     */
    void SetOutputFolder(boost::shared_ptr<OutputFileHandler> pHandler);

    /**
     * Set where to write error summaries to.
     *
     * @param pHandler  handler for writing error summaries
     */
    void SetErrorFolder(boost::shared_ptr<OutputFileHandler> pHandler);

    /**
     * Ensure that all results arrays are initialised with zeros so that they can easily be replicated
     * by doing a global sum.
     */
    virtual void ZeroInitialiseResults();

protected:
    /**
     * Run a simulation, filling in the results.
     *
     * @param pResults  an Environment to be filled in with results
     */
    void Run(EnvironmentPtr pResults);

    /**
     * @return  whether this simulation is capable of running on more than one process.
     */
    virtual bool CanParallelise();

private:
    /** The simulation nested inside this one. */
    boost::shared_ptr<AbstractSimulation> mpNestedSimulation;

    /** What to multiply loop indices by to obtain an overall iteration count. */
    std::vector<unsigned> mParallelMultipliers;

    /**
     * Determine whether the iterations of this simulation are provably independent, and so can be
     * farmed out to separate processes.  We do this by analysing the modifiers on this simulation
     * and that nested inside it, to determine whether each loop resets the model to a state
     * determined outside this entire nested simulation.  If it does, then it doesn't matter what
     * outer loops do - we can parallelise at this level.
     *
     * \todo #2341 We also need to worry about modifiers or loop conditions that might use partial
     * results, since the required data might not be available!
     * While loops can do this, but don't get parallelised.
     * Can SetVariable modifiers access partial results?  Yes - it uses the stepper environment, which is the simulation environment.
     * Only FunctionalStepper can compute values in the Step method - other steppers pre-compute all points when the simulation starts.
     *
     * @param rStatesSaved  the names of model states saved by simulations wrapping this one
     * @return whether the iterations of this loop of the simulation can be farmed out to separate processes.
     */
    bool CanParalleliseHere(std::set<std::string>& rStatesSaved) const;

    /**
     * Set that this simulation can be parallelised, and what to multiply each loop index by to obtain
     * an overall iteration count, and hence determine if the current process should perform this iteration.
     *
     * @param rMultipliers  the multipliers
     */
    void SetParallelMultipliers(const std::vector<unsigned>& rMultipliers);
};

#endif /*NESTEDSIMULATION_HPP_*/
