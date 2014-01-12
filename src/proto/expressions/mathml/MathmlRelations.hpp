/*

Copyright (c) 2005-2014, University of Oxford.
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
