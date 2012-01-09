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

#include "If.hpp"

#include <boost/assign/list_of.hpp>
#include "ProtoHelperMacros.hpp"
#include "BacktraceException.hpp"

If::If(const AbstractExpressionPtr pTest,
       const AbstractExpressionPtr pThen,
       const AbstractExpressionPtr pElse)
    : AbstractExpression()
{
    mChildren = boost::assign::list_of(pTest)(pThen)(pElse);
}

AbstractValuePtr If::operator()(const Environment& rEnv) const
{
    AbstractValuePtr p_test = (*mChildren[0])(rEnv);
    PROTO_ASSERT(p_test->IsDouble(), "The test in an if expression must be a simple value.");
    double test = GET_SIMPLE_VALUE(p_test);
    AbstractValuePtr p_result;
    if (test)
    {
        p_result = (*mChildren[1])(rEnv);
    }
    else
    {
        p_result = (*mChildren[2])(rEnv);
    }
    return p_result;
}
