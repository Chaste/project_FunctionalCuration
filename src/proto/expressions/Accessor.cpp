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

#include "Accessor.hpp"

#include <boost/assign/list_of.hpp>
#include <boost/make_shared.hpp>
#include "ValueTypes.hpp"
#include "NdArray.hpp"
#include "ArrayCreate.hpp"
#include "BacktraceException.hpp"
#include "ProtoHelperMacros.hpp"
#include "PetscTools.hpp"

Accessor::Accessor(const AbstractExpressionPtr pExpression, Attribute attribute)
    : AbstractExpression(std::vector<AbstractExpressionPtr>(1, pExpression)),
      mAttribute(attribute)
{}


AbstractValuePtr Accessor::operator()(const Environment& rEnv) const
{
    AbstractValuePtr p_value = EvaluateChildren(rEnv).front();
    AbstractValuePtr p_result;
    switch (mAttribute)
    {
        case IS_SIMPLE_VALUE:
            p_result = boost::make_shared<SimpleValue>(p_value->IsDouble());
            break;

        case IS_ARRAY:
            p_result = boost::make_shared<SimpleValue>(p_value->IsArray());
            break;

        case IS_STRING:
            p_result = boost::make_shared<SimpleValue>(p_value->IsString());
            break;

        case IS_FUNCTION:
            p_result = boost::make_shared<SimpleValue>(p_value->IsLambda());
            break;

        case IS_TUPLE:
            p_result = boost::make_shared<SimpleValue>(p_value->IsTuple());
            break;

        case IS_NULL:
            p_result = boost::make_shared<SimpleValue>(p_value->IsNull());
            break;

        case IS_DEFAULT:
            p_result = boost::make_shared<SimpleValue>(p_value->IsDefault());
            break;

        case NUM_DIMS:
            PROTO_ASSERT(p_value->IsArray(), "Cannot get the number of dimensions of a non-array.");
            p_result = boost::make_shared<SimpleValue>(GET_ARRAY(p_value).GetNumDimensions());
            break;

        case NUM_ELEMENTS:
            PROTO_ASSERT(p_value->IsArray(), "Cannot get the number of elements of a non-array.");
            p_result = boost::make_shared<SimpleValue>(GET_ARRAY(p_value).GetNumElements());
            break;

        case SHAPE:
        {
            PROTO_ASSERT(p_value->IsArray(), "Cannot get the shape of a non-array.");
            NdArray<double>::Extents shape = GET_ARRAY(p_value).GetShape();
            std::vector<AbstractExpressionPtr> shape_elements;
            for (NdArray<double>::Extents::const_iterator it=shape.begin(); it != shape.end(); ++it)
            {
                shape_elements.push_back(CONST(*it));
            }
            ArrayCreate expr(shape_elements);
            expr.SetLocationInfo(GetLocationInfo());
            p_result = expr(rEnv);
            break;
        }

        default:
            NEVER_REACHED;
    }
    return p_result;
}


Accessor::Attribute Accessor::DecodeAttributeString(const std::string& rAttribute,
                                                    const std::string& rLocInfo)
{
    Attribute attr;
#define CASE(attr_name)  if (rAttribute == #attr_name) attr = attr_name
    CASE(IS_SIMPLE_VALUE);
    else CASE(IS_ARRAY);
    else CASE(IS_STRING);
    else CASE(IS_FUNCTION);
    else CASE(IS_TUPLE);
    else CASE(IS_NULL);
    else CASE(IS_DEFAULT);
    else CASE(NUM_DIMS);
    else CASE(NUM_ELEMENTS);
    else CASE(SHAPE);
    else
    {
        PROTO_EXCEPTION2("Unrecognised attribute name " << rAttribute << ".", rLocInfo);
    }
    return attr;
}
