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

#ifndef TESTS1S2PROTOCOL_HPP_
#define TESTS1S2PROTOCOL_HPP_

#include <string>
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
        // Assume we get to steady state quickly
        ProtocolFileFinder proto_xml_file("projects/FunctionalCuration/test/protocols/test_S1S2.xml", RelativeTo::ChasteSourceRoot);
        DoTest(dirname, proto_xml_file);
    }

    void TestCompactSyntax() throw(Exception, std::bad_alloc)
    {
        std::string dirname = "TestS1S2ProtocolOutputs_Compact";
        // Assume we get to steady state quickly
        ProtocolFileFinder proto_xml_file("projects/FunctionalCuration/test/protocols/compact/test_S1S2.txt", RelativeTo::ChasteSourceRoot);
        DoTest(dirname, proto_xml_file);
    }

private:
    void DoTest(const std::string& rDirName, const ProtocolFileFinder& rProtocolFile)
    {
        FileFinder cellml_file("projects/FunctionalCuration/cellml/luo_rudy_1991.cellml", RelativeTo::ChasteSourceRoot);
        ProtocolRunner runner(cellml_file, rProtocolFile, rDirName, true);

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
        TS_ASSERT_DELTA(*max_slope.Begin(), 0.212, 1e-3);
    }
};

#endif // TESTS1S2PROTOCOL_HPP_
