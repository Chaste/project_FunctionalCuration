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

#ifndef MATHMLPARSER_HPP_
#define MATHMLPARSER_HPP_

#include <string>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include <xercesc/dom/DOM.hpp>
using namespace xercesc;

#include "LocatableConstruct.hpp"
#include "AbstractExpression.hpp"

/**
 * Base class for parsers of our XML protocol language and of SED-ML, providing the core
 * XML handling and parsing of MathML expressions.
 */
class MathmlParser : boost::noncopyable
{
public:
    /**
     * Parse a cn element to obtain a constant number.
     *
     * \todo Cope with base, sep, etc.
     *
     * @param pElement  the cn element
     */
    double ParseNumber(const DOMElement* pElement);

    /**
     * Parse a MathML element encoding an expression.
     *
     * @param pElement  the element
     */
    AbstractExpressionPtr ParseExpression(const DOMElement* pElement);

protected:
    /**
     * Main constructor sets up constant member variables.
     */
    MathmlParser();

    /**
     * Virtual destructor.
     */
    virtual ~MathmlParser();

    // Methods required to be implemented by subclasses

    /**
     * Parse a csymbol element representing a whole expression (as opposed to an operator).
     *
     * @param pElement  the csymbol element
     */
    virtual AbstractExpressionPtr ParseCsymbolExpression(const DOMElement* pElement)=0;

    // Subsidiary parsing methods

    /**
     * Parse a MathML apply element to obtain an expression.
     *
     * The operator should be a ci, csymbol, or MathML operator.
     *
     * @param pElement  the apply element
     */
    AbstractExpressionPtr ParseApply(const DOMElement* pElement);

    /**
     * Parse the operands of an apply element.
     *
     * Note: we pretend that 'degree' and 'logbase' elements are operands rather than qualifiers.
     *
     * @param pElement  the apply element
     */
    std::vector<AbstractExpressionPtr> ParseOperands(const DOMElement* pElement);

    /**
     * Determine whether a given element is a qualifier.
     * Just tests if the element name is bvar or domainofapplication, since they
     * are the only qualifiers we support.
     *
     * @param pElement  the element to test
     */
    bool IsQualifier(const DOMElement* pElement);

    /**
     * Parse a MathML piecewise element to obtain an "if" expression.
     *
     * \todo Only one piece child is allowed (at present).
     *
     * @param pElement  the piecewise element
     */
    AbstractExpressionPtr ParsePiecewise(const DOMElement* pElement);

    /**
     * Parse a MathML bvar element to obtain the name referenced in the enclosed ci element.
     *
     * @param pElement  the bvar element
     */
    std::string ParseBvar(const DOMElement* pElement);

    /**
     * Parse a MathML lambda element into a LambdaExpression.
     *
     * Its content should consist of a sequence of bvar elements defining the parameters,
     * then a single element which parses as an expression.  This forms the function body.
     *
     * @param pElement  the lambda element
     */
    virtual AbstractExpressionPtr ParseFunctionDefinition(const DOMElement* pElement);

    // Utility methods

    /**
     * Set #mLocationInfo based on the given node.
     *
     * @param pNode  the node being parsed
     */
    void SetContext(const DOMNode* pNode);

    /**
     * Set the location information on a language element based on the XML that was parsed to
     * create it.  Also sets whether to trace this object.
     *
     * @param pNode  the XML representation of the construct
     * @param pConstruct  the language construct
     */
    void TransferContext(const DOMNode* pNode, boost::shared_ptr<LocatableConstruct> pConstruct);

    /**
     * Get a description of the location being parsed for use in error messages.
     * This provides enough of the LocatableConstruct interface for the PROTO_EXCEPTION macro to work.
     */
    std::string GetLocationInfo();

    /**
     * Given a csymbol, check that it has a definitionURL with the correct base, and extract the
     * non-base portion.
     *
     * @param pElement  the csymbol element
     */
    std::string GetCsymbolName(const DOMElement* pElement);

    /**
     * Get the operator element (the first child) from an apply element.
     *
     * @param pElement  the apply element
     */
    DOMElement* GetOperator(const DOMElement* pElement);

    /**
     * Get the first child element of the given element, throwing an error if none exists.
     *
     * @param pElement  the parent element
     */
    DOMElement* GetFirstChild(const DOMNode* pElement);

    /**
     * Convert a string to a double.
     * @param rStr  the string
     */
    double String2Double(const std::string& rStr);

    /**
     * Convert a string to an unsigned.
     * @param rStr  the string
     */
    unsigned String2Unsigned(const std::string& rStr);

    /**
     * Convert a string representing an xs:boolean value to a bool.
     * @param rStr  the string
     */
    bool String2Bool(const std::string& rStr);

    /** The MathML namespace. */
    const std::string mMathmlNs;

private:
    /** The prefix for csymbol definitionURLs in our protocol language. */
    const std::string mCsymbolBaseUrl;

    /** Holds a description of where in the file we're parsing. */
    std::string mLocationInfo;
};

#endif // MATHMLPARSER_HPP_
