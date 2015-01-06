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

#ifndef MANIFEST_HPP_
#define MANIFEST_HPP_

#include <map>
#include <string>

#include "OutputFileHandler.hpp"

/**
 * This class is used to keep track of what files are being written as protocol outputs,
 * and to write a manifest file with that information formatted according to the COMBINE
 * archive specification (see http://co.mbine.org/documents/archive).
 */
class Manifest
{
public:
    /**
     * Add a file to the manifest.
     *
     * @param rFileName  the name of the file
     * @param rFormat  the file format; either a MIME type or identifiers.org URL
     * @param allowOverwrite  whether to allow overwriting an existing entry for the given file
     */
    void AddEntry(const std::string& rFileName,
                  const std::string& rFormat,
                  bool allowOverwrite=false);

    /**
     * Write the manifest to disk.
     *
     * @param rOutputFolder  the folder in which to write the manifest.xml file
     */
    void WriteFile(const OutputFileHandler& rOutputFolder);

    /**
     * @return  a map from file name to its format description (MIME type or identifiers.org URI)
     */
    const std::map<std::string, std::string>& rGetEntries() const;

    /** Clear all entries in the manifest. */
    void Clear();

private:
    /** The file name -> format mapping. */
    std::map<std::string, std::string> mEntries;
};

#endif // MANIFEST_HPP_
