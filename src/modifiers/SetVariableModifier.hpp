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
     * @param pModel  the model to modify
     * @param pStepper  controls the current loop of the simulation
     */
    void ReallyApply(boost::shared_ptr<AbstractUntemplatedSystemWithOutputs> pModel,
                     boost::shared_ptr<AbstractStepper> pStepper);

    /** The name of the model variable to change. */
    std::string mVariableName;

    /** Expression to evaluate to determine the value to set. */
    AbstractExpressionPtr mpValueExpression;
};

#endif // SETVARIABLEMODIFIER_HPP_
