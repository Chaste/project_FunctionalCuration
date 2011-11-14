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

#ifndef TESTNESTEDPROTOCOLS_HPP_
#define TESTNESTEDPROTOCOLS_HPP_

#include <cxxtest/TestSuite.h>

#include "ProtocolRunner.hpp"
#include "OutputFileHandler.hpp"
#include "ProtoHelperMacros.hpp"

class TestNestedProtocols : public CxxTest::TestSuite
{
public:
    void TestSimpleExample() throw (Exception)
    {
        std::string dirname = "TestNestedProtocols";
        std::string model_name = "luo_rudy_1991";
        FileFinder cellml_file("projects/FunctionalCuration/cellml/" + model_name + ".cellml", RelativeTo::ChasteSourceRoot);
        FileFinder proto_xml_file("projects/FunctionalCuration/test/protocols/test_nested_protocol.xml", RelativeTo::ChasteSourceRoot);

        ProtocolRunner runner(cellml_file, proto_xml_file, dirname);
        runner.RunProtocol();
        FileFinder success_file(dirname + "/success", RelativeTo::ChasteTestOutput);
        TS_ASSERT(success_file.Exists());

        // Check results
        const Environment& r_outputs = runner.GetProtocol()->rGetOutputsCollection();
        NdArray<double> peak_V = GET_ARRAY(r_outputs.Lookup("peak_voltage", "test"));
        NdArray<double> apd90 = GET_ARRAY(r_outputs.Lookup("apd90", "test"));
        TS_ASSERT_EQUALS(peak_V.GetNumDimensions(), 2u);
        TS_ASSERT_EQUALS(peak_V.GetShape()[0], 2u);
        TS_ASSERT_EQUALS(peak_V.GetNumElements(), 2u);
        TS_ASSERT_EQUALS(apd90.GetNumDimensions(), 2u);
        TS_ASSERT_EQUALS(apd90.GetShape()[0], 2u);
        TS_ASSERT_EQUALS(apd90.GetNumElements(), 2u);
        NdArray<double>::Indices idxs = peak_V.GetIndices();
        TS_ASSERT_DELTA(peak_V[idxs], 46.959, 1e-3);
        TS_ASSERT_DELTA(apd90[idxs], 359.015, 1e-3);
        peak_V.IncrementIndices(idxs);
        TS_ASSERT_DELTA(peak_V[idxs], 46.939, 1e-3);
        TS_ASSERT_DELTA(apd90[idxs], 361.847, 1e-3);
    }
};

#endif // TESTNESTEDPROTOCOLS_HPP_
