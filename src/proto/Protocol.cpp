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

#include "Protocol.hpp"

#include <cassert>
#include <iostream>
#include <algorithm>
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

/*
 *
 * Un-templated abstract class implementation.
 *
 */

Protocol::Protocol()
    : mInputs(true),
      mpModelStateCollection(new ModelStateCollection)
{
    mLibrary.SetDelegateeEnvironment(mInputs.GetAsDelegatee());
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


void Protocol::Run()
{
    std::cout << "Running protocol..." << std::endl;
    // Ensure the final outputs environment exists
    EnvironmentPtr p_proto_outputs(new Environment);
    mOutputs[""] = p_proto_outputs;
    // Load our library, if present
    rGetLibrary();
    Environment post_proc_env(mLibrary.GetAsDelegatee());
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
            p_sim->rGetEnvironment().SetDelegateeEnvironment(mLibrary.GetAsDelegatee());
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
    AddSimulationResultsDelegatees(post_proc_env, mOutputs);
    // Post-process the results
    if (errors.empty())
    {
        std::cout << "Running post-processing..." << std::endl;
        try
        {
            post_proc_env.ExecuteStatements(mPostProcessing);
        }
        catch (const Exception& e)
        {
            errors.push_back(e);
        }
    }
    // Transfer requested outputs to mOutputs
    BOOST_FOREACH(boost::shared_ptr<OutputSpecification> p_spec, mOutputSpecifications)
    {
        const std::string& r_loc = p_spec->GetLocationInfo();
        const std::string& r_ref = p_spec->rGetOutputRef();
        const std::string& r_name = p_spec->rGetOutputName();
        try
        {
            AbstractValuePtr p_output = post_proc_env.Lookup(r_ref, r_loc);
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
                (*p_file) << '"' << p_stepper->GetIndexName() << "\",\"" << p_stepper->GetUnits() << '"';
                for (p_stepper->Reset(); !p_stepper->AtEnd(); p_stepper->Step())
                {
                    (*p_file) << "," << p_stepper->GetCurrentOutputPoint();
                }
                (*p_file) << std::endl;
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

    if (!missing_outputs.empty())
    {
        EXCEPTION("Not all protocol outputs were defined.  Missing names: " << missing_outputs);
    }
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
    AbstractValuePtr p_value = (*pValue)(mInputs);
    mInputs.OverwriteDefinition(rName, p_value, "Setting protocol input");
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
    return mInputs;
}


Environment& Protocol::rGetLibrary()
{
    mLibrary.ExecuteStatements(mLibraryStatements);
    mLibraryStatements.clear();
    return mLibrary;
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
    mLibrary.SetDelegateeEnvironment(pImport->rGetLibrary().GetAsDelegatee(), rPrefix);
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


void Protocol::SetModel(boost::shared_ptr<AbstractCardiacCellInterface> pModel)
{
    mpModel = CheckModel(pModel);
    // Get the model wrapper environments, and ensure every other environment can
    // delegate to them for ontology prefixes.
    mpModel->SetNamespaceBindings(mOntologyNamespaceBindings);
    const std::map<std::string, EnvironmentPtr>& r_model_envs = mpModel->rGetEnvironmentMap();
    BOOST_FOREACH(StringEnvPair binding, r_model_envs)
    {
        mInputs.SetDelegateeEnvironment(binding.second, binding.first);
    }

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
