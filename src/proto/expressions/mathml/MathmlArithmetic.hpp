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

#ifndef MATHMLARITHMETIC_HPP_
#define MATHMLARITHMETIC_HPP_

/**
 * @file
 * Define various MathML arithmetic operators.
 */

#include "MathmlOperator.hpp"

/**
 * MathML's n-ary max operator.
 */
class MathmlMax : public MathmlOperator
{
public:
    /**
     * Create an application of the max operator.
     *
     * @param rOperands  the operands
     */
    MathmlMax(const std::vector<AbstractExpressionPtr>& rOperands);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;
};


/**
 * MathML's n-ary min operator.
 */
class MathmlMin : public MathmlOperator
{
public:
    /**
     * Create an application of the min operator.
     *
     * @param rOperands  the operands
     */
    MathmlMin(const std::vector<AbstractExpressionPtr>& rOperands);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;
};


/**
 * MathML's binary quotient operator.
 */
class MathmlQuotient : public MathmlOperator
{
public:
    /**
     * Create an application of the quotient operator.
     *
     * @param rOperands  the operands
     */
    MathmlQuotient(const std::vector<AbstractExpressionPtr>& rOperands);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;
};


/**
 * MathML's binary remainder operator.
 */
class MathmlRem : public MathmlOperator
{
public:
    /**
     * Create an application of the rem operator.
     *
     * @param rOperands  the operands
     */
    MathmlRem(const std::vector<AbstractExpressionPtr>& rOperands);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;
};


/**
 * MathML's binary power operator.
 */
class MathmlPower : public MathmlOperator
{
public:
    /**
     * Create an application of the power operator.
     *
     * @param rOperands  the operands
     */
    MathmlPower(const std::vector<AbstractExpressionPtr>& rOperands);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;
};


/**
 * MathML's root operator.
 */
class MathmlRoot : public MathmlOperator
{
public:
    /**
     * Create an application of the root operator.
     *
     * @param rOperands  the operands
     */
    MathmlRoot(const std::vector<AbstractExpressionPtr>& rOperands);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;
};


/**
 * MathML's unary absolute-value operator.
 */
class MathmlAbs : public MathmlOperator
{
public:
    /**
     * Create an application of the abs operator.
     *
     * @param rOperands  the operands
     */
    MathmlAbs(const std::vector<AbstractExpressionPtr>& rOperands);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;
};


/**
 * MathML's unary floor operator.
 */
class MathmlFloor : public MathmlOperator
{
public:
    /**
     * Create an application of the floor operator.
     *
     * @param rOperands  the operands
     */
    MathmlFloor(const std::vector<AbstractExpressionPtr>& rOperands);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;
};


/**
 * MathML's unary ceiling operator.
 */
class MathmlCeiling : public MathmlOperator
{
public:
    /**
     * Create an application of the ceiling operator.
     *
     * @param rOperands  the operands
     */
    MathmlCeiling(const std::vector<AbstractExpressionPtr>& rOperands);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;
};

#endif // MATHMLARITHMETIC_HPP_
