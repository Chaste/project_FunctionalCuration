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

#ifndef LAMBDAEXPRESSION_HPP_
#define LAMBDAEXPRESSION_HPP_

#include <string>
#include <vector>

#include <boost/make_shared.hpp>

#include "AbstractExpression.hpp"
#include "AbstractStatement.hpp"

/**
 * An expression defining a function.  It always evaluates to a LambdaClosure containing the defined function.
 */
class LambdaExpression : public AbstractExpression
{
public:
    /**
     * Create a function definition expression.
     *
     * @param rFormalParameters  parameter names for the function
     * @param rBody  the body of the function
     * @param rDefaults  default values for the function's parameters, if any
     */
    LambdaExpression(const std::vector<std::string>& rFormalParameters,
                     const std::vector<AbstractStatementPtr>& rBody,
                     const std::vector<AbstractValuePtr>& rDefaults=std::vector<AbstractValuePtr>());

    /**
     * Create a function definition expression, defining a function that just evaluates a single expression
     * and returns the result.
     *
     * @param rFormalParameters  parameter names for the function
     * @param pBodyExpr  the body of the function
     * @param rDefaults  default values for the function's parameters, if any
     */
    LambdaExpression(const std::vector<std::string>& rFormalParameters,
                     const AbstractExpressionPtr pBodyExpr,
                     const std::vector<AbstractValuePtr>& rDefaults=std::vector<AbstractValuePtr>());

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;

    /**
     * Wrap a MathML operator (which is an expression) into a function taking arguments.
     * The template parameter is the operator to wrap.
     *
     * @param numOperands  the number of operands for the expression
     */
    template<typename OPERATOR>
    static AbstractExpressionPtr WrapMathml(unsigned numOperands);

private:
    /** Parameter names for the function. */
    std::vector<std::string> mFormalParameters;

    /** The body of the function. */
    std::vector<AbstractStatementPtr> mBody;

    /** Default values for the function's parameters, if any. */
    std::vector<AbstractValuePtr> mDefaultParameters;

    /**
     * Check that the correct number of default values have been supplied.
     */
    void CheckLengths() const;
};

//
// Implementation for template methods
//

#include "NameLookup.hpp"
#include "Environment.hpp"

template<typename OPERATOR>
AbstractExpressionPtr LambdaExpression::WrapMathml(unsigned numOperands)
{
    std::vector<std::string> fps;
    std::vector<AbstractExpressionPtr> operands;
    for (unsigned i=0; i<numOperands; ++i)
    {
        fps.push_back(Environment::FreshIdent());
        operands.push_back(boost::make_shared<NameLookup>(fps[i]));
    }
    AbstractExpressionPtr body = boost::make_shared<OPERATOR>(operands);
    return boost::make_shared<LambdaExpression>(fps, body);
}

#endif /* LAMBDAEXPRESSION_HPP_ */
