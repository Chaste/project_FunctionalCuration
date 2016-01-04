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

#include "TaggingDomParser.hpp"

#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/internal/XMLScanner.hpp>
#include <xercesc/sax/Locator.hpp>
using namespace xercesc;

#include <xsd/cxx/xml/sax/bits/error-handler-proxy.hxx>

#include "XmlTools.hpp"
#include "Exception.hpp"

const XMLCh TAG_KEY[] = {chLatin_L, chLatin_C, chLatin_A, chNull};

/**
 * A helper class that deletes tags when they're no longer used.
 */
class TaggingDomParser::TagDataHandler : public xercesc::DOMUserDataHandler
{
public:
    /** Constructor. */
    TagDataHandler()
    {}

    /** Virtual destructor. */
    virtual ~TagDataHandler()
    {}

    /**
     * Handle a node change operation.
     * The only operation we support is node deletion, in which case the node's tag is also deleted.
     *
     * @param operation  the operation
     * @param key  the tag key
     * @param pData  the tag
     * @param pSrc  source node
     * @param pDst  destination node
     */
    virtual void handle(xercesc::DOMUserDataHandler::DOMOperationType operation,
                        const XMLCh* const key, void* pData,
                        const xercesc::DOMNode* pSrc,
#if _XERCES_VERSION < 30000
                        const xercesc::DOMNode* pDst)
#else
                        xercesc::DOMNode* pDst)
#endif
    {
        Tag* p_src_tag = static_cast<Tag*>(pData);
        switch (operation)
        {
            case NODE_DELETED:
                delete p_src_tag;
                break;
            default:
                NEVER_REACHED;
        }
    }
};

TaggingDomParser::Tag::Tag(const XMLCh* pSystemId, int line, int col)
    : mSystemId(X2C(pSystemId)),
      mLineNumber(line),
      mColumnNumber(col)
{}

TaggingDomParser::TagDataHandler* TaggingDomParser::mpDataHandler = NULL;

TaggingDomParser::TaggingDomParser()
{
    if (!mpDataHandler)
    {
        mpDataHandler = new TagDataHandler;
    }
}

TaggingDomParser::~TaggingDomParser()
{
}

void TaggingDomParser::startElement(const xercesc::XMLElementDecl& rElemDecl,
                                    const unsigned int uriId,
                                    const XMLCh* const prefixName,
                                    const xercesc::RefVectorOf<xercesc::XMLAttr>& rAttrList,
#if _XERCES_VERSION < 30000
                                    const unsigned int attrCount,
#else
                                    const XMLSize_t attrCount,
#endif
                                    const bool isEmpty,
                                    const bool isRoot)
{
    XercesDOMParser::startElement(rElemDecl, uriId, prefixName, rAttrList, attrCount, false, isRoot);

    const xercesc::Locator* locator = getScanner()->getLocator();
    Tag* p_tag = new Tag(locator->getSystemId(), locator->getLineNumber(), locator->getColumnNumber());
    XercesDOMParser::fCurrentNode->setUserData(TAG_KEY, reinterpret_cast<void*>(p_tag), mpDataHandler);

    // The base class would normally do this, but then we couldn't get line & column for empty elements
    if (isEmpty)
    {
        endElement(rElemDecl, uriId, isRoot, prefixName);
    }
}

const TaggingDomParser::Tag* TaggingDomParser::GetTag(const xercesc::DOMNode* pNode)
{
    return static_cast<TaggingDomParser::Tag*>(pNode->getUserData(TAG_KEY));
}


xsd::cxx::xml::dom::auto_ptr<DOMDocument> TaggingDomParser::ParseFileToDom(const FileFinder& rXmlFile)
{
    TaggingDomParser parser;

    parser.setCreateCommentNodes(false);
    parser.setCreateEntityReferenceNodes(false);
    parser.setDoNamespaces(true);
    parser.setIncludeIgnorableWhitespace(false);

    // Enable/disable validation
    bool validate = false;
    parser.setDoSchema(validate);
    parser.setValidationScheme(validate ? TaggingDomParser::Val_Always : TaggingDomParser::Val_Never);
    parser.setValidationSchemaFullChecking(false);

    // Set error handler
    xsd::cxx::tree::error_handler<char> eh;
    xsd::cxx::xml::sax::bits::error_handler_proxy<char> ehp(eh);
    parser.setErrorHandler(&ehp);

    // Parse the file
    try
    {
        parser.parse(rXmlFile.GetAbsolutePath().c_str());
        eh.throw_if_failed<xsd::cxx::tree::parsing<char> >();
    }
    catch (const ::xsd::cxx::tree::parsing<char>& e)
    {
        EXCEPTION("XML parsing error reading file " << rXmlFile.GetAbsolutePath() << ": " << e);
    }
    xsd::cxx::xml::dom::auto_ptr<DOMDocument> p_doc(parser.adoptDocument());
    p_doc->normalize();
    return p_doc;
}
