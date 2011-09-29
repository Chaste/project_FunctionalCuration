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

#ifndef RETURNSTATEMENT_HPP_
#define RETURNSTATEMENT_HPP_

#include <vector>

#include "AbstractStatement.hpp"
#include "AbstractExpression.hpp"
#include "AbstractValue.hpp"
#include "Environment.hpp"

/**
 * Return a value (or tuple of values) from a function.
 */
class ReturnStatement : public AbstractStatement
{
public:
    /**
     * Create a tuple return statement.
     *
     * @param rExpressions  the expressions to evaluate to yield the return values
     */
    ReturnStatement(const std::vector<AbstractExpressionPtr>& rExpressions);

    /**
     * Create a single-value return statement.
     *
     * @param pExpression  the expression to evaluate to yield the return value
     */
    ReturnStatement(const AbstractExpressionPtr pExpression);

    /**
     * Execute this statement.
     *
     * @param rEnv  the environment to execute it in
     */
    AbstractValuePtr operator()(Environment& rEnv) const;

private:
    /** The expression(s) to evaluate to yield the return value(s). */
    std::vector<AbstractExpressionPtr> mExpressions;
};

#endif /* RETURNSTATEMENT_HPP_ */
