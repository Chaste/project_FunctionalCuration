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
class ModelStateCollection : private boost::noncopyable
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
