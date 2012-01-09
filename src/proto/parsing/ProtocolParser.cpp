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

#include "ProtocolParser.hpp"

#include <sstream>
#include <iterator>
#include <algorithm>
#include <boost/foreach.hpp>

#include "XmlTools.hpp"

#include "ProtocolLanguage.hpp"
#include "ProtoHelperMacros.hpp"
#include "BacktraceException.hpp"
#include "TaggingDomParser.hpp"
#include "AbstractSimulation.hpp"
#include "NestedSimulation.hpp"
#include "TimecourseSimulation.hpp"
#include "NestedProtocol.hpp"
#include "AbstractModifier.hpp"
#include "ModelResetModifier.hpp"
#include "StateSaverModifier.hpp"
#include "SetVariableModifier.hpp"
#include "ModifierCollection.hpp"
#include "AbstractStepper.hpp"
#include "UniformStepper.hpp"
#include "VectorStepper.hpp"
#include "WhileStepper.hpp"

#include <xercesc/dom/DOM.hpp>
#include <xsd/cxx/xml/sax/bits/error-handler-proxy.hxx>
using namespace xercesc;

/**
 * Implementation of the protocol file parser, containing methods to parse all the different
 * constructs.  We use a separate class for this, rather than putting all the methods in the
 * ProtocolParser class, to reduce re-compiling for users of the parser.
 */
class ProtocolParserImpl : boost::noncopyable
{
public:
    /**
     * Create the object containing the parsed protocol.
     *
     * @param pRootElement  the root of the protocol XML tree
     */
    ProtocolPtr CreateProtocolObject(DOMElement* pRootElement)
    {
        ProtocolPtr p_proto(new Protocol);
        SetProtocolObject(p_proto);
        StoreNamespaceMappings(pRootElement);
        return p_proto;
    }

    /**
     * Set the protocol object that parsed language constructs will be added to.
     * @param pProto  the protocol object
     */
    void SetProtocolObject(ProtocolPtr pProto)
    {
        mpCurrentProtocolObject = pProto;
    }

    /**
     * Parse a nested protocol definition.
     *
     * @param pDefnElt  the defining element
     */
    AbstractSimulationPtr ParseNestedProtocol(DOMElement* pDefnElt)
    {
        ProtocolPtr p_proto = ImportProtocol(pDefnElt);
        // Input specifications for imported protocol
        std::vector<DOMElement*> inputs = XmlTools::FindElements(pDefnElt, "setInput");
        std::map<std::string, AbstractExpressionPtr> input_specs;
        BOOST_FOREACH(DOMElement* p_input, inputs)
        {
            SetContext(p_input);
            PROTO_ASSERT(p_input->hasAttribute(X("name")), "A setInput element must specify which input to set.");
            std::string input_name = X2C(p_input->getAttribute(X("name")));
            AbstractExpressionPtr p_input_value = ParseNumberOrExpression(p_input);
            PROTO_ASSERT(!input_specs[input_name], "Duplicate definition for nested protocol input");
            input_specs[input_name] = p_input_value;
        }
        // Output specifications for imported protocol
        std::vector<DOMElement*> outputs = XmlTools::FindElements(pDefnElt, "selectOutput");
        std::vector<std::string> output_specs;
        BOOST_FOREACH(DOMElement* p_output, outputs)
        {
            SetContext(p_output);
            PROTO_ASSERT(p_output->hasAttribute(X("name")), "A selectOutput element must specify an output name.");
            std::string output_name = X2C(p_output->getAttribute(X("name")));
            output_specs.push_back(output_name);
        }
        AbstractSimulationPtr p_sim(new NestedProtocol(p_proto, input_specs, output_specs));
        return p_sim;
    }

    /**
     * Import the protocol specified by the given element.
     *
     * @param pImportElt  the element defining the import
     */
    ProtocolPtr ImportProtocol(DOMElement* pImportElt)
    {
        PROTO_ASSERT(pImportElt->hasAttribute(X("source")), "Imports must have a source attribute.");
        std::string source_uri = X2C(pImportElt->getAttribute(X("source")));
        ///\todo handle absolute paths?
        FileFinder imported_proto_file(source_uri, mpCurrentProtocolObject->rGetSourceFile());
        return mrParser.ParseFile(imported_proto_file);
    }

    /**
     * Parse a global import element, load the imported protocol, and merge constituents
     * as required.
     *
     * @param pImportElt  the import definition
     */
    void ParseImport(DOMElement* pImportElt)
    {
        SetContext(pImportElt);
        ProtocolPtr p_imported_proto = ImportProtocol(pImportElt);
        // Set inputs for imported protocol
        std::vector<DOMElement*> inputs = XmlTools::FindElements(pImportElt, "setInput");
        BOOST_FOREACH(DOMElement* p_input, inputs)
        {
            SetContext(p_input);
            PROTO_ASSERT(p_input->hasAttribute(X("name")), "A setInput element must specify which input to set.");
            std::string input_name = X2C(p_input->getAttribute(X("name")));
            AbstractExpressionPtr p_input_value = ParseNumberOrExpression(p_input);
            p_imported_proto->SetInput(input_name, p_input_value);
        }
        SetContext(pImportElt);
        // Merge or store protocol
        if (pImportElt->hasAttribute(X("prefix")))
        {
            mpCurrentProtocolObject->AddNamespaceBindings(p_imported_proto->rGetNamespaceBindings());
            mpCurrentProtocolObject->AddImport(X2C(pImportElt->getAttribute(X("prefix"))), p_imported_proto,
                                               GetLocationInfo());
        }
        else if (pImportElt->hasAttribute(X("mergeDefinitions")))
        {
            std::string merge = X2C(pImportElt->getAttribute(X("mergeDefinitions")));
            PROTO_ASSERT(merge == "true" || merge == "1",
                         "An import must define a prefix or merge definitions.");
            mpCurrentProtocolObject->AddNamespaceBindings(p_imported_proto->rGetNamespaceBindings());
            mpCurrentProtocolObject->AddInputDefinitions(p_imported_proto->rGetInputStatements());
            mpCurrentProtocolObject->AddLibrary(p_imported_proto->rGetLibraryStatements());
            mpCurrentProtocolObject->AddSimulations(p_imported_proto->rGetSimulations());
            mpCurrentProtocolObject->AddPostProcessing(p_imported_proto->rGetPostProcessing());
            mpCurrentProtocolObject->AddOutputSpecs(p_imported_proto->rGetOutputSpecifications());
            mpCurrentProtocolObject->AddDefaultPlots(p_imported_proto->rGetPlotSpecifications());
            // Merged simulations need to delegate to this protocol's library
            Environment& r_library = mpCurrentProtocolObject->rGetLibrary();
            BOOST_FOREACH(AbstractSimulationPtr p_sim, p_imported_proto->rGetSimulations())
            {
                p_sim->rGetEnvironment().SetDelegateeEnvironment(r_library.GetAsDelegatee());
            }
        }
        else
        {
            PROTO_EXCEPTION("An import must define a prefix or merge definitions.");
        }
    }

    template<typename OBJ_VEC>
    bool CheckUseImports(DOMElement* pElt,
                         OBJ_VEC& (Protocol::*pGetter)(),
                         void (Protocol::*pAdder)(const OBJ_VEC&))
    {
        bool matched = (X2C(pElt->getLocalName()) == "useImports");
        if (matched)
        {
            SetContext(pElt);
            PROTO_ASSERT(pElt->hasAttribute(X("prefix")),
                         "A useImports element must have a prefix attribute.");
            const std::string prefix = X2C(pElt->getAttribute(X("prefix")));
            ProtocolPtr p_imported_proto = mpCurrentProtocolObject->GetImportedProtocol(prefix);
            PROTO_ASSERT(p_imported_proto, "No protocol imported with the prefix " << prefix);
            OBJ_VEC& r_items = (p_imported_proto.get()->*pGetter)();
            (mpCurrentProtocolObject.get()->*pAdder)(r_items);
        }
        return matched;
    }

    template<typename OBJ_VEC>
    bool CheckUseImportsVec(DOMElement* pElt, OBJ_VEC& (Protocol::*pGetter)(), OBJ_VEC& rDestVec)
    {
        bool matched = (X2C(pElt->getLocalName()) == "useImports");
        if (matched)
        {
            SetContext(pElt);
            PROTO_ASSERT(pElt->hasAttribute(X("prefix")),
                         "A useImports element must have a prefix attribute.");
            const std::string prefix = X2C(pElt->getAttribute(X("prefix")));
            ProtocolPtr p_imported_proto = mpCurrentProtocolObject->GetImportedProtocol(prefix);
            PROTO_ASSERT(p_imported_proto, "No protocol imported with the prefix " << prefix);
            OBJ_VEC& r_items = (p_imported_proto.get()->*pGetter)();
            std::copy(r_items.begin(), r_items.end(), std::back_inserter(rDestVec));
        }
        return matched;
    }

    /**
     * Parse a cn element to obtain a constant number.
     *
     * \todo Cope with base, sep, etc.
     * \todo Make all these methods take const DOMElement* ?
     *
     * @param pElement  the cn element
     */
    double ParseNumber(DOMElement* pElement)
    {
        SetContext(pElement);
        std::string text_value = X2C(pElement->getTextContent());
        return String2Double(text_value);
    }

    /**
     * Parse an element that may contain either a constant number or an expression child element.
     *
     * @param pElement  the element
     */
    AbstractExpressionPtr ParseNumberOrExpression(DOMElement* pElement)
    {
        SetContext(pElement);
        AbstractExpressionPtr p_result;
        std::vector<DOMElement*> children = XmlTools::GetChildElements(pElement);
        if (children.empty())
        {
            // Number
            p_result = CONST(ParseNumber(pElement));
        }
        else
        {
            // Expression
            PROTO_ASSERT(children.size() == 1, "Only one expression is allowed here.");
            p_result = ParseExpression(children.front());
        }
        TransferContext(pElement, p_result);
        return p_result;
    }

    /**
     * Do "LambdaExpression::WrapMathml<OperatorClass>(numArgs)" dealing with converting a run-time
     * string into a compile-time type.
     *
     * @param rOperator  the MathML operator name
     * @param numArgs  the number of arguments it should take
     */
    AbstractExpressionPtr WrapMathml(const std::string& rOperator, unsigned numArgs)
    {
        AbstractExpressionPtr p_expr;
#define ITEM(cls)  p_expr = LambdaExpression::WrapMathml<cls>(numArgs);
        MATHML_OPERATOR_TABLE(ITEM, rOperator)
#undef ITEM
        return p_expr;
    }

    /**
     * Parse a csymbol element representing a whole expression (as opposed to an operator).
     *
     * @param pElement  the csymbol element
     */
    AbstractExpressionPtr ParseCsymbolExpression(DOMElement* pElement)
    {
        SetContext(pElement);
        AbstractExpressionPtr p_expr;
        std::string name = GetCsymbolName(pElement);
        if (name == "defaultParameter")
        {
            p_expr = DEFAULT_EXPR;
        }
        else if (name == "null")
        {
            p_expr = NULL_EXPR;
        }
        else if (name == "string")
        {
            std::string content = X2C(pElement->getTextContent());
            p_expr = VALUE(StringValue, content);
        }
        else if (name.substr(0, 5) == "wrap/")
        {
            std::string operator_name = X2C(pElement->getTextContent());
            unsigned num_args = String2Unsigned(name.substr(5));
            p_expr = WrapMathml(operator_name, num_args);
        }
        TransferContext(pElement, p_expr);
        return p_expr;
    }

    /**
     * Parse a MathML bvar element to obtain the name referenced in the enclosed ci element.
     *
     * @param pElement  the bvar element
     */
    std::string ParseBvar(DOMElement* pElement)
    {
        SetContext(pElement);
        DOMNodeList* p_ci_elts = pElement->getElementsByTagNameNS(X(mMathmlNs), X("ci"));
        PROTO_ASSERT(p_ci_elts->getLength() == 1, "A bvar element should have only one child.");
        return X2C(p_ci_elts->item(0)->getTextContent());
    }

    /**
     * Parse a MathML lambda element into a LambdaExpression.
     *
     * Its content should consist of a sequence of bvar or semantics elements defining the parameters,
     * then a single element which parses as either an expression or a statement list.  This forms the
     * function body.
     *
     * @param pElement  the lambda element
     */
    AbstractExpressionPtr ParseFunctionDefinition(DOMElement* pElement)
    {
        SetContext(pElement);
        AbstractExpressionPtr p_expr;
        std::vector<AbstractValuePtr> default_values;
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
            else if (child_name == "semantics")
            {
                // <semantics><bvar><ci>NAME</ci></bvar><annotation-xml>VALUE</annotation-xml></semantics>
                SetContext(p_child);
                DOMNodeList* p_bvar_elts = p_child->getElementsByTagNameNS(X(mMathmlNs), X("bvar"));
                PROTO_ASSERT(p_bvar_elts->getLength() == 1,
                             "A semantics element should have only one bvar child.");
                parameter_names.push_back(ParseBvar(static_cast<DOMElement*>(p_bvar_elts->item(0))));
                SetContext(p_child);
                DOMNodeList* p_anno_elts = p_child->getElementsByTagNameNS(X(mMathmlNs), X("annotation-xml"));
                PROTO_ASSERT(p_anno_elts->getLength() == 1,
                             "A semantics element should have only one annotation-xml child.");
                SetContext(p_anno_elts->item(0));
                DOMElement* p_value_elt = GetFirstChild(p_anno_elts->item(0));
                AbstractValuePtr p_value = ParseValue(p_value_elt);
                default_values.resize(parameter_names.size());
                default_values.back() = p_value;
            }
            else
            {
                p_body = p_child;
                PROTO_ASSERT(p_child == children.back(), "Unexpected extra content in lambda element.");
                break;
            }
        }
        SetContext(pElement);
        PROTO_ASSERT(p_body != NULL, "No body found in lambda expression.");
        if (IsStatementList(p_body))
        {
            std::vector<AbstractStatementPtr> body_stmts = ParseStatementList(p_body);
            p_expr.reset(new LambdaExpression(parameter_names, body_stmts, default_values));
        }
        else
        {
            AbstractExpressionPtr p_body_expr = ParseExpression(p_body);
            p_expr.reset(new LambdaExpression(parameter_names, p_body_expr, default_values));
        }
        TransferContext(pElement, p_expr);
        return p_expr;
    }

    /**
     * Parse an XML element to obtain a protocol language value.
     * Used for declaring default values for function parameters.
     *
     * @param pElement  the element to parse
     */
    AbstractValuePtr ParseValue(DOMElement* pElement)
    {
        SetContext(pElement);
        AbstractValuePtr p_value;
        PROTO_ASSERT(X2C(pElement->getNamespaceURI()) == mMathmlNs, "Values must be MathML elements.");
        std::string elt_name = X2C(pElement->getLocalName());
        if (elt_name == "cn")
        {
            p_value.reset(new SimpleValue(ParseNumber(pElement)));
        }
        else if (elt_name == "csymbol")
        {
            std::string sym_name = GetCsymbolName(pElement);
            if (sym_name == "defaultParameter")
            {
                p_value.reset(new DefaultParameter);
            }
            else if (sym_name == "null")
            {
                p_value.reset(new NullValue);
            }
            else if (sym_name == "string")
            {
                std::string content = X2C(pElement->getTextContent());
                p_value.reset(new StringValue(content));
            }
            else
            {
                PROTO_EXCEPTION("Unrecognised csymbol " << sym_name << " for constant value.");
            }
        }
        else
        {
            PROTO_EXCEPTION("Unrecognised element " << elt_name << " for constant value.");
        }
        return p_value;
    }

    /**
     * Check whether this is an application of the statementList csymbol.
     *
     * @param pElement  the element to check
     */
    bool IsStatementList(DOMElement* pElement)
    {
        SetContext(pElement);
        bool is_stmt_list = false;
        if (X2C(pElement->getLocalName()) == "apply")
        {
            DOMElement* p_child = GetFirstChild(pElement);
            if (X2C(p_child->getLocalName()) == "csymbol")
            {
                SetContext(p_child);
                std::string name = GetCsymbolName(p_child);
                if (name == "statementList")
                {
                    is_stmt_list = true;
                }
            }
        }
        SetContext(pElement);
        return is_stmt_list;
    }
    /**
     * Parse a MathML piecewise element to obtain an "if" expression.
     *
     * Only one piece child is allowed (at present).
     *
     * @param pElement  the piecewise element
     */
    AbstractExpressionPtr ParsePiecewise(DOMElement* pElement)
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

    /**
     * Parse a MathML apply element to obtain an expression.
     *
     * The operator should be a ci, csymbol, or MathML operator.
     *
     * @param pElement  the apply element
     */
    AbstractExpressionPtr ParseApply(DOMElement* pElement)
    {
        SetContext(pElement);
        AbstractExpressionPtr p_expr;
        DOMElement* p_operator = GetOperator(pElement);
        std::string op_name = X2C(p_operator->getLocalName());
        if (op_name == "ci")
        {
            // Function call, by name
            ///\todo we could actually merge this with the anonymous call block below, since
            /// parsing a ci gives a lookup expression which is what this shortcut expands to.
            std::string fn_name = X2C(p_operator->getTextContent());
            std::vector<AbstractExpressionPtr> fn_args = ParseOperands(pElement);
            p_expr.reset(new FunctionCall(fn_name, fn_args));
        }
        else if (op_name == "csymbol")
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
        else if (op_name == "lambda" || op_name == "piecewise" || op_name == "apply")
        {
            // Anonymous function call
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

    /**
     * Parse the operands of an apply element.
     *
     * Note: we pretend that 'degree' and 'logbase' elements are operands rather than qualifiers.
     *
     * @param pElement  the apply element
     */
    std::vector<AbstractExpressionPtr> ParseOperands(DOMElement* pElement)
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

    /**
     * Determine whether a given element is a qualifier.
     * Just tests if the element name is bvar or domainofapplication, since they
     * are the only qualifiers we support.
     *
     * @param pElement  the element to test
     */
    bool IsQualifier(const DOMElement* pElement)
    {
        std::string elt_name = X2C(pElement->getLocalName());
        return (elt_name == "bvar" || elt_name == "domainofapplication");
    }

    /**
     * Parse a MathML element encoding an expression.
     *
     * @param pElement  the element
     */
    AbstractExpressionPtr ParseExpression(DOMElement* pElement)
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

    /**
     * Parse a MathML element representing a language statement.
     *
     * @param pElement  the element
     */
    AbstractStatementPtr ParseStatement(DOMElement* pElement)
    {
        SetContext(pElement);
        AbstractStatementPtr p_stmt;
        DOMElement* p_operator = GetOperator(pElement);
        std::string op_name = X2C(p_operator->getLocalName());
        std::vector<DOMElement*> children = XmlTools::GetChildElements(pElement);
        if (op_name == "eq")
        {
            // Assignment
            PROTO_ASSERT(children.size() == 3, "An assignment apply must have 3 children, not "
                         << children.size() << ".");
            std::string assignee_type = X2C(children[1]->getLocalName());
            AbstractExpressionPtr p_rhs = ParseExpression(children[2]);
            if (assignee_type == "ci")
            {
                // Single assignment
                std::string assignee = X2C(children[1]->getTextContent());
                p_stmt = ASSIGN_STMT(assignee, p_rhs);
            }
            else if (assignee_type == "apply")
            {
                // Assignment to multiple names
                DOMElement* p_operator = GetOperator(children[1]);
                std::string op_name = X2C(p_operator->getLocalName());
                PROTO_ASSERT(op_name == "csymbol", "Assignments to an apply must be to a tuple, not "
                             << op_name << ".");
                std::string symbol = GetCsymbolName(p_operator);
                PROTO_ASSERT(symbol == "tuple", "Assignments to an apply must be to a tuple, not "
                             << symbol << ".");
                std::vector<DOMElement*> apply_children = XmlTools::GetChildElements(children[1]);
                PROTO_ASSERT(apply_children.size() > 1, "Assignments must be to at least one name.");
                std::vector<std::string> assignees;
                assignees.reserve(apply_children.size()-1);
                for (std::vector<DOMElement*>::iterator it=++apply_children.begin(); it != apply_children.end(); ++it)
                {
                    PROTO_ASSERT(X2C((*it)->getLocalName()) == "ci", "Assignments must be to variables, not "
                                 << X2C((*it)->getLocalName()) << ".");
                    assignees.push_back(X2C((*it)->getTextContent()));
                }
                p_stmt = ASSIGN_STMT(assignees, p_rhs);
            }
            else
            {
                PROTO_EXCEPTION("Assignments must be to a single name or tuple of names, not "
                                << assignee_type << ".");
            }
        }
        else if (op_name == "csymbol")
        {
            std::string symbol = GetCsymbolName(p_operator);
            if (symbol == "assert")
            {
                // Assertion
                PROTO_ASSERT(children.size() == 2, "An assertion must have one operand, not "
                             << (children.size()-1) << ".");
                AbstractExpressionPtr p_assertion = ParseExpression(children.back());
                p_stmt = ASSERT_STMT(p_assertion);
            }
            else if (symbol == "return")
            {
                // Return
                std::vector<AbstractExpressionPtr> operands = ParseOperands(pElement);
                p_stmt = RETURN_STMT(operands);
            }
            else
            {
                PROTO_EXCEPTION("Unrecognised statement csymbol " << symbol << ".");
            }
        }
        else
        {
            PROTO_EXCEPTION("Unrecognised statement operator " << op_name << ".");
        }
        TransferContext(pElement, p_stmt);
        return p_stmt;
    }

    /**
     * Parse a list of statements, i.e. a MathML apply element with the statementList csymbol as
     * its operator.
     *
     * @param pListElt  the containing apply element
     */
    std::vector<AbstractStatementPtr> ParseStatementList(DOMElement* pListElt)
    {
        SetContext(pListElt);
        std::vector<AbstractStatementPtr> stmts;
        PROTO_ASSERT(X2C(pListElt->getNamespaceURI()) == mMathmlNs,
                     "Statement lists must be MathML elements; found namespace "
                     << X2C(pListElt->getNamespaceURI()) << ".");
        PROTO_ASSERT(IsStatementList(pListElt), "Expected a statement list here.");
        std::vector<DOMElement*> child_elts = XmlTools::GetChildElements(pListElt);
        PROTO_ASSERT(child_elts.size() > 1, "A statement list must not be empty.");
        stmts.reserve(child_elts.size()-1);
        for (std::vector<DOMElement*>::iterator it = ++child_elts.begin();
             it != child_elts.end(); ++it)
        {
            stmts.push_back(ParseStatement(*it));
        }
        return stmts;
    }

    /**
     * Parse the definition of a stepper.
     *
     * @param pDefnElt  the definition element
     */
    boost::shared_ptr<AbstractStepper> ParseStepper(DOMElement* pDefnElt)
    {
        SetContext(pDefnElt);
        boost::shared_ptr<AbstractStepper> p_stepper;
        std::string stepper_type = X2C(pDefnElt->getLocalName());
        PROTO_ASSERT(pDefnElt->hasAttribute(X("name")), "Steppers must be given a name.");
        std::string name = X2C(pDefnElt->getAttribute(X("name")));
        PROTO_ASSERT(pDefnElt->hasAttribute(X("units")), "Steppers must have units specified.");
        std::string units = X2C(pDefnElt->getAttribute(X("units")));
        if (stepper_type == "uniformStepper")
        {
            std::vector<DOMElement*> children = XmlTools::GetChildElements(pDefnElt);
            PROTO_ASSERT(children.size() == 3, "A uniform stepper must have start, stop & step specified.");
            std::vector<AbstractExpressionPtr> params(3);
            for (unsigned i=0; i<3; ++i)
            {
                params[i] = ParseNumberOrExpression(children[i]);
            }
            p_stepper.reset(new UniformStepper(name, units, params[0], params[1], params[2]));
        }
        else if (stepper_type == "vectorStepper")
        {
            std::vector<DOMElement*> children = XmlTools::GetChildElements(pDefnElt);
            std::vector<AbstractExpressionPtr> values;
            values.reserve(children.size());
            BOOST_FOREACH(DOMElement* p_child, children)
            {
                SetContext(p_child);
                if (X2C(p_child->getLocalName()) == "value")
                {
                    values.push_back(ParseNumberOrExpression(p_child));
                }
                else
                {
                    values.push_back(ParseExpression(p_child));
                }
            }
            p_stepper.reset(new VectorStepper(name, units, values));
        }
        else if (stepper_type == "whileStepper")
        {
            std::vector<DOMElement*> children = XmlTools::GetChildElements(pDefnElt);
            PROTO_ASSERT(children.size() == 1, "A while stepper must contain a single condition.");
            std::vector<DOMElement*> cond = XmlTools::GetChildElements(children.front());
            PROTO_ASSERT(cond.size() == 1, "A while stepper must contain a single condition.");
            AbstractExpressionPtr p_condition = ParseExpression(cond.front());
            p_stepper.reset(new WhileStepper(name, units, p_condition));
        }
        else
        {
            PROTO_EXCEPTION("Unrecognised stepper element name: " << stepper_type << ".");
        }
        TransferContext(pDefnElt, p_stepper);
        return p_stepper;
    }

    /**
     * Parse the definition of a single modifier.
     *
     * @param pDefnElt  the element
     */
    boost::shared_ptr<AbstractSimulationModifier> ParseModifier(DOMElement* pDefnElt)
    {
        SetContext(pDefnElt);
        boost::shared_ptr<AbstractSimulationModifier> p_modifier;
        std::string modifier_name = X2C(pDefnElt->getLocalName());
        std::vector<DOMElement*> children = XmlTools::GetChildElements(pDefnElt);
        PROTO_ASSERT(!children.empty(), "All modifiers must have at least a 'when' specification.");
        PROTO_ASSERT(X2C(children.front()->getLocalName()) == "when",
                     "The first child of a modifier must be a 'when' element.");
        std::string when_name = X2C(children.front()->getTextContent());
        AbstractSimulationModifier::ApplyWhen when;
        if (when_name == "AT_START_ONLY")
        {
            when = AbstractSimulationModifier::AT_START_ONLY;
        }
        else if (when_name == "EVERY_LOOP")
        {
            when = AbstractSimulationModifier::EVERY_LOOP;
        }
        else if (when_name == "AT_END")
        {
            when = AbstractSimulationModifier::AT_END;
        }
        else
        {
            PROTO_EXCEPTION("Unrecognised when specification: " << when_name << ".");
        }
        if (modifier_name == "resetState")
        {
            std::string state_name;
            if (children.size() == 2)
            {
                state_name = X2C(children.back()->getTextContent());
            }
            ///\todo proper switch between vector types
#ifdef CHASTE_CVODE
            p_modifier.reset(new ModelResetModifier<N_Vector>(when, state_name, mpCurrentProtocolObject->GetStateCollection()));
#else
            p_modifier.reset(new ModelResetModifier<std::vector<double> >(when, state_name, mpCurrentProtocolObject->GetStateCollection()));
#endif
        }
        else if (modifier_name == "saveState")
        {
            std::string state_name;
            if (children.size() == 2)
            {
                state_name = X2C(children.back()->getTextContent());
            }
            ///\todo proper switch between vector types
#ifdef CHASTE_CVODE
            p_modifier.reset(new StateSaverModifier<N_Vector>(when, state_name, mpCurrentProtocolObject->GetStateCollection()));
#else
            p_modifier.reset(new StateSaverModifier<std::vector<double> >(when, state_name, mpCurrentProtocolObject->GetStateCollection()));
#endif
        }
        else if (modifier_name == "setVariable")
        {
            PROTO_ASSERT(children.size() == 3, "A setVariable modifier requires 3 child elements.");
            std::string variable_name = X2C(children[1]->getTextContent());
            AbstractExpressionPtr p_value = ParseNumberOrExpression(children[2]);
            p_modifier.reset(new SetVariableModifier(when, variable_name, p_value));
        }
        else
        {
            PROTO_EXCEPTION("Unrecognised modifier element name " << modifier_name << ".");
        }
        TransferContext(pDefnElt, p_modifier);
        return p_modifier;
    }

    /**
     * Parse the definition of a (possibly empty) collection of modifiers.
     *
     * @param pDefnElt  the modifiers element
     */
    boost::shared_ptr<ModifierCollection> ParseModifiers(DOMElement* pDefnElt)
    {
        SetContext(pDefnElt);
        std::vector<DOMElement*> modifier_elts = XmlTools::GetChildElements(pDefnElt);
        std::vector<boost::shared_ptr<AbstractSimulationModifier> > modifiers;
        modifiers.reserve(modifier_elts.size());
        BOOST_FOREACH(DOMElement* p_modifier_elt, modifier_elts)
        {
            modifiers.push_back(ParseModifier(p_modifier_elt));
        }
        return boost::make_shared<ModifierCollection>(modifiers);
    }

    /**
     * Parse a timecourseSimulation element.
     *
     * @param pDefnElt  the element
     * @param pStepper  the parsed simulation stepper
     * @param pModifiers  the parsed modifiers collection
     */
    boost::shared_ptr<AbstractSimulation> ParseTimecourseSimulation(DOMElement* pDefnElt,
                                                                    boost::shared_ptr<AbstractStepper> pStepper,
                                                                    boost::shared_ptr<ModifierCollection> pModifiers)
    {
        SetContext(pDefnElt);
        boost::shared_ptr<AbstractCardiacCellInterface> p_no_model; ///\todo specify model in XML
        return boost::make_shared<TimecourseSimulation>(p_no_model, pStepper, pModifiers);
    }

    /**
     * Parse a nestedSimulation element.
     *
     * @param pDefnElt  the element
     * @param pStepper  the parsed simulation stepper
     * @param pModifiers  the parsed modifiers collection
     */
    boost::shared_ptr<AbstractSimulation> ParseNestedSimulation(DOMElement* pDefnElt,
                                                                boost::shared_ptr<AbstractStepper> pStepper,
                                                                boost::shared_ptr<ModifierCollection> pModifiers)
    {
        SetContext(pDefnElt);
        std::vector<DOMElement*> children = XmlTools::GetChildElements(pDefnElt);
        PROTO_ASSERT(children.size() == 3, "A nestedSimulation must contain a (single) nested simulation definition.");
        boost::shared_ptr<AbstractSimulation> p_nested_sim = ParseSimulationDefinition(children.back());
        return boost::make_shared<NestedSimulation>(p_nested_sim, pStepper, pModifiers);
    }

    /**
     * Parse a (potentially nested) simulation definition, incorporating the simulation itself
     * along with all steppers and modifiers.
     *
     * @param pDefnElt  the simulation definition element
     */
    boost::shared_ptr<AbstractSimulation> ParseSimulationDefinition(DOMElement* pDefnElt)
    {
        SetContext(pDefnElt);
        boost::shared_ptr<AbstractSimulation> p_sim;
        std::string sim_type = X2C(pDefnElt->getLocalName());
        if (sim_type == "nestedProtocol")
        {
            p_sim = ParseNestedProtocol(pDefnElt);
        }
        else
        {
            std::vector<DOMElement*> children = XmlTools::GetChildElements(pDefnElt);
            PROTO_ASSERT(children.size() >= 2,
                         "A simulation definition must contain stepper and modifier definitions.");
            boost::shared_ptr<AbstractStepper> p_stepper = ParseStepper(children[0]);
            boost::shared_ptr<ModifierCollection> p_modifiers = ParseModifiers(children[1]);
            if (sim_type == "nestedSimulation")
            {
                p_sim = ParseNestedSimulation(pDefnElt, p_stepper, p_modifiers);
            }
            else if (sim_type == "timecourseSimulation")
            {
                p_sim = ParseTimecourseSimulation(pDefnElt, p_stepper, p_modifiers);
            }
            else
            {
                PROTO_EXCEPTION("Unexpected simulation definition element named " << sim_type << ".");
            }
        }
        if (pDefnElt->hasAttribute(X("prefix")))
        {
            std::string sim_prefix = X2C(pDefnElt->getAttribute(X("prefix")));
            p_sim->SetOutputsPrefix(sim_prefix);
        }
        TransferContext(pDefnElt, p_sim);
        return p_sim;
    }

    /**
     * Parse an outputVariables element, which defines which variables should be considered as the outputs
     * for the protocol as a whole.
     *
     * @param pDefnElt  the outputVariables element
     */
    std::vector<boost::shared_ptr<OutputSpecification> > ParseOutputVariables(DOMElement* pDefnElt)
    {
        SetContext(pDefnElt);
        std::vector<boost::shared_ptr<OutputSpecification> > variable_specs;
        std::vector<DOMElement*> spec_elts = XmlTools::GetChildElements(pDefnElt);
        variable_specs.reserve(spec_elts.size());
        BOOST_FOREACH(DOMElement* p_spec_elt, spec_elts)
        {
            if (CheckUseImportsVec(p_spec_elt, &Protocol::rGetOutputSpecifications, variable_specs))
            {
                continue;
            }
            SetContext(p_spec_elt);
            std::string elt_name = X2C(p_spec_elt->getLocalName());
            std::string type;
            if (elt_name == "raw")
            {
                type = "Raw";
            }
            else if (elt_name == "postprocessed")
            {
                type = "Post-processed";
            }
            else
            {
                PROTO_EXCEPTION("Unexpected variable specification element " << elt_name << ".");
            }
            PROTO_ASSERT(p_spec_elt->hasAttribute(X("name")),
                         "An output variable specification must give the variable name.");
            std::string var_name = X2C(p_spec_elt->getAttribute(X("name")));
            std::string var_ref(var_name);
            if (p_spec_elt->hasAttribute(X("ref")))
            {
                var_ref = X2C(p_spec_elt->getAttribute(X("ref")));
            }
            std::string units, desc(var_name);
            if (p_spec_elt->hasAttribute(X("units")))
            {
                units = X2C(p_spec_elt->getAttribute(X("units")));
            }
            if (p_spec_elt->hasAttribute(X("description")))
            {
                desc = X2C(p_spec_elt->getAttribute(X("description")));
            }
            boost::shared_ptr<OutputSpecification> p_spec(new OutputSpecification(var_ref, var_name, desc, units, type));
            TransferContext(p_spec_elt, p_spec);
            variable_specs.push_back(p_spec);
        }
        return variable_specs;
    }

    /**
     * Parse a plots element, which defines default graphs to plot.
     *
     * @param pDefnElt  the plots element
     */
    std::vector<boost::shared_ptr<PlotSpecification> > ParsePlots(DOMElement* pDefnElt)
    {
        std::vector<boost::shared_ptr<PlotSpecification> > plots;
        std::vector<DOMElement*> plot_elts = XmlTools::GetChildElements(pDefnElt);
        plots.reserve(plot_elts.size());
        BOOST_FOREACH(DOMElement* p_plot_elt, plot_elts)
        {
            if (CheckUseImportsVec(p_plot_elt, &Protocol::rGetPlotSpecifications, plots))
            {
                continue;
            }
            SetContext(p_plot_elt);
            boost::shared_ptr<PlotSpecification> p_plot;
            std::vector<DOMElement*> children = XmlTools::GetChildElements(p_plot_elt);
            PROTO_ASSERT(children.size() == 2 || children.size() == 3,
                         "A plot element must have 2 or 3 children.");
            std::string title = X2C(children.front()->getTextContent());
            if (children.size() == 2)
            {
                p_plot.reset(new PlotSpecification(title, X2C(children.back()->getTextContent())));
            }
            else
            {
                std::string x_var = X2C(children[1]->getTextContent());
                std::string y_var = X2C(children[2]->getTextContent());
                p_plot.reset(new PlotSpecification(title, x_var, y_var));
            }
            TransferContext(p_plot_elt, p_plot);
            plots.push_back(p_plot);
        }
        return plots;
    }

    /**
     * Get a map from prefix to namespace URI for all namespaces declared and available for use
     * in referencing model variables.
     */
    std::map<std::string, std::string> GetNamespaceDeclarations() const
    {
        std::map<std::string, std::string> result;
        std::pair<std::string, std::string> binding;
        BOOST_FOREACH(binding, mPrefixDefinitions)
        {
            if (!binding.second.empty())
            {
                result.insert(binding);
            }
        }
        return result;
    }

    /**
     * Main constructor sets up constant member variables.
     *
     * @param rParser  the root parser creating this implementation
     */
    ProtocolParserImpl(ProtocolParser& rParser)
        : mrParser(rParser),
          mMathmlNs("http://www.w3.org/1998/Math/MathML"),
          mCsymbolBaseUrl("https://chaste.cs.ox.ac.uk/nss/protocol/")
    {}

private:
    /** The root parser that created this parser implementation. */
    ProtocolParser& mrParser;

    /** The protocol object that parsed language constructs will be added to. */
    ProtocolPtr mpCurrentProtocolObject;

    /** Holds a description of where in the file we're parsing. */
    std::string mLocationInfo;

    /**
     * Set #mLocationInfo based on the given node.
     *
     * @param pNode  the node being parsed
     */
    void SetContext(const DOMNode* pNode)
    {
        const TaggingDomParser::Tag* p_tag = TaggingDomParser::GetTag(pNode);
        assert(p_tag != NULL);
        std::stringstream loc;
        loc << p_tag->mSystemId << ":" << p_tag->mLineNumber << ":" << p_tag->mColumnNumber
                << "\t" << X2C(pNode->getLocalName());
        mLocationInfo = loc.str();
    }

    /**
     * Set the location information on a language element based on the XML that was parsed to
     * create it.
     *
     * @param pNode  the XML representation of the construct
     * @param pConstruct  the language construct
     */
    void TransferContext(const DOMNode* pNode, boost::shared_ptr<LocatableConstruct> pConstruct)
    {
        SetContext(pNode);
        pConstruct->SetLocationInfo(GetLocationInfo());
    }

    /**
     * Get a description of the location being parsed for use in error messages.
     */
    std::string GetLocationInfo()
    {
        return mLocationInfo;
    }

    /** The MathML namespace. */
    const std::string mMathmlNs;

    /** The prefix for our csymbol definitionURLs. */
    const std::string mCsymbolBaseUrl;

    /**
     * Given a csymbol, check that it has a definitionURL with the correct base, and extract the
     * non-base portion.
     *
     * @param pElement  the csymbol element
     */
    std::string GetCsymbolName(DOMElement* pElement)
    {
        std::string definition_url = X2C(pElement->getAttribute(X("definitionURL")));
        PROTO_ASSERT(!definition_url.empty(), "All csymbol elements must have a definitionURL.");
        const size_t base_len = mCsymbolBaseUrl.length();
        PROTO_ASSERT(definition_url.substr(0, base_len) == mCsymbolBaseUrl,
                     "All csymbol elements must have a definitionURL commencing with " << mCsymbolBaseUrl
                     << "; " << definition_url << " does not match.");
        return definition_url.substr(base_len);
    }

    /**
     * Get the operator element (the first child) from an apply element.
     *
     * @param pElement  the apply element
     */
    DOMElement* GetOperator(DOMElement* pElement)
    {
        assert(X2C(pElement->getLocalName()) == "apply");
        return GetFirstChild(pElement);
    }

    /**
     * Get the first child element of the given element, throwing an error if none exists.
     *
     * @param pElement  the parent element
     */
    DOMElement* GetFirstChild(DOMNode* pElement)
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

    /**
     * Stores the namespace, import & simulation prefix declarations.  This allows us to
     * keep track of duplicate prefixes, and (for namespaces) map prefixes to the full
     * namespace URI.  Other prefixes map to the empty string.
     */
    std::map<std::string, std::string> mPrefixDefinitions;

    /**
     * Add a new prefix declaration, checking for any existing conflicting definition.
     * See also #mPrefixDefinitions.
     * @param rPrefix  the prefix being declared
     * @param rNamespaceUri  the namespace it's bound to, if any
     */
    void AddPrefixDeclaration(const std::string& rPrefix, const std::string& rNamespaceUri="")
    {
        std::map<std::string, std::string>::const_iterator it = mPrefixDefinitions.find(rPrefix);
        if (it != mPrefixDefinitions.end())
        {
            if (it->second != rNamespaceUri)
            {
                PROTO_EXCEPTION("Conflicting declaration of prefix " << rPrefix << " found."
                                " Previously was bound to '" << it->second << "'.");
            }
        }
        else
        {
            mPrefixDefinitions[rPrefix] = rNamespaceUri;
        }
    }

    /**
     * Store any bindings of namespaces to prefixes declared on the given element.
     * @param pElement
     */
    void StoreNamespaceMappings(const DOMNode* pElement)
    {
        const std::string xmlns("http://www.w3.org/2000/xmlns/");
        SetContext(pElement);
        xercesc::DOMNamedNodeMap* p_attrs = pElement->getAttributes();
        if (p_attrs)
        {
            for (XMLSize_t i=0; i<p_attrs->getLength(); i++)
            {
                xercesc::DOMNode* p_attr = p_attrs->item(i);
                if (X2C(p_attr->getNamespaceURI()) == xmlns && X2C(p_attr->getPrefix()) == "xmlns")
                {
                    // It's a namespace declaration
                    std::string prefix = X2C(p_attr->getLocalName());
                    std::string nsuri = X2C(p_attr->getNodeValue());
                    AddPrefixDeclaration(prefix, nsuri);
                }
            }
        }
    }

    /**
     * Convert a string to a double.
     * @param rStr  the string
     */
    double String2Double(const std::string& rStr)
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

    /**
     * Convert a string to an unsigned.
     * @param rStr  the string
     */
    unsigned String2Unsigned(const std::string& rStr)
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
};


/**
 * Parse XML into a DOM tree.
 * Requires Xerces to be initialised by the caller.
 *
 * @param rProtocolFile  the file to parse
 */
xsd::cxx::xml::dom::auto_ptr<DOMDocument> ParseFileToDom(const FileFinder& rProtocolFile)
{
    TaggingDomParser parser;

    parser.setCreateCommentNodes (false);
    parser.setCreateEntityReferenceNodes (false);
    parser.setDoNamespaces (true);
    parser.setIncludeIgnorableWhitespace (false);

    // Enable/disable validation
    bool validate = false;
    parser.setDoSchema (validate);
    parser.setValidationScheme(validate ? TaggingDomParser::Val_Always : TaggingDomParser::Val_Never);
    parser.setValidationSchemaFullChecking(false);

    // Set error handler
    xsd::cxx::tree::error_handler<char> eh;
    xsd::cxx::xml::sax::bits::error_handler_proxy<char> ehp(eh);
    parser.setErrorHandler(&ehp);

    // Parse the file
    try
    {
        parser.parse(rProtocolFile.GetAbsolutePath().c_str());
        eh.throw_if_failed<xsd::cxx::tree::parsing<char> >();
    }
    catch (const ::xsd::cxx::tree::parsing<char>& e)
    {
        PROTO_EXCEPTION2("XML parsing error reading protocol file " << rProtocolFile.GetAbsolutePath()
                         << ": " << e, "");
    }
    xsd::cxx::xml::dom::auto_ptr<DOMDocument> p_proto_doc(parser.adoptDocument());
    p_proto_doc->normalize();
    return p_proto_doc;
}


/**
 * Add constructs parsed from the given XML document into a protocol object.
 * @param pProto  the protocol object to fill
 * @param rParser  the protocol parser to use
 * @param pRootElt  the root of the protocol XML document to parse
 */
void AddElementsToProtocol(ProtocolPtr pProto, ProtocolParserImpl& rParser, DOMElement* pRootElt)
{
    // Parse protocol inputs (if present)
    std::vector<DOMElement*> inputs = XmlTools::FindElements(pRootElt, "inputs/apply");
    if (!inputs.empty())
    {
        pProto->AddInputDefinitions(rParser.ParseStatementList(inputs.front()));
    }

    // Parse imports (if present)
    std::vector<DOMElement*> imports = XmlTools::FindElements(pRootElt, "import");
    BOOST_FOREACH(DOMElement* p_import, imports)
    {
        rParser.ParseImport(p_import);
    }

    // Parse library (if present)
    std::vector<DOMElement*> library = XmlTools::FindElements(pRootElt, "library/apply");
    if (!library.empty())
    {
        pProto->AddLibrary(rParser.ParseStatementList(library.front()));
    }

    // Parse simulation definitions (if present)
    std::vector<DOMElement*> simulations = XmlTools::FindElements(pRootElt, "simulations");
    if (!simulations.empty())
    {
        std::vector<DOMElement*> children = XmlTools::GetChildElements(simulations.front());
        BOOST_FOREACH(DOMElement* p_sim_elt, children)
        {
            if (!rParser.CheckUseImports(p_sim_elt, &Protocol::rGetSimulations, &Protocol::AddSimulations))
            {
                pProto->AddSimulation(rParser.ParseSimulationDefinition(p_sim_elt));
            }
        }
    }

    // Parse post-processing program
    std::vector<DOMElement*> post_proc = XmlTools::FindElements(pRootElt, "post-processing");
    if (!post_proc.empty())
    {
        std::vector<DOMElement*> children = XmlTools::GetChildElements(post_proc.front());
        BOOST_FOREACH(DOMElement* p_child, children)
        {
            // Check for useImports
            if (!rParser.CheckUseImports(p_child, &Protocol::rGetPostProcessing, &Protocol::AddPostProcessing)
                && X2C(p_child->getLocalName()) == "apply")
            {
                // Local definitions
                pProto->AddPostProcessing(rParser.ParseStatementList(p_child));
            }
        }
    }

    // Parse identification of output variables
    std::vector<DOMElement*> outputs = XmlTools::FindElements(pRootElt, "outputVariables");
    if (!outputs.empty())
    {
        pProto->AddOutputSpecs(rParser.ParseOutputVariables(outputs.front()));
    }

    // Parse specification of plots
    std::vector<DOMElement*> plots = XmlTools::FindElements(pRootElt, "plots");
    if (!plots.empty())
    {
        pProto->AddDefaultPlots(rParser.ParsePlots(plots.front()));
    }

    // Transfer prefix bindings
    pProto->AddNamespaceBindings(rParser.GetNamespaceDeclarations());

    // All parsed
    pProto->FinaliseSetup();
}

ProtocolPtr ProtocolParser::ParseFile(const FileFinder& rProtocolFile)
{
    // Read the file to a DOM tree
    xsd::cxx::xml::auto_initializer init_fini(true, true);
    xsd::cxx::xml::dom::auto_ptr<DOMDocument> p_proto_doc(ParseFileToDom(rProtocolFile));

    // Parse the DOM into language constructs
    ProtocolParserImpl proto_parser(*this);
    DOMElement* p_root_elt = p_proto_doc->getDocumentElement();
    ProtocolPtr p_proto = proto_parser.CreateProtocolObject(p_root_elt);
    p_proto->SetSourceFile(rProtocolFile);
    AddElementsToProtocol(p_proto, proto_parser, p_root_elt);

    return p_proto;
}
