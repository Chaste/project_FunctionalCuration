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

#ifndef TESTSIMPLEPROTOCOL_HPP_
#define TESTSIMPLEPROTOCOL_HPP_

#include <vector>
#include <string>
#include <boost/pointer_cast.hpp> // NB: Not available on Boost 1.33.1
#include <boost/assign/list_of.hpp>
#include <boost/shared_ptr.hpp>

#include <cxxtest/TestSuite.h>

#include "OutputFileHandler.hpp"
#include "FileFinder.hpp"
#include "CellMLToSharedLibraryConverter.hpp"
#include "DynamicCellModelLoader.hpp"
#include "AbstractCardiacCell.hpp"
#include "AbstractDynamicallyLoadableEntity.hpp"
#include "AbstractSystemWithOutputs.hpp"
#include "EulerIvpOdeSolver.hpp"
#include "SimpleStimulus.hpp"
#include "VectorHelperFunctions.hpp"

#include "RunAndCheckIonicModels.hpp"
#include "UsefulFunctionsForProtocolTesting.hpp"
#include "PetscSetupAndFinalize.hpp"

#include "LuoRudy1991BackwardEuler.hpp"

class TestSimpleProtocol : public CxxTest::TestSuite
{
    template<typename CELL, typename VECTOR>
    void CheckSimpleProtocolOutputs(boost::shared_ptr<CELL> pCell)
    {
        // Check that the model info matches what we expect from the protocol outputs below
        TS_ASSERT_EQUALS(pCell->rGetStateVariableNames()[0], "membrane_voltage");
        TS_ASSERT_EQUALS(pCell->GetNumberOfDerivedQuantities(), 2u);
        TS_ASSERT_EQUALS(pCell->rGetDerivedQuantityNames()[0], "FonRT");
        TS_ASSERT_EQUALS(pCell->rGetDerivedQuantityNames()[1], "fast_sodium_current__V");
        TS_ASSERT_EQUALS(pCell->GetNumberOfParameters(), 1u);
        TS_ASSERT_EQUALS(pCell->rGetParameterNames()[0], "membrane_fast_sodium_current_conductance");

        // Check protocol outputs specification
        typedef AbstractSystemWithOutputs<VECTOR> ASWO;
        boost::shared_ptr<ASWO> p_cell_op = boost::dynamic_pointer_cast<ASWO>(pCell);
        TS_ASSERT(p_cell_op);
        TS_ASSERT_EQUALS(p_cell_op->GetNumberOfOutputs(), 5u);
        std::vector<std::string> names = p_cell_op->GetOutputNames();
        TS_ASSERT_EQUALS(names.size(), 5u);
        // Should be sorted alphabetically
        TS_ASSERT_EQUALS(names[0], "FonRT");
        TS_ASSERT_EQUALS(p_cell_op->GetOutputIndex("FonRT"), 0u);
        TS_ASSERT_EQUALS(names[1], "environment_time");
        TS_ASSERT_EQUALS(p_cell_op->GetOutputIndex("environment_time"), 1u);
        TS_ASSERT_EQUALS(names[2], "fast_sodium_current__V");
        TS_ASSERT_EQUALS(p_cell_op->GetOutputIndex("fast_sodium_current__V"), 2u);
        TS_ASSERT_EQUALS(names[3], "membrane_fast_sodium_current_conductance");
        TS_ASSERT_EQUALS(p_cell_op->GetOutputIndex("membrane_fast_sodium_current_conductance"), 3u);
        TS_ASSERT_EQUALS(names[4], "membrane_voltage");
        TS_ASSERT_EQUALS(p_cell_op->GetOutputIndex("membrane_voltage"), 4u);
        std::vector<std::string> units = p_cell_op->GetOutputUnits();
        TS_ASSERT_EQUALS(units.size(), 5u);
        TS_ASSERT_EQUALS(units[0], "per_millivolt");
        TS_ASSERT_EQUALS(units[1], "millisecond");
        TS_ASSERT_EQUALS(units[2], "millivolt");
        TS_ASSERT_EQUALS(units[3], "milliS_per_cm2");
        TS_ASSERT_EQUALS(units[4], "millivolt");
        TS_ASSERT_THROWS_THIS(p_cell_op->GetOutputIndex("no_var"), "No output named 'no_var'.");

        // Check output values
        boost::dynamic_pointer_cast<AbstractUntemplatedSystemWithOutputs>(pCell)->SetFreeVariable(1000.0);
        // The above line is needed because we haven't simulated the system using a TimecourseSimulation
        VECTOR fini_outputs = p_cell_op->ComputeOutputs();
        TS_ASSERT_EQUALS(GetVectorSize(fini_outputs), 5u);
        TS_ASSERT_DELTA(GetVectorComponent(fini_outputs, 0), 0.037435728309031795, 1e-12);
        TS_ASSERT_DELTA(GetVectorComponent(fini_outputs, 1), 1000.0, 1e-12);
        TS_ASSERT_DELTA(GetVectorComponent(fini_outputs, 2), pCell->rGetStateVariables()[pCell->GetVoltageIndex()], 1e-12);
        TS_ASSERT_DELTA(GetVectorComponent(fini_outputs, 3), pCell->GetParameter(0u), 1e-12);
        TS_ASSERT_DELTA(GetVectorComponent(fini_outputs, 4), pCell->rGetStateVariables()[pCell->GetVoltageIndex()], 1e-12);

        pCell->ResetToInitialConditions();
        boost::dynamic_pointer_cast<AbstractUntemplatedSystemWithOutputs>(pCell)->SetFreeVariable(0.0);
        VECTOR inits = pCell->GetInitialConditions();
        VECTOR init_outputs = p_cell_op->ComputeOutputs();
        TS_ASSERT_EQUALS(GetVectorSize(init_outputs), 5u);
        TS_ASSERT_DELTA(GetVectorComponent(init_outputs, 0), 0.037435728309031795, 1e-12);
        TS_ASSERT_DELTA(GetVectorComponent(init_outputs, 1), 0.0, 1e-12);
        TS_ASSERT_DELTA(GetVectorComponent(init_outputs, 2), GetVectorComponent(inits, pCell->GetVoltageIndex()), 1e-12);
        TS_ASSERT_DELTA(GetVectorComponent(init_outputs, 3), pCell->GetParameter(0u), 1e-12);
        TS_ASSERT_DELTA(GetVectorComponent(init_outputs, 4), GetVectorComponent(inits, pCell->GetVoltageIndex()), 1e-12);

        DeleteVector(inits);
        DeleteVector(init_outputs);
        DeleteVector(fini_outputs);
    }

public:
    void TestSimpleProtocolOnLr91() throw (Exception)
    {
        // Copy CellML file into output dir
        std::string dirname = "TestSimpleProtocol";
        OutputFileHandler handler(dirname);
        FileFinder cellml_file("heart/src/odes/cellml/LuoRudy1991.cellml", RelativeTo::ChasteSourceRoot);
        CopyFile(handler, cellml_file);

        // Create options file
        FileFinder proto_file("projects/FunctionalCuration/test/protocols/SimpleProtocol.py", RelativeTo::ChasteSourceRoot);
        std::vector<std::string> options = boost::assign::list_of("--use-chaste-stimulus")("-i");
        CreateOptionsFile(handler, proto_file, "LuoRudy1991", options);

        // Do the conversion
        CellMLToSharedLibraryConverter converter(true, "projects/FunctionalCuration");
        FileFinder copied_cellml(dirname + "/LuoRudy1991.cellml", RelativeTo::ChasteTestOutput);
        DynamicCellModelLoader* p_loader;
        p_loader = converter.Convert(copied_cellml);
        boost::shared_ptr<AbstractCardiacCell> p_cell;
        p_cell = boost::dynamic_pointer_cast<AbstractCardiacCell>(RunLr91Test(*p_loader, 0u));
        CheckSimpleProtocolOutputs<AbstractCardiacCell, std::vector<double> >(p_cell);

        // The same test, but with optimised cell models
        options.push_back("--opt");
        OutputFileHandler handler2(dirname + "Opt");
        CopyFile(handler2, cellml_file);
        FileFinder copied_cellml2(handler2.GetOutputDirectoryFullPath() + "/LuoRudy1991.cellml", RelativeTo::Absolute);
        CreateOptionsFile(handler2, proto_file, "LuoRudy1991", options);
        p_loader = converter.Convert(copied_cellml2);
        p_cell = boost::dynamic_pointer_cast<AbstractCardiacCell>(RunLr91Test(*p_loader, true, 2e-3));
        CheckSimpleProtocolOutputs<AbstractCardiacCell, std::vector<double> >(p_cell);

        options.back() = "--backward-euler";
        OutputFileHandler handler3(dirname + "BE");
        CopyFile(handler3, cellml_file);
        FileFinder maple_output_file("heart/src/odes/cellml/LuoRudy1991.out", RelativeTo::ChasteSourceRoot);
        CopyFile(handler3, maple_output_file);
        FileFinder copied_cellml3(handler3.GetOutputDirectoryFullPath() + "/LuoRudy1991.cellml", RelativeTo::Absolute);
        std::string extra_xml = std::string("<global>\n<lookup_tables>\n<lookup_table>\n") +
                "<var type='name'>intracellular_calcium_concentration,Cai</var>\n" +
                "<min>0.00001</min>\n<max>30.00001</max>\n<step>0.0001</step>\n" +
                "</lookup_table>\n</lookup_tables>\n</global>";
        CreateOptionsFile(handler3, proto_file, "LuoRudy1991", options, extra_xml);
        p_loader = converter.Convert(copied_cellml3);
        p_cell = boost::dynamic_pointer_cast<AbstractCardiacCell>(RunLr91Test(*p_loader, true, 0.3, "cytosolic_calcium_concentration"));
        CheckSimpleProtocolOutputs<AbstractCardiacCell, std::vector<double> >(p_cell);

        // Compare the last simulation above with normal Backward Euler to verify that the above tolerance really is needed!
        double magnitude = -25.5; // uA/cm2
        double duration  = 2.0 ;  // ms
        double when = 50.0; // ms
        boost::shared_ptr<SimpleStimulus> p_stimulus(new SimpleStimulus(magnitude, duration, when));
        boost::shared_ptr<EulerIvpOdeSolver> p_solver;
        CellLuoRudy1991FromCellMLBackwardEuler backward_lr91(p_solver, p_stimulus);
        RunOdeSolverWithIonicModel(&backward_lr91,
                                   1000.0,
                                   "BackwardLr91");
        CompareCellModelResults("ProtocolLr91", "BackwardLr91", 1e-12);
    }
};

#endif // TESTSIMPLEPROTOCOL_HPP_
