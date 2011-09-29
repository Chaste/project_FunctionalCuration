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

#ifndef ASSERTSTATEMENT_HPP_
#define ASSERTSTATEMENT_HPP_

#include "AbstractStatement.hpp"
#include "AbstractExpression.hpp"

/**
 * A statement that checks an assertion and throws a BacktraceException if it fails, i.e. if the
 * expression evaluates as zero.
 */
class AssertStatement : public AbstractStatement
{
public:
    /**
     * Create an assertion.
     *
     * @param pAssertion  the expression which should evaluate to non-zero
     */
    AssertStatement(const AbstractExpressionPtr pAssertion);

    /**
     * Execute this statement.
     *
     * @param rEnv  the environment to execute it in
     */
    AbstractValuePtr operator()(Environment& rEnv) const;

private:
    /** The expression which should evaluate to non-zero. */
    AbstractExpressionPtr mpAssertion;
};

#endif // ASSERTSTATEMENT_HPP_
