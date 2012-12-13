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

// For popen
#define _POSIX_C_SOURCE 2
#include <cstdio>

#include "ProtocolFileFinder.hpp"

#include "Exception.hpp"
#include "OutputFileHandler.hpp"

// Note that with all these constructors we need to use the default base class constructor
// and call SetPath ourselves, since it's a virtual method overridden here.

ProtocolFileFinder::ProtocolFileFinder(const std::string& rPath,
                                       RelativeTo::Value relativeTo)
{
    SetPath(rPath, relativeTo);
}


ProtocolFileFinder::ProtocolFileFinder(const std::string& rLeafName,
                                       const FileFinder& rParentOrSibling)
{
    SetPath(rLeafName, rParentOrSibling);
}


ProtocolFileFinder::ProtocolFileFinder(const fs::path& rPath)
{
    SetPath(fs::complete(rPath).string(), RelativeTo::Absolute);
}


ProtocolFileFinder::ProtocolFileFinder()
{}


void ProtocolFileFinder::SetPath(const std::string& rPath, RelativeTo::Value relativeTo)
{
    FileFinder::SetPath(rPath, relativeTo);
    ConvertIfNeeded();
}


void ProtocolFileFinder::SetPath(const std::string& rLeafName, const FileFinder& rParentOrSibling)
{
    FileFinder::SetPath(rLeafName, rParentOrSibling);
    ConvertIfNeeded();
}


const FileFinder& ProtocolFileFinder::rGetOriginalSource() const
{
    return mOriginalFinder;
}


void ProtocolFileFinder::ConvertIfNeeded()
{
    if (!mOriginalFinder.IsPathSet())
    {
        mOriginalFinder = *this;
    }
    if (GetExtension() != ".xml")
    {
        mOriginalFinder = *this; // The same line above doesn't get executed if the finder has been re-targeted
        EXCEPT_IF_NOT(system(NULL));
        // Call a Python script to convert the protocol, and obtain the resulting path from it
        OutputFileHandler handler("ProtocolConverter", false);
        FileFinder this_file(__FILE__, RelativeTo::ChasteSourceRoot);
        FileFinder converter("CompactSyntaxParser.py", this_file);
        std::string cmd = "/usr/bin/env python \"" + converter.GetAbsolutePath()
                + "\" \"" + mOriginalFinder.GetAbsolutePath()
                + "\" \"" + handler.GetOutputDirectoryFullPath() + '"';
        if (msQuiet)
        {
            cmd += " -quiet";
        }
        FILE* pipe = popen(cmd.c_str(), "r");
        EXCEPT_IF_NOT(pipe != NULL);
        std::string output;
        const int BUFSIZE = 1024;
        char buf[BUFSIZE];
        while (fgets(buf, sizeof (buf), pipe))
        {
            output += buf;
        }
        pclose(pipe);
        std::string::size_type line_end = output.find('\n'); // Script output is newline-terminated, if successful
        if (line_end == std::string::npos)
        {
            EXCEPTION("Conversion of text protocol '" << mOriginalFinder.GetAbsolutePath() << "' to XML failed.");
        }
        output.erase(line_end);
        EXCEPT_IF_NOT(output.substr(output.length()-4) == ".xml");
        FileFinder::SetPath(output, RelativeTo::Absolute);
        if (!Exists())
        {
            EXCEPTION("Conversion of text protocol '" << mOriginalFinder.GetAbsolutePath()
                      << "' to XML failed: XML file '" << GetAbsolutePath() << "' not found.");
        }
    }
}


bool ProtocolFileFinder::msQuiet = false;

void ProtocolFileFinder::BeQuiet(bool quiet)
{
    msQuiet = quiet;
}
