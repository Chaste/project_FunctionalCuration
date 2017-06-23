/*

Copyright (c) 2005-2016, University of Oxford.
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

#ifndef TESTHYPOKALAEMIAPROTOCOL_HPP_
#define TESTHYPOKALAEMIAPROTOCOL_HPP_

#include <cxxtest/TestSuite.h>

#include "ProtocolRunner.hpp"
#include "ProtocolLanguage.hpp"
#include "ProtoHelperMacros.hpp"
#include "OutputFileHandler.hpp"
#include "PetscTools.hpp"

#include "UsefulFunctionsForProtocolTesting.hpp"
#include "PetscSetupAndFinalize.hpp"

class TestHypokalaemiaProtocol : public CxxTest::TestSuite
{
private:
    /**
     * Run the Hypokalaemia protocol on the given model.
     *
     * @param rCellMLFileBaseName  the cellml file to dynamically load and use.
     */
    void RunHypokalaemiaProtocol(std::string& rCellMLFileBaseName)
    {
        std::string dirname = "FunctionalCuration/" + rCellMLFileBaseName + "/Hypokalaemia";
        FileFinder cellml_file("projects/FunctionalCuration/cellml/" + rCellMLFileBaseName + ".cellml", RelativeTo::ChasteSourceRoot);
        ProtocolFileFinder proto_xml_file("projects/FunctionalCuration/test/private/protocols/Hypokalaemia.xml", RelativeTo::ChasteSourceRoot);

        try
        {
            ProtocolRunner runner(cellml_file, proto_xml_file, dirname);
            runner.GetProtocol()->SetInput("steady_state_beats", CONST(10));
            runner.RunProtocol();
        }
        catch (Exception& e)
        {
            OUR_WARN(e.GetMessage(), rCellMLFileBaseName, "Hypokalaemia");
        }
    }

public:
    void TestHypokalaemiaProtocolRunning() throw(Exception, std::bad_alloc)
    {
        PetscTools::IsolateProcesses(true);
        if (PetscTools::GetMyRank() == 0u) // Only one process should run this
        {
            std::string dirname = "TestHypokalaemiaProtocolOutputs";
            FileFinder cellml_file("projects/FunctionalCuration/cellml/luo_rudy_1991.cellml", RelativeTo::ChasteSourceRoot);
            ProtocolFileFinder proto_xml_file("projects/FunctionalCuration/test/private/protocols/Hypokalaemia.xml", RelativeTo::ChasteSourceRoot);

            ProtocolRunner runner(cellml_file, proto_xml_file, dirname, true);

            // Assume we get to steady state quickly - CHANGE THIS FOR DECENT ANSWERS!
            runner.GetProtocol()->SetInput("steady_state_beats", CONST(10));
            runner.GetProtocol()->SetInput("pacing_period", CONST(1000)); // Period of pacing in ms.

            // Run
            runner.RunProtocol();

            // Check the results of post-processing are correct.
            const Environment& r_outputs = runner.GetProtocol()->rGetOutputsCollection();
            NdArray<double> peak_voltage = GET_ARRAY(r_outputs.Lookup("peak_voltage"));
            TS_ASSERT_EQUALS(peak_voltage.GetNumElements(), 10u);
            TS_ASSERT_DELTA(*peak_voltage.Begin(), 40.3585, 5e-2);
            NdArray<double> resting_potential = GET_ARRAY(r_outputs.Lookup("resting_potential"));
            TS_ASSERT_EQUALS(resting_potential.GetNumElements(), 10u);

            // These are the results that came out first attempt (they look sensible)
            std::vector<double> reference_apd90 {570.496, 476.702, 418.564, 375.742, 361.8457, 343.841, 318.821, 298.139, 280.4469, 264.9502};
            std::vector<double> reference_resting {-98.9795, -95.7858, -90.6683, -86.0988, -84.4384, -82.1074, -78.5914, -75.4569, -72.6291, -70.0511};

            NdArray<double> apd90 = GET_ARRAY(r_outputs.Lookup("APD90"));
            NdArray<double>::Indices apd90_indices = apd90.GetIndices();
            TS_ASSERT_EQUALS(apd90.GetNumDimensions(), 2u);
            TS_ASSERT_EQUALS(apd90.GetNumElements(), 10u);

            NdArray<double> resting = GET_ARRAY(r_outputs.Lookup("resting_potential"));
            NdArray<double>::Indices resting_indices = resting.GetIndices();
            TS_ASSERT_EQUALS(resting.GetNumDimensions(), 2u);
            TS_ASSERT_EQUALS(resting.GetNumElements(), 10u);

            for (unsigned i=0; i<reference_apd90.size(); i++)
            {
                TS_ASSERT_DELTA(apd90[apd90_indices], reference_apd90[i], 0.6);
                apd90.IncrementIndices(apd90_indices);
                TS_ASSERT_DELTA(resting[resting_indices], reference_resting[i], 1e-2);
                resting.IncrementIndices(resting_indices);
            }
        }
        PetscTools::IsolateProcesses(false);
    }

    void TestProtocolsForManyCellModels() throw(Exception, std::bad_alloc)
    {
        std::vector<std::string> cellml_files = GetAListOfCellMLFiles();
        if (CommandLineArguments::Instance()->OptionExists("--models"))
        {
            cellml_files = CommandLineArguments::Instance()->GetStringsCorrespondingToOption("--models");
        }

        // Collectively ensure the root output folder exists, then isolate processes
        {
            PetscTools::IsolateProcesses(false); // Just in case previous test threw
            OutputFileHandler("FunctionalCuration", false);
            PetscTools::IsolateProcesses(true);
        }

        for (unsigned i=0; i<cellml_files.size(); ++i)
        {
            if (PetscTools::IsParallel() && i % PetscTools::GetNumProcs() != PetscTools::GetMyRank())
            {
                // Let someone else do this model
                continue;
            }

            std::cout << "\nRunning protocols for " << cellml_files[i] << std::endl << std::flush;
            RunHypokalaemiaProtocol(cellml_files[i]);
        }
    }
};

#endif // TESTHYPOKALAEMIAPROTOCOL_HPP_
