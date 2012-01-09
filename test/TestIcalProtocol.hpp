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

#ifndef TESTICALPROTOCOL_HPP_
#define TESTICALPROTOCOL_HPP_

#include <string>
#include <cxxtest/TestSuite.h>

#include "ProtocolRunner.hpp"
#include "ProtocolLanguage.hpp"
#include "ProtoHelperMacros.hpp"

#include "FileFinder.hpp"
#include "OutputFileHandler.hpp"

#include "NumericFileComparison.hpp"

class TestIcalProtocol : public CxxTest::TestSuite
{
public:
    void TestShortIcal() throw (Exception)
    {
        std::string dirname = "TestICaLProtocolOutputs";
        std::string model_name = "fox_mcharg_gilmour_2002";
        FileFinder cellml_file("projects/FunctionalCuration/cellml/" + model_name + ".cellml", RelativeTo::ChasteSourceRoot);
        FileFinder proto_xml_file("projects/FunctionalCuration/test/protocols/ICaL.xml", RelativeTo::ChasteSourceRoot);

        ProtocolRunner runner(cellml_file, proto_xml_file, dirname);

        // Don't do too many runs
        std::vector<AbstractExpressionPtr> test_potentials
            = EXPR_LIST(CONST(-45.01))(CONST(-25.01))(CONST(0.01))(CONST(15.01))(CONST(40.01))(CONST(79.99));
        DEFINE(test_potentials_expr, boost::make_shared<ArrayCreate>(test_potentials));
        runner.GetProtocol()->SetInput("test_potentials", test_potentials_expr);
        runner.GetProtocol()->SetInput("steady_state_time", CONST(1000));

        runner.RunProtocol();
        FileFinder success_file(dirname + "/success", RelativeTo::ChasteTestOutput);
        TS_ASSERT(success_file.Exists());

        // Check the key outputs haven't changed.
        const Environment& r_outputs = runner.GetProtocol()->rGetOutputsCollection();
        NdArray<double> min_LCC = GET_ARRAY(r_outputs.Lookup("min_LCC"));
        TS_ASSERT_EQUALS(min_LCC.GetNumElements(), 18u);
        TS_ASSERT_EQUALS(min_LCC.GetNumDimensions(), 2u);
        TS_ASSERT_EQUALS(min_LCC.GetShape()[0], 3u);
        const double expected[] = {-0.0144, -0.1105, -0.4718, -0.3094, -0.0929, -0.0066,
                                   -0.0273, -0.2159, -0.9366, -0.6148, -0.1848, -0.0143,
                                   -0.0398, -0.3192, -1.3979, -0.9178, -0.2758, -0.0219};
        NdArray<double>::ConstIterator it = min_LCC.Begin();
        for (unsigned i=0; i<18u; i++)
        {
            TS_ASSERT_DELTA(*it++, expected[i], 1e-3);
        }
    }
};

#endif // TESTICALPROTOCOL_HPP_
