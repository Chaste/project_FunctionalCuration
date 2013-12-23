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

/*= High throughput functional curation of cellular electrophysiology models =
 *
 * This test reproduces the results from our
 * [http://dx.doi.org/10.1016/j.pbiomolbio.2011.06.003 reference publication on Functional Curation]
 * (J. Cooper, G. Mirams, S. Niederer; Prog Biophys Mol Biol; 107(1):11-20, 2011).
 *
 * It is kept up-to-date to run using the latest version of the framework, and tested automatically
 * each week to ensure that the results are, to within tolerances, essentially unchanged from those published.
 *
 * (Aside: the reference data included in this project do not all exactly match those used in the paper.
 * For one model subsequent investigation has revealed the results in the paper to be slightly
 * incorrect.  Tightening of tolerances and refinement of sampling intervals have also
 * improved the quality of the results for several models, especially under the ICaL protocol,
 * beyond the default comparison tolerance of 0.5%.)
 *
 * Another model (Faber-Rudy) shows extremely
 * high sensitivity to code generation settings in the peak transmembrane potential, which in
 * turn affects the APD90 calculation in the S1-S2 protocol.  However, all these results have been
 * verified manually to match the original publication qualitatively.
 *
 * Another notable exception is that (through the process of functional curation) we found that
 * the Decker 2009 model did not replicate results shown in its original paper (S1-S2 curve was
 * very different). This led to us finding a bug in the CellML encoding which we have corrected,
 * and now the Decker 2009 model gives a sensible S1-S2 curve, unlike that shown in our paper!
 *
 * You can run these simulations using the following command from within the Chaste source tree:
 * {{{
 * scons cl=1 b=GccOptNative ts=projects/FunctionalCuration/test/TestFunctionalCurationPaper.hpp
 * }}}
 *
 * If you have multiple cores available, these may be used to speed up simulation.  For instance, to use
 * 8 cores, run
 * {{{
 * scons cl=1 b=GccOptNative_8 ts=projects/FunctionalCuration/test/TestFunctionalCurationPaper.hpp
 * }}}
 *
 * Output will appear in `/tmp/$USER/testoutput/FunctionalCuration` by default (unless the
 * environment variable `CHASTE_TEST_OUTPUT` is set to point elsewhere; it defaults to
 * `/tmp/$USER/testoutput`.  This location should be on local disk).  The test should pass
 * (printing an 'OK' line towards the end) to show that the protocol results generated are
 * consistent with those in the paper.  (We also compare against some additional reference results,
 * for model/protocol combinations analysed after the paper was published.)
 *
 * Note that some warnings will be printed at the end of the test output.  These are generally
 * expected, for model/protocol combinations where we cannot run the protocol to completion
 * (for instance, some models lack extracellular calcium and so the ICaL protocol is not appropriate).
 */

/* -----
 *
 * The test starts by including required headers.
 */

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

#include "UsefulFunctionsForProtocolTesting.hpp"

typedef N_Vector VECTOR;

/* This class contains the main functionality for the test.
 * It begins with private utility methods; the main method which runs the protocols (`TestProtocolsForManyCellModels`)
 * is at the end.
 */
class TestFunctionalCurationPaper : public CxxTest::TestSuite
{
private:
    /** Keep track of the current directory */
    boost::shared_ptr<OutputFileHandler> mpHandler;

    /**
     * This method compares the final results against historical data, to ensure that code changes aren't
     * introducing errors.
     *
     * @param rModelName  the name of the model, and hence the CellML file
     * @param rProtocolName  the protocol name
     * @param rOutputFileNames  the names of the output variables to compare (i.e. no extension)
     * @param relTol  relative tolerance
     * @param absTol  absolute tolerance
     * @return  true if the results match historical data, provided the latter are present
     */
    bool CompareToHistoricalResults(const std::string& rModelName,
                                    const std::string& rProtocolName,
                                    const std::vector<std::string>& rOutputNames,
                                    double relTol, double absTol)
    {
        bool all_match = true;
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
                return true;
            }
            TSM_ASSERT("No results found but historical data exists for model " + rModelName + " protocol " + rProtocolName,
                       !ref_output.Exists() || test_output.Exists());
            if (!test_output.Exists())
            {
                std::cout << std::endl;
                return !ref_output.Exists();
            }

            NumericFileComparison comp(test_output.GetAbsolutePath(), ref_output.GetAbsolutePath());
            bool match = comp.CompareFiles(absTol, 0, relTol);
            TS_ASSERT(match);
            std::cout << "done." << std::endl;
            all_match = all_match && match;
        }
        return all_match;
    }

    /**
     * Set up #mpHandler to point to the correct model and protocol subfolder.
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
     * Run the S1-S2 protocol on a single model.
     *
     * @param rCellMLFileBaseName  the CellML file to dynamically load and use.
     * @param s1PacingCycleLength  the S1 duration to use (to correspond with experimental data for the particular species)
     */
    void RunS1S2Protocol(std::string& rCellMLFileBaseName, double s1PacingCycleLength)
    {
        std::string dirname = SetupOutputFileHandler(rCellMLFileBaseName, "S1S2");

        FileFinder cellml_file("projects/FunctionalCuration/cellml/" + rCellMLFileBaseName + ".cellml", RelativeTo::ChasteSourceRoot);
        ProtocolFileFinder proto_xml_file("projects/FunctionalCuration/test/protocols/S1S2.xml", RelativeTo::ChasteSourceRoot);

        ProtocolRunner runner(cellml_file, proto_xml_file, dirname);
        runner.GetProtocol()->SetInput("s1_interval", CONST(s1PacingCycleLength));

        // Special case for non-conservative models which break after lots of paces.
        std::set<std::string> non_conservative_models =
                boost::assign::list_of("jafri_rice_winslow_model_1998")
                                      ("winslow_model_1999");
        if (non_conservative_models.find(rCellMLFileBaseName) != non_conservative_models.end())
        {
            runner.GetProtocol()->SetInput("steady_state_beats", CONST(10));
        }

        // Augment the plot specification for the S1-S2 curve to match presentation settings used in the original paper.
        BOOST_FOREACH(PlotSpecificationPtr p_plot_spec, runner.GetProtocol()->rGetPlotSpecifications())
        {
            if (p_plot_spec->rGetTitle() == "S1-S2 curve")
            {
                p_plot_spec->SetDisplayTitle(GetTitleFromDirectory(rCellMLFileBaseName));
                p_plot_spec->SetGnuplotTerminal("postscript eps enhanced size 12.75cm, 8cm font 18");
                std::vector<std::string> extra_setup;
                extra_setup.push_back("set xtics 400");
                p_plot_spec->SetGnuplotExtraCommands(extra_setup);
                p_plot_spec->SetStyle("linespoints pointtype 7");
            }
        }

        runner.RunProtocol();
    }

    /**
     * Run the ICaL protocol on the given model.
     *
     * @param rCellMLFileBaseName  the CellML file to dynamically load and use.
     */
    void RunICaLVoltageClampProtocol(std::string& rCellMLFileBaseName)
    {
        std::string dirname = SetupOutputFileHandler(rCellMLFileBaseName, "ICaL");
        FileFinder cellml_file("projects/FunctionalCuration/cellml/" + rCellMLFileBaseName + ".cellml", RelativeTo::ChasteSourceRoot);
        ProtocolFileFinder proto_xml_file("projects/FunctionalCuration/test/protocols/ICaL.xml", RelativeTo::ChasteSourceRoot);

        ProtocolRunner runner(cellml_file, proto_xml_file, dirname);

        // A special case - some versions of CVODE fail to solve this model for some test potentials otherwise!
        if (rCellMLFileBaseName == "iribe_model_2006")
        {
            boost::shared_ptr<AbstractSystemWithOutputs> p_model = runner.GetProtocol()->GetModel();
            dynamic_cast<AbstractCvodeCell*>(p_model.get())->SetTolerances(1e-5, 1e-7);
        }

        // Augment the (single) plot specification to match presentation settings used in the original paper.
        BOOST_FOREACH(PlotSpecificationPtr p_plot_spec, runner.GetProtocol()->rGetPlotSpecifications())
        {
            p_plot_spec->SetDisplayTitle(GetTitleFromDirectory(rCellMLFileBaseName));
            p_plot_spec->SetGnuplotTerminal("postscript eps enhanced size 12.75cm, 8cm font 18");
            std::vector<std::string> extra_setup;
            extra_setup.push_back("set xrange [-60:80]");
            p_plot_spec->SetGnuplotExtraCommands(extra_setup);
            p_plot_spec->SetStyle("linespoints pointtype 7");
        }

        runner.RunProtocol();
    }

    /* This method creates the S1-S2 restitution curves for the reference experimental data using Gnuplot.
     * Originally the protocol versions also used this method, but plot generation is now built in to the main code.
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
        *p_gnuplot_script  << "set terminal postscript eps size 12.75cm, 8cm font 18" << std::endl; // This is different
        *p_gnuplot_script  << "set output \"" << output_dir << "S1S2.eps\"" << std::endl;
        *p_gnuplot_script  << "set title \"" << GetTitleFromDirectory(rDirectory) << "\"" << std::endl;
        *p_gnuplot_script  << "set xlabel \"Diastolic Interval (ms)\"" << std::endl;
        *p_gnuplot_script  << "set xtics 400" << std::endl; // This line is additional to the default script
        *p_gnuplot_script  << "set ylabel \"APD90 (ms)\"" << std::endl;
        *p_gnuplot_script  << "set grid" << std::endl;
        *p_gnuplot_script  << "set autoscale" << std::endl;
        *p_gnuplot_script  << "set key off" << std::endl;
        *p_gnuplot_script << "set datafile separator \",\"" << std::endl;
        *p_gnuplot_script  << "plot \"" + data_file_name + "\" using 1:2 with linespoints pointtype 7"; // This is now different to default
        *p_gnuplot_script << std::endl << std::flush;

        p_gnuplot_script->close();

        // Run Gnuplot on the script written above to generate image files.
        EXPECT0(system, "gnuplot " + output_dir + filename);
    }

    /*
     * This method creates the Gnuplots of the ICaL IV curves for the reference experimental data.
     * Originally the protocol versions also used this method, but plot generation is now built in to the main code.
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
        *p_gnuplot_script  << "set terminal postscript eps enhanced size 12.75cm, 8cm font 18" << std::endl; // This is different
        *p_gnuplot_script  << "set output \"" << output_dir << "ICaL_IV.eps\"" << std::endl;
        *p_gnuplot_script  << "set title \"" << GetTitleFromDirectory(rDirectory) << "\"" << std::endl;
        *p_gnuplot_script  << "set xlabel \"Test Voltage (mV)\"" << std::endl;
        *p_gnuplot_script  << "set ylabel \"Maximum Current ({/Symbol m}A/cm^{2})\"" << std::endl;
        *p_gnuplot_script  << "set grid" << std::endl;
        *p_gnuplot_script  << "set autoscale" << std::endl;
        *p_gnuplot_script  << "set xrange [-60:80]" << std::endl; // This is additional to the default script
        *p_gnuplot_script  << "set key off" << std::endl;
        *p_gnuplot_script  << "set datafile separator \",\"" << std::endl;

        *p_gnuplot_script  << "plot ";
        unsigned num_cols = 3u;
        for (unsigned i=1; i<=num_cols; ++i)
        {
            *p_gnuplot_script << "\"" + data_file_name + "\" using 1:" << i+1
                              << " with linespoints pointtype 7";
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
    /* The main test method which runs both protocols on all available CellML files.
     */
    void TestProtocolsForManyCellModels() throw(Exception, std::bad_alloc)
    {
        std::vector<std::string> cellml_files = GetAListOfCellMLFiles();
        if (CommandLineArguments::Instance()->OptionExists("--models"))
        {
            cellml_files = CommandLineArguments::Instance()->GetStringsCorrespondingToOption("--models");
        }

        std::vector<std::string> ical_outputs = boost::assign::list_of("outputs_min_LCC")("outputs_final_membrane_voltage");
        std::vector<std::string> s1s2_outputs = boost::assign::list_of("outputs_APD90")("outputs_DI");

        /* We use Chaste's process isolation facility to process models in parallel, if running on multiple processes.
         * The main output folder needs to be created with a collective call (so we don't have multiple processes
         * trying to make the same folder) but thereafter each protocol run can be done completely independently.
         */
        {
            OutputFileHandler("FunctionalCuration", false);
            PetscTools::IsolateProcesses(true);
        }

        // Models corresponding to particular species
        std::set<std::string> dog_models = boost::assign::list_of("decker_2009")
                                                                 ("hund_rudy_2004")
                                                                 ("livshitz_rudy_2007")
                                                                 ("fox_mcharg_gilmour_2002")
                                                                 ("winslow_model_1999");
        std::set<std::string> human_models = boost::assign::list_of("fink_noble_giles_model_2008")
                                                                   ("grandi_pasqualini_bers_2010_ss")
                                                                   ("iyer_2004_s1s2_curve")
                                                                   ("iyer_model_2007_s1s2_curve")
                                                                   ("priebe_beuckelmann_1998_s1s2_curve")
                                                                   ("ten_tusscher_model_2004_epi_s1s2_curve")
                                                                   ("ten_tusscher_model_2006_epi_s1s2_curve");

        // Track which model/protocol combinations fail
        std::vector<std::string> failed_combinations;

        for (unsigned i=0; i<cellml_files.size(); ++i)
        {
            if (PetscTools::IsParallel() && i % PetscTools::GetNumProcs() != PetscTools::GetMyRank())
            {
                // Let someone else do this model
                continue;
            }
            std::cout << "\nRunning protocols for " << cellml_files[i] << std::endl << std::flush;

            // Figure out which S1 interval to use, to compare against different experimental data sets.
            double s1_freq = 1000; // ms
            if (dog_models.find(cellml_files[i]) != dog_models.end())
            {
                s1_freq = 2000;
            }
            if (human_models.find(cellml_files[i]) != human_models.end())
            {
                s1_freq = 600;
            }

            // Run each protocol and compare against historical data.
            try
            {
                RunS1S2Protocol(cellml_files[i], s1_freq);
            }
            catch (Exception& e)
            {
                OUR_WARN(e.GetMessage(), cellml_files[i], "S1S2");
            }
            if (!CompareToHistoricalResults(cellml_files[i], "S1S2", s1s2_outputs, 0.005, 1e-6)) // 0.5% rel tol
            {
                failed_combinations.push_back(cellml_files[i] + " / S1S2");
            }

            try {
                RunICaLVoltageClampProtocol(cellml_files[i]);
            }
            catch (Exception &e)
            {
                OUR_WARN(e.GetMessage(), cellml_files[i], "ICaL");
            }
            if (!CompareToHistoricalResults(cellml_files[i], "ICaL", ical_outputs, 0.005, 1e-5)) // 0.5% rel tol
            {
                failed_combinations.push_back(cellml_files[i] + " / ICaL");
            }
        }

        /* Next, the master process makes comparison plots of some experimental data we got
         * by digitising paper graphs.
         */
        PetscTools::IsolateProcesses(false);

        std::vector<std::string> exp_data;
        exp_data.push_back("sicouri_dog_ventricle");
        exp_data.push_back("kim_human_atrial");
        exp_data.push_back("morgan_human_ventricle");

        // Also specify the calcium experimental data.
        exp_data.push_back("sun_rat_ventricle");
        // Note that this uses calcium concentrations of 0.1, 0.3, and 1.0 uM.

        for (unsigned i=0; i<exp_data.size(); ++i)
        {
            // Copy the data from here
            FileFinder finder("projects/FunctionalCuration/test/data/" + exp_data[i], RelativeTo::ChasteSourceRoot);
            TS_ASSERT_EQUALS(finder.IsDir(), true);
            // Creating and cleaning the output folder must be done as a collective call
            mpHandler.reset(new OutputFileHandler("FunctionalCuration/" + exp_data[i], true));
            if (PetscTools::AmMaster())
            {
                FileFinder dest = mpHandler->FindFile("");
                BOOST_FOREACH(const FileFinder& r_data_file, finder.FindMatches("*"))
                {
                    r_data_file.CopyTo(dest);
                }

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

        /* Finally, we compute and print a summary of which model/protocol combinations failed across
         * the whole run, since it can be tricky to determine this by hand when running on many processes.
         */
        Warnings::NoisyDestroy();
        unsigned num_local_failures = failed_combinations.size();
        unsigned total_num_failures = 0u;
        MPI_Allreduce(&num_local_failures, &total_num_failures, 1, MPI_UNSIGNED, MPI_SUM, PetscTools::GetWorld());
        if (PetscTools::AmMaster())
        {
            std::cout << std::endl << "Ran " << (2 * cellml_files.size()) << " model / protocol combinations." << std::endl;
            if (total_num_failures > 0u)
            {
                std::cout << "Failed " << total_num_failures << " unexpectedly:" << std::endl;
            }
            else
            {
                std::cout << "All combinations with historical results matched to within tolerances." << std::endl;
            }
        }
        if (total_num_failures > 0u)
        {
            PetscTools::BeginRoundRobin();
            BOOST_FOREACH(const std::string& r_combo, failed_combinations)
            {
                std::cout << "    " << r_combo << std::endl;
            }
            PetscTools::EndRoundRobin();
        }
        PetscTools::Barrier(); // Prevent printing cxxtest pass/fail lines until above completed for all processes
    }
};

#endif // TESTFUNCTIONALCURATIONPAPER_HPP_
