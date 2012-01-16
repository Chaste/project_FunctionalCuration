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

#ifndef TESTHYPOKALAEMIAPROTOCOL_HPP_
#define TESTHYPOKALAEMIAPROTOCOL_HPP_

#include <cxxtest/TestSuite.h>
#include <boost/assign/list_of.hpp>
using boost::assign::list_of;

#include "ProtocolRunner.hpp"
#include "ProtocolLanguage.hpp"
#include "ProtoHelperMacros.hpp"
#include "OutputFileHandler.hpp"

#include "UsefulFunctionsForProtocolTesting.hpp"
#include "PetscSetupAndFinalize.hpp"

class TestHypokalaemiaProtocol : public CxxTest::TestSuite
{
private:
    /**
     * Run the Hypokalaemia protocol on the given model.
     *
     * @param rCellMLFileBaseName  the cellml file to dynamically load and use.
     */
    void RunHypokalaemiaProtocol(std::string& rCellMLFileBaseName)
    {
        std::string dirname = "FunctionalCuration/" + rCellMLFileBaseName + "/Hypokalaemia";
        FileFinder cellml_file("projects/FunctionalCuration/cellml/" + rCellMLFileBaseName + ".cellml", RelativeTo::ChasteSourceRoot);
        FileFinder proto_xml_file("projects/FunctionalCuration/test/private/protocols/Hypokalaemia.xml", RelativeTo::ChasteSourceRoot);

        try
        {
            ProtocolRunner runner(cellml_file, proto_xml_file, dirname);
            runner.GetProtocol()->SetInput("steady_state_beats", CONST(10));
            runner.RunProtocol();
        }
        catch (Exception& e)
        {
            OUR_WARN(e.GetMessage(), rCellMLFileBaseName, "Hypokalaemia");
        }
    }

public:
    void TestHypokalaemiaProtocolRunning() throw(Exception, std::bad_alloc)
    {
        std::string dirname = "TestHypokalaemiaProtocolOutputs";
        FileFinder cellml_file("projects/FunctionalCuration/cellml/luo_rudy_1991.cellml", RelativeTo::ChasteSourceRoot);
        FileFinder proto_xml_file("projects/FunctionalCuration/test/private/protocols/Hypokalaemia.xml", RelativeTo::ChasteSourceRoot);

        ProtocolRunner runner(cellml_file, proto_xml_file, dirname, true);

        // Assume we get to steady state quickly - CHANGE THIS FOR DECENT ANSWERS!
        runner.GetProtocol()->SetInput("steady_state_beats", CONST(10));
        runner.GetProtocol()->SetInput("pacing_period", CONST(1000)); // Period of pacing in ms.

        // Run
        runner.RunProtocol();

        // Check the results of post-processing are correct.
        const Environment& r_outputs = runner.GetProtocol()->rGetOutputsCollection();
        NdArray<double> peak_voltage = GET_ARRAY(r_outputs.Lookup("peak_voltage"));
        TS_ASSERT_EQUALS(peak_voltage.GetNumElements(), 9u);
        TS_ASSERT_DELTA(*peak_voltage.Begin(), 42.7223, 5e-2);
        NdArray<double> resting_potential = GET_ARRAY(r_outputs.Lookup("resting_potential"));
        TS_ASSERT_EQUALS(resting_potential.GetNumElements(), 9u);

        // These are the results that came out first attempt (they look sensible)
        std::vector<double> reference_apd90 = list_of(264.9502)(280.4469)(298.139)(318.821)(343.841)(375.742)(418.564)(476.702)(570.496);
        std::vector<double> reference_resting = list_of(-70.0511)(-72.6291)(-75.4569)(-78.5914)(-82.1074)(-86.0988)(-90.6683)(-95.7858)(-98.9795);

        NdArray<double> apd90 = GET_ARRAY(r_outputs.Lookup("APD90"));
        NdArray<double>::Indices apd90_indices = apd90.GetIndices();
        TS_ASSERT_EQUALS(apd90.GetNumDimensions(), 2u);
        TS_ASSERT_EQUALS(apd90.GetNumElements(), 9u);

        NdArray<double> resting = GET_ARRAY(r_outputs.Lookup("resting_potential"));
        NdArray<double>::Indices resting_indices = resting.GetIndices();
        TS_ASSERT_EQUALS(resting.GetNumDimensions(), 2u);
        TS_ASSERT_EQUALS(resting.GetNumElements(), 9u);

        for (unsigned i=0; i<reference_apd90.size(); i++)
        {
        	TS_ASSERT_DELTA(apd90[apd90_indices], reference_apd90[i], 0.6);
        	apd90.IncrementIndices(apd90_indices);
        	TS_ASSERT_DELTA(resting[resting_indices], reference_resting[i], 1e-2);
            resting.IncrementIndices(resting_indices);
        }
    }

    void TestProtocolsForManyCellModels() throw(Exception, std::bad_alloc)
    {
        std::vector<std::string> cellml_files = GetAListOfCellMLFiles();
        if (CommandLineArguments::Instance()->OptionExists("--models"))
        {
            cellml_files = CommandLineArguments::Instance()->GetStringsCorrespondingToOption("--models");
        }

        for (unsigned i=0; i<cellml_files.size(); ++i)
        {
            std::cout << "\nRunning protocols for " << cellml_files[i] << std::endl << std::flush;
            RunHypokalaemiaProtocol(cellml_files[i]);
        }
    }
};

#endif // TESTHYPOKALAEMIAPROTOCOL_HPP_
