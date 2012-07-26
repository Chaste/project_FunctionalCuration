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

#ifndef MODIFIERCOLLECTION_HPP_
#define MODIFIERCOLLECTION_HPP_

#include <vector>
#include <boost/shared_ptr.hpp>

#include "AbstractSimulationModifier.hpp"
#include "AbstractStepper.hpp"
#include "AbstractCardiacCellInterface.hpp"

/**
 * A collection of modifiers to be applied during a simulation.
 * 
 * This is essentially a thin wrapper around a vector of AbstractModifier objects,
 * providing the same operator() interface so that a simulation engine doesn't need
 * to do the iteration itself.
 */
class ModifierCollection
{
public:
    /**
     * Create a collection of modifiers.
     * 
     * @param rModifiers  the objects making up the collection
     */
    ModifierCollection(const std::vector<AbstractSimulationModifierPtr>& rModifiers);

    /**
     * Create an empty collection with room for numModifiers objects.
     * These slots must then be assigned using operator[].
     * 
     * @param numModifiers  number of modifiers in this collection
     */
    ModifierCollection(unsigned numModifiers);

    /**
     * Access an individual modifier in this collection.
     * 
     * @param i  the index of the modifier to access.
     */
    AbstractSimulationModifierPtr& operator[](unsigned i);

    /**
     * Add another modifier to this collection.
     *
     * @param pModifier  the modifier to add
     */
    void AddModifier(AbstractSimulationModifierPtr pModifier);

    /**
     * Apply each modifier in this collection in turn, starting with the first.
     * 
     * @param pModel  the model to modify
     * @param pStepper  controls the current loop of the simulation
     */
    void operator()(boost::shared_ptr<AbstractSystemWithOutputs> pModel,
                    AbstractStepperPtr pStepper);

    /**
     * Apply each modifier in this collection that takes effect AT_END in turn, starting with the first.
     *
     * @param pModel  the model to modify
     * @param pStepper  controls the current loop of the simulation
     */
    void ApplyAtEnd(boost::shared_ptr<AbstractSystemWithOutputs> pModel,
                    AbstractStepperPtr pStepper);

private:
    /** To reduce typing. */
    typedef std::vector<AbstractSimulationModifierPtr> Collection;
    
    /** The underlying collection of modifiers. */
    Collection mModifiers;
};


#endif // MODIFIERCOLLECTION_HPP_
