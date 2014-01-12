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

#ifndef COMBINEDSIMULATION_HPP_
#define COMBINEDSIMULATION_HPP_

#include "AbstractSimulation.hpp"

#include <vector>

/**
 * This simulation type combines multiple other simulations into a single unit which may be executed
 * as a whole.
 */
class CombinedSimulation : public AbstractSimulation
{
public:
    /**
     * This type is used to specify whether the ordering of child simulations matters, and
     * hence whether their execution may be parallelised trivially or not.
     */
    enum Scheduling
    {
        SEQUENTIAL, /**< Child simulations will be executed in order */
        PARALLEL    /**< The ordering of child simulations does not matter */
    };

    /**
     * Create a new combined simulation.
     *
     * @param rChildSims  the child simulations comprising this collection
     * @param scheduling  how the execution of child simulations should be scheduled
     */
    CombinedSimulation(const std::vector<AbstractSimulationPtr>& rChildSims,
                       Scheduling scheduling);

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
     * Ensure that all results arrays are initialised with zeros so that they can easily be replicated
     * by doing a global sum.
     */
    virtual void ZeroInitialiseResults();

protected:
    /**
     * Run a simulation, filling in the results if requested.
     *
     * If the supplied pointer is empty then no results are being stored for this simulation,
     * and we just run the simulation.
     *
     * @param pResults  an Environment to be filled in with results, or an empty pointer
     */
    void Run(EnvironmentPtr pResults);

private:
    /** The child simulations comprising this collection */
    std::vector<AbstractSimulationPtr> mChildSims;

    /** How the execution of child simulations should be scheduled */
    Scheduling mScheduling;
};

#endif // COMBINEDSIMULATION_HPP_
