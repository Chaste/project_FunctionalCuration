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
