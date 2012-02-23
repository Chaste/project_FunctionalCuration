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

#include "MathmlParser.hpp"

#include <boost/foreach.hpp>

#include "XmlTools.hpp"

#include "ProtocolLanguage.hpp"
#include "ProtoHelperMacros.hpp"
#include "BacktraceException.hpp"
#include "TaggingDomParser.hpp"


void MathmlParser::SetUseImplicitMap(bool useImplicitMap)
{
    mUseImplicitMap = useImplicitMap;
}


double MathmlParser::ParseNumber(const DOMElement* pElement)
{
    SetContext(pElement);
    std::string text_value = X2C(pElement->getTextContent());
    return String2Double(text_value);
}


AbstractExpressionPtr MathmlParser::ParseExpression(const DOMElement* pElement)
{
    SetContext(pElement);
    AbstractExpressionPtr p_expr;
    PROTO_ASSERT(X2C(pElement->getNamespaceURI()) == mMathmlNs, "Expressions must be MathML elements.");
    std::string name = X2C(pElement->getLocalName());
    if (name == "cn")
    {
        p_expr = VALUE(SimpleValue, ParseNumber(pElement));
    }
    else if (name == "ci")
    {
        std::string text_value = X2C(pElement->getTextContent());
        p_expr = LOOKUP(text_value);
    }
    else if (name == "csymbol")
    {
        p_expr = ParseCsymbolExpression(pElement);
    }
    else if (name == "lambda")
    {
        p_expr = ParseFunctionDefinition(pElement);
    }
    else if (name == "apply")
    {
        p_expr = ParseApply(pElement);
    }
    else if (name == "piecewise")
    {
        p_expr = ParsePiecewise(pElement);
    }
    else if (name == "degree" || name == "logbase")
    {
        // It's just a wrapper element
        p_expr = ParseExpression(GetFirstChild(pElement));
    }
    else
    {
        PROTO_EXCEPTION("Element name " << name << " is not a valid expression.");
    }
    TransferContext(pElement, p_expr);
    return p_expr;
}


AbstractExpressionPtr MathmlParser::ParseApply(const DOMElement* pElement)
{
    SetContext(pElement);
    AbstractExpressionPtr p_expr;
    DOMElement* p_operator = GetOperator(pElement);
    std::string op_name = X2C(p_operator->getLocalName());
    if (op_name == "csymbol")
    {
        p_expr = ParseCsymbolApply(pElement, p_operator);
    }
    else if (op_name == "ci" || op_name == "lambda" || op_name == "piecewise" || op_name == "apply")
    {
        // Function call
        AbstractExpressionPtr fn_expr = ParseExpression(p_operator);
        std::vector<AbstractExpressionPtr> fn_args = ParseOperands(pElement);
        p_expr.reset(new FunctionCall(fn_expr, fn_args));
    }
    else
    {
        // Better be a MathML operator that we understand
        std::vector<AbstractExpressionPtr> operands = ParseOperands(pElement);
        if (!mUseImplicitMap || operands.empty())
        {
#define ITEM(cls)  p_expr.reset(new cls(operands));
            MATHML_OPERATOR_TABLE(ITEM, op_name)
#undef ITEM
        }
        else
        {
            // Wrap the MathML in an implicit map
            AbstractExpressionPtr p_func = WrapMathml(op_name, operands.size());
            std::vector<AbstractExpressionPtr> new_operands(1, p_func);
            BOOST_FOREACH(AbstractExpressionPtr p_op, operands) new_operands.push_back(p_op);
            p_expr.reset(new Map(new_operands, true));
        }
    }
    TransferContext(pElement, p_expr);
    return p_expr;
}


std::vector<AbstractExpressionPtr> MathmlParser::ParseOperands(const DOMElement* pElement)
{
    std::vector<DOMElement*> children = XmlTools::GetChildElements(pElement);
    std::vector<AbstractExpressionPtr> operands;
    operands.reserve(children.size()-1);
    for (std::vector<DOMElement*>::iterator it = ++children.begin(); it != children.end(); ++it)
    {
        if (!IsQualifier(*it))
        {
            operands.push_back(ParseExpression(*it));
        }
    }
    SetContext(pElement);
    return operands;
}


bool MathmlParser::IsQualifier(const DOMElement* pElement)
{
    std::string elt_name = X2C(pElement->getLocalName());
    return (elt_name == "bvar" || elt_name == "domainofapplication");
}


AbstractExpressionPtr MathmlParser::ParsePiecewise(const DOMElement* pElement)
{
    SetContext(pElement);
    std::vector<DOMElement*> children = XmlTools::GetChildElements(pElement);
    PROTO_ASSERT(children.size() == 2, "A piecewise element must have exactly 2 children, not "
                 << children.size() << ".");
    PROTO_ASSERT(X2C(children.front()->getLocalName()) == "piece",
                 "A piecewise element must have a piece element as its first child.");
    PROTO_ASSERT(X2C(children.back()->getLocalName()) == "otherwise",
                 "A piecewise element must have an otherwise element as its second child.");
    // Get the test & then part from the piece element
    SetContext(children.front());
    std::vector<DOMElement*> piece_children = XmlTools::GetChildElements(children.front());
    PROTO_ASSERT(piece_children.size() == 2, "A piece element must have exactly 2 children, not "
                 << piece_children.size() << ".");
    AbstractExpressionPtr p_test = ParseExpression(piece_children.back());
    AbstractExpressionPtr p_then = ParseExpression(piece_children.front());
    // Get the else part from the otherwise element
    SetContext(children.back());
    std::vector<DOMElement*> otherwise_children = XmlTools::GetChildElements(children.back());
    PROTO_ASSERT(otherwise_children.size() == 1, "An otherwise element must have exactly 1 child, not "
                 << otherwise_children.size() << ".");
    AbstractExpressionPtr p_else = ParseExpression(otherwise_children.front());
    // Create the result
    AbstractExpressionPtr p_expr = IF(p_test, p_then, p_else);
    TransferContext(pElement, p_expr);
    return p_expr;
}


AbstractExpressionPtr MathmlParser::ParseFunctionDefinition(const DOMElement* pElement)
{
    std::vector<std::string> parameter_names;
    DOMElement* p_body = NULL;
    std::vector<DOMElement*> children = XmlTools::GetChildElements(pElement);
    for (std::vector<DOMElement*>::iterator it = children.begin(); it != children.end(); ++it)
    {
        DOMElement* p_child = *it;
        std::string child_name = X2C(p_child->getLocalName());
        if (child_name == "bvar")
        {
            // <bvar><ci>NAME</ci></bvar>
            parameter_names.push_back(ParseBvar(p_child));
        }
        else
        {
            SetContext(pElement);
            p_body = p_child;
            PROTO_ASSERT(p_child == children.back(), "Unexpected extra content in lambda element.");
            break;
        }
    }
    SetContext(pElement);
    PROTO_ASSERT(p_body != NULL, "No body found in lambda expression.");
    AbstractExpressionPtr p_body_expr = ParseExpression(p_body);
    AbstractExpressionPtr p_expr(new LambdaExpression(parameter_names, p_body_expr));
    TransferContext(pElement, p_expr);
    return p_expr;
}


std::string MathmlParser::ParseBvar(const DOMElement* pElement)
{
    SetContext(pElement);
    DOMNodeList* p_ci_elts = pElement->getElementsByTagNameNS(X(mMathmlNs), X("ci"));
    PROTO_ASSERT(p_ci_elts->getLength() == 1, "A bvar element should have only one child.");
    return X2C(p_ci_elts->item(0)->getTextContent());
}


AbstractExpressionPtr MathmlParser::WrapMathml(const std::string& rOperator, unsigned numArgs)
{
    AbstractExpressionPtr p_expr;
#define ITEM(cls)  p_expr = LambdaExpression::WrapMathml<cls>(numArgs);
    MATHML_OPERATOR_TABLE(ITEM, rOperator)
#undef ITEM
    return p_expr;
}


MathmlParser::MathmlParser()
    : mMathmlNs("http://www.w3.org/1998/Math/MathML"),
      mUseImplicitMap(false)
{}

MathmlParser::~MathmlParser()
{}


void MathmlParser::SetContext(const DOMNode* pNode)
{
    const TaggingDomParser::Tag* p_tag = TaggingDomParser::GetTag(pNode);
    assert(p_tag != NULL);
    std::stringstream loc;
    loc << p_tag->mSystemId << ":" << p_tag->mLineNumber << ":" << p_tag->mColumnNumber
            << "\t" << X2C(pNode->getLocalName());
    mLocationInfo = loc.str();
}


void MathmlParser::TransferContext(const DOMNode* pNode, boost::shared_ptr<LocatableConstruct> pConstruct)
{
    SetContext(pNode);
    pConstruct->SetLocationInfo(GetLocationInfo());
    if (pNode->getNodeType() == DOMNode::ELEMENT_NODE)
    {
        const DOMElement* p_elt = static_cast<const DOMElement*>(pNode);
        if (p_elt->hasAttributeNS(X("https://chaste.cs.ox.ac.uk/nss/protocol/0.1#"), X("trace")))
        {
            pConstruct->SetTrace();
        }
    }
}


std::string MathmlParser::GetLocationInfo()
{
    return mLocationInfo;
}


DOMElement* MathmlParser::GetOperator(const DOMElement* pElement)
{
    assert(X2C(pElement->getLocalName()) == "apply");
    return GetFirstChild(pElement);
}


DOMElement* MathmlParser::GetFirstChild(const DOMNode* pElement)
{
    DOMNode* p_child = pElement->getFirstChild();
    while (p_child != NULL && p_child->getNodeType() != DOMNode::ELEMENT_NODE)
    {
        p_child = p_child->getNextSibling();
    }
    if (p_child == NULL)
    {
        PROTO_EXCEPTION("No child element found.");
    }
    return static_cast<DOMElement*>(p_child);
}


double MathmlParser::String2Double(const std::string& rStr)
{
    std::istringstream stream(rStr);
    double number;
    stream >> number;
    if (stream.fail())
    {
        PROTO_EXCEPTION("String '" << rStr << "' could not be interpreted as a floating point number.");
    }
    return number;
}


unsigned MathmlParser::String2Unsigned(const std::string& rStr)
{
    std::istringstream stream(rStr);
    unsigned number;
    stream >> number;
    if (stream.fail())
    {
        PROTO_EXCEPTION("String '" << rStr << "' could not be interpreted as a non-negative integer.");
    }
    return number;
}


bool MathmlParser::String2Bool(const std::string& rStr)
{
    bool value = false;
    if (rStr == "true" || rStr == "1")
    {
        value = true;
    }
    else if (rStr != "false" && rStr != "0")
    {
        PROTO_EXCEPTION("String '" << rStr << "' is not a valid boolean value.");
    }
    return value;
}
