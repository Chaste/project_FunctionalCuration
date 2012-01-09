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

#include "ModelStateCollection.hpp"

#include <boost/pointer_cast.hpp> // NB: Not available on Boost 1.33.1

#include "VectorHelperFunctions.hpp"

/** Opaque model state type. */
class ModelStateCollection::AbstractModelState
{
public:
    /** Virtual destructor to make us polymorphic. */
    virtual ~AbstractModelState()
    {}
};

/**
 * Actual model state type.
 */
template<typename VECTOR>
class ModelState : public ModelStateCollection::AbstractModelState
{
public:
    /**
     * Create a new state from a state vector.
     * @param rStateVector  the vector of state variables
     */
    ModelState(const VECTOR& rStateVector)
        : mStateVector(rStateVector)
    {}

    /**
     * Get the encapsulated state vector.
     */
    const VECTOR& rGetStateVector()
    {
        return mStateVector;
    }

    /**
     * Delete the encapulated vector, if required.
     */
    ~ModelState()
    {
        DeleteVector(mStateVector);
    }

private:
    /** The encapsulated state vector. */
    VECTOR mStateVector;
};


template<typename VECTOR>
void ModelStateCollection::SaveState(const std::string& rName,
                                     const boost::shared_ptr<AbstractParameterisedSystem<VECTOR> > pModel)
{
    const VECTOR state_vec = pModel->GetStateVariables();
    boost::shared_ptr<ModelStateCollection::AbstractModelState> p_state(new ModelState<VECTOR>(state_vec));
    mStates[rName] = p_state;
}


template<typename VECTOR>
void ModelStateCollection::SetModelState(boost::shared_ptr<AbstractParameterisedSystem<VECTOR> > pModel,
                                         const std::string& rName)
{
    boost::shared_ptr<ModelState<VECTOR> > p_state = boost::dynamic_pointer_cast<ModelState<VECTOR> >(mStates[rName]);
    pModel->SetStateVariables(p_state->rGetStateVector());
}


//////////////////////////////////////////////////////////////////////
// Explicit instantiation
//////////////////////////////////////////////////////////////////////

/**
 * \cond
 * Get Doxygen to ignore, since it's confused by explicit instantiation of templated methods
 */
#ifdef CHASTE_CVODE
template void ModelStateCollection::SaveState(const std::string&, const boost::shared_ptr<AbstractParameterisedSystem<N_Vector> >);
template void ModelStateCollection::SetModelState(boost::shared_ptr<AbstractParameterisedSystem<N_Vector> >, const std::string&);
#endif
template void ModelStateCollection::SaveState(const std::string&, const boost::shared_ptr<AbstractParameterisedSystem<std::vector<double> > >);
template void ModelStateCollection::SetModelState(boost::shared_ptr<AbstractParameterisedSystem<std::vector<double> > >, const std::string&);
/**
 * \endcond
 * Get Doxygen to ignore, since it's confused by explicit instantiation of templated methods
 */
