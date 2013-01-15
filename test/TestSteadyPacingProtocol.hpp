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
    void TestSteadyStateCompact() throw (Exception)
    {
        std::string dirname = "TestSteadyStateCompact";
        FileFinder cellml_file("projects/FunctionalCuration/cellml/luo_rudy_1991.cellml", RelativeTo::ChasteSourceRoot);
        ProtocolFileFinder proto_xml_file("projects/FunctionalCuration/test/protocols/compact/SteadyStateRunner.txt", RelativeTo::ChasteSourceRoot);

        ProtocolRunner runner(cellml_file, proto_xml_file, dirname, true);

        // Assume we get to steady state quickly
        runner.GetProtocol()->SetInput("max_paces", CONST(100));

        // Run
        runner.RunProtocol();
    }

    void TestSteadyPacingProtocolRunning() throw(Exception, std::bad_alloc)
    {
        std::string dirname = "TestSteadyPacingProtocolOutputs";
        FileFinder cellml_file("projects/FunctionalCuration/cellml/luo_rudy_1991.cellml", RelativeTo::ChasteSourceRoot);
        ProtocolFileFinder proto_xml_file("projects/FunctionalCuration/test/protocols/SteadyPacing.xml", RelativeTo::ChasteSourceRoot);

        ProtocolRunner runner(cellml_file, proto_xml_file, dirname, true);

        // Assume we get to steady state quickly - CHANGE THIS FOR DECENT ANSWERS!
        runner.GetProtocol()->SetInput("steady_state_beats", CONST(10));
        runner.GetProtocol()->SetInput("pacing_period", CONST(1000)); // Period of pacing in ms.

        // Run
        runner.RunProtocol();

        // Check the results of postprocessing are correct.
        const Environment& r_outputs = runner.GetProtocol()->rGetOutputsCollection();
        NdArray<double> peak_voltage = GET_ARRAY(r_outputs.Lookup("peak_voltage"));
        TS_ASSERT_EQUALS(peak_voltage.GetNumDimensions(), 0u);
        TS_ASSERT_EQUALS(peak_voltage.GetNumElements(), 1u);
        TS_ASSERT_DELTA(*peak_voltage.Begin(), 46.94, 1e-2);

        NdArray<double> apd90 = GET_ARRAY(r_outputs.Lookup("apd"));
        TS_ASSERT_EQUALS(apd90.GetNumDimensions(), 0u);
        TS_ASSERT_EQUALS(apd90.GetNumElements(), 1u);
        TS_ASSERT_DELTA(*apd90.Begin(), 361.8468, 1e-2);
    }
};

#endif // TESTSTEADYPACINGPROTOCOL_HPP_
