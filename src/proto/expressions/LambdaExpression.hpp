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
