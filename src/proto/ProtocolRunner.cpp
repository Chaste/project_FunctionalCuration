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

#include "ProtocolRunner.hpp"

#include <vector>
#include <iostream>
#include <boost/pointer_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

#include "AbstractCvodeCell.hpp"
#include "CellMLToSharedLibraryConverter.hpp"
#include "DynamicCellModelLoader.hpp"
#include "AbstractDynamicallyLoadableEntity.hpp"

#include "ProtocolParser.hpp"
#include "ProtocolTimer.hpp"


typedef N_Vector VECTOR;

ProtocolRunner::ProtocolRunner(const FileFinder& rModelFile,
                               const ProtocolFileFinder& rProtoXmlFile,
                               const std::string& rOutputFolder,
                               bool optimiseModel)
    : mHandler(rOutputFolder)
{
    ProtocolTimer::Enable();
    ProtocolTimer::BeginEvent(ProtocolTimer::ALL);
    ProtocolTimer::BeginEvent(ProtocolTimer::LOAD_MODEL);
    std::cout << "Running protocol '" << rProtoXmlFile.rGetOriginalSource().GetAbsolutePath() << "' on model '"
              << rModelFile.GetAbsolutePath() << "' and writing output to "
              << mHandler.GetOutputDirectoryFullPath() << std::endl;
    // Copy CellML file into output dir and create conf file
    std::string model_name = rModelFile.GetLeafNameNoExtension();
    std::cout << "Generating modified model " << model_name << std::endl;
    FileFinder copied_model = mHandler.CopyFileTo(rModelFile);
    std::vector<std::string> options = boost::assign::list_of("--cvode")("--expose-annotated-variables");
    options.push_back("--protocol=" + rProtoXmlFile.GetAbsolutePath());
    if (optimiseModel)
    {
        options.push_back("--opt");
    }

    // Do the conversion
    CellMLToSharedLibraryConverter converter(true, "projects/FunctionalCuration");
    converter.CreateOptionsFile(mHandler, model_name, options);
    DynamicCellModelLoaderPtr p_loader = converter.Convert(copied_model); // Note: collective call unless processes isolated
    boost::shared_ptr<AbstractStimulusFunction> p_stimulus;
    boost::shared_ptr<AbstractIvpOdeSolver> p_solver;
    boost::shared_ptr<AbstractCvodeCell> p_cell(dynamic_cast<AbstractCvodeCell*>(p_loader->CreateCell(p_solver, p_stimulus)));
    // Check we have the right bases
    assert(dynamic_cast<AbstractDynamicallyLoadableEntity*>(p_cell.get()));
    boost::shared_ptr<AbstractSystemWithOutputs> p_model = boost::dynamic_pointer_cast<AbstractSystemWithOutputs>(p_cell);
    assert(p_model);
    // Write info about the model into the output folder
    out_stream p_model_info = mHandler.OpenOutputFile("model_info.txt");
    (*p_model_info) << p_model->rGetInputNames().size() << " inputs:\n";
    BOOST_FOREACH(const std::string& r_name, p_model->rGetInputNames())
    {
        (*p_model_info) << "\t" << r_name << "\n";
    }
    (*p_model_info) << p_model->GetNumberOfOutputs() << " outputs:\n";
    BOOST_FOREACH(const std::string& r_name, p_model->rGetOutputNames())
    {
        (*p_model_info) << "\t" << r_name << "\n";
    }
    (*p_model_info) << p_cell->GetNumberOfStateVariables() << " state variables:\n";
    BOOST_FOREACH(const std::string& r_name, p_cell->rGetStateVariableNames())
    {
        (*p_model_info) << "\t" << r_name << "\n";
    }
    ProtocolTimer::EndEvent(ProtocolTimer::LOAD_MODEL);

    // Load the XML protocol
    ProtocolTimer::BeginEvent(ProtocolTimer::LOAD_PROTO);
    ProtocolParser parser;
    mpProtocol = parser.ParseFile(rProtoXmlFile);
    mpProtocol->SetOutputFolder(mHandler);

    p_cell->SetMaxSteps(2e7); // We need to allow CVODE to take lots of internal steps for some protocols
    p_cell->SetTimestep(0.5); // Max dt = 0.5ms to ensure stimulus isn't missed
    p_cell->SetTolerances(/*rel*/1e-6, /*abs*/1e-8); // Guard against changes to defaults
    p_cell->SetAutoReset(false); // Speedier simulations
    mpProtocol->SetModel(p_model);
    ProtocolTimer::EndEvent(ProtocolTimer::LOAD_PROTO);
    ProtocolTimer::EndEvent(ProtocolTimer::ALL);
}

void ProtocolRunner::SetPngOutput(bool writePng)
{
    mpProtocol->SetPngOutput(writePng);
}


void ProtocolRunner::RunProtocol()
{
    mpProtocol->RunAndWrite("outputs");
    ProtocolTimer::Headings();
    ProtocolTimer::Report();
}


ProtocolPtr ProtocolRunner::GetProtocol()
{
    return mpProtocol;
}
