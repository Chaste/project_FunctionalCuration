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

#ifndef TESTPROTOCOLFILEFINDER_HPP_
#define TESTPROTOCOLFILEFINDER_HPP_

#include <cxxtest/TestSuite.h>

#include "ProtocolFileFinder.hpp"

#include "OutputFileHandler.hpp"

class TestProtocolFileFinder : public CxxTest::TestSuite
{
public:
    void TestFinder() throw (Exception)
    {
        ProtocolFileFinder xml_proto1("projects/FunctionalCuration/protocols/xml/GraphState.xml", RelativeTo::ChasteSourceRoot);
        ProtocolFileFinder txt_proto1("projects/FunctionalCuration/protocols/GraphState.txt", RelativeTo::ChasteSourceRoot);
        FileFinder this_file(__FILE__, RelativeTo::ChasteSourceRoot);
        ProtocolFileFinder xml_proto2("../protocols/xml/GraphState.xml", this_file);
        ProtocolFileFinder txt_proto2("../protocols/GraphState.txt", this_file);

        TS_ASSERT(AreSameFile(xml_proto1, xml_proto2));
        TS_ASSERT(!AreSameFile(txt_proto1, txt_proto2));

        TS_ASSERT(AreSameFile(xml_proto1.rGetOriginalSource(), xml_proto1));
        TS_ASSERT(AreSameFile(xml_proto2.rGetOriginalSource(), xml_proto2));
        TS_ASSERT(!AreSameFile(txt_proto1.rGetOriginalSource(), txt_proto1));
        TS_ASSERT(!AreSameFile(txt_proto2.rGetOriginalSource(), txt_proto2));

        FileFinder src_proto1("projects/FunctionalCuration/protocols/GraphState.txt", RelativeTo::ChasteSourceRoot);
        FileFinder src_proto2("../protocols/GraphState.txt", this_file);
        TS_ASSERT(AreSameFile(txt_proto1.rGetOriginalSource(), src_proto1));
        TS_ASSERT(AreSameFile(txt_proto2.rGetOriginalSource(), src_proto2));
        TS_ASSERT(AreSameFile(txt_proto1.rGetOriginalSource(), txt_proto2.rGetOriginalSource()));

        CheckOutputIsXml(txt_proto1);
        CheckOutputIsXml(txt_proto2);

        // Check other constructors and calling SetPath directly work too
        fs::path boost_path("projects/FunctionalCuration/protocols/GraphState.txt");
        ProtocolFileFinder txt_proto3(boost_path);
        TS_ASSERT(AreSameFile(txt_proto3.rGetOriginalSource(), src_proto1));
        CheckOutputIsXml(txt_proto3);
        ProtocolFileFinder txt_proto4;
        txt_proto4.SetPath(txt_proto3.rGetOriginalSource().GetAbsolutePath(), RelativeTo::Absolute);
        TS_ASSERT(AreSameFile(txt_proto4.rGetOriginalSource(), src_proto1));
        CheckOutputIsXml(txt_proto4);

        // Check we can re-target a finder successfully
        FileFinder src("projects/FunctionalCuration/test/protocols/test_find_index.txt", RelativeTo::ChasteSourceRoot);
        txt_proto4.SetPath(src.GetAbsolutePath(), RelativeTo::Absolute);
        TS_ASSERT(AreSameFile(txt_proto4.rGetOriginalSource(), src));
        CheckOutputIsXml(txt_proto4);
    }

    void TestErrors() throw (Exception)
    {
        ProtocolFileFinder::BeQuiet();
        TS_ASSERT_THROWS_CONTAINS(ProtocolFileFinder no_proto("projects/FunctionalCuration/not_a_protocol.txt", RelativeTo::ChasteSourceRoot),
                                  "Conversion of text protocol '");
        ProtocolFileFinder::BeQuiet(false);
    }

private:
    void CheckOutputIsXml(const ProtocolFileFinder& rFile)
    {
        const std::string path = rFile.GetAbsolutePath();
        TS_ASSERT_EQUALS(path.find(OutputFileHandler::GetChasteTestOutputDirectory()), 0u);
        TS_ASSERT_EQUALS(path.substr(path.length()-4), ".xml");
    }

    bool AreSameFile(const FileFinder& rFile1, const FileFinder& rFile2)
    {
        fs::path path1(rFile1.GetAbsolutePath());
        fs::path path2(rFile2.GetAbsolutePath());
        return fs::equivalent(path1, path2);
    }
};

#endif // TESTPROTOCOLFILEFINDER_HPP_
