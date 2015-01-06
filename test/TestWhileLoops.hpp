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

#ifndef TESTWHILELOOPS_HPP_
#define TESTWHILELOOPS_HPP_

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

#include <cxxtest/TestSuite.h>

#include "ProtocolRunner.hpp"
#include "ProtocolLanguage.hpp"
#include "ProtoHelperMacros.hpp"
#include "VectorStreaming.hpp"
#include "ProtocolFileFinder.hpp"

#include "FileFinder.hpp"
#include "NumericFileComparison.hpp"

class TestWhileLoops : public CxxTest::TestSuite
{
public:
    void TestSimpleIteration() throw (Exception)
    {
        std::string dirname = "TestWhileLoops";
        ProtocolFileFinder proto_xml_file("projects/FunctionalCuration/test/protocols/xml/test_while_loop.xml", RelativeTo::ChasteSourceRoot);
        DoTest(dirname, proto_xml_file);
    }

    void TestCompactSyntax() throw (Exception)
	{
        std::string dirname = "TestWhileLoops_Compact";
        ProtocolFileFinder proto_file("projects/FunctionalCuration/test/protocols/test_while_loop.txt", RelativeTo::ChasteSourceRoot);
        DoTest(dirname, proto_file);
	}

private:
    void DoTest(const std::string& rDirname, const ProtocolFileFinder& rProtocol)
    {
        FileFinder cellml_file("projects/FunctionalCuration/cellml/luo_rudy_1991.cellml", RelativeTo::ChasteSourceRoot);

        ProtocolRunner runner(cellml_file, rProtocol, rDirname);

        // How many loops to run
        const unsigned N = 10u;
        runner.GetProtocol()->SetInput("num_iters", CONST(N));

        // Run
        runner.RunProtocol();
        FileFinder success_file(rDirname + "/success", RelativeTo::ChasteTestOutput);
        TS_ASSERT(success_file.Exists());

        // Check the final Vs are correct
        const Environment& r_outputs = runner.GetProtocol()->rGetOutputsCollection();
        std::vector<std::string> V_names = boost::assign::list_of("V0")("V1")("V2")("V3")("V4")("V5");
        BOOST_FOREACH(const std::string& r_name, V_names)
        {
            CheckFinalV(r_outputs, r_name, N);
        }
        CheckFinalV(r_outputs, "V_3001", 3001);
        CheckFinalV(r_outputs, "V_empty", 1);
    }

    void CheckFinalV(const Environment& rOutputs, const std::string& rOutputName, unsigned N)
    {
        NdArray<double> V = GET_ARRAY(rOutputs.Lookup(rOutputName));
        TSM_ASSERT_EQUALS(rOutputName, V.GetNumElements(), N);
        NdArray<double>::Indices idxs = V.GetIndices();
        idxs[0] = N-1;
        TSM_ASSERT_DELTA(rOutputName, V[idxs], N-1, 1e-12);
    }
};

#endif // TESTWHILELOOPS_HPP_
