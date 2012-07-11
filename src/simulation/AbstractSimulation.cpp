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

#include "AbstractSimulation.hpp"

#include <boost/foreach.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/make_shared.hpp>
#include <boost/assign/list_of.hpp>

#include "AbstractSystemWithOutputs.hpp"
#include "BacktraceException.hpp"
#include "NdArray.hpp"
#include "ValueTypes.hpp"
#include "ProtoHelperMacros.hpp"
#include "View.hpp"
#include "NameLookup.hpp"
#include "TupleExpression.hpp"


// For use with BOOST_FOREACH and std::map
typedef std::pair<std::string, EnvironmentPtr> StringEnvPair;


AbstractSimulation::AbstractSimulation(boost::shared_ptr<AbstractUntemplatedSystemWithOutputs> pModel,
                                       boost::shared_ptr<AbstractStepper> pStepper,
                                       boost::shared_ptr<ModifierCollection> pModifiers,
                                       boost::shared_ptr<std::vector<boost::shared_ptr<AbstractStepper> > > pSteppers)
    : mpModel(pModel),
      mpStepper(pStepper),
      mpModifiers(pModifiers),
      mpSteppers(pSteppers),
      mpEnvironment(new Environment),
      mpResultsEnvironment(new Environment)
{
    if (!mpSteppers)
    {
        // Create the stepper collection vector
        mpSteppers.reset(new std::vector<boost::shared_ptr<AbstractStepper> >);
    }
    PROTO_ASSERT(mpSteppers->empty() || mpSteppers->front()->IsEndFixed(),
                 "A while loop may only be the outermost loop for a simulation.");
    // Add our stepper to the front of the shared collection (innermost is last)
    mpSteppers->insert(mpSteppers->begin(), mpStepper);
    mpStepper->SetEnvironment(mpEnvironment);
}


AbstractSimulation::~AbstractSimulation()
{
}


void AbstractSimulation::SetModel(boost::shared_ptr<AbstractUntemplatedSystemWithOutputs> pModel)
{
    mpModel = pModel;
    const std::map<std::string, EnvironmentPtr>& r_model_envs = mpModel->rGetEnvironmentMap();
    BOOST_FOREACH(StringEnvPair binding, r_model_envs)
    {
        mpEnvironment->SetDelegateeEnvironment(binding.second, binding.first);
        mpResultsEnvironment->SetDelegateeEnvironment(binding.second, binding.first);
    }
}


void AbstractSimulation::InitialiseSteppers()
{
    BOOST_FOREACH(boost::shared_ptr<AbstractStepper> p_stepper, *mpSteppers)
    {
        p_stepper->Initialise();
    }
    if (!mpStepper->IsEndFixed() && !mOutputsPrefix.empty())
    {
        // Create an environment to contain views of the simulation outputs thus far, for
        // use in the loop condition test if needed.
        EnvironmentPtr p_view_env(new Environment(true/* allow overwrite */));
        mpEnvironment->SetDelegateeEnvironment(p_view_env, mOutputsPrefix);
    }
}


Environment& AbstractSimulation::rGetEnvironment()
{
    return *mpEnvironment;
}


EnvironmentPtr AbstractSimulation::GetResultsEnvironment()
{
    return mpResultsEnvironment;
}


void AbstractSimulation::SetOutputsPrefix(const std::string& rPrefix)
{
    mOutputsPrefix = rPrefix;
}


std::string AbstractSimulation::GetOutputsPrefix() const
{
    return mOutputsPrefix;
}


void AbstractSimulation::SetOutputFolder(boost::shared_ptr<OutputFileHandler> pHandler)
{
    mpOutputHandler = pHandler;
}


FileFinder AbstractSimulation::GetOutputFolder() const
{
    FileFinder folder;
    if (mpOutputHandler)
    {
        folder = mpOutputHandler->FindFile("");
    }
    return folder;
}

//
// Methods for creating and collecting simulation results
//

EnvironmentPtr AbstractSimulation::Run()
{
    EnvironmentPtr p_results;
    bool store_results = !GetOutputsPrefix().empty();
    if (store_results)
    {
        p_results = mpResultsEnvironment;
        if (mpSteppers->back()->GetNumberOfOutputPoints() > 0u) // Not a nested protocol
        {
            CreateOutputArrays();
        }
    }
    Run(p_results);
    if (store_results)
    {
        ResizeOutputs();
    }
    return mpResultsEnvironment;
}


void AbstractSimulation::LoopBodyStartHook()
{
    if (mpModifiers)
    {
        (*mpModifiers)(mpModel, mpStepper);
    }
    // If this is a while loop, we may need to allocate more memory for the results arrays
    if (mpResultsEnvironment && !mpStepper->IsEndFixed()
        && mpResultsEnvironment->GetNumberOfDefinitions() > 0u)
    {
        const std::string any_name = mpResultsEnvironment->GetDefinedNames().front();
        NdArray<double> array = GET_ARRAY(mpResultsEnvironment->Lookup(any_name, GetLocationInfo()));
        if (mpStepper->GetNumberOfOutputPoints() >= array.GetShape()[0])
        {
            ResizeOutputs();
        }
    }
}


void AbstractSimulation::LoopBodyEndHook()
{
    // If this is a while loop, update the views of the results thus far
    if (!mpStepper->IsEndFixed())
    {
        CreateResultViews();
    }
}


void AbstractSimulation::LoopEndHook()
{
    if (mpModifiers)
    {
        mpModifiers->ApplyAtEnd(mpModel, mpStepper);
    }
}


void AbstractSimulation::CreateOutputArrays()
{
    // Ensure there's an environment to put results in
    assert(!GetOutputsPrefix().empty());
    assert(mpResultsEnvironment);

    // Get the system being simulated
    const std::vector<boost::shared_ptr<AbstractStepper> >& r_steppers = rGetSteppers();

    // Figure out the (initial) sizes
    const unsigned nesting_depth = r_steppers.size();
    NdArray<double>::Extents outputs_shape(nesting_depth);
    for (unsigned dim=0; dim<nesting_depth; ++dim)
    {
        outputs_shape[dim] = r_steppers[dim]->GetNumberOfOutputPoints();
    }

    // Create output arrays
    const std::vector<std::string> output_names = mpModel->GetOutputNames();
    const std::vector<std::string> output_units = mpModel->GetOutputUnits();
    const unsigned num_outputs = output_names.size();
    assert(num_outputs == output_units.size());
    for (unsigned i=0; i<num_outputs; ++i)
    {
        NdArray<double> array(outputs_shape);
        AbstractValuePtr p_value = boost::make_shared<ArrayValue>(array);
        p_value->SetUnits(output_units[i]);
        mpResultsEnvironment->DefineName(output_names[i], p_value, GetLocationInfo());
    }

    // Create arrays for vector outputs
    const std::vector<std::string>& r_vector_output_names = mpModel->rGetVectorOutputNames();
    const unsigned num_vector_outputs = r_vector_output_names.size();
    const std::vector<unsigned> vector_output_lengths = mpModel->GetVectorOutputLengths();
    for (unsigned i=0; i<num_vector_outputs; ++i)
    {
        NdArray<double>::Extents shape(outputs_shape);
        // Last dimension varies over the vector elements
        shape.push_back(vector_output_lengths[i]);
        NdArray<double> array(shape);
        AbstractValuePtr p_value = boost::make_shared<ArrayValue>(array);
        mpResultsEnvironment->DefineName(r_vector_output_names[i], p_value, GetLocationInfo());
    }
}

void AbstractSimulation::ResizeOutputs()
{
    assert(!GetOutputsPrefix().empty());

    if (!mpStepper->IsEndFixed())
    {
        const std::vector<std::string> output_names = mpResultsEnvironment->GetDefinedNames();
        BOOST_FOREACH(const std::string& r_name, output_names)
        {
            NdArray<double> array = GET_ARRAY(mpResultsEnvironment->Lookup(r_name, GetLocationInfo()));
            NdArray<double>::Extents shape = array.GetShape();
            shape[0] = mpStepper->GetNumberOfOutputPoints();
            array.Resize(shape);
        }
    }
}

void AbstractSimulation::CreateResultViews()
{
    assert(!mpStepper->IsEndFixed());
    const std::string prefix = GetOutputsPrefix();

    if (!prefix.empty())
    {
        const unsigned view_size = mpStepper->GetCurrentOutputNumber() + 1;
        const std::vector<std::string> output_names = mpResultsEnvironment->GetDefinedNames();
        EnvironmentPtr p_view_env = boost::const_pointer_cast<Environment>(rGetEnvironment().GetDelegateeEnvironment(prefix));
        DEFINE_TUPLE(dim_default, EXPR_LIST(NULL_EXPR)(NULL_EXPR)(CONST(1))(NULL_EXPR));
        DEFINE_TUPLE(dim0, EXPR_LIST(CONST(0))(CONST(0))(CONST(1))(CONST(view_size)));
        std::vector<AbstractExpressionPtr> view_args = boost::assign::list_of(dim0)(dim_default);
        BOOST_FOREACH(const std::string& r_name, output_names)
        {
            DEFINE(view, boost::make_shared<View>(LOOKUP(r_name), view_args));
            if (view_size == 1)
            {
                p_view_env->DefineName(r_name, (*view)(*mpResultsEnvironment), mpStepper->GetLocationInfo());
            }
            else
            {
                p_view_env->OverwriteDefinition(r_name, (*view)(*mpResultsEnvironment), mpStepper->GetLocationInfo());
            }
        }
   }
}
