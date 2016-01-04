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
    return TraceResult(p_result);
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
