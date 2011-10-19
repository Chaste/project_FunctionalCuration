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

#ifndef TESTACCESSINGSTATEVECTOR_HPP_
#define TESTACCESSINGSTATEVECTOR_HPP_

#include <cxxtest/TestSuite.h>
#include <boost/foreach.hpp>

#include "ProtocolRunner.hpp"
#include "OutputFileHandler.hpp"

#include "NumericFileComparison.hpp"

class TestAccessingStateVector : public CxxTest::TestSuite
{
public:
    void TestGraphState() throw (Exception)
    {
        std::string dirname = "TestAccessingStateVector";
        std::string model_name = "luo_rudy_1991";
        FileFinder cellml_file("projects/FunctionalCuration/cellml/" + model_name + ".cellml", RelativeTo::ChasteSourceRoot);
        FileFinder proto_xml_file("projects/FunctionalCuration/test/private/GraphState.xml", RelativeTo::ChasteSourceRoot);

        ProtocolRunner runner(cellml_file, proto_xml_file, dirname);
        runner.RunProtocol();
        FileFinder success_file(dirname + "/success", RelativeTo::ChasteTestOutput);
        TS_ASSERT(success_file.Exists());

        // Compare the results with original data

        // Test metadata files
        std::vector<std::string> filenames = list_of("-contents.csv")("-steppers.csv")("-default-plots.csv");
        BOOST_FOREACH(std::string filename, filenames)
        {
            FileFinder ref_file("projects/FunctionalCuration/test/private/data/TestAccessingStateVector/outputs" + filename, RelativeTo::ChasteSourceRoot);
            FileFinder test_file(dirname + "/" + filename, RelativeTo::ChasteTestOutput);
            TS_ASSERT_THROWS_NOTHING(EXPECT0(system, "diff -u " + test_file.GetAbsolutePath() + " " + ref_file.GetAbsolutePath()));
        }

//        for (unsigned i=0; i<2; i++)
//        {
//            std::string output_name;
//            if (0u==i)
//            {
//                output_name = "outputs_min_LCC";
//            }
//            else
//            {
//                output_name = "outputs_final_membrane_voltage";
//            }
//
//            std::cout << "Comparing results of ICaL protocol: " << output_name << "...";
//            FileFinder ref_output("projects/FunctionalCuration/test/data/historic/" + model_name + "/ICaL/" + output_name + ".dat",
//                                  RelativeTo::ChasteSourceRoot);
//            OutputFileHandler handler(dirname, false);
//            FileFinder test_output = handler.FindFile(output_name + ".csv");
//
//            NumericFileComparison comp(test_output.GetAbsolutePath(), ref_output.GetAbsolutePath());
//            TS_ASSERT(comp.CompareFiles(2e-4));
//
//            std::cout << "done.\n";
//        }
    }
};

#endif // TESTACCESSINGSTATEVECTOR_HPP_
