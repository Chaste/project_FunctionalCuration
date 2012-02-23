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

#ifndef ABSTRACTVALUE_HPP_
#define ABSTRACTVALUE_HPP_

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

class AbstractValue;
typedef boost::shared_ptr<AbstractValue> AbstractValuePtr;

/**
 * Abstract base class for values in the protocol language.
 *
 * Values consist of double, array, lambda-closure, string, tuples, and two 'special' values: a default
 * parameter indicator, and a null value.  This class supplies test methods to determine which kind of
 * value is being dealt with.
 */
class AbstractValue : boost::noncopyable
{
public:
    /** Default constructor. */
    AbstractValue()
        : mUnits("unspecified")
    {}

    /** Whether this is just a floating-point value */
    virtual bool IsDouble() const
    {
        return false;
    }

    /** Whether this is an n-dimensional array */
    virtual bool IsArray() const
    {
        return false;
    }

    /** Whether this is a lambda closure */
    virtual bool IsLambda() const
    {
        return false;
    }

    /** Whether this is a tuple of values, e.g. returned from a function */
    virtual bool IsTuple() const
    {
        return false;
    }

    /** Whether this is the magic 'no value' */
    virtual bool IsNull() const
    {
        return false;
    }

    /** Whether this is the magic 'default parameter' value */
    virtual bool IsDefault() const
    {
        return false;
    }

    /** Whether this is a string */
    virtual bool IsString() const
    {
        return false;
    }

    /**
     * Get the units of this value, if any are known.
     * If no units have been set with SetUnits, returns "unspecified".
     */
    std::string GetUnits() const
    {
        return mUnits;
    }

    /**
     * Set the units of this value.
     * @param rUnits  the units
     */
    void SetUnits(const std::string& rUnits)
    {
        mUnits = rUnits;
    }

    /** Needed since we have virtual methods */
    virtual ~AbstractValue()
    {}

private:
    /** The units of this value. */
    std::string mUnits;
};


#endif // ABSTRACTVALUE_HPP_
