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
