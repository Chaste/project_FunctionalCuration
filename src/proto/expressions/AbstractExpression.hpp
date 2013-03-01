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

#ifndef ABSTRACTEXPRESSION_HPP_
#define ABSTRACTEXPRESSION_HPP_

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

class AbstractExpression;
typedef boost::shared_ptr<AbstractExpression> AbstractExpressionPtr;

#include "LocatableConstruct.hpp"
#include "AbstractValue.hpp"
#include "Environment.hpp"

/**
 * Abstract base class for expressions in the protocol language.
 */
class AbstractExpression : private boost::noncopyable, public LocatableConstruct
{
public:
    /**
     * Construct this expression.
     *
     * @param rChildren  our children in the expression tree
     */
    AbstractExpression(const std::vector<AbstractExpressionPtr>& rChildren);

    /**
     * Construct a leaf expression with no children.
     */
    AbstractExpression();

    /** Needed since we have virtual methods */
    virtual ~AbstractExpression();

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    virtual AbstractValuePtr operator()(const Environment& rEnv) const =0;

protected:
    /**
     * Evaluate our child expressions within the given environment.
     *
     * @param rEnv  the environment
     */
    std::vector<AbstractValuePtr> EvaluateChildren(const Environment& rEnv) const;

    /**
     * If tracing has been requested, this will print the passed in result of this expression,
     * and return the passed-in value.
     *
     * @param pResult  the result value for this expression
     */
    AbstractValuePtr TraceResult(AbstractValuePtr pResult) const;

    /** Our children in the expression tree */
    std::vector<AbstractExpressionPtr> mChildren;
};

#endif /* ABSTRACTEXPRESSION_HPP_ */
