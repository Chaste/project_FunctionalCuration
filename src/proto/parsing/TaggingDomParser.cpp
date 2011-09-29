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

#include "TaggingDomParser.hpp"

#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/internal/XMLScanner.hpp>
#include <xercesc/sax/Locator.hpp>
using namespace xercesc;

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
