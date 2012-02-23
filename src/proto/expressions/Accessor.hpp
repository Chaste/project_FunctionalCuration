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
