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

#ifndef ARRAYCREATE_HPP_
#define ARRAYCREATE_HPP_

#include "FunctionCall.hpp"

/**
 * Generic array creation functionality, providing the fundamental "array comprehension"
 * operations for the protocol language.
 *
 * Arrays may be built up by listing each element of the 0th dimension as an expression.  Nesting
 * such arrays allows a multi-dimensional array to be created.  Each element expression must yield
 * a sub-array (or value) of the same shape, in order to create a regular array.
 *
 * Array comprehensions build up a new array by evaluating a single expression to compute sub-arrays,
 * which are tiled.  Variables can be defined to provide an index into each desired dimension in the
 * new array, and the range of these can be specified.  These variables can be read in the expression
 * to vary the sub-array for each position.  Note that the sub-arrays generated must all be the same
 * shape, and fill the unspecified dimension(s).
 *
 * Some examples:
 * \code
 *  simple = { {1,2}, {3,4}, {5,6} }
 *  counting1d = { i for #0#i=0:10 }  --> {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
 *  counting2d = { i * 3 + j for #0#i=1:3, #1#j=0:3 }  --> { {3,4,5}, {6,7,8} }
 *  blocks = { {{-10+j,j},{10+j,20+j}} for #1#j=0:2 }  --> {{{-10,0}, {-9,1}},  {{10,20}, {11,21}}}
 * \endcode
 *
 * \todo  Improve efficiency of array comprehensions.
 */
class ArrayCreate : public FunctionCall
{
public:
    /**
     * Simple array creation, taking a list of the elements of the 0th dimension.
     *
     * @param rElements  expressions yielding the elements
     */
    ArrayCreate(const std::vector<AbstractExpressionPtr>& rElements);

    /**
     * An array comprehension.
     *
     * @param pElementGenerator  the expression generating elements of the array
     * @param rRanges  range specifications for dimensions of the array being created
     */
    ArrayCreate(const AbstractExpressionPtr pElementGenerator,
                const std::vector<AbstractExpressionPtr>& rRanges);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;

private:
    /** The expression generating elements of the array, if this is an array comprehension. */
    AbstractExpressionPtr mpElementGenerator;
};

#endif // ARRAYCREATE_HPP_
