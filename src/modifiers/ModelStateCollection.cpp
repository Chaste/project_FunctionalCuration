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
     * Delete the encapsulated vector, if required.
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
