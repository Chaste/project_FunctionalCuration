/*

Copyright (c) 2005-2016, University of Oxford.
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

#ifndef PROTOHELPERMACROS_HPP_
#define PROTOHELPERMACROS_HPP_

/**
 * @file
 * Defines various helper macros to reduce typing when using the protocol language constructs.
 */

#include <boost/make_shared.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/preprocessor/stringize.hpp>
#include "ValueTypes.hpp"
#include "ValueExpression.hpp"

//////////////////////////////////////////////////////////////////////
// Extract C++ types from language types
//////////////////////////////////////////////////////////////////////

/**
 * Extract the double value from a SimpleValue.
 * @param p_value  shared pointer to the SimpleValue.
 */
#define GET_SIMPLE_VALUE(p_value)  static_cast<SimpleValue*>((p_value).get())->GetValue()

/**
 *
 * Extract the NdArray<double> from an ArrayValue.
 * @param p_value  shared pointer to the ArrayValue.
 */
#define GET_ARRAY(p_value)  static_cast<ArrayValue*>((p_value).get())->GetArray()

//////////////////////////////////////////////////////////////////////
// Create expression objects
//////////////////////////////////////////////////////////////////////

/**
 * Create an expression that just wraps a value.
 * @param type  the type of value to wrap
 * @param value  the single constructor parameter for the value
 */
#define VALUE(type, value)  boost::make_shared<ValueExpression>(boost::make_shared<type>(value))

/**
 * Create an expression wrapping a constant number.
 * @param value  the number to wrap
 */
#define CONST(value)  VALUE(SimpleValue, value)

/**
 * Create an expression wrapping the null value.
 */
#define NULL_EXPR  VALUE(NullValue, )

/**
 * Create an expression specifying use of a default parameter value.
 */
#define DEFAULT_EXPR  VALUE(DefaultParameter, )

/**
 * Create a NameLookup expression.
 * @param name  the name to look up
 */
#define LOOKUP(name)  boost::make_shared<NameLookup>(name)

/**
 * Create an If expression.
 * @param test
 * @param then
 * @param else_
 */
#define IF(test, then, else_)  boost::make_shared<If>(test, then, else_)

//////////////////////////////////////////////////////////////////////
// Create value objects
//////////////////////////////////////////////////////////////////////

/**
 * Create a constant SimpleValue.
 * @param v  the value it should contain
 */
#define CV(v)  boost::make_shared<SimpleValue>(v)

//////////////////////////////////////////////////////////////////////
// Create statement objects
//////////////////////////////////////////////////////////////////////

/**
 * Create an assignment statement.
 * @param name  the name to assign to
 * @param value  the expression to assign
 */
#define ASSIGN_STMT(name, value)  boost::make_shared<AssignmentStatement>(name, value)

/**
 * Create an assertion.
 * @param expr  the expression to assert
 */
#define ASSERT_STMT(expr)  boost::make_shared<AssertStatement>(expr)

/**
 * Create a return statement.
 * @param expr  the expression to return the value of
 */
#define RETURN_STMT(expr)  boost::make_shared<ReturnStatement>(expr)

//////////////////////////////////////////////////////////////////////
// Define variables holding language constructs
//////////////////////////////////////////////////////////////////////

/**
 * Set the location information for a language construct based on the C++ source file defining it
 * (or rather, calling this macro), so that backtraces are more useful.
 * @param ptr  (shared pointer to) the object to locate
 */
#define LOC(ptr)  ptr->SetLocationInfo(__FILE__ ":" BOOST_PP_STRINGIZE(__LINE__) ":0\t" #ptr)

/**
 * Define a (C++) variable to contain an expression, and set location information.
 * @param name  the variable's name
 * @param expr  the expression
 */
#define DEFINE(name, expr)  AbstractExpressionPtr name = expr; LOC(name)

/**
 * Define a variable holding a TupleExpression, and set location information.
 * @param name  the variable's name
 * @param list  a list_of or vector containing the constituent expressions
 */
#define DEFINE_TUPLE(name, list)    \
    AbstractExpressionPtr name;     \
    { std::vector<AbstractExpressionPtr> items = list; \
      name = boost::make_shared<TupleExpression>(items);    \
    } LOC(name)

/**
 * Define a variable holding a statement, and set location information.
 * @param name  the variable's name
 * @param stmt  the statement
 */
#define DEFINE_STMT(name, stmt)  AbstractStatementPtr name = stmt; LOC(name)

//////////////////////////////////////////////////////////////////////
// Miscellaneous
//////////////////////////////////////////////////////////////////////

/**
 * Shorthand for when defining a list of expressions on-the-fly.
 */
#define EXPR_LIST  boost::assign::list_of<AbstractExpressionPtr>

#endif /* PROTOHELPERMACROS_HPP_ */
