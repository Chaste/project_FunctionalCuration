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

#ifndef MODELSTATECOLLECTION_HPP_
#define MODELSTATECOLLECTION_HPP_

#include <string>
#include <map>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include "AbstractParameterisedSystem.hpp"

/**
 * A collection of saved model states that can be applied back to the model.
 */
class ModelStateCollection : boost::noncopyable
{
public:
    /**
     * Save the model state into this collection.
     * @param rName  the name for the saved state
     * @param pModel  the model whose state to save
     */
    template<typename VECTOR>
    void SaveState(const std::string& rName,
                   const boost::shared_ptr<AbstractParameterisedSystem<VECTOR> > pModel);

    /**
     * Set a model's state to one previously saved
     * @param pModel  the model whose state to set
     * @param rName  the name of the previously saved state
     */
    template<typename VECTOR>
    void SetModelState(boost::shared_ptr<AbstractParameterisedSystem<VECTOR> > pModel,
                       const std::string& rName);

private:
    /** Opaque model state type. */
    class AbstractModelState;

    /** The states stored in this collection. */
    std::map<std::string, boost::shared_ptr<AbstractModelState> > mStates;
};

#endif // MODELSTATECOLLECTION_HPP_
