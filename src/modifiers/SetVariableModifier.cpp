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

#include "SetVariableModifier.hpp"

#include "ValueTypes.hpp"
#include "BacktraceException.hpp"

#include "AbstractCvodeSystem.hpp"
#include "ProtoHelperMacros.hpp"

SetVariableModifier::SetVariableModifier(ApplyWhen when,
                                         const std::string& rVariableName,
                                         const AbstractExpressionPtr pValue)
    : AbstractSimulationModifier(when),
      mVariableName(rVariableName),
      mpValueExpression(pValue)
{
}


void SetVariableModifier::ReallyApply(boost::shared_ptr<AbstractCardiacCellInterface> pCell,
                                      boost::shared_ptr<AbstractStepper> pStepper)
{
    AbstractValuePtr p_value = (*mpValueExpression)(pStepper->rGetEnvironment());
    PROTO_ASSERT(p_value->IsDouble(), "The value computed by a setVariable modifier must be a real number.");

    AbstractCvodeSystem* p_sys = dynamic_cast<AbstractCvodeSystem*>(pCell.get());
    if (p_sys)
    {
        const double old_value = GET_SIMPLE_VALUE(pStepper->rGetEnvironment().Lookup(mVariableName, GetLocationInfo()));
        const double new_value = GET_SIMPLE_VALUE(p_value);
        if (old_value != new_value)
        {
            // We're effectively changing the RHS function, and need to tell CVODE
            p_sys->ResetSolver();
        }
    }

    pStepper->rGetEnvironment().OverwriteDefinition(mVariableName, p_value, GetLocationInfo());
}
