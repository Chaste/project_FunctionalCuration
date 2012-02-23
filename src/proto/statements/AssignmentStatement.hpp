/*

Copyright (c) 2005-2012, University of Oxford.
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
