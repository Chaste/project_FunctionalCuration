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
