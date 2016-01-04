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

#ifndef SIMPLEVALUE_HPP_
#define SIMPLEVALUE_HPP_

/**
 * @file
 * We define most of the concrete value types in one header, since they're all tiny.
 * The exception is LambdaClosure, which we include here for convenience.
 */

#include "AbstractValue.hpp"

#include <vector>
#include "NdArray.hpp"

#include "LambdaClosure.hpp"

/**
 * A value type representing a single floating point number.
 */
class SimpleValue : public AbstractValue
{
public:
    /**
     * Create an instance.
     *
     * @param value  the number to encapsulate
     */
    SimpleValue(double value)
        : mValue(value)
    {}

    /**
     * Get the encapsulated number.
     */
    double GetValue() const
    {
        return *mValue.Begin();
    }

    /**
     * Used for testing that this is a SimpleValue.
     */
    bool IsDouble() const
    {
        return true;
    }

    /**
     * A simple value can also be considered as a 0-dimensional array.
     */
    bool IsArray() const
    {
        return true;
    }

    /**
     * Get this value as a 0-dimensional array.
     */
    NdArray<double> GetArray() const
    {
        return mValue;
    }

protected:
    /** The encapsulated number. */
    NdArray<double> mValue;
};

/**
 * The magic 'no value'
 */
class NullValue : public AbstractValue
{
public:
    /**
     * Used for testing that this is a NullValue.
     */
    bool IsNull() const
    {
        return true;
    }
};


/**
 * Represents a tuple of values returned from a function.
 */
class TupleValue : public AbstractValue
{
public:
    /**
     * Create a tuple.
     *
     * @param rValues  the components of the tuple
     */
    TupleValue(const std::vector<AbstractValuePtr>& rValues)
        : mValues(rValues)
    {}

    /**
     * Used for testing that this is a TupleValue.
     */
    bool IsTuple() const
    {
        return true;
    }

    /** Get the number of items in this tuple. */
    unsigned GetNumItems() const
    {
        return mValues.size();
    }

    /**
     * Get an item in this tuple.
     *
     * @param i  the item to get
     */
    const AbstractValuePtr GetItem(unsigned i) const
    {
        return mValues[i];
    }

private:
    /** The components of the tuple. */
    std::vector<AbstractValuePtr> mValues;
};

/**
 * An n-dimensional array.  Just a light wrapper around NdArray<double>.
 * Note that a 0-dimensional array can also be considered as equivalent to a SimpleValue.
 */
class ArrayValue : public AbstractValue
{
public:
    /**
     * Create an array value.
     *
     * @param rArray  the actual array object to wrap
     */
    ArrayValue(const NdArray<double>& rArray)
        : mArray(rArray)
    {}

    /**
     * Used for testing that this is an ArrayValue.
     */
    bool IsArray() const
    {
        return true;
    }

    /**
     * Get the encapsulated array.
     */
    const NdArray<double> GetArray() const
    {
        return mArray;
    }

    /**
     * A SimpleValue is equivalent to a 0-dimensional array.
     */
    bool IsDouble() const
    {
        return mArray.GetNumDimensions() == 0u;
    }

    /**
     * Get the single value from a 0-dimensional array.
     */
    double GetValue() const
    {
        assert(IsDouble());
        return *mArray.Begin();
    }

private:
    /** The encapsulated array. */
    NdArray<double> mArray;
};

/**
 * The magic 'default parameter' value.
 */
class DefaultParameter : public AbstractValue
{
public:
    /**
     * Used for testing that this is a DefaultParameter.
     */
    bool IsDefault() const
    {
        return true;
    }
};

/**
 * A value type representing a string.
 */
class StringValue : public AbstractValue
{
public:
    /**
     * Create an instance.
     *
     * @param rValue  the string to encapsulate
     */
    StringValue(const std::string& rValue)
        : mValue(rValue)
    {}

    /**
     * Get the encapsulated string.
     */
    std::string GetString() const
    {
        return mValue;
    }

    /**
     * Used for testing that this is a StringValue.
     */
    bool IsString() const
    {
        return true;
    }

private:
    /** The encapsulated string. */
    std::string mValue;
};

#endif /* SIMPLEVALUE_HPP_ */
