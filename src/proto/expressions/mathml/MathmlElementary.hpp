/*

Copyright (c) 2005-2015, University of Oxford.
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

#ifndef MATHMLELEMENTARY_HPP_
#define MATHMLELEMENTARY_HPP_

/**
 * @file
 * Define various MathML elementary classical functions.
 */

#include "MathmlOperator.hpp"

/**
 * MathML's unary exponential function.
 */
class MathmlExp : public MathmlOperator
{
public:
    /**
     * Create an application of the exp function.
     *
     * @param rOperands  the operands
     */
    MathmlExp(const std::vector<AbstractExpressionPtr>& rOperands);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;
};

/**
 * MathML's unary natural logarithm function.
 */
class MathmlLn : public MathmlOperator
{
public:
    /**
     * Create an application of the ln function.
     *
     * @param rOperands  the operands
     */
    MathmlLn(const std::vector<AbstractExpressionPtr>& rOperands);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;
};

/**
 * MathML's generic logarithm function.
 */
class MathmlLog : public MathmlOperator
{
public:
    /**
     * Create an application of the log function.
     *
     * @param rOperands  the operands
     */
    MathmlLog(const std::vector<AbstractExpressionPtr>& rOperands);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;
};

/**
 * We take a short-cut for all the trig functions!
 * @param fn  function name from MathML, first letter uppercased
 */
#define TRIG_HEADER(fn)                           \
    class Mathml ## fn : public MathmlOperator    \
    {                                             \
    public:                                       \
        Mathml ## fn(const std::vector<AbstractExpressionPtr>& rOperands); \
        AbstractValuePtr operator()(const Environment& rEnv) const;        \
    }

/**
 * @cond Doxygen parses these as functions, but we'd rather it didn't.
 */
TRIG_HEADER(Sin);
TRIG_HEADER(Cos);
TRIG_HEADER(Tan);
TRIG_HEADER(Sec);
TRIG_HEADER(Csc);
TRIG_HEADER(Cot);
TRIG_HEADER(Sinh);
TRIG_HEADER(Cosh);
TRIG_HEADER(Tanh);
TRIG_HEADER(Sech);
TRIG_HEADER(Csch);
TRIG_HEADER(Coth);

TRIG_HEADER(ArcSin);
TRIG_HEADER(ArcCos);
TRIG_HEADER(ArcTan);
TRIG_HEADER(ArcSec);
TRIG_HEADER(ArcCsc);
TRIG_HEADER(ArcCot);
TRIG_HEADER(ArcSinh);
TRIG_HEADER(ArcCosh);
TRIG_HEADER(ArcTanh);
TRIG_HEADER(ArcSech);
TRIG_HEADER(ArcCsch);
TRIG_HEADER(ArcCoth);

/**
 * @endcond  Stop Doxygen ignoring things.
 */

#undef TRIG_HEADER

#endif // MATHMLELEMENTARY_HPP_
