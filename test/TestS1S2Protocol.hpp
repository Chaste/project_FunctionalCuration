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

#ifndef TESTS1S2PROTOCOL_HPP_
#define TESTS1S2PROTOCOL_HPP_

#include <string>
#include <set>
#include <map>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <cxxtest/TestSuite.h>

#include "ProtocolRunner.hpp"
#include "ProtocolLanguage.hpp"
#include "ProtoHelperMacros.hpp"

#include "FileFinder.hpp"
#include "NumericFileComparison.hpp"

class TestS1S2Protocol : public CxxTest::TestSuite
{
public:
    void TestXmlSyntax() throw(Exception, std::bad_alloc)
    {
        std::string dirname = "TestS1S2ProtocolOutputs";
        FileFinder cellml_file("projects/FunctionalCuration/cellml/luo_rudy_1991.cellml", RelativeTo::ChasteSourceRoot);
        // Assume we get to steady state quickly
        ProtocolFileFinder proto_xml_file("projects/FunctionalCuration/test/protocols/xml/test_S1S2.xml", RelativeTo::ChasteSourceRoot);
        DoTest(dirname, proto_xml_file, cellml_file, 0.212);
    }

    void TestCompactSyntax() throw(Exception, std::bad_alloc)
    {
        std::string dirname = "TestS1S2ProtocolOutputs_Compact";
        FileFinder cellml_file("projects/FunctionalCuration/cellml/luo_rudy_1991.cellml", RelativeTo::ChasteSourceRoot);
        // Assume we get to steady state quickly
        ProtocolFileFinder proto_xml_file("projects/FunctionalCuration/test/protocols/test_S1S2.txt", RelativeTo::ChasteSourceRoot);
        DoTest(dirname, proto_xml_file, cellml_file, 0.212);
    }

    // This model has time units in seconds, so we're checking that conversion works
    void TestNobleModel() throw(Exception, std::bad_alloc)
    {
        std::string dirname = "TestS1S2ProtocolOutputs_EarmNobleModel";
        FileFinder cellml_file("projects/FunctionalCuration/cellml/earm_noble_model_1990.cellml", RelativeTo::ChasteSourceRoot);
        // Assume we get to steady state quickly
        ProtocolFileFinder proto_xml_file("projects/FunctionalCuration/test/protocols/xml/test_S1S2.xml", RelativeTo::ChasteSourceRoot);
        DoTest(dirname, proto_xml_file, cellml_file, 0.0264);
    }

private:
    void DoTest(const std::string& rDirName, const ProtocolFileFinder& rProtocolFile, const FileFinder& rCellmlFile,
                double expectedSlope)
    {
        ProtocolRunner runner(rCellmlFile, rProtocolFile, rDirName, true);

        // Don't do too many runs
        std::vector<AbstractExpressionPtr> s2_intervals
            = EXPR_LIST(CONST(1000))(CONST(900))(CONST(800))(CONST(700))(CONST(600))(CONST(500));
        DEFINE(s2_intervals_expr, boost::make_shared<ArrayCreate>(s2_intervals));
        runner.GetProtocol()->SetInput("s2_intervals", s2_intervals_expr);

        // Run
        runner.RunProtocol();
        FileFinder success_file(rDirName + "/success", RelativeTo::ChasteTestOutput);
        TS_ASSERT(success_file.Exists());

        // Check the max slope hasn't changed.
        const Environment& r_outputs = runner.GetProtocol()->rGetOutputsCollection();
        NdArray<double> max_slope = GET_ARRAY(r_outputs.Lookup("max_S1S2_slope"));
        TS_ASSERT_EQUALS(max_slope.GetNumElements(), 1u);
        TS_ASSERT_DELTA(*max_slope.Begin(), expectedSlope, 1e-3);

        // Check we did the right number of timesteps (overridden protocol input)
        NdArray<double> voltage = GET_ARRAY(r_outputs.Lookup("membrane_voltage"));
        TS_ASSERT_EQUALS(voltage.GetNumDimensions(), 2u);
        TS_ASSERT_EQUALS(voltage.GetShape()[0], s2_intervals.size());
        TS_ASSERT_EQUALS(voltage.GetShape()[1], 2001u);

        CheckManifest(rDirName, rCellmlFile.GetLeafNameNoExtension(), runner.GetProtocol());
    }

    void CheckManifest(const std::string& rDirName, const std::string& rModelName, ProtocolPtr pProto)
    {
        FileFinder manifest_file(rDirName + "/manifest.xml", RelativeTo::ChasteTestOutput);
        TS_ASSERT(manifest_file.Exists());
//      <content location="./manifest.xml" format="http://identifiers.org/combine.specifications/omex-manifest"/>
        std::map<std::string, std::string> ext_map = boost::assign::map_list_of
                ("eps", "application/postscript")
                ("csv", "text/csv")
                ("txt", "text/plain")
                ("cellml", "http://identifiers.org/combine.specifications/cellml.1.0")
                ("xml", "text/xml")
                ("hpp", "text/plain")
                ("cpp", "text/plain")
                ("gp", "text/plain")
                ("so", "application/octet-stream");
        std::set<std::string> entries = boost::assign::list_of("machine_info_0.txt")("model_info.txt")("provenance_info_0.txt")
                ("trace.txt")("outputs-contents.csv")("outputs-default-plots.csv")
                ("Action_potential_traces.eps")("outputs_Action_potential_traces_gnuplot_data.csv")("outputs_Action_potential_traces_gnuplot_data.gp")
                ("S1-S2_curve.eps")("outputs_S1-S2_curve_gnuplot_data.csv")("outputs_S1-S2_curve_gnuplot_data.gp")
                ("outputs_APD90.csv")("outputs_DI.csv")("outputs_max_S1S2_slope.csv")("outputs_membrane_voltage.csv")
                ("outputs_raw_APD90.csv")("outputs_raw_DI.csv")("outputs_S1S2_slope.csv")("outputs_s2_intervals.csv")("outputs_time_1d.csv")
                ("outputs_PCLs.csv")("outputs_restitution_curve_gnuplot_data.csv")
                ("restitution_curve.eps")("outputs_restitution_curve_gnuplot_data.gp");
        entries.insert("lib" + rModelName + ".so");
        std::vector<std::string> suffixes = boost::assign::list_of(".cellml")("-conf.xml")(".cpp")(".hpp");
        BOOST_FOREACH(std::string suffix, suffixes)
        {
            entries.insert(rModelName + suffix);
        }
        std::map<std::string,std::string> expected;
        expected["success"] = "text/plain";
        expected["manifest.xml"] = "http://identifiers.org/combine.specifications/omex-manifest";
        BOOST_FOREACH(const std::string& r_entry, entries)
        {
            std::size_t dot_pos = r_entry.rfind('.');
            std::string ext = r_entry.substr(dot_pos + 1);
            expected[r_entry] = ext_map[ext];
        }
        TS_ASSERT_EQUALS(expected, pProto->rGetManifest().rGetEntries());
    }
};

#endif // TESTS1S2PROTOCOL_HPP_
