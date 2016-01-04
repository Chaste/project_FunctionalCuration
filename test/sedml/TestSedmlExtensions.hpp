/*

Copyright (c) 2005-2016, University of Oxford.
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
#include "FileComparison.hpp"
#include "Exception.hpp"

class TestSedmlExtensions : public CxxTest::TestSuite
{
public:
    void TestRepeatedTask() throw (Exception)
    {
        OutputFileHandler handler("TestSedmlExtensions_RepeatedTask");
        FileFinder this_test(__FILE__, RelativeTo::ChasteSourceRoot);
        FileFinder sedml_file("data/test_repeated_task.xml", this_test);

        // Parse the SED-ML
        SedmlParser sedml_parser;
        ProtocolPtr p_proto = sedml_parser.ParseSedml(sedml_file, handler);

        // Run the protocol
        p_proto->SetOutputFolder(handler);
        p_proto->RunAndWrite("outputs");
        TS_ASSERT(handler.FindFile("success").Exists());

        // Test the results
        std::cout << "Checking results..." << std::endl;
        const Environment& r_outputs = p_proto->rGetOutputsCollection();

        std::vector<std::string> tasks = boost::assign::list_of("utc")("repeat")("utc_repeat")("utc_set_model")
                ("functional_range1")("functional_range2");
        std::vector<double> t_offsets = boost::assign::list_of(0)(0)(1)(1)(1)(1);
        std::vector<double> V_offsets = boost::assign::list_of(0)(4)(1)(1)(11)(11);
        std::vector<double> V_increments = boost::assign::list_of(1)(1)(1)(0)(1)(1);
        std::vector<unsigned> extra_dim_sizes = boost::assign::list_of(0)(3)(0)(0)(0)(0);

        for (unsigned i=0; i<6u; ++i)
        {
            std::string task = "task_" + tasks[i];
            NdArray<double> V = GET_ARRAY(r_outputs.Lookup("V_"+task, "TestRepeatedTask-"+task));
            NdArray<double> t = GET_ARRAY(r_outputs.Lookup("t_"+task, "TestRepeatedTask-"+task));
            // Check result shapes
            unsigned num_dims = extra_dim_sizes[i] > 0u ? 2u : 1u;
            TSM_ASSERT_EQUALS(task, V.GetNumDimensions(), num_dims);
            TSM_ASSERT_EQUALS(task, t.GetNumDimensions(), num_dims);
            TSM_ASSERT_EQUALS(task, V.GetShape().back(), 11u);
            TSM_ASSERT_EQUALS(task, t.GetShape().back(), 11u);
            unsigned dim0_size = extra_dim_sizes[i] > 0u ? extra_dim_sizes[i] : 1u;
            if (num_dims == 2u)
            {
                TSM_ASSERT_EQUALS(task, V.GetShape().front(), dim0_size);
                TSM_ASSERT_EQUALS(task, t.GetShape().front(), dim0_size);
            }
            // Check result values
            NdArray<double>::ConstIterator V_it = V.Begin();
            NdArray<double>::ConstIterator t_it = t.Begin();
            for (unsigned dim0=0; dim0<dim0_size; ++dim0)
            {
                double expected_t = t_offsets[i];
                double expected_V = V_offsets[i] - dim0;
                for (unsigned dim1=0; dim1<11u; ++dim1)
                {
                    TSM_ASSERT_DELTA(task, *V_it, expected_V, 1e-6);
                    TSM_ASSERT_DELTA(task, *t_it, expected_t, 1e-6);
                    expected_t += 1;
                    expected_V += V_increments[i];
                    ++V_it;
                    ++t_it;
                }
            }
        }

        // Check the graphs
        {
            ///\todo (#1999) this only works for gnuplot 4.2
            FILE* pipe = popen("gnuplot --version", "r");
            EXCEPT_IF_NOT(pipe != NULL);
            std::string output;
            const int BUFSIZE = 1024;
            char buf[BUFSIZE];
            while (fgets(buf, sizeof (buf), pipe))
            {
                output += buf;
            }
            pclose(pipe);
            if (output.substr(0, 11) == "gnuplot 4.2")
            {
                std::vector<std::string> graph_names = boost::assign::list_of("Simple_uniform_timecourse_-_c1")
                    ("Simple_repetition_-_c2")("Uniform_timecourse_by_repeatedTask_-_c3")
                    ("Setting_model_variable_-_c4")("Test_functional_range_-_c5")
                    ("Test_functional_range_with_shorthands_-_c6");
                BOOST_FOREACH(const std::string& r_graph_name, graph_names)
                {
                    FileFinder ref_graph("data/repeated_task_graphs/" + r_graph_name + ".eps", this_test);
                    FileFinder new_graph = handler.FindFile(r_graph_name + ".eps");
                    FileComparison comp(ref_graph, new_graph);
                    comp.SetIgnoreCommentLines(false);
                    comp.SetIgnoreLinesBeginningWith("%");
                    comp.IgnoreLinesContaining("CreationDate");
                    comp.IgnoreLinesContaining("Title");
                    comp.IgnoreLinesContaining("Author");
                    TS_ASSERT(comp.CompareFiles());
                }
            }
        }
    }

    void TestCombinedTask() throw (Exception)
    {
        OutputFileHandler handler("TestSedmlExtensions_CombinedTask");
        FileFinder this_test(__FILE__, RelativeTo::ChasteSourceRoot);
        FileFinder sedml_file("data/test_combined_task.xml", this_test);

        // Parse the SED-ML
        SedmlParser sedml_parser;
        ProtocolPtr p_proto = sedml_parser.ParseSedml(sedml_file, handler);

        // Run the protocol
        p_proto->SetOutputFolder(handler);
        p_proto->RunAndWrite("outputs");
        TS_ASSERT(handler.FindFile("success").Exists());

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
