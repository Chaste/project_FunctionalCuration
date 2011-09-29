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

#ifndef ASSIGNMENTSTATEMENT_HPP_
#define ASSIGNMENTSTATEMENT_HPP_

#include <string>
#include <vector>
#include "AbstractStatement.hpp"
#include "AbstractExpression.hpp"
#include "AbstractValue.hpp"
#include "Environment.hpp"

/**
 * Bind a name to a value within the environment, or bind multiple names to the corresponding items
 * in a tuple value.
 */
class AssignmentStatement : public AbstractStatement
{
public:
    /**
     * Create an assignment statement.
     *
     * @param rNameToAssign  the name to assign
     * @param pRhs  the expression to evaluate to obtain the value to assign to it
     */
    AssignmentStatement(const std::string& rNameToAssign,
                        const AbstractExpressionPtr pRhs);

    /**
     * Create an assignment statement binding multiple names.
     *
     * @param rNamesToAssign  the names to assign
     * @param pRhs  the expression to evaluate to obtain the tuple of values to assign to them
     */
    AssignmentStatement(const std::vector<std::string>& rNamesToAssign,
                        const AbstractExpressionPtr pRhs);

    /**
     * Execute this statement.
     *
     * @param rEnv  the environment to execute it in
     */
    AbstractValuePtr operator()(Environment& rEnv) const;

private:
    /** The name(s) to assign. */
    std::vector<std::string> mNamesToAssign;

    /** The expression to evaluate to obtain the value(s) to assign. */
    AbstractExpressionPtr mpRhs;
};

#endif /* ASSIGNMENTSTATEMENT_HPP_ */
