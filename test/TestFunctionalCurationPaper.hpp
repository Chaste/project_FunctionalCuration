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

#ifndef TESTFUNCTIONALCURATIONPAPER_HPP_
#define TESTFUNCTIONALCURATIONPAPER_HPP_


#include <boost/pointer_cast.hpp> // NB: Not available on Boost 1.33.1
#include <boost/shared_ptr.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

#include <cxxtest/TestSuite.h>

#include "OutputFileHandler.hpp"
#include "AbstractCvodeCell.hpp"
#include "AbstractCardiacCellInterface.hpp"
#include "AbstractSystemWithOutputs.hpp"

#include "ProtocolRunner.hpp"
#include "ProtoHelperMacros.hpp"
#include "NumericFileComparison.hpp"

#include "PetscSetupAndFinalize.hpp"

#include "Debug.hpp"
#include "UsefulFunctionsForProtocolTesting.hpp"

typedef N_Vector VECTOR;

class TestFunctionalCurationPaper : public CxxTest::TestSuite
{
private:
    /** How many different S2 intervals are used. */
    unsigned mNumDifferentS2Intervals;

    /** The calcium concentrations to step to from holding for the L-type Calcium current protocol. */
    std::vector<double> mStepCalcium;

    /** Keep track of the current directory */
    boost::shared_ptr<OutputFileHandler> mpHandler;

    /**
     * Compare the final results against historical data, to ensure that code changes aren't
     * introducing errors.
     *
     * @param rModelName  the name of the model, and hence the CellML file
     * @param rProtocolName  the protocol name
     * @param rOutputFileNames  the names of the output variables to compare (i.e. no extension)
     * @param relTol  relative tolerance
     * @param absTol  absolute tolerance
     */
    void CompareToHistoricalResults(const std::string& rModelName,
                                    const std::string& rProtocolName,
                                    const std::vector<std::string>& rOutputNames,
                                    double relTol, double absTol)
    {
        BOOST_FOREACH(std::string output_name, rOutputNames)
        {
            std::cout << "Comparing results of " << rProtocolName << " protocol on " << rModelName << ": " << output_name << "...";
            FileFinder ref_output("projects/FunctionalCuration/test/data/historic/" + rModelName + "/" +
                                  rProtocolName + "/" + output_name + ".dat",
                                  RelativeTo::ChasteSourceRoot);
            FileFinder test_output = mpHandler->FindFile(output_name + ".csv");
            if (!ref_output.Exists() && test_output.Exists())
            {
                TS_WARN("No historical data for model " + rModelName + " protocol " + rProtocolName
                        + " - please save results for future comparison");
                return;
            }
            TSM_ASSERT("No results found but historical data exists for model " + rModelName + " protocol " + rProtocolName,
                       !ref_output.Exists() || test_output.Exists());
            if (!test_output.Exists())
            {
                std::cout << std::endl;
                return;
            }

            NumericFileComparison comp(test_output.GetAbsolutePath(), ref_output.GetAbsolutePath());
            TS_ASSERT(comp.CompareFiles(absTol, 0, relTol));
            std::cout << "done." << std::endl;
        }
    }

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
     * @param rCellMLFileBaseName  the cellml file to dynamically load and use.
     * @param s1PacingCycleLength  the S1 duration to use
     */
    void RunS1S2Protocol(std::string& rCellMLFileBaseName, double s1PacingCycleLength)
    {
        std::string dirname = SetupOutputFileHandler(rCellMLFileBaseName, "S1S2");


        FileFinder cellml_file("projects/FunctionalCuration/cellml/" + rCellMLFileBaseName + ".cellml", RelativeTo::ChasteSourceRoot);
        FileFinder proto_xml_file("projects/FunctionalCuration/test/protocols/S1S2.xml", RelativeTo::ChasteSourceRoot);

        ProtocolRunner runner(cellml_file, proto_xml_file, dirname);
        runner.GetProtocol()->SetInput("s1_interval", CONST(s1PacingCycleLength));

        // Special case for non-conservative models which break after lots of paces
        std::set<std::string> non_conservative_models =
                boost::assign::list_of("jafri_rice_winslow_model_1998")
                                      ("winslow_model_1999");
        if (non_conservative_models.find(rCellMLFileBaseName) != non_conservative_models.end())
        {
            runner.GetProtocol()->SetInput("steady_state_beats", CONST(10));
        }

        runner.RunProtocol();
        AbstractValuePtr p_V = runner.GetProtocol()->rGetOutputsCollection().Lookup("membrane_voltage", "RunS1S2Protocol");
        mNumDifferentS2Intervals = GET_ARRAY(p_V).GetShape()[0];
    }

    /**
     * Creates the gnuplots of the full S1-S2 timecourses
     */
    void GenerateGnuplotsFullTraces(const std::string& rDirectory,
                                    const std::string& rFilenamePrefix)
    {
        std::string filename = "V.gp";
        unsigned num_cols = mNumDifferentS2Intervals;
        out_stream p_gnuplot_script = mpHandler->OpenOutputFile(filename);
        std::string output_dir = mpHandler->GetOutputDirectoryFullPath();
        *p_gnuplot_script << "# Gnuplot script file generated by Chaste." << std::endl;

        // Write the plot data to a single file
        std::string steppers_file = output_dir + rFilenamePrefix + "-steppers.csv";
        std::string V_file = output_dir + rFilenamePrefix + "_membrane_voltage.csv";
        std::string awk_cmd = "tail -1 " + steppers_file + " | awk 'BEGIN { FS=\",\" ; OFS=\"\\n\"} ; {$1=\"\" ; $2=\"#\" ; print $0}' - ";
        std::string cmd = awk_cmd + " | tail -n +2 | paste -d, - " + V_file + " > " + output_dir + "V.dat";
        EXPECT0(system, cmd);

        // Write out a gnuplot script
        *p_gnuplot_script << "# First plot is of the actual action potentials overlaid on one another." << std::endl;
        *p_gnuplot_script  << "set terminal postscript eps size 3, 2" << std::endl;
        *p_gnuplot_script  << "set output \"" << output_dir << "V.eps\"" << std::endl;
        *p_gnuplot_script  << "set title \"" << GetTitleFromDirectory(rDirectory) << "\"" << std::endl;
        *p_gnuplot_script  << "set xlabel \"Time (ms)\"" << std::endl;
        *p_gnuplot_script  << "set ylabel \"Voltage (mV)\"" << std::endl;
        *p_gnuplot_script  << "set grid" << std::endl;
        *p_gnuplot_script  << "set autoscale" << std::endl;
        *p_gnuplot_script  << "set style data lines" << std::endl;
        *p_gnuplot_script  << "set key off" << std::endl;
        *p_gnuplot_script << "set datafile separator \",\"" << std::endl;

        *p_gnuplot_script  << "plot ";
        for (unsigned i=1; i<=num_cols ; ++i)
        {
            *p_gnuplot_script << "\"" + output_dir + "V.dat\" using "
                              << 1 << ":" << i+1 << " with lines title \"" << i << "\"";
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
     * Creates the gnuplot S1-S2 restitution curves
     */
    void GenerateGnuplotsS1S2Curve(const std::string& rDirectory,
                                   const std::string& rFilenamePrefix)
    {
        std::string filename = "S1S2.gp";
        out_stream p_gnuplot_script = mpHandler->OpenOutputFile(filename);
        std::string output_dir = mpHandler->GetOutputDirectoryFullPath();
        *p_gnuplot_script << "# Gnuplot script file generated by Chaste." << std::endl;

        // Put the plot data in a single file
        std::string data_prefix = output_dir + rFilenamePrefix + "_";
        std::string cmd = "paste -d, " + data_prefix + "DI.csv "
                + data_prefix + "APD90.csv | tail -n +3 > " + output_dir + "S1S2.dat";
        EXPECT0(system, cmd);

        *p_gnuplot_script  << "# Second plot is of the resulting restitution curve." << std::endl;
        *p_gnuplot_script  << "set terminal postscript eps size 3, 2 font 16" << std::endl;
        *p_gnuplot_script  << "set output \"" << output_dir << "S1S2.eps\"" << std::endl;
        *p_gnuplot_script  << "set title \"" << GetTitleFromDirectory(rDirectory) << "\"" << std::endl;
        *p_gnuplot_script  << "set xlabel \"Diastolic Interval (ms)\"" << std::endl;
        *p_gnuplot_script  << "set xtics 400" << std::endl;
        *p_gnuplot_script  << "set ylabel \"APD90 (ms)\"" << std::endl;
        *p_gnuplot_script  << "set grid" << std::endl;
        *p_gnuplot_script  << "set autoscale" << std::endl;
        *p_gnuplot_script  << "set key off" << std::endl;
        *p_gnuplot_script << "set datafile separator \",\"" << std::endl;
        *p_gnuplot_script  << "plot \"" + output_dir + "S1S2.dat\" using 1:2 with linespoints pointtype 7";
        *p_gnuplot_script << std::endl << std::flush;

        p_gnuplot_script->close();

        // Run Gnuplot on the script written above to generate image files.
        EXPECT0(system, "gnuplot " + output_dir + filename);
    }

    /**
     * Run the ICaL protocol on the given model.
     *
     * @param rCellMLFileBaseName  the cellml file to dynamically load and use.
     * @return  whether the ICaL protocol was completed successfully
     */
    bool RunICaLVoltageClampProtocol(std::string& rCellMLFileBaseName)
    {
        std::string dirname = SetupOutputFileHandler(rCellMLFileBaseName, "ICaL");
        FileFinder cellml_file("projects/FunctionalCuration/cellml/" + rCellMLFileBaseName + ".cellml", RelativeTo::ChasteSourceRoot);
        FileFinder proto_xml_file("projects/FunctionalCuration/test/protocols/ICaL.xml", RelativeTo::ChasteSourceRoot);

        try
        {
            ProtocolRunner runner(cellml_file, proto_xml_file, dirname);
            runner.RunProtocol();
            // Fill in mStepCalcium for benefit of gnuplot output
            AbstractValuePtr p_Cao = runner.GetProtocol()->rGetLibrary().Lookup("default_Cao", "RunICaLVoltageClampProtocol");
            double default_Cao = GET_SIMPLE_VALUE(p_Cao);
            mStepCalcium = boost::assign::list_of(.5*default_Cao)(default_Cao)(1.5*default_Cao);
//            std::cout << "[Ca_o] = " << default_Cao << " mM" << std::endl << std::flush;
        }
        catch (Exception& e)
        {
            OUR_WARN(e.GetMessage(), rCellMLFileBaseName, "ICaL");
            return false;
        }

        // Successful run
        return true;
    }

    /**
     * Creates the gnuplots of the ICaL IV curves.
     */
    void GenerateGnuplotsIVCurves(const std::string& rDirectory,
                                  const std::string& rFilenamePrefix)
    {
        std::string filename = "ICaL_IV.gp";
        out_stream p_gnuplot_script = mpHandler->OpenOutputFile(filename);
        std::string output_dir = mpHandler->GetOutputDirectoryFullPath();
        *p_gnuplot_script << "# Gnuplot script file generated by Chaste." << std::endl;

        // Generate a single file with the plot data
        std::string data_prefix = output_dir + rFilenamePrefix + "_";
        std::string cmd = "paste -d, " + data_prefix + "final_membrane_voltage.csv "
                + data_prefix + "min_LCC.csv > " + output_dir + "ICaL_IV.dat";
        EXPECT0(system, cmd);

        *p_gnuplot_script  << "# Second plot is of the resulting restitution curve." << std::endl;
        *p_gnuplot_script  << "set terminal postscript eps enhanced size 3, 2 font 16" << std::endl;
        *p_gnuplot_script  << "set output \"" << output_dir << "ICaL_IV.eps\"" << std::endl;
        *p_gnuplot_script  << "set title \"" << GetTitleFromDirectory(rDirectory) << "\"" << std::endl;
        *p_gnuplot_script  << "set xlabel \"Test Voltage (mV)\"" << std::endl;
        *p_gnuplot_script  << "set ylabel \"Maximum Current ({/Symbol m}A/cm^{2})\" 0.75,0.0" << std::endl;
        *p_gnuplot_script  << "set grid" << std::endl;
        *p_gnuplot_script  << "set autoscale" << std::endl;
        *p_gnuplot_script  << "set xrange [-60:80]" << std::endl;
        *p_gnuplot_script  << "set key off" << std::endl;
        *p_gnuplot_script << "set datafile separator \",\"" << std::endl;

        *p_gnuplot_script  << "plot ";
        unsigned num_cols = mStepCalcium.size();
        for (unsigned i=1; i<=num_cols; ++i)
        {
            *p_gnuplot_script << "\"" + output_dir + "ICaL_IV.dat\" using " << i << ":" << i+num_cols
                              << " with linespoints pointtype 7 title \"[Cao] = " << mStepCalcium[i-1] << "\"";
            if (i != num_cols)
            {
                *p_gnuplot_script << ",\\";
            }
            *p_gnuplot_script << std::endl;
        }
        *p_gnuplot_script << std::endl << std::flush;

        p_gnuplot_script->close();

        // Run Gnuplot on the script written above to generate image files.
        EXPECT0(system, "gnuplot " + output_dir + filename);
    }



public:
    void TestProtocolsForManyCellModels() throw(Exception, std::bad_alloc)
    {
        std::vector<std::string> cellml_files = GetAListOfCellMLFiles();
        if (CommandLineArguments::Instance()->OptionExists("--models"))
        {
            cellml_files = CommandLineArguments::Instance()->GetStringsCorrespondingToOption("--models");
        }

        std::vector<std::string> ical_outputs = boost::assign::list_of("outputs_min_LCC")("outputs_final_membrane_voltage");
        std::vector<std::string> s1s2_outputs = boost::assign::list_of("outputs_APD90")("outputs_DI");

        for (unsigned i=0; i<cellml_files.size(); ++i)
        {
            std::cout << "\nRunning protocols for " << cellml_files[i] << std::endl << std::flush;

            // Figure out if it is a dog model (different S1 to match experimental data)
            double s1_freq = 1000;
            if (  cellml_files[i] == "decker_2009"
               || cellml_files[i] == "hund_rudy_2004"
               || cellml_files[i] == "livshitz_rudy_2007"
               || cellml_files[i] == "fox_mcharg_gilmour_2002"
               || cellml_files[i] == "winslow_model_1999" )
            {
                s1_freq = 2000;
            }

            // If it is a human model compare with this Morgan data for 600ms pacing.
            if (  cellml_files[i] == "fink_noble_giles_model_2008"
               || cellml_files[i] == "grandi_pasqualini_bers_2010_ss"
               || cellml_files[i] == "iyer_2004_s1s2_curve"
               || cellml_files[i] == "iyer_model_2007_s1s2_curve"
               || cellml_files[i] == "priebe_beuckelmann_1998_s1s2_curve"
               || cellml_files[i] == "ten_tusscher_model_2004_epi_s1s2_curve"
               || cellml_files[i] == "ten_tusscher_model_2006_epi_s1s2_curve" )
            {
                s1_freq = 600;
            }

            // Protocols and graphs must be done together so that
            // mpHandler points to the correct protocol subfolder.

            try
            {
                RunS1S2Protocol(cellml_files[i], s1_freq);
                GenerateGnuplotsFullTraces(cellml_files[i], "outputs");
                GenerateGnuplotsS1S2Curve(cellml_files[i], "outputs");
            }
            catch (Exception& e)
            {
                OUR_WARN(e.GetMessage(), cellml_files[i], "S1S2");
            }
            CompareToHistoricalResults(cellml_files[i], "S1S2", s1s2_outputs, 0.005, 1e-6); // 0.5% rel tol

            if (RunICaLVoltageClampProtocol(cellml_files[i]))
            {
                try
                {
                    GenerateGnuplotsIVCurves(cellml_files[i], "outputs");
                }
                catch (Exception &e)
                {
                    OUR_WARN(e.GetMessage(), cellml_files[i], "ICaL");
                }
            }
            CompareToHistoricalResults(cellml_files[i], "ICaL", ical_outputs, 0.005, 1e-6); // 0.5% rel tol
        }

        ////////////////////////////////////////////////////////////////////////////////////
        // Also make plots of some experimental data we got by digitising some paper graphs.
        ////////////////////////////////////////////////////////////////////////////////////

        std::vector<std::string> exp_data;
        exp_data.push_back("sicouri_dog_ventricle");
        exp_data.push_back("kim_human_atrial");
        exp_data.push_back("morgan_human_ventricle");

        // Also specify the calcium experimental data.
        exp_data.push_back("sun_rat_ventricle");
        mStepCalcium.clear();
        mStepCalcium.push_back(0.1);
        mStepCalcium.push_back(0.3);
        mStepCalcium.push_back(1.0);

        for (unsigned i=0; i<exp_data.size(); ++i)
        {
            // Copy the data from
            FileFinder finder("projects/FunctionalCuration/test/data/" + exp_data[i], RelativeTo::ChasteSourceRoot);
            TS_ASSERT_EQUALS(finder.IsDir(), true);
            mpHandler.reset(new OutputFileHandler("FunctionalCuration/" + exp_data[i], true)); // Clean this dir
            EXPECT0(system, "cp " + finder.GetAbsolutePath() + "* " + mpHandler->GetOutputDirectoryFullPath());

            if (i<3)
            {   // This is S1-S2 data
                GenerateGnuplotsS1S2Curve(exp_data[i], exp_data[i]);
            }
            else
            {   // This is ICaL data
                try
                {
                    GenerateGnuplotsIVCurves(exp_data[i], exp_data[i]);
                }
                catch (Exception& e)
                {
                    OUR_WARN(e.GetMessage(), exp_data[i], "ICaL");
                }
            }
        }
    }
};

#endif // TESTFUNCTIONALCURATIONPAPER_HPP_
