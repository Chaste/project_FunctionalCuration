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

#include "MathmlElementary.hpp"

//#include <algorithm>
#include <cmath>
//#include <boost/numeric/conversion/bounds.hpp>
#include <boost/make_shared.hpp>
#include "BacktraceException.hpp"
#include "ValueTypes.hpp"
#include "ProtoHelperMacros.hpp"

MathmlExp::MathmlExp(const std::vector<AbstractExpressionPtr>& rOperands)
    : MathmlOperator("exp", rOperands)
{
    PROTO_ASSERT(mChildren.size() == 1, "Exponential operator requires 1 operand.");
}

AbstractValuePtr MathmlExp::operator()(const Environment& rEnv) const
{
    std::vector<AbstractValuePtr> operands = EvaluateChildren(rEnv);
    PROTO_ASSERT(operands[0]->IsDouble(), "Exponential operator requires its operand to be a simple value.");
    double result = exp(GET_SIMPLE_VALUE(operands[0]));
    return TraceResult(boost::make_shared<SimpleValue>(result));
}


MathmlLn::MathmlLn(const std::vector<AbstractExpressionPtr>& rOperands)
    : MathmlOperator("ln", rOperands)
{
    PROTO_ASSERT(mChildren.size() == 1, "Natural logarithm operator requires 1 operand.");
}

AbstractValuePtr MathmlLn::operator()(const Environment& rEnv) const
{
    std::vector<AbstractValuePtr> operands = EvaluateChildren(rEnv);
    PROTO_ASSERT(operands[0]->IsDouble(), "Natural logarithm operator requires its operand to be a simple value.");
    double result = log(GET_SIMPLE_VALUE(operands[0]));
    return TraceResult(boost::make_shared<SimpleValue>(result));
}


MathmlLog::MathmlLog(const std::vector<AbstractExpressionPtr>& rOperands)
    : MathmlOperator("log", rOperands)
{
    PROTO_ASSERT(mChildren.size() == 1, "Logarithm operator requires 1 operand, and optionally a logbase qualifier.");
}

AbstractValuePtr MathmlLog::operator()(const Environment& rEnv) const
{
    std::vector<AbstractValuePtr> operands = EvaluateChildren(rEnv);
    PROTO_ASSERT(operands[0]->IsDouble(), "Logarithm operator requires its operand to be a simple value.");
    double operand = GET_SIMPLE_VALUE(operands.back());
    double logbase = 10;
    if (operands.size() == 2)
    {
        PROTO_ASSERT(operands.front()->IsDouble(), "Logarithm operator requires its qualifier to be a simple value.");
        logbase = GET_SIMPLE_VALUE(operands.front());
    }
    double result;
    if (logbase == 10)
    {
        result = log10(operand);
    }
    else
    {
        result = log(operand) / log(logbase);
    }
    return TraceResult(boost::make_shared<SimpleValue>(result));
}

/**
 * Implement the constructor for a unary MathML operator.
 *
 * @param mn  MathML operator name
 * @param cns  suffix for our class name
 */
#define MATHML_UNARY_CONSTRUCTOR(mn, cns)                                          \
    Mathml##cns::Mathml##cns(const std::vector<AbstractExpressionPtr>& rOperands)  \
        : MathmlOperator(mn, rOperands)                                            \
    {                                                                              \
        PROTO_ASSERT(mChildren.size() == 1, #cns " operator requires 1 operand."); \
    }

/**
 * Implement an 'easy' trig function that has a C++ cmath function.
 *
 * @param mn  MathML operator name
 * @param cns  suffix for our class name
 * @param fn  C++ function name
 */
#define TRIG_SIMPLE(mn, cns, fn)                                                                            \
    MATHML_UNARY_CONSTRUCTOR(mn, cns)                                                                       \
    AbstractValuePtr Mathml##cns::operator()(const Environment& rEnv) const                                 \
    {                                                                                                       \
        std::vector<AbstractValuePtr> operands = EvaluateChildren(rEnv);                                    \
        PROTO_ASSERT(operands[0]->IsDouble(), #cns " operator requires its operand to be a simple value."); \
        double result = fn(GET_SIMPLE_VALUE(operands[0]));                                                  \
        return TraceResult(boost::make_shared<SimpleValue>(result));                                                     \
    }

/**
 * Implement a 'reciprocal' trig function that has a C++ cmath function for the normal function.
 *
 * @param mn  MathML operator name
 * @param cns  suffix for our class name
 * @param fn  C++ function name
 */
#define TRIG_RECIP(mn, cns, fn)                                                                             \
    MATHML_UNARY_CONSTRUCTOR(mn, cns)                                                                       \
    AbstractValuePtr Mathml##cns::operator()(const Environment& rEnv) const                                 \
    {                                                                                                       \
        std::vector<AbstractValuePtr> operands = EvaluateChildren(rEnv);                                    \
        PROTO_ASSERT(operands[0]->IsDouble(), #cns " operator requires its operand to be a simple value."); \
        double result = 1.0 / fn(GET_SIMPLE_VALUE(operands[0]));                                                  \
        return TraceResult(boost::make_shared<SimpleValue>(result));                                                     \
    }

/**
 * Implement a 'reciprocal' inverse trig function by doing effectively mn(x) = fn(1/x).
 *
 * @param mn  MathML operator name
 * @param cns  suffix for our class name
 * @param fn  C++ function name
 */
#define TRIG_RECIP_ARG(mn, cns, fn)                                                                             \
    MATHML_UNARY_CONSTRUCTOR(mn, cns)                                                                       \
    AbstractValuePtr Mathml##cns::operator()(const Environment& rEnv) const                                 \
    {                                                                                                       \
        std::vector<AbstractValuePtr> operands = EvaluateChildren(rEnv);                                    \
        PROTO_ASSERT(operands[0]->IsDouble(), #cns " operator requires its operand to be a simple value."); \
        double result = fn(1.0/GET_SIMPLE_VALUE(operands[0]));                                                  \
        return TraceResult(boost::make_shared<SimpleValue>(result));                                                     \
    }

TRIG_SIMPLE("sin", Sin, sin)
TRIG_SIMPLE("cos", Cos, cos)
TRIG_SIMPLE("tan", Tan, tan)

TRIG_SIMPLE("arcsin", ArcSin, asin)
TRIG_SIMPLE("arccos", ArcCos, acos)
TRIG_SIMPLE("arctan", ArcTan, atan)

TRIG_SIMPLE("sinh", Sinh, sinh)
TRIG_SIMPLE("cosh", Cosh, cosh)
TRIG_SIMPLE("tanh", Tanh, tanh)

TRIG_SIMPLE("arcsinh", ArcSinh, asinh)
TRIG_SIMPLE("arccosh", ArcCosh, acosh)
TRIG_SIMPLE("arctanh", ArcTanh, atanh)

TRIG_RECIP("sec", Sec, cos)
TRIG_RECIP("csc", Csc, sin)
TRIG_RECIP("cot", Cot, tan)

TRIG_RECIP("sech", Sech, cosh)
TRIG_RECIP("csch", Csch, sinh)
TRIG_RECIP("coth", Coth, tanh)

TRIG_RECIP_ARG("arcsec", ArcSec, acos)
TRIG_RECIP_ARG("arccsc", ArcCsc, asin)
TRIG_RECIP_ARG("arccot", ArcCot, atan)

TRIG_RECIP_ARG("arcsech", ArcSech, acosh)
TRIG_RECIP_ARG("arccsch", ArcCsch, asinh)
TRIG_RECIP_ARG("arccoth", ArcCoth, atanh)
