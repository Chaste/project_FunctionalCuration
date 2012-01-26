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
        if (CommandLineArguments::Instance()->OptionExists("--protos"))
        {
            mProtocolFiles = CommandLineArguments::Instance()->GetStringsCorrespondingToOption("--protos");
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
