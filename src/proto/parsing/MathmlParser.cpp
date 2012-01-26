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

#include "MathmlParser.hpp"

#include "XmlTools.hpp"

#include "ProtocolLanguage.hpp"
#include "ProtoHelperMacros.hpp"
#include "BacktraceException.hpp"
#include "TaggingDomParser.hpp"



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
        // Built-in operation: fold, map, newArray, view, find, index, tuple, or accessor
        std::string symbol = GetCsymbolName(p_operator);
        std::vector<AbstractExpressionPtr> operands = ParseOperands(pElement);
        if (symbol == "fold")
        {
            p_expr.reset(new Fold(operands));
        }
        else if (symbol == "map")
        {
            p_expr.reset(new Map(operands));
        }
        else if (symbol == "newArray")
        {
            std::vector<DOMElement*> children = XmlTools::GetChildElements(pElement);
            if (children.size() == operands.size() + 2)
            {
                // Array comprehension
                DOMElement* p_doa = children[1];
                PROTO_ASSERT(X2C(p_doa->getLocalName()) == "domainofapplication",
                             "An array comprehension must have a domainofapplication.");
                PROTO_ASSERT(operands.size() == 1, "An array comprehension must have only one operand.");
                std::vector<DOMElement*> range_spec_elts = XmlTools::GetChildElements(p_doa);
                std::vector<AbstractExpressionPtr> range_specs;
                range_specs.reserve(range_spec_elts.size());
                for (std::vector<DOMElement*>::iterator it = range_spec_elts.begin(); it != range_spec_elts.end(); ++it)
                {
                    range_specs.push_back(ParseExpression(*it));
                }
                p_expr.reset(new ArrayCreate(operands.front(), range_specs));
            }
            else
            {
                p_expr.reset(new ArrayCreate(operands));
            }
        }
        else if (symbol == "view")
        {
            PROTO_ASSERT(operands.size() > 1,
                         "A view operation must include the array and at least one range specification.");
            AbstractExpressionPtr p_array = operands[0];
            std::vector<AbstractExpressionPtr> ranges(++operands.begin(), operands.end());
            p_expr.reset(new View(p_array, ranges));
        }
        else if (symbol == "find")
        {
            PROTO_ASSERT(operands.size() == 1, "Find takes only one operand.");
            p_expr.reset(new Find(operands.front()));
        }
        else if (symbol == "index")
        {
            p_expr.reset(new Index(operands));
        }
        else if (symbol == "accessor")
        {
            PROTO_ASSERT(operands.size() == 1, "The accessor csymbol takes 1 operand.");
            Accessor::Attribute attr = Accessor::DecodeAttributeString(X2C(p_operator->getTextContent()),
                                                                       GetLocationInfo());
            p_expr.reset(new Accessor(operands[0], attr));
        }
        else if (symbol == "tuple")
        {
            p_expr.reset(new TupleExpression(operands));
        }
        else
        {
            PROTO_EXCEPTION("Application of csymbol " << symbol << " is not recognised.");
        }
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
#define ITEM(cls)  p_expr.reset(new cls(operands));
        MATHML_OPERATOR_TABLE(ITEM, op_name)
#undef ITEM
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


MathmlParser::MathmlParser()
    : mMathmlNs("http://www.w3.org/1998/Math/MathML"),
      mCsymbolBaseUrl("https://chaste.cs.ox.ac.uk/nss/protocol/")
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


std::string MathmlParser::GetCsymbolName(const DOMElement* pElement)
{
    std::string definition_url = X2C(pElement->getAttribute(X("definitionURL")));
    PROTO_ASSERT(!definition_url.empty(), "All csymbol elements must have a definitionURL.");
    const size_t base_len = mCsymbolBaseUrl.length();
    PROTO_ASSERT(definition_url.substr(0, base_len) == mCsymbolBaseUrl,
                 "All csymbol elements must have a definitionURL commencing with " << mCsymbolBaseUrl
                 << "; " << definition_url << " does not match.");
    return definition_url.substr(base_len);
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
