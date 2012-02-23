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
