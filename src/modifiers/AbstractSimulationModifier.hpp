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

#ifndef ABSTRACTSIMULATIONMODIFIER_HPP_
#define ABSTRACTSIMULATIONMODIFIER_HPP_

#include <boost/shared_ptr.hpp>

#include "AbstractStepper.hpp"
#include "AbstractSystemWithOutputs.hpp"
#include "LocatableConstruct.hpp"


class AbstractSimulationModifier;
typedef boost::shared_ptr<AbstractSimulationModifier> AbstractSimulationModifierPtr;


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
     * Apply this modification to the model.
     *
     * This method is called by the simulation engine, checks the stepper's state against
     * our #mWhen specification, and calls ReallyApply if the conditions match.  It may
     * be overridden by subclasses if they need to do something completely different.
     *
     * @param pModel  the model to modify
     * @param pStepper  controls the current loop of the simulation
     */
    virtual void operator()(boost::shared_ptr<AbstractSystemWithOutputs> pModel,
                            boost::shared_ptr<AbstractStepper> pStepper);

    /**
     * Apply this modification to the model if it takes effect at the end of a loop.
     *
     * @param pModel  the model to modify
     * @param pStepper  controls the current loop of the simulation
     */
    virtual void ApplyAtEnd(boost::shared_ptr<AbstractSystemWithOutputs> pModel,
                            boost::shared_ptr<AbstractStepper> pStepper);

    /**
     * @return  when this modifier will be applied.
     */
    ApplyWhen GetWhenApplied() const;

    /**
     * @return  if this is a save or reset state modifier, the associated state name.
     * Otherwise returns the empty string.
     */
    virtual std::string GetStateName() const;

    /**
     * @return  whether this is a reset state modifier.
     */
    virtual bool IsReset() const;

protected:
    /**
     * Method subclasses must supply which really implements the modification.
     *
     * @param pModel  the cell to modify
     * @param pStepper  controls the current loop of the simulation
     */
    virtual void ReallyApply(boost::shared_ptr<AbstractSystemWithOutputs> pModel,
                             boost::shared_ptr<AbstractStepper> pStepper) =0;

    /** When to apply this modification. */
    ApplyWhen mWhen;
};


#endif // ABSTRACTSIMULATIONMODIFIER_HPP_
