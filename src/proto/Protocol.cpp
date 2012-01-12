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

#include "Protocol.hpp"

#include <cassert>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <boost/pointer_cast.hpp> // NB: Not available on Boost 1.33.1
#include <boost/foreach.hpp> // NB: Not available on Boost 1.33.1
#include <boost/assign/list_of.hpp>

#include "Exception.hpp"

#include "ProtoHelperMacros.hpp"
#include "VectorStreaming.hpp"
#include "BacktraceException.hpp"

// Typedefs for use with BOOST_FOREACH and std::maps
typedef std::pair<std::string, std::string> StringPair;
typedef std::pair<std::string, EnvironmentPtr> StringEnvPair;
typedef std::pair<std::string, ProtocolPtr> StringProtoPair;

/*
 *
 * Un-templated abstract class implementation.
 *
 */

Protocol::Protocol()
    : mpInputs(new Environment(true)),
      mpLibrary(new Environment),
      mpModelStateCollection(new ModelStateCollection)
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


void Protocol::InitialiseLibrary()
{
    const unsigned library_size = mpLibrary->GetNumberOfDefinitions();
    assert(library_size == 0 || library_size == mLibraryStatements.size());
    if (library_size == 0)
    {
        mpLibrary->ExecuteStatements(mLibraryStatements);
    }
}


void Protocol::Run()
{
    std::cout << "Running protocol..." << std::endl;
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
    if (!errors.empty())
    {
        ///\todo Make an Exception subclass which reports a set of errors?
        throw errors.front();
    }
    std::cout << "Finished running protocol." << std::endl;
}


void Protocol::WriteToFile(const OutputFileHandler& rHandler,
                           const std::string& rFileNameBase) const
{
    ///\todo Improve format?
    const Environment& r_outputs = rGetOutputsCollection();
    std::vector<std::string> missing_outputs;
    // Variable metadata file
    {
        std::string index_file_name = rFileNameBase + "-contents.csv";
        out_stream p_file = rHandler.OpenOutputFile(index_file_name);
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
                missing_outputs.push_back(r_name);
                continue;
            }
            if (p_output->IsArray())
            {
                NdArray<double> output = GET_ARRAY(p_output);
                (*p_file) << '"' << r_name << "\",\"" << p_spec->rGetOutputDescription()
                          << "\",\"" << p_output->GetUnits()
                          << "\"," << output.GetNumDimensions()
                          << "," << (rFileNameBase + "_" + r_name + ".csv")
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
        out_stream p_file = rHandler.OpenOutputFile(index_file_name);
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
        out_stream p_file = rHandler.OpenOutputFile(file_name);
        (*p_file) << "Plot title,First variable,Optional second variable" << std::endl;
        BOOST_FOREACH(boost::shared_ptr<PlotSpecification> p_spec, mPlotSpecifications)
        {
            const std::string& r_title = p_spec->rGetTitle();
            (*p_file) << '"' << r_title << '"';
            BOOST_FOREACH(std::string varname, p_spec->rGetVariableNames())
            {
                (*p_file) << ",\"" << varname << "\"";
            }
            (*p_file) << std::endl;
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

        std::string file_name = rFileNameBase + "_" + r_name + ".csv";
        out_stream p_file = rHandler.OpenOutputFile(file_name);
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

    // \todo #1999 LOADS OF TESTS!!
    BOOST_FOREACH(boost::shared_ptr<PlotSpecification> p_plot_spec, mPlotSpecifications)
    {
        const std::string& r_title = p_plot_spec->rGetTitle();
        const std::vector<std::string>& r_names = p_plot_spec->rGetVariableNames();

        // We only deal with 2D data at present...
        if(r_names.size()==2u)
        {
            bool both_vars_present = true;

            // Ensure that both of the requested outputs are present
            for (unsigned variable_idx = 0; variable_idx<2; ++variable_idx)
            {
                const std::string r_name = r_names[variable_idx];
                AbstractValuePtr p_output;
                try
                {
                    p_output = r_outputs.Lookup(r_name);
                }
                catch (const Exception& e)
                {
                    std::cerr << "Plot requests protocol output \"" << r_name << "\", which is missing.\n";
                    missing_outputs.push_back(r_name);
                    both_vars_present = false;
                }
            }

            if (both_vars_present)
            {
                std::cout << "Plotting " << r_title << ":\t";
                std::cout << r_names[1] << " against " << r_names[0] << "\n" << std::flush;

                // Examine the dimensions and shapes of the two specified objects to check they are 1d vectors
                // \todo #1999 also make it work with 2D arrays of shape 1 in one direction?
                AbstractValuePtr p_output_x = r_outputs.Lookup(r_names[0]);
                AbstractValuePtr p_output_y = r_outputs.Lookup(r_names[1]);
                // Take a copy of the units information and add to the Plot Spec for gnuplot.
                p_plot_spec->SetVariableUnits(p_output_x->GetUnits(),p_output_y->GetUnits());

                // Get info on the shapes and dimensions
                NdArray<double> output_x = GET_ARRAY(p_output_x);
                NdArray<double> output_y = GET_ARRAY(p_output_y);
                const unsigned num_dims_x = output_x.GetNumDimensions();
                NdArray<double>::Extents shape_x = output_x.GetShape();
                const unsigned num_dims_y = output_y.GetNumDimensions();
                NdArray<double>::Extents shape_y = output_y.GetShape();
                std::cout << "X dims = " << num_dims_x << "\t";
                for (unsigned i=0; i<num_dims_x; i++)
                {
                    std::cout << ", X shape [" << i << "] = " << shape_x[i] << "\t";
                }
                std::cout << "\nY dims = " << num_dims_y << "\t";
                for (unsigned i=0; i<num_dims_y; i++)
                {
                    std::cout << ", Y shape [" << i << "] = " << shape_y[i] << "\t";
                }
                std::cout << std::endl << std::flush;

                // Only plot 1D arrays of equal length (or shape[0])
                if ((num_dims_x==num_dims_y) && (num_dims_x==1u) && shape_x[0]==shape_y[0])
                {
                    std::string file_name = rFileNameBase + "_" + r_title + "_gnuplot_data.csv";
                    FileFinder::ReplaceSpacesWithUnderscores(file_name);
                    out_stream p_file = rHandler.OpenOutputFile(file_name);
                    // Tabular format with no header line for easy processing by gnuplot
                    NdArray<double>::Indices x_idxs = output_x.GetIndices();
                    NdArray<double>::Indices y_idxs = output_y.GetIndices();
                    for (x_idxs[0]=0; x_idxs[0]<shape_x[0]; ++x_idxs[0])
                    {
                        (*p_file) << output_x[x_idxs] << "," << output_y[x_idxs]<< std::endl;
                    }
                    p_file->close();

                    Gnuplotter(p_plot_spec,rHandler,file_name);
                }
                else
                {
                    std::cout << "\\todo #1999: Different numbers of dimensions in specified plots\n";
                }
            }
        }
        else
        {
            std::cout << "\\todo #1999: In plot \"" << r_title << "\" no <x> and <y> in plot specification - skipping for now\n";
        }
    }

    if (!missing_outputs.empty())
    {
        EXCEPTION("Not all protocol outputs were defined.  Missing names: " << missing_outputs);
    }
}

void Protocol::Gnuplotter(boost::shared_ptr<PlotSpecification> pPlotSpec, const OutputFileHandler& rHandler, const std::string& rFileName) const
{
    // At present this is hardcoded to 2 columns of data x,y points.
    // \todo #1999 generalise to other situations
    // \todo #1999 make the plot title include the model as well as protocol name

    // Find the csv file name, and remove .csv to make the .gp file name
    size_t dot = rFileName.rfind('.');
    std::string filename = rFileName.substr(0,dot) + ".gp";

    // Generate axes labels
    std::string xlabel = pPlotSpec->rGetVariableNames()[0];     // get variable name
    FileFinder::ReplaceUnderscoresWithSpaces(xlabel);           // remove underscores (interpreted as subscripts)
    xlabel += " (" + pPlotSpec->rGetVariableUnits()[0] + ")";   // add units

    std::string ylabel = pPlotSpec->rGetVariableNames()[1];     // get variable name
    FileFinder::ReplaceUnderscoresWithSpaces(ylabel);           // remove underscores (interpreted as subscripts)
    ylabel += " (" + pPlotSpec->rGetVariableUnits()[1] + ")";   // add units

    // Write out a gnuplot script
    out_stream p_gnuplot_script = rHandler.OpenOutputFile(filename);
    std::string output_dir = rHandler.GetOutputDirectoryFullPath();
    *p_gnuplot_script << "# Gnuplot script file generated by Chaste Functional Curation system." << std::endl;
    *p_gnuplot_script << "# Plot of " << pPlotSpec->rGetTitle() << "." << std::endl;
    *p_gnuplot_script << "set terminal postscript eps enhanced size 3, 2 font 16" << std::endl;
    *p_gnuplot_script << "set output \"" << output_dir << pPlotSpec->rGetTitle() << ".eps\"" << std::endl;
    *p_gnuplot_script << "set title \"" << pPlotSpec->rGetTitle() << "\"" << std::endl;
    *p_gnuplot_script << "set xlabel \"" << xlabel << "\"" << std::endl;
    *p_gnuplot_script << "set ylabel \"" << ylabel << "\"" << std::endl;
    //*p_gnuplot_script << "set xtics 400" << std::endl;
    *p_gnuplot_script << "set grid" << std::endl;
    *p_gnuplot_script << "set autoscale" << std::endl;
    *p_gnuplot_script << "set key off" << std::endl;
    *p_gnuplot_script << "set datafile separator \",\"" << std::endl;
    *p_gnuplot_script << "plot \"" + output_dir + rFileName + "\" using 1:2 with linespoints pointtype 7";
    *p_gnuplot_script << std::endl << std::flush;
    p_gnuplot_script->close();

    // Run Gnuplot on the script written above to generate image files.
    EXPECT0(system, "gnuplot " + output_dir + filename);
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
    AbstractValuePtr p_value = (*pValue)(*mpInputs);
    mpInputs->OverwriteDefinition(rName, p_value, "Setting protocol input");
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

boost::shared_ptr<AbstractUntemplatedSystemWithOutputs> Protocol::CheckModel(boost::shared_ptr<AbstractCardiacCellInterface> pModel) const
{
    boost::shared_ptr<AbstractUntemplatedSystemWithOutputs> p_model
        = boost::dynamic_pointer_cast<AbstractUntemplatedSystemWithOutputs>(pModel);
    if (!p_model)
    {
        EXCEPTION("Given model doesn't have any protocol outputs.");
    }
    if (p_model->GetNumberOfOutputs() == 0 && p_model->rGetVectorOutputNames().size() == 0)
    {
        EXCEPTION("Number of model outputs is zero.");
    }
    if (!boost::dynamic_pointer_cast<AbstractUntemplatedParameterisedSystem>(p_model))
    {
        EXCEPTION("Model has an incorrect base class!");
    }
    return p_model;
}


void Protocol::SetModelEnvironments(const std::map<std::string, EnvironmentPtr>& rModelEnvs)
{
    BOOST_FOREACH(StringEnvPair binding, rModelEnvs)
    {
        mpInputs->SetDelegateeEnvironment(binding.second, binding.first);
    }
    BOOST_FOREACH(StringProtoPair import, mImports)
    {
        import.second->SetModelEnvironments(rModelEnvs);
    }
    // Now run the library program, if present
    InitialiseLibrary();
}


void Protocol::SetModel(boost::shared_ptr<AbstractCardiacCellInterface> pModel)
{
    mpModel = CheckModel(pModel);
    // Get the model wrapper environments, and ensure that every other environment (including in
    // imported protocols) can delegate to them for ontology prefixes.
    mpModel->SetNamespaceBindings(mOntologyNamespaceBindings);
    const std::map<std::string, EnvironmentPtr>& r_model_envs = mpModel->rGetEnvironmentMap();
    SetModelEnvironments(r_model_envs);
    // Associate each simulation with this model
    for (unsigned simulation=0; simulation<mSimulations.size(); ++simulation)
    {
        mSimulations[simulation]->SetCell(pModel);
    }
}


boost::shared_ptr<AbstractUntemplatedParameterisedSystem> Protocol::GetModel()
{
    EXCEPT_IF_NOT(mpModel);
    boost::shared_ptr<AbstractUntemplatedParameterisedSystem> p_system = boost::dynamic_pointer_cast<AbstractUntemplatedParameterisedSystem>(mpModel);
    assert(p_system);
    return p_system;
}
