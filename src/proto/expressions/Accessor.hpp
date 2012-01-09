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

#ifndef ACCESSOR_HPP_
#define ACCESSOR_HPP_

#include "AbstractExpression.hpp"

/**
 * An expression that accesses attributes of the child expression.
 */
class Accessor : public AbstractExpression
{
public:
    /**
     * The available attributes.  Apart from the IS_ attributes, not all attributes may be available
     * on all kinds of value.
     */
    enum Attribute
    {
        IS_SIMPLE_VALUE, /**< Test whether this is a simple value */
        IS_ARRAY,        /**< Test whether this is an array */
        IS_STRING,       /**< Test whether this is a string */
        IS_FUNCTION,     /**< Test whether this is a callable function */
        IS_TUPLE,        /**< Test whether this is a tuple */
        IS_NULL,         /**< Test whether this is the Null value */
        IS_DEFAULT,      /**< Test whether this signifies a default parameter */
        NUM_DIMS,        /**< Get the number of dimensions of an array */
        NUM_ELEMENTS,    /**< Get the number of elements in an array */
        SHAPE            /**< Get the shape of an array (as an array) */
    };

    /**
     * Convert a string containing an attribute name to the corresponding attribute code.
     *
     * @param rAttribute  the attribute name
     * @param rLocInfo  the location information to use in error messages
     */
    static Attribute DecodeAttributeString(const std::string& rAttribute,
                                           const std::string& rLocInfo);

    /**
     * Create an accessor expression.
     *
     * @param pExpression  the expression whose result to access an attribute of
     * @param attribute  the attribute to access
     */
    Accessor(const AbstractExpressionPtr pExpression, Attribute attribute);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;

private:
    /** The attribute to access. */
    Attribute mAttribute;
};

#endif // ACCESSOR_HPP_
