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
    /** Keep track of the current directory */
    boost::shared_ptr<OutputFileHandler> mpHandler;

    /**
     * Sets up #mpHandler to point to correct model and protocol subfolder.
     *
     * @param rCellMLFileBaseName  The name of the cellML file (without .cellml on the end).
     * @param rProtocolName  The protocol name.
     * @param reset  Whether to wipe the directory (defaults to false).
     *
     * @return  The directory path relative to $CHASTE_TEST_OUTPUT
     */
    std::string SetupOutputFileHandler(const std::string& rCellMLFileBaseName, const std::string& rProtocolName, bool reset=false)
    {
        std::string dirname = "FunctionalCuration/" + rCellMLFileBaseName + "/" + rProtocolName;
        mpHandler.reset(new OutputFileHandler(dirname, reset)); // Create a new directory for this model
        return dirname;
    }

    /**
     * Creates the gnuplots of the full voltage timecourses
     */
    void GenerateGnuplotsFullTraces(const std::string& rDirectory,
                                    const std::string& rFilenamePrefix)
    {
        std::string filename = "V.gp";
        unsigned num_cols = 7; // Hardcoded - \todo get this out of protocol somehow.
        out_stream p_gnuplot_script = mpHandler->OpenOutputFile(filename);
        std::string output_dir = mpHandler->GetOutputDirectoryFullPath();
        *p_gnuplot_script << "# Gnuplot script file generated by Chaste." << std::endl;

        // Write the plot data to a single file
        std::string steppers_file = output_dir + rFilenamePrefix + "_steady_state_time.csv";
        std::string V_file = output_dir + rFilenamePrefix + "_steady_state_membrane_voltage.csv";

        std::string cmd = "paste -d , " + steppers_file + " " + V_file + " | tail -n +2 > " + output_dir + "V.dat";
        //std::cout << cmd << "\n" << std::flush;
        EXPECT0(system, cmd);

        // Write out a gnuplot script
        *p_gnuplot_script << "# First plot is of the actual action potentials overlaid on one another." << std::endl;
        *p_gnuplot_script << "set terminal postscript eps size 3, 2" << std::endl;
        *p_gnuplot_script << "set output \"" << output_dir << "V.eps\"" << std::endl;
        *p_gnuplot_script << "set title \"" << GetTitleFromDirectory(rDirectory) << "\"" << std::endl;
        *p_gnuplot_script << "set xlabel \"Time (ms)\"" << std::endl;
        *p_gnuplot_script << "set ylabel \"Voltage (mV)\"" << std::endl;
        *p_gnuplot_script << "set grid" << std::endl;
        *p_gnuplot_script << "set autoscale" << std::endl;
        *p_gnuplot_script << "set style data lines" << std::endl;
        *p_gnuplot_script << "set key off" << std::endl;
        *p_gnuplot_script << "set datafile separator \",\"" << std::endl;

        *p_gnuplot_script  << "plot ";
        for (unsigned i=1; i<=num_cols ; ++i)
        {
            *p_gnuplot_script << "\"" + output_dir + "V.dat\" using "
                              << 1 << ":" << i+num_cols << " with lines title \"" << i << "\"";
            if (i != num_cols)
            {
                *p_gnuplot_script << ",\\";
            }
            *p_gnuplot_script << std::endl;
        }

        p_gnuplot_script->close();

        // Run Gnuplot on the script written above to generate image files.
        EXPECT0(system, "gnuplot " + output_dir + filename);
    }

    /**
     * Run the Hypokalaemia protocol on the given model.
     *
     * @param rCellMLFileBaseName  the cellml file to dynamically load and use.
     * @return  whether the protocol was completed successfully
     */
    bool RunHypokalaemiaProtocol(std::string& rCellMLFileBaseName)
    {
        std::string dirname = SetupOutputFileHandler(rCellMLFileBaseName, "Hypokalaemia");
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
            return false;
        }

        // Successful run
        return true;
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
        TS_ASSERT_EQUALS(peak_voltage.GetNumElements(), 7u);
        TS_ASSERT_DELTA(*peak_voltage.Begin(), 50.3177, 5e-2);
        NdArray<double> resting_potential = GET_ARRAY(r_outputs.Lookup("resting_potential"));
        TS_ASSERT_EQUALS(resting_potential.GetNumElements(), 7u);
        TS_ASSERT_DELTA(*resting_potential.Begin(), -75.4569, 1e-2);

        // These are the results that came out first attempt (they look sensible)
        std::vector<double> reference_apd90 = list_of(298.139)(318.821)(343.841)(375.742)(418.564)(476.702)(570.496);
        std::vector<double> reference_resting = list_of(-75.4569)(-78.5914)(-82.1074)(-86.0988)(-90.6683)(-95.7858)(-98.9795);

        NdArray<double> apd90 = GET_ARRAY(r_outputs.Lookup("raw_APD90"));
        NdArray<double>::Indices apd90_indices = apd90.GetIndices();
        TS_ASSERT_EQUALS(apd90.GetNumDimensions(), 2u);
        TS_ASSERT_EQUALS(apd90.GetNumElements(), 7u);

        NdArray<double> resting = GET_ARRAY(r_outputs.Lookup("resting_potential"));
        NdArray<double>::Indices resting_indices = resting.GetIndices();
        TS_ASSERT_EQUALS(resting.GetNumDimensions(), 2u);
        TS_ASSERT_EQUALS(resting.GetNumElements(), 7u);

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

            // Protocols and graphs must be done together so that
            // mpHandler points to the correct protocol subfolder.

            if (RunHypokalaemiaProtocol(cellml_files[i]))
            {
                try
                {
                	GenerateGnuplotsFullTraces(cellml_files[i], "outputs");
                }
                catch (Exception &e)
                {
                    OUR_WARN(e.GetMessage(), cellml_files[i], "Hypokalaemia");
                }
            }
        }

    }
};

#endif // TESTHYPOKALAEMIAPROTOCOL_HPP_
