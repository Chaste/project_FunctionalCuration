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

#ifndef USEFULFUNCTIONSFORPROTOCOLTESTING_HPP_
#define USEFULFUNCTIONSFORPROTOCOLTESTING_HPP_

#include <sys/types.h>
#include <dirent.h>
#include <vector>
#include <string>
#include <cxxtest/TestSuite.h>

#ifdef CHASTE_CVODE
// CVODE headers
#include <nvector/nvector_serial.h>
#endif

#include "SmartPointers.hpp"
#include "OutputFileHandler.hpp"
#include "FileFinder.hpp"
#include "DynamicCellModelLoader.hpp"
#include "PetscTools.hpp"
#include "Exception.hpp"
#include "AbstractCardiacCellInterface.hpp"
#include "AbstractDynamicallyLoadableEntity.hpp"
#include "EulerIvpOdeSolver.hpp"
#include "SimpleStimulus.hpp"

#include "RunAndCheckIonicModels.hpp"
#include "Warnings.hpp"

#define OUR_WARN(msg, model, proto) \
    do { std::string m=msg; std::cout << std::endl << m << std::endl << std::flush; \
         WARNING("Model: " + model + ", proto: " + proto + ": " + m); } while (false)

//
// Function declarations
//

boost::shared_ptr<AbstractCardiacCellInterface> RunLr91Test(DynamicCellModelLoader& rLoader,                                                            bool testTables=false,
                                                            double tolerance=1e-3,
                                                            const std::string caiTableName="");

void SimulateLr91AndCompare(boost::shared_ptr<AbstractCardiacCellInterface> pCell,
                            double tolerance);

AbstractCardiacCellInterface* CreateCellFromLoader(DynamicCellModelLoader& rLoader);

void CreateOptionsFile(const OutputFileHandler& rHandler,
                       const FileFinder& rProtoFile,
                       const std::string& rModelName,
                       const std::vector<std::string>& rArgs,
                       const std::string rExtraXml="");

void CopyFile(const OutputFileHandler& rDestDir,
              const FileFinder& rSourceFile);

void TestTables(AbstractCardiacCellInterface* pCell,
                const std::string& rKeyName,
                double badKeyValue);

std::vector<std::string> GetAListOfCellMLFiles(void);

static int returnOne (const struct dirent *unused);

std::string GetTitleFromDirectory(const std::string& rDirectory);

//
// Implementation
//

template<typename VECTOR>
void TestTables(AbstractParameterisedSystem<VECTOR>* pSystem,
                AbstractCardiacCellInterface* pCell,
                const std::string& rKeyName,
                double badKeyValue)
{
    assert(pCell);
    assert(pSystem);
    unsigned key_index = pSystem->GetStateVariableIndex(rKeyName);
    double current_value = pSystem->GetStateVariable(key_index);
    pSystem->SetStateVariable(key_index, badKeyValue);
    TS_ASSERT_THROWS_CONTAINS(pCell->GetIIonic(), rKeyName + " outside lookup table range");
    pSystem->SetStateVariable(key_index, current_value);
}

void TestTables(AbstractCardiacCellInterface* pCell,
                const std::string& rKeyName,
                double badKeyValue)
{
    AbstractParameterisedSystem<std::vector<double> >* p_normal
        = dynamic_cast<AbstractParameterisedSystem<std::vector<double> >*>(pCell);
    if (p_normal)
    {
        TestTables(p_normal, pCell, rKeyName, badKeyValue);
    }
#ifdef CHASTE_CVODE
    else
    {
        AbstractParameterisedSystem<N_Vector>* p_cvode
            = dynamic_cast<AbstractParameterisedSystem<N_Vector>*>(pCell);
        TestTables(p_cvode, pCell, rKeyName, badKeyValue);
    }
#endif
}

boost::shared_ptr<AbstractCardiacCellInterface> RunLr91Test(DynamicCellModelLoader& rLoader,
                                                            bool testTables,
                                                            double tolerance,
                                                            const std::string caiTableName)
{
    boost::shared_ptr<AbstractCardiacCellInterface> p_cell(CreateCellFromLoader(rLoader));
    SimulateLr91AndCompare(p_cell, tolerance);

    if (testTables)
    {
        double v = p_cell->GetVoltage();
        p_cell->SetVoltage(-100000);
        TS_ASSERT_THROWS_CONTAINS(p_cell->GetIIonic(), "membrane_voltage outside lookup table range");
        p_cell->SetVoltage(v);

        if (!caiTableName.empty())
        {
            TestTables(p_cell.get(), caiTableName, -100.0);
        }
    }

    return p_cell;
}

void SimulateLr91AndCompare(boost::shared_ptr<AbstractCardiacCellInterface> pCell,
                            double tolerance)
{
    double end_time = 1000.0; //One second in milliseconds
    // Solve and write to file
    clock_t ck_start = clock();

    // Don't use RunOdeSolverWithIonicModel() as this is hardcoded to AbstractCardiacCells
    OdeSolution solution1 = pCell->Compute(0.0, end_time);
    solution1.WriteToFile("TestIonicModels", "ProtocolLr91", "ms", 100, false, 4);

    clock_t ck_end = clock();
    double forward = (double)(ck_end - ck_start)/CLOCKS_PER_SEC;
    std::cout << "\n\tSolve time: " << forward << std::endl;

    // Compare with 'normal' LR91 model results
    CheckCellModelResults("ProtocolLr91", "Lr91DelayedStim", tolerance);

    // Test GetIIonic against hardcoded result from TestIonicModels.hpp
    // Don't use RunOdeSolverWithIonicModel() as this is hardcoded to AbstractCardiacCells
    OdeSolution solution2 = pCell->Compute(0.0, 60.0);
    solution2.WriteToFile("TestIonicModels", "ProtocolLr91GetIIonic", "ms", 100, false, 4);

    TS_ASSERT_DELTA(pCell->GetIIonic(), 1.9411, 1e-3);
}

AbstractCardiacCellInterface* CreateCellFromLoader(DynamicCellModelLoader& rLoader)
{
    // Set stimulus
    double magnitude = -25.5; // uA/cm^2
    double duration = 2.0; // ms
    double when = 50.0; // ms

    boost::shared_ptr<SimpleStimulus> p_stimulus(new SimpleStimulus(magnitude, duration, when));
    boost::shared_ptr<EulerIvpOdeSolver> p_solver(new EulerIvpOdeSolver);

    // Load the cell model dynamically
    AbstractCardiacCellInterface* p_cell = rLoader.CreateCell(p_solver, p_stimulus);

    AbstractDynamicallyLoadableEntity* p_entity = dynamic_cast<AbstractDynamicallyLoadableEntity*>(p_cell);
    TS_ASSERT(p_entity != NULL);
    if (p_entity != NULL)
    {
        TS_ASSERT_EQUALS(&rLoader, p_entity->GetLoader().get());
    }

    return p_cell;
}

void CreateOptionsFile(const OutputFileHandler& rHandler,
                       const FileFinder& rProtoFile,
                       const std::string& rModelName,
                       const std::vector<std::string>& rArgs,
                       const std::string rExtraXml)
{
    if (PetscTools::AmMaster())
    {
        out_stream p_optfile = rHandler.OpenOutputFile(rModelName + "-conf.xml");
        (*p_optfile) << "<?xml version='1.0'?>" << std::endl
                     << "<pycml_config>" << std::endl
                     << "<command_line_args>" << std::endl
                     << "<arg>--protocol=" << rProtoFile.GetAbsolutePath() << "</arg>" << std::endl;
        for (unsigned i=0; i<rArgs.size(); i++)
        {
            (*p_optfile) << "<arg>" << rArgs[i] << "</arg>" << std::endl;
        }
        (*p_optfile) << "</command_line_args>" << std::endl
                     << rExtraXml << std::endl
                     << "</pycml_config>" << std::endl;
        p_optfile->close();
    }
    PetscTools::Barrier("CreateOptionsFile");
}

void CopyFile(const OutputFileHandler& rDestDir,
              const FileFinder& rSourceFile)
{
    rDestDir.CopyFileTo(rSourceFile);
}

/**
 * Get all of the .cellml files in the directory of the FunctionalCuration project (for dynamic loading)
 * they are returned without ".cellml" on the end, so they can also be used for folder names.
 *
 * \todo Use Boost Filesystem for this.
 *
 * @return vector of cellml file names
 */
std::vector<std::string> GetAListOfCellMLFiles(void)
{
    std::string cell_ml_directory = "projects/FunctionalCuration/cellml";

    // Make a list of the cellml files in the directory...
    std::vector<std::string> cellml_files;

    struct dirent **eps;

    int n = scandir(cell_ml_directory.c_str(), &eps, returnOne, alphasort);
    if (n >= 0)
    {
         for (int cnt = 0; cnt < n; ++cnt)
         {
              // The 'scandir' command picks up all the hidden files and svn stuff.
              std::string file_name = eps[cnt]->d_name;
              size_t found = file_name.find(".cellml");
              if (found>0 && found==file_name.length()-7) // If we found ".cellml" and it was at the end
              {
                   cellml_files.push_back(file_name.substr(0,found)); // Don't include the ".cellml"
              }
         }
    }
    else
    {
         EXCEPTION("Couldn't open the directory: " + cell_ml_directory);
    }

    return cellml_files;
}

/**
 * This is just required to tell scandir to return all of the files it finds
 * I found it easier to compare the std::strings to look for cellML
 * rather than try to use these crazy pointers to weird objects.
 */
static int returnOne (const struct dirent *unused)
{
    return 1;
}

/**
 * Provide a string which can be used as a graph title by
 * formatting the directory name to remove underscores and insert spaces,
 * and capitalise the first letter of each word.
 *
 * @param rDirectory  The directory of this model (from CellML file name)
 * @return  A string of the name with spaces and capital letters for printing.
 */
std::string GetTitleFromDirectory(const std::string& rDirectory)
{
    // Format the title of the graph to remove underscores and insert spaces.
    std::string plot_title = rDirectory;
    std::string find_this = "_";
    std::string put_this = " ";
    plot_title[0] = toupper(plot_title[0]); // Capitalise the first letter.
    size_t pos = plot_title.find(find_this); // Find the first underscore
    while ( pos != std::string::npos )
    {
        plot_title.replace(pos, find_this.size(), put_this); // replace "_" with " "
        plot_title[pos+1] = toupper(plot_title[pos+1]); // Capitalise the next word
        pos = plot_title.find(find_this); // Find the next underscore
    }
    return plot_title;
}

#endif // USEFULFUNCTIONSFORPROTOCOLTESTING_HPP_
