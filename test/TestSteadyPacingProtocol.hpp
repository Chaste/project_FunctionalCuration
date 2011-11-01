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

#ifndef TESTSTEADYPACINGPROTOCOL_HPP_
#define TESTSTEADYPACINGPROTOCOL_HPP_

#include <string>
#include <cxxtest/TestSuite.h>

#include "ProtocolRunner.hpp"
#include "ProtocolLanguage.hpp"
#include "ProtoHelperMacros.hpp"

#include "PetscSetupAndFinalize.hpp"

class TestSteadyPacingProtocol : public CxxTest::TestSuite
{
public:
    void TestSteadyPacingProtocolRunning() throw(Exception, std::bad_alloc)
    {
        std::string dirname = "TestSteadyPacingProtocolOutputs";
        FileFinder cellml_file("projects/FunctionalCuration/cellml/luo_rudy_1991.cellml", RelativeTo::ChasteSourceRoot);
        FileFinder proto_xml_file("projects/FunctionalCuration/test/protocols/SteadyPacing.xml", RelativeTo::ChasteSourceRoot);

        ProtocolRunner runner(cellml_file, proto_xml_file, dirname, true);

        // Assume we get to steady state quickly - CHANGE THIS FOR DECENT ANSWERS!
        runner.GetProtocol()->SetInput("steady_state_beats", CONST(10));
        runner.GetProtocol()->SetInput("pacing_period", CONST(1000)); // Period of pacing in ms.

        // Run
        runner.RunProtocol();

        // Check the results of postprocessing are correct.
        const Environment& r_outputs = runner.GetProtocol()->rGetOutputsCollection();
        NdArray<double> peak_voltage = GET_ARRAY(r_outputs.Lookup("peak_voltage"));
        TS_ASSERT_EQUALS(peak_voltage.GetNumElements(), 1u);
        TS_ASSERT_DELTA(*peak_voltage.Begin(), 46.94, 1e-2);

        NdArray<double> apd90 = GET_ARRAY(r_outputs.Lookup("apd90"));
        TS_ASSERT_EQUALS(apd90.GetNumDimensions(), 1u);
        TS_ASSERT_EQUALS(apd90.GetNumElements(), 1u);
        TS_ASSERT_DELTA(*apd90.Begin(), 361.8468, 1e-2);
    }
};

#endif // TESTSTEADYPACINGPROTOCOL_HPP_
