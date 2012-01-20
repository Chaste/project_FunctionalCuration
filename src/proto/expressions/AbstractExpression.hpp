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
class AbstractExpression : boost::noncopyable, public LocatableConstruct
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
