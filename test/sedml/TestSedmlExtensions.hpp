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

#ifndef TESTSEDMLEXTENSIONS_HPP_
#define TESTSEDMLEXTENSIONS_HPP_

#include <cxxtest/TestSuite.h>

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

#include "SedmlParser.hpp"
#include "Protocol.hpp"
#include "ProtoHelperMacros.hpp"
#include "NdArray.hpp"

#include "FileFinder.hpp"
#include "OutputFileHandler.hpp"

class TestSedmlExtensions : public CxxTest::TestSuite
{
public:
    void TestCombinedTask() throw (Exception)
    {
        OutputFileHandler handler("TestSedmlExtensions_CombinedTask");
        FileFinder this_test(__FILE__, RelativeTo::ChasteSourceRoot);
        FileFinder sedml_file("data/test_combined_task.xml", this_test);

        // Parse the SED-ML
        SedmlParser sedml_parser;
        ProtocolPtr p_proto = sedml_parser.ParseSedml(sedml_file, handler);

        // Run the protocol
        p_proto->Run();
        p_proto->WriteToFile(handler, "outputs");

        // Test the results
        std::cout << "Checking results..." << std::endl;
        const Environment& r_outputs = p_proto->rGetOutputsCollection();
        std::vector<std::string> time1_names = boost::assign::list_of("time1")("time1p")("time1c")("time1n");
        BOOST_FOREACH(std::string name, time1_names)
        {
            NdArray<double> result = GET_ARRAY(r_outputs.Lookup(name, "TestCombinedTask"));
            const unsigned num_dims = (*name.rbegin() == 'n' ? 2 : 1);
            const unsigned num_runs = (*name.rbegin() == 'n' ? 3 : 1);
            TS_ASSERT_EQUALS(result.GetNumDimensions(), num_dims);
            NdArray<double>::Iterator iter = result.Begin();
            for (unsigned d=0; d<num_runs; ++d)
            {
                for (unsigned t=0; t<=10; ++t)
                {
                    TS_ASSERT_DELTA(*iter, (double)t, 1e-10);
                    iter++;
                }
            }
        }
        std::vector<std::string> time2_names = boost::assign::list_of("time2")("time2p")("time2c")("time2n");
        BOOST_FOREACH(std::string name, time2_names)
        {
            NdArray<double> result = GET_ARRAY(r_outputs.Lookup(name, "TestCombinedTask"));
            const unsigned num_dims = (*name.rbegin() == 'n' ? 2 : 1);
            const unsigned num_runs = (*name.rbegin() == 'n' ? 3 : 1);
            TS_ASSERT_EQUALS(result.GetNumDimensions(), num_dims);
            NdArray<double>::Iterator iter = result.Begin();
            for (unsigned d=0; d<num_runs; ++d)
            {
                for (unsigned t=10; t<=20; ++t)
                {
                    TS_ASSERT_DELTA(*iter, (double)t, 1e-10);
                    iter++;
                }
            }
        }
        std::vector<std::string> V1_names = boost::assign::list_of("V1")("V1p")("V1c")("V1n");
        BOOST_FOREACH(std::string name, V1_names)
        {
            NdArray<double> result = GET_ARRAY(r_outputs.Lookup(name, "TestCombinedTask"));
            const unsigned num_dims = (*name.rbegin() == 'n' ? 2 : 1);
            const unsigned num_runs = (*name.rbegin() == 'n' ? 3 : 1);
            TS_ASSERT_EQUALS(result.GetNumDimensions(), num_dims);
            NdArray<double>::Iterator iter = result.Begin();
            for (unsigned d=0; d<num_runs; ++d)
            {
                for (unsigned t=0; t<=10; ++t)
                {
                    TS_ASSERT_DELTA(*iter, (double)(t + d*20), 1e-10);
                    iter++;
                }
            }
        }
        std::vector<std::string> V2_names = boost::assign::list_of("V2")("V2p")("V2c")("V2n");
        BOOST_FOREACH(std::string name, V2_names)
        {
            NdArray<double> result = GET_ARRAY(r_outputs.Lookup(name, "TestCombinedTask"));
            const unsigned num_dims = (*name.rbegin() == 'n' ? 2 : 1);
            const unsigned num_runs = (*name.rbegin() == 'n' ? 3 : 1);
            const unsigned reset_factor = (*name.rbegin() != 'p');
            TS_ASSERT_EQUALS(result.GetNumDimensions(), num_dims);
            NdArray<double>::Iterator iter = result.Begin();
            for (unsigned d=0; d<num_runs; ++d)
            {
                for (unsigned t=0; t<=10; ++t)
                {
                    TS_ASSERT_DELTA(*iter, (double)(t + (10+d*20)*reset_factor), 1e-10);
                    iter++;
                }
            }
        }
    }
};

#endif // TESTSEDMLEXTENSIONS_HPP_
