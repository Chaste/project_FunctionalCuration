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

#ifndef ABSTRACTSTEPPER_HPP_
#define ABSTRACTSTEPPER_HPP_

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "Environment.hpp"
#include "ValueTypes.hpp"
#include "AbstractExpression.hpp"
#include "LocatableConstruct.hpp"

class AbstractStepper;
typedef boost::shared_ptr<AbstractStepper> AbstractStepperPtr;

/**
 * Base class for simulation protocol steppers - the objects that control
 * iteration when performing a simulation.
 *
 * This inherits from SimpleValue so as to provide the current value of the stepper in
 * a natural fashion to a SetVariableModifier.
 */
class AbstractStepper : public SimpleValue, public LocatableConstruct
{
public:
    /**
     * Create a new stepper.
     *
     * @param rIndexName  the name of the index variable iterated over by this stepper
     * @param rIndexUnits  the units of the index variable
     */
    AbstractStepper(const std::string& rIndexName,
                    const std::string& rIndexUnits);

    /** Virtual destructor. */
    virtual ~AbstractStepper();

    /**
     * Test whether this stepper has reached the end.
     *
     * The default implementation compares GetNumberOfOutputPoints to GetCurrentOutputNumber.
     */
    virtual bool AtEnd();

    /**
     * Return the number of output points that will be generated by this stepper.
     * This allows us to create the correct size of output array before filling it
     * with data.
     */
    virtual unsigned GetNumberOfOutputPoints() const =0;

    /**
     * Return whether the end point is fixed, and hence the number of output points known at
     * the start, or whether it is determined by a dynamic condition, and hence the number of
     * output points is unknown until AtEnd returns true.
     */
    virtual bool IsEndFixed() const =0;

    /**
     * If this stepper was constructed with expressions rather than values, evaluate the
     * expressions in the stepper's environment (which must have been set with SetEnvironment)
     * to convert them to the parameters for this stepper.
     */
    virtual void Initialise() =0;

    /**
     * Reset this stepper to its starting point.
     */
    virtual void Reset() =0;

    /**
     * Increment this stepper to its next output point, and return the corresponding
     * value of the index variable.  The value is undefined if AtEnd returns true
     * after this call.
     */
    virtual double Step() =0;

    /**
     * Get the number of the current step (i.e. how many steps this stepper has taken
     * so far).
     */
    unsigned GetCurrentOutputNumber() const;

    /**
     * Get the value of the current output point.  This value is undefined if AtEnd()
     * is true.
     */
    double GetCurrentOutputPoint() const;

    /** Get method for #mIndexName. */
    std::string GetIndexName() const;

    /** Get method for #mIndexUnits. */
    std::string GetUnits() const;

    /**
     * Set the environment in which this stepper's value is bound.
     *
     * @param pEnv
     */
    virtual void SetEnvironment(EnvironmentPtr pEnv);

    /** Get the environment in which this stepper's value is bound. */
    Environment& rGetEnvironment() const;

protected:
    /** The name of our index variable */
    std::string mIndexName;

    /** The units of our index variable */
    std::string mIndexUnits;

    /** What step we're on currently. */
    unsigned mCurrentStep;

    /** The environment in which this stepper's value is bound. */
    EnvironmentPtr mpEnvironment;

    /** Expressions which should evaluate to the parameters for this stepper. */
    std::vector<AbstractExpressionPtr> mExpressions;

    /**
     * Evaluate the parameters for this stepper, check they are all simple values,
     * and return the decoded values.
     *
     * @param allowArray  whether to allow parameters to be 1d arrays too
     */
    std::vector<double> EvaluateParameters(bool allowArray=false);

    /**
     * Allow subclasses to set the value of the current output point.
     * @param value
     */
    void SetCurrentOutputPoint(double value);
};

#endif // ABSTRACTSTEPPER_HPP_
