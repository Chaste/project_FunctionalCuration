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
