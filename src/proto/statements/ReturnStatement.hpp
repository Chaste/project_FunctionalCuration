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
