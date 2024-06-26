/*

Copyright (c) 2005-2016, University of Oxford.
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

#ifndef VECTORSTEPPER_HPP_
#define VECTORSTEPPER_HPP_

#include <vector>
#include <string>

#include "AbstractStepper.hpp"

/**
 * A simulation protocol stepper that outputs an arbitrary predefined list of points.
 */
class VectorStepper : public AbstractStepper
{
public:
    /**
     * Create a vector stepper over the given list of values.
     *
     * @param rIndexName  the name of the index variable iterated over by this stepper
     * @param rIndexUnits  the units of the index variable
     * @param rValues  the values to step through
     */
    VectorStepper(const std::string& rIndexName,
                  const std::string& rIndexUnits,
                  const std::vector<double>& rValues);

    /**
     * Create a vector stepper over a list of values.  In this constructor the values
     * are given by expressions, which will be evaluated in the stepper's environment
     * to yield the actual values to step over.
     *
     * @param rIndexName  the name of the index variable iterated over by this stepper
     * @param rIndexUnits  the units of the index variable
     * @param rValues  expressions yielding the values to step through
     */
    VectorStepper(const std::string& rIndexName,
                  const std::string& rIndexUnits,
                  const std::vector<AbstractExpressionPtr>& rValues);

    /**
     * Return the number of output points that will be generated by this stepper.
     * This allows us to create the correct size of output array before filling it
     * with data.
     */
    unsigned GetNumberOfOutputPoints() const;

    /**
     * The end point is fixed, and hence the number of output points is known at the start.
     */
    bool IsEndFixed() const;

    /**
     * If this stepper was constructed with expressions rather than values, evaluate the
     * expressions in the stepper's environment (which must have been set with SetEnvironment)
     * to convert them to the parameters for this stepper.
     */
    void Initialise();

    /**
     * Reset this stepper to its starting point.
     */
    void Reset();

    /**
     * Increment this stepper to its next output point, and return the corresponding
     * value of the index variable.  The value is undefined if AtEnd returns true
     * after this call.
     */
    double Step();

private:
    /** The values to iterate over. */
    std::vector<double> mValues;
};
#endif // VECTORSTEPPER_HPP_
