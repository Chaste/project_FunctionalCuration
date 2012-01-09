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

#include "ModelResetModifier.hpp"
#include "AbstractParameterisedSystem.hpp"
#include "VectorHelperFunctions.hpp"

template<typename VECTOR>
ModelResetModifier<VECTOR>::ModelResetModifier(ApplyWhen when,
                                               const std::string& rName,
                                               boost::shared_ptr<ModelStateCollection> pStateCollection)
    : AbstractSimulationModifier(when),
      mpStateCollection(pStateCollection),
      mStateName(rName)
{
}


template<typename VECTOR>
void ModelResetModifier<VECTOR>::ReallyApply(boost::shared_ptr<AbstractCardiacCellInterface> pCell,
                                             boost::shared_ptr<AbstractStepper> pStepper)
{
    boost::shared_ptr<AbstractParameterisedSystem<VECTOR> > p_system = boost::dynamic_pointer_cast<AbstractParameterisedSystem<VECTOR> >(pCell);
    assert(p_system);
    // Do the reset
    if (mStateName.empty())
    {
        p_system->ResetToInitialConditions();
    }
    else
    {
        mpStateCollection->SetModelState(p_system, mStateName);
    }
}



//////////////////////////////////////////////////////////////////////
// Explicit instantiation
//////////////////////////////////////////////////////////////////////

#ifdef CHASTE_CVODE
template class ModelResetModifier<N_Vector>;
#endif
template class ModelResetModifier<std::vector<double> >;
