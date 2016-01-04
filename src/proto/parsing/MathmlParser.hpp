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
class MathmlParser : private boost::noncopyable
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
    
    /**
     * Set whether to make all MathML operators that take arguments into implicit
     * maps if at least one of the arguments is an array.  This is needed for SED-ML,
     * but is not enabled by default.
     *
     * @param useImplicitMap
     */
    void SetUseImplicitMap(bool useImplicitMap=true);

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

    /**
     * Parse an application of a csymbol operator.
     *
     * @param pApplyElement  the apply element
     * @param pOperator  the csymbol operator element
     */
    virtual AbstractExpressionPtr ParseCsymbolApply(const DOMElement* pApplyElement,
                                                    const DOMElement* pOperator)=0;

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

    /**
     * Do "LambdaExpression::WrapMathml<OperatorClass>(numArgs)" dealing with converting a run-time
     * string into a compile-time type.
     *
     * @param rOperator  the MathML operator name
     * @param numArgs  the number of arguments it should take
     */
    AbstractExpressionPtr WrapMathml(const std::string& rOperator, unsigned numArgs);

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
    /** Holds a description of where in the file we're parsing. */
    std::string mLocationInfo;
    
    /** Whether to use implicit maps. */
    bool mUseImplicitMap;
};

#endif // MATHMLPARSER_HPP_
