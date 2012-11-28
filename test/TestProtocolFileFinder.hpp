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
    	std::string chaste_test_output = OutputFileHandler::GetChasteTestOutputDirectory();
    	ProtocolFileFinder xml_proto1("projects/FunctionalCuration/test/protocols/GraphState.xml", RelativeTo::ChasteSourceRoot);
    	ProtocolFileFinder txt_proto1("projects/FunctionalCuration/test/protocols/compact/GraphState.txt", RelativeTo::ChasteSourceRoot);
    	FileFinder this_file(__FILE__, RelativeTo::ChasteSourceRoot);
    	ProtocolFileFinder xml_proto2("protocols/GraphState.xml", this_file);
		ProtocolFileFinder txt_proto2("protocols/compact/GraphState.txt", this_file);

		TS_ASSERT(AreSameFile(xml_proto1, xml_proto2));
		TS_ASSERT(!AreSameFile(txt_proto1, txt_proto2));

		const std::string txt_path1 = txt_proto1.GetAbsolutePath();
		const std::string txt_path2 = txt_proto2.GetAbsolutePath();

		TS_ASSERT_EQUALS(txt_path1.find(chaste_test_output), 0);
		TS_ASSERT_EQUALS(txt_path2.find(chaste_test_output), 0);
		TS_ASSERT_EQUALS(txt_path1.substr(txt_path1.length()-4), ".xml");
		TS_ASSERT_EQUALS(txt_path2.substr(txt_path2.length()-4), ".xml");
    }

private:
    bool AreSameFile(const FileFinder& rFile1, const FileFinder& rFile2)
    {
    	fs::path path1(rFile1.GetAbsolutePath());
    	fs::path path2(rFile2.GetAbsolutePath());
    	return fs::equivalent(path1, path2);
    }
};

#endif // TESTPROTOCOLFILEFINDER_HPP_
