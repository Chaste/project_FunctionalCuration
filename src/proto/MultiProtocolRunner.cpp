/*

Copyright (c) 2005-2014, University of Oxford.
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

#include "MultiProtocolRunner.hpp"

#include <vector>
#include <string>
#include <cstdlib> // For setenv()
#include <iostream>
#include <boost/foreach.hpp>

#include "CommandLineArguments.hpp"
#include "OutputFileHandler.hpp"
#include "PetscTools.hpp"
#include "Warnings.hpp"

#include "ProtocolRunner.hpp"

/**
 * Utility method to find any command line arguments that aren't associated with our options.
 */
std::vector<std::string> GetNonOptionArguments()
{
    std::vector<std::string> args;
    bool ignore_next = false;
    unsigned num_args = *CommandLineArguments::Instance()->p_argc;
    for (unsigned i=1 /* arg 0 is program name! */; i<num_args; i++)
    {
        std::string arg((*CommandLineArguments::Instance()->p_argv)[i]);
        bool is_opt = (arg.substr(0, 2) == "--");
        if (is_opt)
        {
            if (arg == "--models" || arg == "--protocols" || arg == "--output-dir")
            {
                ignore_next = true;
            }
            else
            {
                ignore_next = false;
            }
        }
        else if (!ignore_next)
        {
            args.push_back(arg);
        }
    }
    return args;
}


/**
 * Change CHASTE_TEST_OUTPUT if an absolute path has been specified for output folder,
 * since we can only write output under CHASTE_TEST_OUTPUT.
 * @param outputDir  the user-specified output folder
 * @return  an output folder path relative to CHASTE_TEST_OUTPUT.
 */
std::string MakeRelative(std::string outputDir)
{
    if (FileFinder::IsAbsolutePath(outputDir))
    {
        // Change CHASTE_TEST_OUTPUT if outputDir is an absolute path, since we can only
        // create output under CHASTE_TEST_OUTPUT.
        FileFinder folder(outputDir);
        outputDir = folder.GetLeafName();
        std::string test_output = folder.GetParent().GetAbsolutePath();
        setenv("CHASTE_TEST_OUTPUT", test_output.c_str(), 1/*Overwrite*/);
    }
    return outputDir;
}


void MultiProtocolRunner::SetDefaultModels(const std::vector<FileFinder>& rModelPaths)
{
    mDefaultModels = rModelPaths;
}


void MultiProtocolRunner::SetDefaultProtocols(const std::vector<ProtocolFileFinder>& rProtocolPaths)
{
    mDefaultProtocols = rProtocolPaths;
}


void MultiProtocolRunner::RunProtocols()
{
    // Determine protocols to run
    std::vector<ProtocolFileFinder> protocols;
    {
        if (CommandLineArguments::Instance()->OptionExists("--protocols"))
        {
            std::vector<std::string> protocol_files = CommandLineArguments::Instance()->GetStringsCorrespondingToOption("--protocols");
            protocols.reserve(protocol_files.size());
            BOOST_FOREACH(const std::string& r_proto_file, protocol_files)
            {
                protocols.push_back(ProtocolFileFinder(r_proto_file, RelativeTo::AbsoluteOrCwd));
            }
        }
        else
        {
            protocols = mDefaultProtocols;
        }
    }

    // Determine models to process
    std::vector<FileFinder> models;
    {
        if (CommandLineArguments::Instance()->OptionExists("--models"))
        {
            std::vector<std::string> model_files = CommandLineArguments::Instance()->GetStringsCorrespondingToOption("--models");
            models.reserve(model_files.size());
            BOOST_FOREACH(const std::string& r_model_file, model_files)
            {
                models.push_back(FileFinder(r_model_file, RelativeTo::AbsoluteOrCwd));
            }
        }
        else
        {
            models = mDefaultModels;
        }
    }

    // Determine base output folder
    FileFinder output_folder("FunctionalCuration", RelativeTo::ChasteTestOutput);
    {
        if (CommandLineArguments::Instance()->OptionExists("--output-dir"))
        {
            output_folder.SetPath(MakeRelative(CommandLineArguments::Instance()->GetStringCorrespondingToOption("--output-dir")),
                                  RelativeTo::ChasteTestOutput);
        }
    }

    // If no models and protocols have been given with options, then a single combination could be given as non-option arguments.
    // If using this form, the output folder is given as an optional third argument.
    if (models.empty() && protocols.empty())
    {
        std::vector<std::string> args = GetNonOptionArguments();
        if (args.size() == 2u || args.size() == 3u)
        {
            models.push_back(FileFinder(args[0], RelativeTo::AbsoluteOrCwd));
            protocols.push_back(ProtocolFileFinder(args[1], RelativeTo::AbsoluteOrCwd));
        }
        if (args.size() == 3u)
        {
            output_folder.SetPath(MakeRelative(args[2]), RelativeTo::ChasteTestOutput);
        }
    }

    // Determine whether to write PNG graphs
    bool png_output = CommandLineArguments::Instance()->OptionExists("--png");

    // Check arguments
    if (protocols.empty())
    {
        EXCEPTION("No protocols to run specified!");
    }
    if (models.empty())
    {
        EXCEPTION("No models to process specified!");
    }


    // Collectively ensure base output folders exist, then isolate processes
    OutputFileHandler base_handler(output_folder, false);
    BOOST_FOREACH(const FileFinder& r_model, models)
    {
        OutputFileHandler sub_handler(base_handler.FindFile(r_model.GetLeafNameNoExtension()), false);
    }
    PetscTools::IsolateProcesses(true);

    // Run protocols
    unsigned counter = 0u;
    FileFinder chaste_test_output("", RelativeTo::ChasteTestOutput);
    BOOST_FOREACH(const FileFinder& r_model, models)
    {
        BOOST_FOREACH(const ProtocolFileFinder& r_protocol, protocols)
        {
            if (counter++ % PetscTools::GetNumProcs() != PetscTools::GetMyRank())
            {
                continue; // Let another process do this combination
            }
            const std::string proto_name(r_protocol.rGetOriginalSource().GetLeafNameNoExtension());
            const std::string model_name(r_model.GetLeafNameNoExtension());
            FileFinder sub_output_folder(proto_name, FileFinder(model_name, output_folder));

            try
            {
                ProtocolRunner runner(r_model, r_protocol, sub_output_folder.GetRelativePath(chaste_test_output));
                runner.SetPngOutput(png_output);
                runner.RunProtocol();
            }
            catch (const Exception& r_e)
            {
                std::cerr << r_e.GetMessage();
                WARNING("Running protocol " << proto_name << " on model " << model_name << " failed:"
                        << r_e.GetMessage());
            }
        }
    }
}
