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

#ifndef SETVARIABLEMODIFIER_HPP_
#define SETVARIABLEMODIFIER_HPP_

#include <string>

#include "AbstractSimulationModifier.hpp"
#include "AbstractExpression.hpp"

/**
 * A modifier which can set the value of any model variable, addressed by oxmeta name,
 * by computing the value of an expression in the protocol language.
 */
class SetVariableModifier : public AbstractSimulationModifier
{
public:
    /**
     * Create a new modifier.
     * @param when  when this modifier should be applied
     * @param rVariableName  the name of the model variable to change
     * @param pValue  expression to evaluate to determine the value to set
     */
    SetVariableModifier(ApplyWhen when,
                        const std::string& rVariableName,
                        const AbstractExpressionPtr pValue);

private:
    /**
     * Apply this modification to the model.
     *
     * @param pCell  the cell to modify
     * @param pStepper  controls the current loop of the simulation
     */
    void ReallyApply(boost::shared_ptr<AbstractCardiacCellInterface> pCell,
                     boost::shared_ptr<AbstractStepper> pStepper);

    /** The name of the model variable to change. */
    std::string mVariableName;

    /** Expression to evaluate to determine the value to set. */
    AbstractExpressionPtr mpValueExpression;
};

#endif // SETVARIABLEMODIFIER_HPP_
