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

#include "Protocol.hpp"

#include <cassert>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <set>
#include <boost/pointer_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>

#include "Exception.hpp"

#include "ProtoHelperMacros.hpp"
#include "VectorStreaming.hpp"
#include "BacktraceException.hpp"
#include "DebugProto.hpp"

// Typedefs for use with BOOST_FOREACH and std::maps
typedef std::pair<std::string, std::string> StringPair;
typedef std::pair<std::string, EnvironmentPtr> StringEnvPair;
typedef std::pair<std::string, ProtocolPtr> StringProtoPair;


Protocol::Protocol()
    : mpInputs(new Environment(true)),
      mpLibrary(new Environment),
      mpModelStateCollection(new ModelStateCollection),
      mWritePng(false)
{
    mpLibrary->SetDelegateeEnvironment(mpInputs->GetAsDelegatee());
}


/**
 * Ensure the given environment can access simulation results.
 *
 * @param rEnv  the environment that needs access
 * @param rResultsEnvs  mapping from prefix to results environment
 */
void AddSimulationResultsDelegatees(Environment& rEnv, std::map<std::string, EnvironmentPtr>& rResultsEnvs)
{
    BOOST_FOREACH(StringEnvPair binding, rResultsEnvs)
    {
        if (!binding.first.empty())
        {
            rEnv.SetDelegateeEnvironment(binding.second, binding.first);
        }
    }
}


/**
 * Reset the provided outputs collection, in order to allow a protocol to be (re-)run.
 * @param rOutputs  the protocol outputs
 */
void ResetOutputs(std::map<std::string, EnvironmentPtr>& rOutputs)
{
    rOutputs.clear();
    // Ensure the final outputs environment exists
    EnvironmentPtr p_proto_outputs(new Environment);
    rOutputs[""] = p_proto_outputs;
}


void Protocol::FinaliseSetup()
{
    // Set default input definitions
    mpInputs->ExecuteStatements(mInputStatements);
    // Add delegatees for any imported libraries
    Environment& r_library = rGetLibrary();
    BOOST_FOREACH(StringProtoPair import, mImports)
    {
        r_library.SetDelegateeEnvironment(import.second->rGetLibrary().GetAsDelegatee(), import.first);
    }
    // Set delegatees for simulations
    BOOST_FOREACH(boost::shared_ptr<AbstractSimulation> p_sim, mSimulations)
    {
        Environment& r_sim_env = p_sim->rGetEnvironment();
        // Don't change the delegatee for an imported simulation
        if (!r_sim_env.GetDelegateeEnvironment())
        {
            r_sim_env.SetDelegateeEnvironment(r_library.GetAsDelegatee());
        }
    }
}


void Protocol::InitialiseLibrary(bool reinit)
{
    if (reinit)
    {
        mpLibrary->Clear();
    }
    // First initialise the libraries of imported protocols
    BOOST_FOREACH(StringProtoPair import, mImports)
    {
        import.second->InitialiseLibrary(reinit);
    }
    // Now do our library
    const unsigned library_size = mpLibrary->GetNumberOfDefinitions();
    assert(library_size == 0 || library_size == mLibraryStatements.size());
    if (library_size == 0 || reinit)
    {
        mpLibrary->ExecuteStatements(mLibraryStatements);
    }
}


void Protocol::Run()
{
    std::cout << "Running protocol..." << std::endl;
    if (mpOutputHandler)
    {
        DebugProto::SetTraceFolder(*mpOutputHandler);
    }
    ResetOutputs(mOutputs);
    // If we get an error at any stage, we want to ensure as many partial results as possible
    // are stored, but still report the error(s)
    std::vector<Exception> errors;
    // Run the simulation(s)
    try
    {
        unsigned simulation_number = 0u;
        BOOST_FOREACH(boost::shared_ptr<AbstractSimulation> p_sim, mSimulations)
        {
            const std::string prefix = p_sim->GetOutputsPrefix();
            std::cout << "Running simulation " << simulation_number << " " << prefix << "..." << std::endl;
            AddSimulationResultsDelegatees(p_sim->rGetEnvironment(), mOutputs);
            p_sim->InitialiseSteppers();
            EnvironmentPtr p_results = p_sim->Run();
            if (!prefix.empty())
            {
                // Store the outputs
                assert(p_results);
                PROTO_ASSERT2(!mOutputs[prefix],
                              "The simulation prefix " << prefix << " has been used for multiple simulations.",
                              p_sim->GetLocationInfo());
                mOutputs[prefix] = p_results;
            }
            if (mpOutputHandler)
            {
                // Re-set the trace folder in case a nested protocol changed it
                DebugProto::SetTraceFolder(*mpOutputHandler);
                // Remove the simulation output folder if empty
                FileFinder sim_debug_output = p_sim->GetOutputFolder();
                if (sim_debug_output.IsPathSet() && sim_debug_output.IsDir() && sim_debug_output.IsEmpty())
                {
                    sim_debug_output.Remove();
                }
            }
            simulation_number++;
        }
    }
    catch (const Exception& e)
    {
        errors.push_back(e);
    }
    // Ensure post-processing can access simulation results
    EnvironmentPtr p_post_proc_env(new Environment(mpLibrary->GetAsDelegatee()));
    AddSimulationResultsDelegatees(*p_post_proc_env, mOutputs);
    // Post-process the results
    if (errors.empty())
    {
        std::cout << "Running post-processing..." << std::endl;
        try
        {
            p_post_proc_env->ExecuteStatements(mPostProcessing);
        }
        catch (const Exception& e)
        {
            errors.push_back(e);
        }
    }
    // Transfer requested outputs to mOutputs
    EnvironmentPtr p_proto_outputs = mOutputs[""];
    BOOST_FOREACH(boost::shared_ptr<OutputSpecification> p_spec, mOutputSpecifications)
    {
        const std::string& r_loc = p_spec->GetLocationInfo();
        const std::string& r_ref = p_spec->rGetOutputRef();
        const std::string& r_name = p_spec->rGetOutputName();
        try
        {
            AbstractValuePtr p_output = p_post_proc_env->Lookup(r_ref, r_loc);
            if (p_spec->rGetOutputType() == "Post-processed")
            {
                const std::string& r_units = p_spec->rGetOutputUnits();
                if (!r_units.empty())
                {
                    p_output->SetUnits(r_units);
                }
            }
            p_proto_outputs->DefineName(r_name, p_output, r_loc);
        }
        catch (const Exception& e)
        {
            errors.push_back(e);
        }
    }
    if (mpOutputHandler)
    {
        DebugProto::StopTracing();
    }
    if (!errors.empty())
    {
        ///\todo Make an Exception subclass which reports a set of errors?
        throw errors.front();
    }
    std::cout << "Finished running protocol." << std::endl;
}


void Protocol::SetOutputFolder(const OutputFileHandler& rHandler)
{
    mpOutputHandler.reset(new OutputFileHandler(rHandler));
    BOOST_FOREACH(boost::shared_ptr<AbstractSimulation> p_sim, mSimulations)
    {
        const std::string prefix = p_sim->GetOutputsPrefix();
        if (!prefix.empty())
        {
            FileFinder subfolder = rHandler.FindFile("simulation_" + prefix);
            boost::shared_ptr<OutputFileHandler> p_sim_handler(new OutputFileHandler(subfolder));
            p_sim->SetOutputFolder(p_sim_handler);
        }
    }
}


void Protocol::WriteToFile(const OutputFileHandler& rHandler,
                           const std::string& rFileNameBase)
{
    SetOutputFolder(rHandler);
    WriteToFile(rFileNameBase);
}


std::string SanitiseFileName(const std::string& rFileName)
{
    std::string name = rFileName;
    FileFinder::ReplaceSpacesWithUnderscores(name);
    // Strip quote characters
    for (std::string::iterator it = name.begin(); it != name.end(); ++it)
    {
        while (*it == '\'' || *it == '"')
        {
            it = name.erase(it);
        }
    }
    return name;
}

std::string PlotFileName(boost::shared_ptr<PlotSpecification> pPlotSpec,
                         bool png=false)
{
    std::string ext;
    if (!png)
    {
        ext = ".eps";
    }
    else
    {
        ext = ".png";
    }
    std::string file_name = pPlotSpec->rGetTitle() + ext;
    return SanitiseFileName(file_name);
}


void Protocol::WriteToFile(const std::string& rFileNameBase) const
{
    if (!mpOutputHandler)
    {
        EXCEPTION("SetOutputFolder must be called prior to using WriteToFile.");
    }
    ///\todo Improve format?
    const Environment& r_outputs = rGetOutputsCollection();
    std::set<std::string> missing_outputs;
    // Variable metadata file
    {
        std::string index_file_name = rFileNameBase + "-contents.csv";
        out_stream p_file = mpOutputHandler->OpenOutputFile(index_file_name);
        (*p_file) << "Variable id,Variable name,Units,Number of dimensions,File name,Type,Dimensions" << std::endl;
        BOOST_FOREACH(boost::shared_ptr<OutputSpecification> p_spec, mOutputSpecifications)
        {
            const std::string& r_name = p_spec->rGetOutputName();
            AbstractValuePtr p_output;
            try
            {
                p_output = r_outputs.Lookup(r_name);
            }
            catch (const Exception& e)
            {
                std::cerr << "Missing protocol output '" << r_name << "'; ignoring for now." << std::endl;
                missing_outputs.insert(r_name);
                continue;
            }
            if (p_output->IsArray())
            {
                NdArray<double> output = GET_ARRAY(p_output);
                (*p_file) << '"' << r_name << "\",\"" << p_spec->rGetOutputDescription()
                          << "\",\"" << p_output->GetUnits()
                          << "\"," << output.GetNumDimensions()
                          << "," << SanitiseFileName(rFileNameBase + "_" + r_name + ".csv")
                          << "," << p_spec->rGetOutputType();
                BOOST_FOREACH(NdArray<double>::Index len, output.GetShape())
                {
                    (*p_file) << "," << len;
                }
                (*p_file) << std::endl;
            }
            else
            {
                std::cerr << "Unexpected non-array protocol output '" << r_name << "'; ignoring." << std::endl;
            }
        }
    }

    // Stepper values file
    {
        std::string index_file_name = rFileNameBase + "-steppers.csv";
        out_stream p_file = mpOutputHandler->OpenOutputFile(index_file_name);
        (*p_file) << "Stepper name,Units,Values" << std::endl;
        boost::shared_ptr<AbstractSimulation> p_sim = mSimulations.back();
        std::vector<boost::shared_ptr<AbstractStepper> >& r_steppers = p_sim->rGetSteppers();
        BOOST_FOREACH(boost::shared_ptr<AbstractStepper> p_stepper, r_steppers)
        {
            if (p_stepper) // See #1911
            {
                try
                {
                    (*p_file) << '"' << p_stepper->GetIndexName() << "\",\"" << p_stepper->GetUnits() << '"';
                    for (p_stepper->Reset(); !p_stepper->AtEnd(); p_stepper->Step())
                    {
                        (*p_file) << "," << p_stepper->GetCurrentOutputPoint();
                    }
                    (*p_file) << std::endl;
                }
                catch (const Exception& e)
                {
                    std::cerr << "Error writing stepper " << p_stepper->GetIndexName() << ":" << e.GetMessage();
                }
            }
        }
    }

    // Default plots file
    if (!mPlotSpecifications.empty())
    {
        std::string file_name = rFileNameBase + "-default-plots.csv";
        out_stream p_file = mpOutputHandler->OpenOutputFile(file_name);
        (*p_file) << "Plot title,File name,First variable,Optional second variable" << std::endl;
        BOOST_FOREACH(boost::shared_ptr<PlotSpecification> p_spec, mPlotSpecifications)
        {
            bool all_variables_exist = true;
            std::vector<std::string> units;
            std::vector<std::string> descriptions;
            // Check all the variables are also outputs, or we can't plot them
            BOOST_FOREACH(const std::string& r_name, p_spec->rGetVariableNames())
            {
                try
                {
                    AbstractValuePtr p_output = r_outputs.Lookup(r_name);
                    units.push_back(p_output->GetUnits());
                    // Look up the output description for this plot
                    BOOST_FOREACH(boost::shared_ptr<OutputSpecification> p_output_spec, mOutputSpecifications)
                    {
                        if (r_name == p_output_spec->rGetOutputName())
                        {
                            descriptions.push_back(p_output_spec->rGetOutputDescription());
                            break;
                        }
                    }
                }
                catch (const Exception& e)
                {
                    std::cerr << "Plot requests protocol output '" << r_name << "', which has not been specified as an output." << std::endl;
                    missing_outputs.insert(r_name);
                    all_variables_exist = false;
                }
            }
            if (all_variables_exist)
            {
                p_spec->SetVariableUnits(units); // Store for later plotting use
                p_spec->SetVariableDescriptions(descriptions); // Store for later plotting use
                const std::string& r_title = p_spec->rGetTitle();
                (*p_file) << '"' << r_title << "\"," << PlotFileName(p_spec);
                BOOST_FOREACH(const std::string& r_name, p_spec->rGetVariableNames())
                {
                    (*p_file) << ",\"" << r_name << "\"";
                }
                (*p_file) << std::endl;
            }
        }
    }

    // Output data, one file per variable
    BOOST_FOREACH(boost::shared_ptr<OutputSpecification> p_spec, mOutputSpecifications)
    {
        const std::string& r_name = p_spec->rGetOutputName();
        AbstractValuePtr p_output;
        try
        {
            p_output = r_outputs.Lookup(r_name);
        }
        catch (const Exception& e)
        {
            continue;
        }

        std::string file_name = SanitiseFileName(rFileNameBase + "_" + r_name + ".csv");
        out_stream p_file = mpOutputHandler->OpenOutputFile(file_name);
        (*p_file) << "# " << p_spec->rGetOutputDescription() << std::endl;

        if (p_output->IsArray())
        {
            NdArray<double> output = GET_ARRAY(p_output);
            const unsigned num_dims = output.GetNumDimensions();
            NdArray<double>::Extents shape = output.GetShape();
            if (num_dims == 2)
            {
                // Tabular format for easy processing by other software
                NdArray<double>::Indices idxs = output.GetIndices();
                for (idxs[1]=0; idxs[1]<shape[1]; ++idxs[1])
                {
                    for (idxs[0]=0; idxs[0]<shape[0]; ++idxs[0])
                    {
                        (*p_file) << (idxs[0] == 0 ? "" : ",") << output[idxs];
                    }
                    (*p_file) << std::endl;
                }
            }
            else
            {
                // General format - one entry per line, with header giving the shape
                (*p_file) << num_dims;
                for (unsigned i=0; i<num_dims; ++i)
                {
                    (*p_file) << "," << shape[i];
                }
                (*p_file) << std::endl;
                for (NdArray<double>::Iterator it = output.Begin(); it != output.End(); ++it)
                {
                    (*p_file) << *it << std::endl;
                }
            }
        }
        p_file->close();
    }

    GeneratePlots(rFileNameBase);

    if (!missing_outputs.empty())
    {
        std::vector<std::string> missing_names(missing_outputs.begin(), missing_outputs.end());
        EXCEPTION("Not all protocol outputs were defined.  Missing names: " << missing_names);
    }
}


void Protocol::SetPngOutput(bool writePng)
{
    mWritePng = writePng;
}


void Protocol::GeneratePlots(const std::string& rFileNameBase) const
{
    const Environment& r_outputs = rGetOutputsCollection();

    // \todo #1999 LOADS OF TESTS!!
    BOOST_FOREACH(boost::shared_ptr<PlotSpecification> p_plot_spec, mPlotSpecifications)
    {
        const std::string& r_title = p_plot_spec->rGetTitle();
        const std::vector<std::string>& r_names = p_plot_spec->rGetVariableNames();

        // Get the arrays to plot
        std::vector<AbstractValuePtr> arrays;
        bool missing_array = false;
        BOOST_FOREACH(const std::string& r_name, r_names)
        {
            try
            {
                AbstractValuePtr p_output = r_outputs.Lookup(r_name);
                if (p_output->IsArray())
                {
                    arrays.push_back(p_output);
                }
                else
                {
                    std::cerr << "Cannot plot non-array output '" << r_name << "'." << std::endl;
                    missing_array = true;
                }
            }
            catch (const Exception& e)
            {
                missing_array = true;
                // Error for missing output was already printed by caller
            }
        }
        // Don't plot if any required output is missing
        if (missing_array)
        {
            continue;
        }

        // Figure out axis labels, and the X axis data array
        std::string label_x, label_y;
        NdArray<double> output_x;
        if (r_names.size() == 1u)
        {
            // Find the output specification for this variable
            BOOST_FOREACH(boost::shared_ptr<OutputSpecification> p_output_spec, mOutputSpecifications)
            {
                if (r_names.front() == p_output_spec->rGetOutputName())
                {
                    PROTO_ASSERT2(p_output_spec->rGetOutputType() == "Raw",
                                  "Only 'Raw' type outputs may be plotted using the 'data' form.",
                                  p_plot_spec->GetLocationInfo());
                    // Find the simulation it comes from
                    const std::string& r_ref = p_output_spec->rGetOutputRef();
                    size_t colon = r_ref.find(':');
                    PROTO_ASSERT2(colon != std::string::npos,
                                  "A raw output reference must be a prefixed name, not '" << r_ref << "'.",
                                  p_output_spec->GetLocationInfo());
                    std::string prefix = r_ref.substr(0, colon);
                    BOOST_FOREACH(boost::shared_ptr<AbstractSimulation> p_sim, mSimulations)
                    {
                        if (prefix == p_sim->GetOutputsPrefix())
                        {
                            std::vector<boost::shared_ptr<AbstractStepper> >& r_steppers = p_sim->rGetSteppers();
                            if (!r_steppers.empty() && r_steppers.back())
                            {
                                // Create a new array containing the stepper data
                                boost::shared_ptr<AbstractStepper> p_stepper = r_steppers.back();
                                if (p_stepper->GetNumberOfOutputPoints() == 0u)
                                {
                                    ///\todo Avoid nested protocols having a fake stepper!
                                    assert(r_steppers.size() > 1u);
                                    p_stepper = r_steppers[r_steppers.size()-2];
                                }
                                PROTO_ASSERT2(p_stepper->IsEndFixed(),
                                              "Unable to plot against inner while loop at present.",
                                              p_plot_spec->GetLocationInfo());
                                label_x = p_stepper->GetIndexName() + " (" + p_stepper->GetUnits() + ")";
                                NdArray<double>::Extents x_shape(1u);
                                x_shape[0] = p_stepper->GetNumberOfOutputPoints();
                                NdArray<double> x(x_shape);
                                output_x = x;
                                NdArray<double>::Iterator it_x = x.Begin();
                                for (p_stepper->Reset(); !p_stepper->AtEnd(); p_stepper->Step())
                                {
                                    *it_x++ = p_stepper->GetCurrentOutputPoint();
                                }
                            }
                            break;
                        }
                    }
                    break;
                }
            }
            PROTO_ASSERT2(!label_x.empty(), "Unable to find X axis label.", p_plot_spec->GetLocationInfo());
            PROTO_ASSERT2(output_x.GetNumElements() > 0u, "Unable to find X axis data.",
                          p_plot_spec->GetLocationInfo());
            label_y = p_plot_spec->rGetVariableDescriptions()[0] + " (" + p_plot_spec->rGetVariableUnits()[0] + ")";
        }
        else
        {
            label_x = p_plot_spec->rGetVariableDescriptions()[0] + " (" + p_plot_spec->rGetVariableUnits()[0] + ")";
            if (r_names.size() == 2u)
            {
                label_y = p_plot_spec->rGetVariableDescriptions()[1] + " (" + p_plot_spec->rGetVariableUnits()[1] + ")";
            }
            else
            {
                label_y = "Various";
            }
            output_x = GET_ARRAY(arrays[0]);
            arrays.erase(arrays.begin());
        }

        // Tracing what we're doing
        std::cout << "Plotting " << r_title << ":\t" << label_y << " against " << label_x << "..." << std::endl;

        // Check the shapes of the arrays to plot
        // X must be 1d
        if (output_x.GetNumDimensions() != 1)
        {
            std::cerr << "The X data for a plot must be a 1d array, not " << output_x.GetNumDimensions() << "d." << std::endl;
            continue;
        }
        const unsigned x_length = output_x.GetShape()[0];
        // Y arrays can be n-d, but the last dimension must match the length of X
        std::vector<NdArray<double> > y_arrays;
        std::vector<unsigned> num_traces;
        unsigned total_num_traces = 0u;
        BOOST_FOREACH(AbstractValuePtr p_array_val, arrays)
        {
            NdArray<double> output_y = GET_ARRAY(p_array_val);
            y_arrays.push_back(output_y);
            if (output_y.GetNumDimensions() < 1)
            {
                std::cerr << "The Y data for a plot must be a true array, not a single value." << std::endl;
                continue;
            }
            if (output_y.GetShape().back() != x_length)
            {
                std::cerr << "The last dimension of the Y data for a plot must be the same size as the X data. "
                          << "Y shape " << output_y.GetShape() << " is not compatible with X length " << x_length << std::endl;
                continue;
            }
            // Each Y array may represent multiple traces if it has more than one dimension
            unsigned my_num_traces = 1u;
            for (unsigned i=0; i<output_y.GetNumDimensions()-1; ++i)
            {
                my_num_traces *= output_y.GetShape()[i];
            }
            total_num_traces += my_num_traces;
            num_traces.push_back(my_num_traces);
        }

        // Write data for plotting
        std::string file_name = SanitiseFileName(rFileNameBase + "_" + r_title + "_gnuplot_data.csv");
        out_stream p_file = mpOutputHandler->OpenOutputFile(file_name);
        // Tabular format with no header line for easy processing by gnuplot
        for (NdArray<double>::ConstIterator it_x=output_x.Begin();
             it_x != output_x.End();
             ++it_x)
        {
            (*p_file) << *it_x;
            // Now we iterate over Y data
            for (unsigned i=0; i<y_arrays.size(); ++i)
            {
                NdArray<double>& output_y = y_arrays[i];
                // Find the first Y entry for this X position
                NdArray<double>::ConstIterator it_y = output_y.Begin();
                it_y += it_x.rGetIndices().back();
                for (unsigned trace=0; trace<num_traces[i]; ++trace)
                {
                    (*p_file) << "," << *it_y;
                    // Increment by the size of the last dimension to point at the next trace for this X
                    if (trace < num_traces[i]-1)
                    {
                        it_y += output_y.GetShape().back();
                    }
                }
            }
            (*p_file) << std::endl;
        }
        p_file->close();

        // Plot!
        PlotWithGnuplot(p_plot_spec, file_name, total_num_traces, x_length, label_x, label_y, false);
        if (mWritePng)
        {
            PlotWithGnuplot(p_plot_spec, file_name, total_num_traces, x_length, label_x, label_y, true);
        }
    }
}


void Protocol::PlotWithGnuplot(boost::shared_ptr<PlotSpecification> pPlotSpec,
                               const std::string& rDataFileName,
                               const unsigned numTraces,
                               const unsigned numPointsInTrace,
                               const std::string& xLabel,
                               const std::string& yLabel,
                               bool writePng) const
{
    // At present this is hardcoded to 2 columns of data x,y points.
    /// \todo #1999 generalise to other situations
    /// \todo #1999 make the plot title include the model as well as protocol name

    // Find the csv file name, and remove .csv to make the .gp file name
    size_t dot = rDataFileName.rfind('.');
    std::string script_name = rDataFileName.substr(0, dot) + ".gp";
    std::string fig_file_name = PlotFileName(pPlotSpec, writePng);

    // Decide whether to plot with points or lines
    std::string points_or_lines;
    if (numPointsInTrace > 100)
    {   // Magic number choice here - if there are over 100 data points we visualize as a continuous line.
        points_or_lines = "lines";
    }
    else
    {   // otherwise we visualize as points joined by straight lines.
        points_or_lines = "linespoints pointtype 7";
    }

    // Write out a Gnuplot script
    out_stream p_gnuplot_script = mpOutputHandler->OpenOutputFile(script_name);
    std::string output_dir = mpOutputHandler->GetOutputDirectoryFullPath();
    *p_gnuplot_script << "# Gnuplot script file generated by Chaste Functional Curation system." << std::endl;
    *p_gnuplot_script << "# Plot of " << pPlotSpec->rGetTitle() << "." << std::endl;
    *p_gnuplot_script << "set terminal ";
    if (!writePng)
    {
        // eps page size in inches, default font
        *p_gnuplot_script << "postscript eps enhanced size 3,2 font 16";
    }
    else
    {
        // png size in pixels, font requires something like:
        // export GDFONTPATH=/usr/share/fonts/truetype/ttf-liberation/
        *p_gnuplot_script << "png enhanced size 900,600 font \"LiberationSans-Regular\" 16";
    }
    *p_gnuplot_script << std::endl;
    *p_gnuplot_script << "set output \"" << output_dir << fig_file_name << "\"" << std::endl;
    *p_gnuplot_script << "set title \"" << pPlotSpec->rGetTitle() << "\"" << std::endl;
    *p_gnuplot_script << "set xlabel \"" << xLabel << "\"" << std::endl;
    *p_gnuplot_script << "set ylabel \"" << yLabel << "\"" << std::endl;
    //*p_gnuplot_script << "set xtics 400" << std::endl;
    *p_gnuplot_script << "set grid" << std::endl;
    *p_gnuplot_script << "set autoscale" << std::endl;
    *p_gnuplot_script << "set key off" << std::endl;
    *p_gnuplot_script << "set datafile separator \",\"" << std::endl;

    // The actual plot command...
    *p_gnuplot_script  << "plot ";
    for (unsigned i=0; i<numTraces; ++i)
    {
        *p_gnuplot_script << "\"" << output_dir << rDataFileName << "\" using 1:" << i+2
                          << " with " << points_or_lines;
        if (i < numTraces-1)
        {
            *p_gnuplot_script << ",\\"; // Escape the newline that is written below
        }
        *p_gnuplot_script << std::endl;
    }
    *p_gnuplot_script << std::endl << std::flush;

    p_gnuplot_script->close();

    // Run Gnuplot on the script written above to generate image files.
    EXPECT0(system, "gnuplot " + output_dir + script_name);
}


unsigned Protocol::GetNumberOfOutputs(unsigned simulation) const
{
    if (simulation == static_cast<unsigned>(-1))
    {
        simulation = mSimulations.size() - 1;
    }
    EXCEPT_IF_NOT(simulation < mSimulations.size());
    EXCEPT_IF_NOT(!mSimulations[simulation]->GetOutputsPrefix().empty());
    return GetNumberOfOutputs(mSimulations[simulation]->GetOutputsPrefix());
}


const Environment& Protocol::rGetOutputsCollection(unsigned simulation) const
{
    if (simulation == static_cast<unsigned>(-1))
    {
        simulation = mSimulations.size() - 1;
    }
    EXCEPT_IF_NOT(simulation < mSimulations.size());
    EXCEPT_IF_NOT(!mSimulations[simulation]->GetOutputsPrefix().empty());
    return rGetOutputsCollection(mSimulations[simulation]->GetOutputsPrefix());
}


unsigned Protocol::GetNumberOfOutputs(const std::string& rPrefix) const
{
    std::map<std::string, EnvironmentPtr>::const_iterator it = mOutputs.find(rPrefix);
    EXCEPT_IF_NOT(it != mOutputs.end());
    return it->second->GetNumberOfDefinitions();
}


const Environment& Protocol::rGetOutputsCollection(const std::string& rPrefix) const
{
    std::map<std::string, EnvironmentPtr>::const_iterator it = mOutputs.find(rPrefix);
    EXCEPT_IF_NOT(it != mOutputs.end());
    return *(it->second);
}


boost::shared_ptr<ModelStateCollection> Protocol::GetStateCollection()
{
    return mpModelStateCollection;
}


void Protocol::SetInput(const std::string& rName, AbstractExpressionPtr pValue)
{
    mpInputs->RemoveDefinition(rName, "Setting protocol input");
    AbstractValuePtr p_value = (*pValue)(*mpInputs);
    mpInputs->DefineName(rName, p_value, "Setting protocol input");
}


const FileFinder& Protocol::rGetSourceFile() const
{
    return mSourceFilePath;
}


ProtocolPtr Protocol::GetImportedProtocol(const std::string& rPrefix)
{
    return mImports[rPrefix];
}


const std::map<std::string, std::string>& Protocol::rGetNamespaceBindings() const
{
    return mOntologyNamespaceBindings;
}


Environment& Protocol::rGetInputsEnvironment()
{
    return *mpInputs;
}


std::vector<AbstractStatementPtr>& Protocol::rGetInputStatements()
{
    return mInputStatements;
}


Environment& Protocol::rGetLibrary()
{
    return *mpLibrary;
}


std::vector<AbstractStatementPtr>& Protocol::rGetLibraryStatements()
{
    return mLibraryStatements;
}


std::vector<boost::shared_ptr<AbstractSimulation> >& Protocol::rGetSimulations()
{
    return mSimulations;
}


std::vector<AbstractStatementPtr>& Protocol::rGetPostProcessing()
{
    return mPostProcessing;
}


std::vector<boost::shared_ptr<OutputSpecification> >& Protocol::rGetOutputSpecifications()
{
    return mOutputSpecifications;
}


std::vector<boost::shared_ptr<PlotSpecification> >& Protocol::rGetPlotSpecifications()
{
    return mPlotSpecifications;
}


void Protocol::SetSourceFile(const FileFinder& rSourceFile)
{
    mSourceFilePath = rSourceFile;
}


void Protocol::AddImport(const std::string& rPrefix, ProtocolPtr pImport, const std::string& rLoc)
{
    std::pair<std::map<std::string, ProtocolPtr>::iterator, bool> result = mImports.insert(std::make_pair(rPrefix, pImport));
    PROTO_ASSERT2(result.second,
                  "The prefix " << rPrefix << " has already been used for an imported protocol.",
                  rLoc);
    // Make the import's library, if any, available to our library and hence its users
    mpLibrary->SetDelegateeEnvironment(pImport->rGetLibrary().GetAsDelegatee(), rPrefix);
}


void Protocol::AddNamespaceBindings(const std::map<std::string, std::string>& rBindings)
{
    BOOST_FOREACH(StringPair new_binding, rBindings)
    {
        std::pair<std::map<std::string, std::string>::iterator,bool> result = mOntologyNamespaceBindings.insert(new_binding);
        if (!result.second)
        {
            // Binding of this prefix already existed.  Check it used the same URI.
            PROTO_ASSERT2(result.first->second == new_binding.second,
                          "Conflicting definitions of namespace prefix " << new_binding.first << ": was "
                          << result.first->second << " but tried to bind to " << new_binding.second,
                          "Protocol definition");
        }
    }
}


void Protocol::AddInputDefinitions(const std::vector<AbstractStatementPtr>& rStatements)
{
    std::copy(rStatements.begin(), rStatements.end(), std::back_inserter(mInputStatements));
}


void Protocol::AddPostProcessing(const std::vector<AbstractStatementPtr>& rStatements)
{
    mPostProcessing.resize(mPostProcessing.size() + rStatements.size());
    std::copy(rStatements.begin(), rStatements.end(), mPostProcessing.end()-rStatements.size());
}


void Protocol::AddLibrary(const std::vector<AbstractStatementPtr>& rStatements)
{
    mLibraryStatements.resize(mLibraryStatements.size() + rStatements.size());
    std::copy(rStatements.begin(), rStatements.end(), mLibraryStatements.end()-rStatements.size());
}


void Protocol::AddSimulation(boost::shared_ptr<AbstractSimulation> pSimulation)
{
    mSimulations.push_back(pSimulation);
}


void Protocol::AddSimulations(const std::vector<boost::shared_ptr<AbstractSimulation> >& rSimulations)
{
    std::copy(rSimulations.begin(), rSimulations.end(), std::back_inserter(mSimulations));
}


void Protocol::AddOutputSpecs(const std::vector<boost::shared_ptr<OutputSpecification> >& rSpecifications)
{
    mOutputSpecifications.resize(mOutputSpecifications.size() + rSpecifications.size());
    std::copy(rSpecifications.begin(), rSpecifications.end(), mOutputSpecifications.end()-rSpecifications.size());
}

void Protocol::AddDefaultPlots(const std::vector<boost::shared_ptr<PlotSpecification> >& rSpecifications)
{
    mPlotSpecifications.resize(mPlotSpecifications.size() + rSpecifications.size());
    std::copy(rSpecifications.begin(), rSpecifications.end(), mPlotSpecifications.end()-rSpecifications.size());
}


//
// Associating a protocol with a model
//

void Protocol::CheckModel(boost::shared_ptr<AbstractUntemplatedSystemWithOutputs> pModel) const
{
    if (!pModel)
    {
        EXCEPTION("Given model doesn't have any protocol outputs.");
    }
    if (pModel->GetNumberOfOutputs() == 0 && pModel->rGetVectorOutputNames().size() == 0)
    {
        EXCEPTION("Number of model outputs is zero.");
    }
    if (!boost::dynamic_pointer_cast<AbstractUntemplatedParameterisedSystem>(pModel))
    {
        EXCEPTION("Model has an incorrect base class!");
    }
}


void Protocol::SetModelEnvironments(const std::map<std::string, EnvironmentPtr>& rModelEnvs)
{
    BOOST_FOREACH(StringEnvPair binding, rModelEnvs)
    {
        mpLibrary->SetDelegateeEnvironment(binding.second, binding.first);
    }
    BOOST_FOREACH(StringProtoPair import, mImports)
    {
        import.second->SetModelEnvironments(rModelEnvs);
    }
}


void Protocol::SetModel(boost::shared_ptr<AbstractUntemplatedSystemWithOutputs> pModel)
{
    CheckModel(pModel);
    mpModel = pModel;
    // Get the model wrapper environments, and ensure that every other environment (including in
    // imported protocols) can delegate to them for ontology prefixes.
    mpModel->SetNamespaceBindings(mOntologyNamespaceBindings);
    const std::map<std::string, EnvironmentPtr>& r_model_envs = mpModel->rGetEnvironmentMap();
    SetModelEnvironments(r_model_envs);
    // Associate each simulation with this model
    for (unsigned simulation=0; simulation<mSimulations.size(); ++simulation)
    {
        mSimulations[simulation]->SetModel(mpModel);
    }
    // Now run all the library programs, if present
    InitialiseLibrary();
}


boost::shared_ptr<AbstractUntemplatedParameterisedSystem> Protocol::GetModel()
{
    EXCEPT_IF_NOT(mpModel);
    boost::shared_ptr<AbstractUntemplatedParameterisedSystem> p_system = boost::dynamic_pointer_cast<AbstractUntemplatedParameterisedSystem>(mpModel);
    assert(p_system);
    return p_system;
}
