/*

Copyright (C) University of Oxford, 2005-2011

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.

This file is part of Chaste.

Chaste is free software: you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Chaste is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details. The offer of Chaste under the terms of the
License is subject to the License being interpreted in accordance with
English Law and subject to any action against the University of Oxford
being under the jurisdiction of the English Courts.

You should have received a copy of the GNU Lesser General Public License
along with Chaste. If not, see <http://www.gnu.org/licenses/>.

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

#include "FileFinder.hpp"
#include "NumericFileComparison.hpp"

class TestWhileLoops : public CxxTest::TestSuite
{
public:
    void TestSimpleIteration() throw (Exception)
    {
        std::string dirname = "TestWhileLoops";
        FileFinder cellml_file("projects/FunctionalCuration/cellml/luo_rudy_1991.cellml", RelativeTo::ChasteSourceRoot);
        FileFinder proto_xml_file("projects/FunctionalCuration/test/protocols/test_while_loop.xml", RelativeTo::ChasteSourceRoot);

        ProtocolRunner runner(cellml_file, proto_xml_file, dirname);

        // How many loops to run
        const unsigned N = 10u;
        runner.GetProtocol()->SetInput("num_iters", CONST(N));

        // Run
        runner.RunProtocol();
        FileFinder success_file(dirname + "/success", RelativeTo::ChasteTestOutput);
        TS_ASSERT(success_file.Exists());

        // Check the final Vs are correct
        const Environment& r_outputs = runner.GetProtocol()->rGetOutputsCollection();
        std::vector<std::string> V_names = boost::assign::list_of("V0")("V1")("V2")("V3");
        BOOST_FOREACH(const std::string& r_name, V_names)
        {
            CheckFinalV(r_outputs, r_name, N);
        }
        CheckFinalV(r_outputs, "V_3001", 3001);
    }

    void TestErrorOnEmptyLoop() throw (Exception)
    {
        std::string dirname = "TestWhileLoops/EmptyLoop";
        FileFinder cellml_file("projects/FunctionalCuration/cellml/luo_rudy_1991.cellml", RelativeTo::ChasteSourceRoot);
        FileFinder proto_xml_file("projects/FunctionalCuration/test/protocols/test_while_loop.xml", RelativeTo::ChasteSourceRoot);
        ProtocolRunner runner(cellml_file, proto_xml_file, dirname);
        runner.GetProtocol()->SetInput("num_iters", CONST(0));
        TS_ASSERT_THROWS_CONTAINS(runner.RunProtocol(), "A while loop condition must hold initially.");
    }
private:
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
