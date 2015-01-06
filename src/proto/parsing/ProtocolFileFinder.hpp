/*

Copyright (c) 2005-2015, University of Oxford.
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

#ifndef PROTOCOLFILEFINDER_HPP_
#define PROTOCOLFILEFINDER_HPP_

#include "FileFinder.hpp"

/**
 * This specialised FileFinder subclass allows us to abstract away the file format for protocols.
 * It can be initialised to point at a protocol in either XML or compact textual syntax.
 * However, if the text syntax is used, the instance will translate it to XML when SetPath is called,
 * and provide the path of the XML version to other code.
 */
class ProtocolFileFinder : public FileFinder
{
public:
    /**
     * Main constructor.
     *
     * @param rPath  the path to the file/dir to find
     * @param relativeTo  how to interpret this path
     */
    ProtocolFileFinder(const std::string& rPath, RelativeTo::Value relativeTo);

    /**
     * Find a file (or folder) relative to some file or directory.
     * If the second argument is a directory, we look for the given leaf name within it.
     * If the second argument is a file, then we look for a sibling.
     * An exception is raised if rParentOrSibling does not exist.
     *
     * @param rLeafName  the leaf name of the file/dir to find
     * @param rParentOrSibling  where to look for it
     */
    ProtocolFileFinder(const std::string& rLeafName, const FileFinder& rParentOrSibling);

    /**
     * Conversion constructor from a Boost Filesystem path object.
     * The path will be interpreted as relative to the current working directory,
     * unless it is an absolute path.
     *
     * @param rPath  the path to the file/dir to find
     */
    ProtocolFileFinder(const fs::path& rPath);

    /**
     * Default constructor for an uninitialised finder.
     */
    ProtocolFileFinder();

    /**
     * Change this finder to point at a new location.
     *
     * @param rPath  the path to the file/dir to find
     * @param relativeTo  how to interpret this path
     */
    void SetPath(const std::string& rPath, RelativeTo::Value relativeTo);

    /**
     * Change this finder to point at a new location, relative to some file or directory.
     *
     * @param rLeafName  the leaf name of the file/dir to find
     * @param rParentOrSibling  where to look for it
     */
    void SetPath(const std::string& rLeafName, const FileFinder& rParentOrSibling);

    /**
     * Get a finder for the original protocol file, whatever format it was in.
     * This is crucial for interpreting relative URIs in the protocol.
     */
    const FileFinder& rGetOriginalSource() const;

    /**
     * Set whether to report errors in protocol format conversion.
     *
     * @param quiet  if set, errors will not be reported
     */
    static void BeQuiet(bool quiet=true);

private:
    /**
     * Convert the protocol this finder points at to XML if it isn't already.
     * Called by the various constructors.
     */
    void ConvertIfNeeded();

    /** This refers to the location we originally pointed at, whether text or XML syntax. */
    FileFinder mOriginalFinder;

    /** Whether to report errors in protocol format conversion. */
    static bool msQuiet;
};

#endif // PROTOCOLFILEFINDER_HPP_
