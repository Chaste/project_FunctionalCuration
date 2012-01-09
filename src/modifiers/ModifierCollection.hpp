/*

Copyright (C) University of Oxford, 2005-2012

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
    ModifierCollection(const std::vector<boost::shared_ptr<AbstractSimulationModifier> >& rModifiers);

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
    boost::shared_ptr<AbstractSimulationModifier>& operator[](unsigned i);

    /**
     * Apply each modifier in this collection in turn, starting with the first.
     * 
     * @param pCell  the cell to modify
     * @param pStepper  controls the current loop of the simulation
     */
    void operator()(boost::shared_ptr<AbstractCardiacCellInterface> pCell,
                    boost::shared_ptr<AbstractStepper> pStepper);

    /**
     * Apply each modifier in this collection that takes effect AT_END in turn, starting with the first.
     *
     * @param pCell  the cell to modify
     * @param pStepper  controls the current loop of the simulation
     */
    void ApplyAtEnd(boost::shared_ptr<AbstractCardiacCellInterface> pCell,
                    boost::shared_ptr<AbstractStepper> pStepper);

private:
    /** To reduce typing. */
    typedef std::vector<boost::shared_ptr<AbstractSimulationModifier> > Collection;
    
    /** The underlying collection of modifiers. */
    Collection mModifiers;
};


#endif // MODIFIERCOLLECTION_HPP_
