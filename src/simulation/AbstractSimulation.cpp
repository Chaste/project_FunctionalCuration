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
#include "VectorStreaming.hpp"


AbstractSimulation::AbstractSimulation(boost::shared_ptr<AbstractSystemWithOutputs> pModel,
                                       AbstractStepperPtr pStepper,
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
    // Add our stepper (if present) to the front of the shared collection (innermost is last)
    if (mpStepper)
    {
        mpSteppers->insert(mpSteppers->begin(), mpStepper);
        mpStepper->SetEnvironment(mpEnvironment);
    }
}


AbstractSimulation::~AbstractSimulation()
{
}


void AbstractSimulation::SetModel(boost::shared_ptr<AbstractSystemWithOutputs> pModel)
{
    mpModel = pModel;
    const std::map<std::string, EnvironmentPtr>& r_model_envs = mpModel->rGetEnvironmentMap();
    BOOST_FOREACH(StringEnvPair binding, r_model_envs)
    {
        PROTO_ASSERT(!binding.first.empty(), "Model environment wrappers must be bound to a prefix.");
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
    if (!mOutputsPrefix.empty() && mpStepper && !mpStepper->IsEndFixed())
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
    assert(mpStepper);
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
    assert(mpStepper);
    // If this is a while loop, update the views of the results thus far
    if (!mpStepper->IsEndFixed())
    {
        CreateResultViews();
    }
}


void AbstractSimulation::LoopEndHook()
{
    assert(mpStepper);
    if (mpModifiers)
    {
        mpModifiers->ApplyAtEnd(mpModel, mpStepper);
    }
}


void AbstractSimulation::AddIterationOutputs(EnvironmentPtr pResults,
                                             EnvironmentCPtr pIterationOutputs,
                                             std::string outputNamePrefix)
{
    if (pResults)
    {
        const unsigned num_local_dims = rGetSteppers().size();
        if (pResults == pIterationOutputs)
        {
            // Special case for CombinedSimulation at the top level;
            // will apply also to other non-looped simulation types in the future.
            assert(num_local_dims == 0u);
            return;
        }
        bool first_run = (pResults->GetNumberOfDefinitions() == 0u);

        BOOST_FOREACH(const std::string& r_output_name, pIterationOutputs->GetDefinedNames())
        {
            AbstractValuePtr p_output = pIterationOutputs->Lookup(r_output_name, GetLocationInfo());
            PROTO_ASSERT(p_output->IsArray(),
                         "Model produced non-array output " << r_output_name << ".");
            const NdArray<double> output_array = GET_ARRAY(p_output);
            const NdArray<double>::Extents output_shape = output_array.GetShape();
            NdArray<double> result_array;

            // Create output array, if not already done
            if (first_run)
            {
                mModelOutputShapes[outputNamePrefix + r_output_name] = output_shape;
                NdArray<double>::Extents shape(num_local_dims + output_shape.size());
                for (unsigned i=0; i<num_local_dims; i++)
                {
                    shape[i] = rGetSteppers()[i]->GetNumberOfOutputPoints();
                }
                std::copy(output_shape.begin(), output_shape.end(), shape.begin() + num_local_dims);
                NdArray<double> result(shape);
                result_array = result;
                AbstractValuePtr p_result = boost::make_shared<ArrayValue>(result);
                p_result->SetUnits(p_output->GetUnits());
                pResults->DefineName(r_output_name, p_result, GetLocationInfo());
            }
            else
            {
                // Check the sub array shape matches the original run
                PROTO_ASSERT(output_shape == mModelOutputShapes[outputNamePrefix + r_output_name],
                             "The outputs of a model must not change shape during a simulation; output "
                             << outputNamePrefix + r_output_name << " with shape now " << output_shape
                             << " does not match the original shape "
                             << mModelOutputShapes[outputNamePrefix + r_output_name] << ".");
                result_array = GET_ARRAY(pResults->Lookup(r_output_name, GetLocationInfo()));
            }

            // Add model output into result array
            NdArray<double>::Indices idxs = result_array.GetIndices();
            for (unsigned i=0; i<num_local_dims; i++)
            {
                idxs[i] = rGetSteppers()[i]->GetCurrentOutputNumber();
            }
            NdArray<double>::Iterator result_it(idxs, result_array);
            std::copy(output_array.Begin(), output_array.End(), result_it);
        }

        // Check for any results sub-environments, and add them too, recursively.
        BOOST_FOREACH(const std::string& r_sub_prefix, pIterationOutputs->rGetSubEnvironmentNames())
        {
            EnvironmentPtr p_sub_results
                = boost::const_pointer_cast<Environment>(pResults->GetDelegateeEnvironment(r_sub_prefix));
            if (!p_sub_results)
            {
                // Set up a new delegatee in the overall results for these sub-results
                p_sub_results.reset(new Environment);
                pResults->SetDelegateeEnvironment(p_sub_results, r_sub_prefix);
            }
            // Add the sub-results for this iteration
            AddIterationOutputs(p_sub_results, pIterationOutputs->GetDelegateeEnvironment(r_sub_prefix),
                                outputNamePrefix + r_sub_prefix + ":");
        }
    }
}


void AbstractSimulation::ResizeOutputs()
{
    assert(!GetOutputsPrefix().empty());

    if (mpStepper && !mpStepper->IsEndFixed())
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
    assert(mpStepper);
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
