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

#ifndef TESTNIGHTLYPROTOCOLS_HPP_
#define TESTNIGHTLYPROTOCOLS_HPP_

#include <cxxtest/TestSuite.h>

#include <string>

#include "ProtocolRunner.hpp"

#include "OutputFileHandler.hpp"
#include "NumericFileComparison.hpp"

/**
 * Test longer runs of our "standard" protocols against historic data.
 */
class TestNightlyProtocols : public CxxTest::TestSuite
{
public:
    void TestS1S2Courtemanche() throw(Exception, std::bad_alloc)
    {
        std::string dirname = "TestS1S2Courtemanche";
        std::string model_name = "courtemanche_ramirez_nattel_1998";
        FileFinder cellml_file("projects/FunctionalCuration/cellml/" + model_name + ".cellml", RelativeTo::ChasteSourceRoot);
        ProtocolFileFinder proto_xml_file("projects/FunctionalCuration/protocols/S1S2.txt", RelativeTo::ChasteSourceRoot);

        ProtocolRunner runner(cellml_file, proto_xml_file, dirname);
        runner.RunProtocol();
        FileFinder success_file(dirname + "/success", RelativeTo::ChasteTestOutput);
        TS_ASSERT(success_file.Exists());

        // Compare two files, for the x and y values of the main plots.
        for (unsigned i=0; i<2; i++)
        {
            std::string output_name;
            if (0u==i)
            {
                output_name = "outputs_APD90";
            }
            else
            {
                output_name = "outputs_DI";
            }

            std::cout << "Comparing results of S1-S2 protocol: " << output_name << "...";
            FileFinder ref_output("projects/FunctionalCuration/test/data/historic/" + model_name + "/S1S2/" + output_name + ".csv",
                                  RelativeTo::ChasteSourceRoot);
            OutputFileHandler handler(dirname, false);
            FileFinder test_output = handler.FindFile(output_name + ".csv");

            NumericFileComparison comp(test_output.GetAbsolutePath(), ref_output.GetAbsolutePath());
            TS_ASSERT(comp.CompareFiles(0.1));
            // Difference in APD/DI of 0.1ms against values ~100ms doesn't seem extravagant, given that CVODE's max step changed.

            std::cout << "done.\n";
        }
    }

    void TestIcalAslanidi() throw(Exception, std::bad_alloc)
    {
        std::string dirname = "TestIcalAslanidi";
        std::string model_name = "aslanidi_Purkinje_model_2009";
        FileFinder cellml_file("projects/FunctionalCuration/cellml/" + model_name + ".cellml", RelativeTo::ChasteSourceRoot);
        ProtocolFileFinder proto_xml_file("projects/FunctionalCuration/protocols/ICaL.txt", RelativeTo::ChasteSourceRoot);

        ProtocolRunner runner(cellml_file, proto_xml_file, dirname);
        runner.RunProtocol();
        FileFinder success_file(dirname + "/success", RelativeTo::ChasteTestOutput);
        TS_ASSERT(success_file.Exists());

        // Compare two files, for the x and y values of the main plots.
        for (unsigned i=0; i<2; i++)
        {
            std::string output_name;
            if (0u==i)
            {
                output_name = "outputs_min_LCC";
            }
            else
            {
                output_name = "outputs_final_membrane_voltage";
            }

            std::cout << "Comparing results of ICaL protocol: " << output_name << "...";
            FileFinder ref_output("projects/FunctionalCuration/test/data/historic/" + model_name + "/ICaL/" + output_name + ".csv",
                                  RelativeTo::ChasteSourceRoot);
            OutputFileHandler handler(dirname, false);
            FileFinder test_output = handler.FindFile(output_name + ".csv");

            NumericFileComparison comp(test_output.GetAbsolutePath(), ref_output.GetAbsolutePath());
            TS_ASSERT(comp.CompareFiles(5e-4));

            std::cout << "done.\n";
        }
    }
};

#endif // TESTNIGHTLYPROTOCOLS_HPP_
