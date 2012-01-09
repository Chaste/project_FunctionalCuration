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

#include "ProtocolRunner.hpp"

#include <vector>
#include <iostream>
#include <boost/pointer_cast.hpp> // NB: Not available on Boost 1.33.1
#include <boost/shared_ptr.hpp>
#include <boost/assign/list_of.hpp>

#include "AbstractCvodeCell.hpp"
#include "CellMLToSharedLibraryConverter.hpp"
#include "DynamicCellModelLoader.hpp"
#include "AbstractDynamicallyLoadableEntity.hpp"
#include "ProtocolParser.hpp"


typedef N_Vector VECTOR;

ProtocolRunner::ProtocolRunner(const FileFinder& rModelFile,
                               const FileFinder& rProtoXmlFile,
                               const std::string& rOutputFolder,
                               bool optimiseModel)
    : mHandler(rOutputFolder)
{
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
    if (model_name == "pasek_simurda_orchard_christe_2008") // This model triggers a bug in rnv and needs this.
    {
        options.push_back("--assume-valid");
    }

    // Do the conversion
    CellMLToSharedLibraryConverter converter(true, "projects/FunctionalCuration");
    converter.CreateOptionsFile(mHandler, model_name, options);
    DynamicCellModelLoader* p_loader = converter.Convert(copied_model);
    boost::shared_ptr<AbstractStimulusFunction> p_stimulus;
    boost::shared_ptr<AbstractIvpOdeSolver> p_solver;
    boost::shared_ptr<AbstractCvodeCell> p_cell(dynamic_cast<AbstractCvodeCell*>(p_loader->CreateCell(p_solver, p_stimulus)));
    // Check we have the right bases
    assert(dynamic_cast<AbstractDynamicallyLoadableEntity*>(p_cell.get()));
    assert(dynamic_cast<AbstractUntemplatedSystemWithOutputs*>(p_cell.get()));

    // Load the XML protocol
    ProtocolParser parser;
    mpProtocol = parser.ParseFile(rProtoXmlFile);

    p_cell->SetMaxSteps(2e7); // We need to allow CVODE to take lots of internal steps for some protocols
    p_cell->SetTimestep(0.5); // Max dt = 0.5ms to ensure stimulus isn't missed
    p_cell->SetTolerances(/*rel*/1e-6, /*abs*/1e-8); // Guard against changes to defaults
    p_cell->SetAutoReset(false); // Speedier simulations
    mpProtocol->SetModel(p_cell);
}


void ProtocolRunner::RunProtocol()
{
    try
    {
        mpProtocol->Run();
    }
    catch (...)
    {
        try
        {
            std::cout << "Error running protocol. Trying to write intermediate results to file..." << std::endl;
            mpProtocol->WriteToFile(mHandler, "outputs");
            std::cout << "Intermediate results written; re-throwing error..." << std::endl;
        }
        catch (const Exception& e)
        {
            std::cout << "Failed to write intermediate results:" << std::endl << e.GetMessage();
        }
        catch (...)
        {
            std::cout << "Failed to write intermediate results." << std::endl;
        }
        throw;
    }
    std::cout << "Writing results to file..." << std::endl;
    mpProtocol->WriteToFile(mHandler, "outputs");
    // Indicate successful completion
    std::cout << "Done!" << std::endl;
    out_stream p_file = mHandler.OpenOutputFile("success");
    (*p_file) << "Protocol completed successfully." << std::endl;
    p_file->close();
}


ProtocolPtr ProtocolRunner::GetProtocol()
{
    return mpProtocol;
}
