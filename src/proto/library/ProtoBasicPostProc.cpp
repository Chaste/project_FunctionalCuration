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

#include "ProtoBasicPostProc.hpp"

#include "ProtocolLanguage.hpp"
#include "ProtoHelperMacros.hpp"

using boost::make_shared;
using boost::assign::list_of;

void ProtoBasicPostProc::DefineAll(Environment& rEnv)
{
    DefineDiff(rEnv);
}

/*
 * def diff(array, dim=array.num_dims-1):
 *     map(-, array[<dim>1:], array[<dim>:-1])
 */
void ProtoBasicPostProc::DefineDiff(Environment& rEnv)
{
    std::vector<AbstractStatementPtr> body;
    std::vector<AbstractExpressionPtr> args;

    // Test arguments
    DEFINE_STMT(is_array, ASSERT_STMT(make_shared<Accessor>(LOOKUP("array"), Accessor::IS_ARRAY)));
    body.push_back(is_array);
    args = EXPR_LIST(make_shared<Accessor>(LOOKUP("array"), Accessor::NUM_DIMS))(CONST(1));
    DEFINE(default_dim, make_shared<MathmlMinus>(args));
    DEFINE(dim_if, IF(make_shared<Accessor>(LOOKUP("dim'"), Accessor::IS_DEFAULT), default_dim, LOOKUP("dim'")));
    DEFINE_STMT(dim_def, ASSIGN_STMT("dim", dim_if));
    body.push_back(dim_def);
    DEFINE_STMT(is_num, ASSERT_STMT(make_shared<Accessor>(LOOKUP("dim"), Accessor::IS_SIMPLE_VALUE)));
    body.push_back(is_num);
    args = EXPR_LIST(LOOKUP("dim"))(make_shared<Accessor>(LOOKUP("array"), Accessor::NUM_DIMS));
    DEFINE(dim_lt, make_shared<MathmlLt>(args));
    DEFINE_STMT(dim_ok, ASSERT_STMT(dim_lt));
    body.push_back(dim_ok);

    // Calculate result
    DEFINE_TUPLE(default_range, EXPR_LIST(NULL_EXPR)(NULL_EXPR)(CONST(1))(NULL_EXPR));
    DEFINE_TUPLE(range1, EXPR_LIST(LOOKUP("dim"))(CONST(1))(CONST(1))(NULL_EXPR));
    std::vector<AbstractExpressionPtr> ranges = EXPR_LIST(range1)(default_range);
    DEFINE(view1, make_shared<View>(LOOKUP("array"), ranges));

    DEFINE_TUPLE(range2, EXPR_LIST(LOOKUP("dim"))(CONST(0))(CONST(1))(CONST(-1)));
    ranges = EXPR_LIST(range2)(default_range);
    DEFINE(view2, make_shared<View>(LOOKUP("array"), ranges));

    DEFINE(minus, LambdaExpression::WrapMathml<MathmlMinus>(2));
    args = EXPR_LIST(minus)(view1)(view2);
    DEFINE(map, make_shared<Map>(args));
    DEFINE_STMT(ret, RETURN_STMT(map));
    body.push_back(ret);

    // Create function
    std::vector<std::string> fps = list_of("array")("dim'");
    std::vector<AbstractValuePtr> defaults(2);
    defaults[1] = make_shared<DefaultParameter>();
    DEFINE(diff, make_shared<LambdaExpression>(fps, body, defaults));

    DEFINE_STMT(defn, ASSIGN_STMT("diff", diff));
    rEnv.ExecuteStatement(defn);
}
