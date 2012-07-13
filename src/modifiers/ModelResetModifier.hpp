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
#ifndef MODELRESETMODIFIER_HPP_
#define MODELRESETMODIFIER_HPP_

#include "AbstractSimulationModifier.hpp"

#include <string>
#include <boost/shared_ptr.hpp>

#include "ModelStateCollection.hpp"

/**
 * Reset the model either to its initial conditions or a saved state from a ModelStateCollection.
 */
template<typename VECTOR>
class ModelResetModifier : public AbstractSimulationModifier
{
public:
    /**
     * Create the modifier.
     *
     * @param when  when this modifier should be applied
     * @param rName  the name of the saved state in the collection.  Use the empty string to reset
     *     to initial conditions.
     * @param pStateCollection  the collection from which to retrieve the saved state
     */
    ModelResetModifier(ApplyWhen when,
                       const std::string& rName="",
                       boost::shared_ptr<ModelStateCollection> pStateCollection=boost::shared_ptr<ModelStateCollection>());

private:
    /**
     * Apply this modification to the model.
     *
     * @param pModel  the model to modify
     * @param pStepper  controls the current loop of the simulation
     */
    void ReallyApply(boost::shared_ptr<AbstractSystemWithOutputs> pModel,
                     boost::shared_ptr<AbstractStepper> pStepper);

    /** The collection from which to retrieve the saved state. */
    boost::shared_ptr<ModelStateCollection> mpStateCollection;

    /** The name of the saved state in the collection. */
    std::string mStateName;
};

#endif // MODELRESETMODIFIER_HPP_



