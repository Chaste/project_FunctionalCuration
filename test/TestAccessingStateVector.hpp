/*

Copyright (C) University of Oxford, 2005-2012

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

#ifndef TESTACCESSINGSTATEVECTOR_HPP_
#define TESTACCESSINGSTATEVECTOR_HPP_

#include <cxxtest/TestSuite.h>
#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>

#include "ProtocolRunner.hpp"
#include "OutputFileHandler.hpp"

#include "ProtoHelperMacros.hpp"
#include "NumericFileComparison.hpp"

class TestAccessingStateVector : public CxxTest::TestSuite
{
public:
    void TestGraphState() throw (Exception)
    {
        std::string dirname = "TestAccessingStateVector";
        std::string model_name = "luo_rudy_1991";
        FileFinder cellml_file("projects/FunctionalCuration/cellml/" + model_name + ".cellml", RelativeTo::ChasteSourceRoot);
        FileFinder proto_xml_file("projects/FunctionalCuration/test/protocols/GraphState.xml", RelativeTo::ChasteSourceRoot);

        ProtocolRunner runner(cellml_file, proto_xml_file, dirname);
        runner.RunProtocol();
        FileFinder success_file(dirname + "/success", RelativeTo::ChasteTestOutput);
        TS_ASSERT(success_file.Exists());

        // Double-check transpose
        NdArray<double> state_trans = GET_ARRAY(runner.GetProtocol()->rGetOutputsCollection().Lookup("state", "test"));
        NdArray<double> state_raw = GET_ARRAY(runner.GetProtocol()->rGetOutputsCollection().Lookup("raw_state", "test"));
        for (NdArray<double>::ConstIterator it=state_raw.Begin(); it != state_raw.End(); ++it)
        {
            NdArray<double>::Indices idxs(2);
            idxs[0] = it.rGetIndices()[1];
            idxs[1] = it.rGetIndices()[0];
            TS_ASSERT_EQUALS(*it, state_trans[idxs]);
        }

        // Compare the results with original data
        FileFinder ref_dir("projects/FunctionalCuration/test/data/TestAccessingStateVector", RelativeTo::ChasteSourceRoot);
        FileFinder out_dir(dirname, RelativeTo::ChasteTestOutput);

        // Test metadata files
        std::vector<std::string> filenames = boost::assign::list_of("-contents.csv")("-steppers.csv")("-default-plots.csv");
        BOOST_FOREACH(std::string filename, filenames)
        {
            FileFinder ref_file("outputs" + filename, ref_dir);
            FileFinder test_file("outputs" + filename, out_dir);
            TS_ASSERT_THROWS_NOTHING(EXPECT0(system, "diff -u " + test_file.GetAbsolutePath() + " " + ref_file.GetAbsolutePath()));
        }

        filenames = boost::assign::list_of("outputs_raw_state")("outputs_state");
        BOOST_FOREACH(std::string output_name, filenames)
        {
            FileFinder ref_output(output_name + ".csv", ref_dir);
            FileFinder test_output(output_name + ".csv", out_dir);
            NumericFileComparison comp(test_output.GetAbsolutePath(), ref_output.GetAbsolutePath());
            TS_ASSERT(comp.CompareFiles(1e-4, 0, false));
        }

    }
};

#endif // TESTACCESSINGSTATEVECTOR_HPP_
