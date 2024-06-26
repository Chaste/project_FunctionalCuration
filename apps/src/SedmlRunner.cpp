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

/**
 * @file
 *
 * This executable presents a simple interface to the SED-ML support within the functional curation system.
 * It takes a SED-ML file (path) as input, runs the protocol, and writes outputs and graphs in the
 * requested folder.
 */

#include <string>
#include <cstdlib> // For setenv()

#include "ExecutableSupport.hpp"
#include "Exception.hpp"
#include "FileFinder.hpp"
#include "OutputFileHandler.hpp"

#include "SedmlParser.hpp"

int main(int argc, char *argv[])
{
    // This sets up PETSc and prints out copyright information, etc.
    ExecutableSupport::StandardStartup(&argc, &argv);

    int exit_code = ExecutableSupport::EXIT_OK;

    // You should put all the main code within a try-catch, to ensure that
    // you clean up PETSc before quitting.
    try
    {
        bool png_output = (argc > 1 && std::string(argv[1]) == "--png");
        if (png_output)
        {
            // Strip the option from args
            argc--;
            argv++;
        }
        if (argc<2 || argc>3)
        {
            ExecutableSupport::PrintError("Usage: SedmlRunner [--png] proto.sedml [output_dir]", true);
            exit_code = ExecutableSupport::EXIT_BAD_ARGUMENTS;
        }
        else
        {
            FileFinder proto(argv[1], RelativeTo::AbsoluteOrCwd);

            std::string output_folder("SedmlOutput/" + proto.GetLeafNameNoExtension());

            if (argc == 3)
            {
                output_folder = argv[2];

                if (FileFinder::IsAbsolutePath(output_folder))
                {
                    // Change CHASTE_TEST_OUTPUT if output_folder is an absolute path, since we can only
                    // create output under CHASTE_TEST_OUTPUT.
                    FileFinder folder(output_folder);
                    output_folder = folder.GetLeafName();
                    std::string test_output = folder.GetParent().GetAbsolutePath();
                    setenv("CHASTE_TEST_OUTPUT", test_output.c_str(), 1/*Overwrite*/);
                }
            }

            // Load the protocol
            SedmlParser parser;
            OutputFileHandler handler(output_folder);
            ProtocolPtr p_proto = parser.ParseSedml(proto, handler);
            p_proto->SetPngOutput(png_output);

            // Run the protocol
            p_proto->SetOutputFolder(handler);
            p_proto->RunAndWrite("outputs");
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
