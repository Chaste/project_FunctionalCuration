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

#ifndef FUNCTIONALSTEPPER_HPP_
#define FUNCTIONALSTEPPER_HPP_

#include "AbstractStepper.hpp"

#include "AbstractExpression.hpp"

/**
 * This implements the functionalRange in Frank Bergmann's nested tasks proposal for SED-ML.
 *
 * It applies a mathematical function to other stepper(s) to obtain its values.
 */
class FunctionalStepper : public AbstractStepper
{
public:
    /**
     * Create a new functionalRange.
     * Note that GetCurrentOutputPoint returns an undefined value until Initialise has been called.
     *
     * @param rName  the stepper variable name
     * @param rUnits  the stepper variable units
     * @param pExpr  the function giving the stepper value
     */
    FunctionalStepper(const std::string& rName, const std::string& rUnits,
                      AbstractExpressionPtr pExpr);

    /**
     * A functionalRange should always be a subsidiary stepper, so the end point cannot be determined.
     * Hence this method always return false.
     */
    bool AtEnd();

    /**
     * A functionalRange should always be a subsidiary stepper, so the number of output points
     * cannot be determined.  Hence this method always throws.
     */
    unsigned GetNumberOfOutputPoints() const;

    /**
     * A functionalRange should always be a subsidiary stepper, so the end point cannot be determined.
     * Hence this method always return false.
     */
    bool IsEndFixed() const;

    /**
     * Initialise this stepper, evaluating the defining function to obtain the first value.
     */
    void Initialise();

    /**
     * Reset this stepper to its starting point, re-evaluating the defining function.
     */
    void Reset();

    /**
     * Increment this stepper to its next output point, re-evaluating the defining function.
     */
    double Step();

private:
    /** The function giving this stepper's value at each step. */
    AbstractExpressionPtr mpExpr;
};

#endif // FUNCTIONALSTEPPER_HPP_
