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
