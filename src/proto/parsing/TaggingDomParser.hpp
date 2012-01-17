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

#ifndef TAGGINGDOMPARSER_HPP_
#define TAGGINGDOMPARSER_HPP_

#include <string>

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XercesVersion.hpp>
#include <xsd/cxx/xml/dom/auto-ptr.hxx>

#include "FileFinder.hpp"

/**
 * A specialised version of Xerces' DOM parser that annotates elements with their file/line/column position
 * in the parsed XML document, so that later error messages can report the location.
 */
class TaggingDomParser : public xercesc::XercesDOMParser
{
public:
    /**
     * A helper method which constructs a TaggingDomParser, parses the given file (without schema
     * validation at present), and returns the parsed document if successful.  Any parse error
     * will result in an Exception being thrown.
     *
     * Requires the Xerces runtime to have been initialised by the caller.
     *
     * @param rXmlFile  the XML file to parse
     */
    static xsd::cxx::xml::dom::auto_ptr<xercesc::DOMDocument> ParseFileToDom(const FileFinder& rXmlFile);

    /**
     * The location tag added to nodes.
     */
    class Tag
    {
    public:
        /**
         * Create a new tag.
         * @param pSystemId  the file path
         * @param line  the line number
         * @param col  the column number
         */
        Tag(const XMLCh* pSystemId, int line, int col);
        std::string mSystemId; /**< The file path */
        int mLineNumber;       /**< The line number */
        int mColumnNumber;     /**< The column number */
    };

    /**
     * Get the tag associated with the given node.
     *
     * @param pNode  the node, which should be an element
     */
    static const Tag* GetTag(const xercesc::DOMNode* pNode);

    /** Construct the parser. */
    TaggingDomParser();

    /** We have virtual methods. */
    virtual ~TaggingDomParser();

    /**
     * The main workhorse method that adds the tags for each element.
     *
     * @param rElemDecl  the element
     * @param uriId  namespace?
     * @param prefixName  namespace prefix
     * @param rAttrList  attributes
     * @param attrCount  number of attributes
     * @param isEmpty  whether this is an empty element
     * @param isRoot  whether this is the root element
     */
    virtual void startElement(const xercesc::XMLElementDecl& rElemDecl,
                              const unsigned int uriId,
                              const XMLCh* const prefixName,
                              const xercesc::RefVectorOf<xercesc::XMLAttr>& rAttrList,
#if _XERCES_VERSION < 30000
                              const unsigned int attrCount,
#else
                              const XMLSize_t attrCount,
#endif
                              const bool isEmpty,
                              const bool isRoot);

private:
    /** A helper class that deletes tags when they're no longer used. */
    class TagDataHandler;

    /** Our tag handler instance. */
    static TagDataHandler* mpDataHandler;
};



#endif // TAGGINGDOMPARSER_HPP_
