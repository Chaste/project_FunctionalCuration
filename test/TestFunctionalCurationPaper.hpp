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
#include "PetscTools.hpp"

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

            std::string base_name = "projects/FunctionalCuration/test/data/historic/" + rModelName + "/" +
                    rProtocolName + "/" + output_name;
            FileFinder ref_output(base_name + ".dat", RelativeTo::ChasteSourceRoot);

            if (!ref_output.Exists()) // New output is in .csv format so if missing look for that instead...
            {
                ref_output.SetPath(base_name + ".csv", RelativeTo::ChasteSourceRoot);
            }

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
        ProtocolFileFinder proto_xml_file("projects/FunctionalCuration/test/protocols/S1S2.xml", RelativeTo::ChasteSourceRoot);

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
        std::string data_file_name = data_prefix + "S1-S2_curve_gnuplot_data.csv";

        *p_gnuplot_script  << "# Second plot is of the resulting restitution curve." << std::endl;
        *p_gnuplot_script  << "set terminal postscript eps size 12.75cm, 8cm font 18" << std::endl;
        *p_gnuplot_script  << "set output \"" << output_dir << "S1S2.eps\"" << std::endl;
        *p_gnuplot_script  << "set title \"" << GetTitleFromDirectory(rDirectory) << "\"" << std::endl;
        *p_gnuplot_script  << "set xlabel \"Diastolic Interval (ms)\"" << std::endl;
        *p_gnuplot_script  << "set xtics 400" << std::endl;
        *p_gnuplot_script  << "set ylabel \"APD90 (ms)\"" << std::endl;
        *p_gnuplot_script  << "set grid" << std::endl;
        *p_gnuplot_script  << "set autoscale" << std::endl;
        *p_gnuplot_script  << "set key off" << std::endl;
        *p_gnuplot_script << "set datafile separator \",\"" << std::endl;
        *p_gnuplot_script  << "plot \"" + data_file_name + "\" using 1:2 with linespoints pointtype 7";
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
        ProtocolFileFinder proto_xml_file("projects/FunctionalCuration/test/protocols/ICaL.xml", RelativeTo::ChasteSourceRoot);

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
        std::string data_file_name = data_prefix + "IV_curve_gnuplot_data.csv";

        *p_gnuplot_script  << "# Second plot is of the resulting restitution curve." << std::endl;
        *p_gnuplot_script  << "set terminal postscript eps enhanced size 12.75cm, 8cm font 18" << std::endl;
        *p_gnuplot_script  << "set output \"" << output_dir << "ICaL_IV.eps\"" << std::endl;
        *p_gnuplot_script  << "set title \"" << GetTitleFromDirectory(rDirectory) << "\"" << std::endl;
        *p_gnuplot_script  << "set xlabel \"Test Voltage (mV)\"" << std::endl;
        *p_gnuplot_script  << "set ylabel \"Maximum Current ({/Symbol m}A/cm^{2})\"" << std::endl;
        *p_gnuplot_script  << "set grid" << std::endl;
        *p_gnuplot_script  << "set autoscale" << std::endl;
        *p_gnuplot_script  << "set xrange [-60:80]" << std::endl;
        *p_gnuplot_script  << "set key off" << std::endl;
        *p_gnuplot_script << "set datafile separator \",\"" << std::endl;

        *p_gnuplot_script  << "plot ";
        unsigned num_cols = mStepCalcium.size();
        for (unsigned i=1; i<=num_cols; ++i)
        {
            *p_gnuplot_script << "\"" + data_file_name + "\" using 1:" << i+1
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

        // Collectively ensure the root output folder exists, then isolate processes
        {
            OutputFileHandler("FunctionalCuration", false);
            PetscTools::IsolateProcesses(true);
        }

        for (unsigned i=0; i<cellml_files.size(); ++i)
        {
            if (PetscTools::IsParallel() && i % PetscTools::GetNumProcs() != PetscTools::GetMyRank())
            {
                // Let someone else do this model
                continue;
            }
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
            CompareToHistoricalResults(cellml_files[i], "ICaL", ical_outputs, 0.005, 1e-5); // 0.5% rel tol
        }

        ////////////////////////////////////////////////////////////////////////////////////
        // Also make plots of some experimental data we got by digitising some paper graphs.
        ////////////////////////////////////////////////////////////////////////////////////
        if (!PetscTools::AmTopMost())
        {
            // Note: AmTopMost still identifies a single process even when processes are isolated.
            // Only one process should plot the experimental data.
            return;
        }

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
                try
                {
                    GenerateGnuplotsS1S2Curve(exp_data[i], exp_data[i]);
                }
                catch (Exception& e)
                {
                    OUR_WARN(e.GetMessage(), exp_data[i], "S1S2");
                }
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
