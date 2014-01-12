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
