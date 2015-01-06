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

#ifndef TESTSEDMLPARSING_HPP_
#define TESTSEDMLPARSING_HPP_

#include <cxxtest/TestSuite.h>

#include <vector>
#include <string>
#include <iostream>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

#include "SedmlParser.hpp"
#include "Protocol.hpp"

#include "OutputFileHandler.hpp"
#include "FileFinder.hpp"
#include "NumericFileComparison.hpp"

class TestSedmlParsing : public CxxTest::TestSuite
{
public:
    void TestBasicSedmlSupport() throw (Exception)
    {
        OutputFileHandler handler("TestSedml");
        FileFinder this_test(__FILE__, RelativeTo::ChasteSourceRoot);
        FileFinder sedml_file("data/sedMLleloup_gonze_goldbeter_1999_version01.xml", this_test);

        // Parse the SED-ML
        SedmlParser sedml_parser;
        ProtocolPtr p_proto = sedml_parser.ParseSedml(sedml_file, handler);

        // Run the protocol
        p_proto->Run();
        p_proto->WriteToFile(handler, "outputs");

        // Test the results
        std::cout << "Comparing results against saved data..." << std::endl;
        FileFinder data_folder("data", this_test);
        std::vector<std::string> output_names = boost::assign::list_of("tim1")("tim2")("per_tim")("per_tim2")("time")("tim1_norm");
        BOOST_FOREACH(std::string output_name, output_names)
        {
            std::string filename = "outputs_" + output_name + ".csv";
            FileFinder ref_output(filename, data_folder);
            FileFinder test_output = handler.FindFile(filename);
            NumericFileComparison comp(test_output.GetAbsolutePath(), ref_output.GetAbsolutePath());
            TS_ASSERT(comp.CompareFiles(5e-4));
        }
    }
};

#endif // TESTSEDMLPARSING_HPP_
