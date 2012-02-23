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

#ifndef TESTRUNNINGALLCELLMODELTESTS_HPP_
#define TESTRUNNINGALLCELLMODELTESTS_HPP_


#include <boost/pointer_cast.hpp> // NB: Not available on Boost 1.33.1
#include <boost/shared_ptr.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

#include <cxxtest/TestSuite.h>

#include "OutputFileHandler.hpp"

#include "ProtocolRunner.hpp"
#include "ProtoHelperMacros.hpp"
#include "PetscTools.hpp"

#include "PetscSetupAndFinalize.hpp"

#include "Debug.hpp"
#include "UsefulFunctionsForProtocolTesting.hpp"

typedef N_Vector VECTOR;

/**
 * This test attempts to run all of the protocols listed for all of the cellml models in the FunctionalCuration/cellml folder.
 *
 * It collects warnings about the ones which failed (generally this is because of missing metadata tags).
 */
class TestRunningAllCellModelTests : public CxxTest::TestSuite
{
    std::vector<std::string> mProtocolFiles;
public:

    void TestProtocolsForManyCellModels() throw(Exception, std::bad_alloc)
    {
        mProtocolFiles.clear();
        mProtocolFiles.push_back("protocols/ICaL.xml");
        mProtocolFiles.push_back("protocols/S1S2.xml");
        mProtocolFiles.push_back("protocols/SteadyPacing.xml");
        mProtocolFiles.push_back("private/protocols/INa_IV_curve.xml");
        mProtocolFiles.push_back("private/protocols/Hypokalaemia.xml");
        mProtocolFiles.push_back("private/protocols/SteadyStateRunner.xml");
        mProtocolFiles.push_back("private/protocols/IK1_IV_curve.xml");
        mProtocolFiles.push_back("private/protocols/IKr_IV_curve.xml");
        mProtocolFiles.push_back("private/protocols/IKs_IV_curve.xml");
        if (CommandLineArguments::Instance()->OptionExists("--protocols"))
        {
            mProtocolFiles = CommandLineArguments::Instance()->GetStringsCorrespondingToOption("--protocols");
        }

        std::vector<std::string> cellml_files = GetAListOfCellMLFiles();
        if (CommandLineArguments::Instance()->OptionExists("--models"))
        {
            cellml_files = CommandLineArguments::Instance()->GetStringsCorrespondingToOption("--models");
        }

        // Collectively ensure the root output folder exists, then isolate processes
        {
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

            // RUN ALL PROTOCOLS IN THE LIST
            FileFinder model("projects/FunctionalCuration/cellml/" + cellml_files[i] + ".cellml", RelativeTo::ChasteSourceRoot);
            for (unsigned protocol_idx = 0; protocol_idx<mProtocolFiles.size(); ++protocol_idx)
            {
                FileFinder proto_xml("projects/FunctionalCuration/test/" + mProtocolFiles[protocol_idx], RelativeTo::ChasteSourceRoot);
                std::string output_folder("FunctionalCuration/" + model.GetLeafNameNoExtension() + "/" + proto_xml.GetLeafNameNoExtension());

                try
                {
                    ProtocolRunner runner(model, proto_xml, output_folder);
                    runner.RunProtocol();
                }
                catch (Exception& e)
                {
                    std::cerr << "A protocol failed : " << e.GetMessage() << std::endl;
                    WARNING("Model " << cellml_files[i] << " and Protocol " << mProtocolFiles[protocol_idx] << " failed.");
                }
            }
        }
    }
};

#endif // TESTRUNNINGALLCELLMODELTESTS_HPP_
