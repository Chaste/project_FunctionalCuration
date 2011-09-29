/*

Copyright (C) University of Oxford, 2005-2011

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

#ifndef MATHMLLOGIC_HPP_
#define MATHMLLOGIC_HPP_

#include "MathmlOperator.hpp"

/**
 * @file
 * Define the MathML logical operators.
 */

/**
 * MathML's nary boolean "and" operator.
 */
class MathmlAnd : public MathmlOperator
{
public:
    /**
     * Create an application of the and operator.
     *
     * @param rOperands  the operands
     */
    MathmlAnd(const std::vector<AbstractExpressionPtr>& rOperands);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;
};


/**
 * MathML's nary boolean "or" operator.
 */
class MathmlOr : public MathmlOperator
{
public:
    /**
     * Create an application of the or operator.
     *
     * @param rOperands  the operands
     */
    MathmlOr(const std::vector<AbstractExpressionPtr>& rOperands);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;
};


/**
 * MathML's nary boolean "xor" operator.
 */
class MathmlXor : public MathmlOperator
{
public:
    /**
     * Create an application of the xor operator.
     *
     * @param rOperands  the operands
     */
    MathmlXor(const std::vector<AbstractExpressionPtr>& rOperands);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;
};


/**
 * MathML's unary boolean "not" operator.
 */
class MathmlNot : public MathmlOperator
{
public:
    /**
     * Create an application of the not operator.
     *
     * @param rOperands  the operands
     */
    MathmlNot(const std::vector<AbstractExpressionPtr>& rOperands);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;
};


#endif // MATHMLLOGIC_HPP_
