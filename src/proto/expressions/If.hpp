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

#ifndef IF_HPP_
#define IF_HPP_

#include "AbstractExpression.hpp"

/**
 * This represents an if(test, then, else) expression.  If the test evaluates to non-zero, then
 * the whole expression evaluates to the result of the then expression; otherwise the else
 * expression is used instead.  Short-circuiting is performed: only one of then & else will be
 * evaluated.
 */
class If : public AbstractExpression
{
public:
    /**
     * Create an if expression.
     *
     * @param pTest  the expression to test for true/false
     * @param pThen  the expression to evaluate if the test is true (non-zero)
     * @param pElse  the expression to evaluate if the test is false
     */
    If(const AbstractExpressionPtr pTest,
       const AbstractExpressionPtr pThen,
       const AbstractExpressionPtr pElse);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;
};

#endif // IF_HPP_
