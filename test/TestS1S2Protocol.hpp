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
    void TestS1S2ProtocolRunning() throw(Exception, std::bad_alloc)
    {
        std::string dirname = "TestS1S2ProtocolOutputs";
        FileFinder cellml_file("projects/FunctionalCuration/cellml/luo_rudy_1991.cellml", RelativeTo::ChasteSourceRoot);
        // Assume we get to steady state quickly
        FileFinder proto_xml_file("projects/FunctionalCuration/test/protocols/test_S1S2.xml", RelativeTo::ChasteSourceRoot);

        ProtocolRunner runner(cellml_file, proto_xml_file, dirname, true);

        // Don't do too many runs
        std::vector<AbstractExpressionPtr> s2_intervals
            = EXPR_LIST(CONST(1000))(CONST(900))(CONST(800))(CONST(700))(CONST(600))(CONST(500));
        DEFINE(s2_intervals_expr, boost::make_shared<ArrayCreate>(s2_intervals));
        runner.GetProtocol()->SetInput("s2_intervals", s2_intervals_expr);

        // Run
        runner.RunProtocol();
        FileFinder success_file(dirname + "/success", RelativeTo::ChasteTestOutput);
        TS_ASSERT(success_file.Exists());

        // Check the max slope hasn't changed.
        const Environment& r_outputs = runner.GetProtocol()->rGetOutputsCollection();
        NdArray<double> max_slope = GET_ARRAY(r_outputs.Lookup("max_S1S2_slope"));
        TS_ASSERT_EQUALS(max_slope.GetNumElements(), 1u);
        TS_ASSERT_DELTA(*max_slope.Begin(), 0.212, 1e-3);
    }
};

#endif // TESTS1S2PROTOCOL_HPP_
