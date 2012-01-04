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

/**
 * @file
 *
 * This executable presents a simple interface to the functional curation backend.
 * It takes (paths to) model and protocol files, runs the protocol on the model,
 * and stores the CSV format results where requested.
 *
 * \todo Do we actually need to initialise PETSc?
 */

#include <string>
#include <cstdlib> // For setenv()

#include "ExecutableSupport.hpp"
#include "Exception.hpp"
#include "PetscTools.hpp"
#include "PetscException.hpp"

#include "FileFinder.hpp"
#include "ProtocolRunner.hpp"

int main(int argc, char *argv[])
{
    // This sets up PETSc and prints out copyright information, etc.
    ExecutableSupport::StandardStartup(&argc, &argv);

    int exit_code = ExecutableSupport::EXIT_OK;

    // You should put all the main code within a try-catch, to ensure that
    // you clean up PETSc before quitting.
    try
    {
        if (argc<3 || argc>4)
        {
            ExecutableSupport::PrintError("Usage: FunctionalCuration model.cellml proto.xml [output_dir]", true);
            exit_code = ExecutableSupport::EXIT_BAD_ARGUMENTS;
        }
        else
        {
            FileFinder model(argv[1], RelativeTo::AbsoluteOrCwd);
            FileFinder proto_xml(argv[2], RelativeTo::AbsoluteOrCwd);

            std::string output_folder("FunctionalCuration/" + model.GetLeafNameNoExtension() + "/" + proto_xml.GetLeafNameNoExtension());

            if (argc == 4)
            {
                output_folder = argv[3];

                if (output_folder.find('/') != std::string::npos)
                {
                    // Change CHASTE_TEST_OUTPUT if output_folder is an absolute path
                    FileFinder folder(output_folder, RelativeTo::AbsoluteOrCwd);
                    size_t full_len = folder.GetAbsolutePath().length();
                    output_folder = folder.GetLeafName();
                    std::string test_output = folder.GetAbsolutePath().substr(0, full_len - output_folder.length());
                    setenv("CHASTE_TEST_OUTPUT", test_output.c_str(), 1/*Overwrite*/);
                }
            }
            ProtocolRunner runner(model, proto_xml, output_folder);
            runner.RunProtocol();
        }
    }
    catch (const Exception& e)
    {
        ExecutableSupport::PrintError(e.GetMessage());
        exit_code = ExecutableSupport::EXIT_ERROR;
    }

    // End by finalizing PETSc, and returning a suitable exit code.
    ExecutableSupport::FinalizePetsc();
    return exit_code;
}
