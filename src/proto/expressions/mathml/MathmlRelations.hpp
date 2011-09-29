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

#ifndef MATHMLRELATIONS_HPP_
#define MATHMLRELATIONS_HPP_

#include "MathmlOperator.hpp"

/**
 * @file
 * Define the MathML relational operators.
 */

/**
 * MathML's binary equality operator.
 */
class MathmlEq : public MathmlOperator
{
public:
    /**
     * Create an application of the eq operator.
     *
     * @param rOperands  the operands
     */
    MathmlEq(const std::vector<AbstractExpressionPtr>& rOperands);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;
};

/**
 * MathML's binary "not equal to" operator.
 */
class MathmlNeq : public MathmlOperator
{
public:
    /**
     * Create an application of the neq operator.
     *
     * @param rOperands  the operands
     */
    MathmlNeq(const std::vector<AbstractExpressionPtr>& rOperands);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;
};

/**
 * MathML's binary less-than operator.
 */
class MathmlLt : public MathmlOperator
{
public:
    /**
     * Create an application of the lt operator.
     *
     * @param rOperands  the operands
     */
    MathmlLt(const std::vector<AbstractExpressionPtr>& rOperands);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;
};

/**
 * MathML's binary greater-than operator.
 */
class MathmlGt : public MathmlOperator
{
public:
    /**
     * Create an application of the gt operator.
     *
     * @param rOperands  the operands
     */
    MathmlGt(const std::vector<AbstractExpressionPtr>& rOperands);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;
};

/**
 * MathML's binary less-than-or-equals operator.
 */
class MathmlLeq : public MathmlOperator
{
public:
    /**
     * Create an application of the leq operator.
     *
     * @param rOperands  the operands
     */
    MathmlLeq(const std::vector<AbstractExpressionPtr>& rOperands);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;
};

/**
 * MathML's binary greater-than-or-equals operator.
 */
class MathmlGeq : public MathmlOperator
{
public:
    /**
     * Create an application of the geq operator.
     *
     * @param rOperands  the operands
     */
    MathmlGeq(const std::vector<AbstractExpressionPtr>& rOperands);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;
};

#endif // MATHMLRELATIONS_HPP_
