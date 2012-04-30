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

#ifndef NESTEDSIMULATION_HPP_
#define NESTEDSIMULATION_HPP_

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
    NestedSimulation(boost::shared_ptr<AbstractSimulation> pNestedSimulation,
                     boost::shared_ptr<AbstractStepper> pStepper,
                     boost::shared_ptr<ModifierCollection> pModifiers=boost::shared_ptr<ModifierCollection>());

    /**
     * Set method for #mpCell used by the initial parser implementation.
     * @param pCell  the cell model the protocol is being run on
     */
    virtual void SetCell(boost::shared_ptr<AbstractCardiacCellInterface> pCell);

    /**
     * Set where to write any debug tracing to, if desired.
     *
     * @param pHandler  handler for the output folder
     */
    void SetOutputFolder(boost::shared_ptr<OutputFileHandler> pHandler);

protected:
    /**
     * Run a simulation, filling in the results.
     *
     * @param pResults  an Environment containing results arrays to fill in
     */
    void Run(EnvironmentPtr pResults);

private:
    /** The simulation nested inside this one. */
    boost::shared_ptr<AbstractSimulation> mpNestedSimulation;
};

#endif /*NESTEDSIMULATION_HPP_*/
