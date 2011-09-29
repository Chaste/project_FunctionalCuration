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

#ifndef USEFULFUNCTIONSFORPROTOCOLTESTING_HPP_
#define USEFULFUNCTIONSFORPROTOCOLTESTING_HPP_

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
        TS_ASSERT_EQUALS(&rLoader, p_entity->GetLoader());
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
        if (rModelName=="pasek_simurda_orchard_christe_2008") // This model triggers a bug in rnv and needs this.
        {
            (*p_optfile) << "<arg>--assume-valid</arg>" << std::endl;
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


#endif // USEFULFUNCTIONSFORPROTOCOLTESTING_HPP_
