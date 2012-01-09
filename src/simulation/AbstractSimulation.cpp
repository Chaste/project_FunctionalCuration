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


AbstractSimulation::AbstractSimulation(boost::shared_ptr<AbstractCardiacCellInterface> pCell,
                                       boost::shared_ptr<AbstractStepper> pStepper,
                                       boost::shared_ptr<ModifierCollection> pModifiers,
                                       boost::shared_ptr<std::vector<boost::shared_ptr<AbstractStepper> > > pSteppers)
    : mpCell(pCell),
      mpStepper(pStepper),
      mpModifiers(pModifiers),
      mpSteppers(pSteppers),
      mpEnvironment(new Environment)
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


void AbstractSimulation::InitialiseSteppers()
{
    BOOST_FOREACH(boost::shared_ptr<AbstractStepper> p_stepper, *mpSteppers)
    {
        p_stepper->Initialise();
    }
}


Environment& AbstractSimulation::rGetEnvironment()
{
    return *mpEnvironment;
}


void AbstractSimulation::SetOutputsPrefix(const std::string& rPrefix)
{
    mOutputsPrefix = rPrefix;
    if (!mpStepper->IsEndFixed() && !rPrefix.empty())
    {
        // Create an environment to contain views of the simulation outputs thus far, for
        // use in the loop condition test if needed.
        EnvironmentPtr p_view_env(new Environment(true/* allow overwrite */));
        mpEnvironment->SetDelegateeEnvironment(p_view_env, rPrefix);
    }
}


std::string AbstractSimulation::GetOutputsPrefix() const
{
    return mOutputsPrefix;
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
        p_results.reset(new Environment);
        if (mpSteppers->back()->GetNumberOfOutputPoints() > 0u) // Not a nested protocol
        {
            CreateOutputArrays(p_results);
        }
    }
    Run(p_results);
    if (store_results)
    {
        ResizeOutputs(p_results);
    }
    return p_results;
}


void AbstractSimulation::LoopBodyStartHook(EnvironmentPtr pResults)
{
    if (mpModifiers)
    {
        (*mpModifiers)(mpCell, mpStepper);
    }
    // If this is a while loop, we may need to allocate more memory for the results arrays
    if (pResults && !mpStepper->IsEndFixed() && pResults->GetNumberOfDefinitions() > 0u)
    {
        const std::string any_name = pResults->GetDefinedNames().front();
        NdArray<double> array = GET_ARRAY(pResults->Lookup(any_name, GetLocationInfo()));
        if (mpStepper->GetNumberOfOutputPoints() >= array.GetShape()[0])
        {
            ResizeOutputs(pResults);
        }
    }
}


void AbstractSimulation::LoopBodyEndHook(EnvironmentPtr pResults)
{
    // If this is a while loop, update the views of the results thus far
    if (!mpStepper->IsEndFixed())
    {
        CreateResultViews(pResults);
    }
}


void AbstractSimulation::LoopEndHook(EnvironmentPtr pResults)
{
    if (mpModifiers)
    {
        mpModifiers->ApplyAtEnd(mpCell, mpStepper);
    }
}


void AbstractSimulation::CreateOutputArrays(EnvironmentPtr pResults)
{
    // Ensure there's an environment to put results in
    assert(!GetOutputsPrefix().empty());
    assert(pResults);

    // Get the system being simulated
    boost::shared_ptr<const AbstractUntemplatedSystemWithOutputs> p_system
        = boost::dynamic_pointer_cast<const AbstractUntemplatedSystemWithOutputs>(mpCell);
    assert(p_system);
    const std::vector<boost::shared_ptr<AbstractStepper> >& r_steppers = rGetSteppers();

    // Figure out the (initial) sizes
    const unsigned nesting_depth = r_steppers.size();
    NdArray<double>::Extents outputs_shape(nesting_depth);
    for (unsigned dim=0; dim<nesting_depth; ++dim)
    {
        outputs_shape[dim] = r_steppers[dim]->GetNumberOfOutputPoints();
    }

    // Create output arrays
    const std::vector<std::string> output_names = p_system->GetOutputNames();
    const std::vector<std::string> output_units = p_system->GetOutputUnits();
    const unsigned num_outputs = output_names.size();
    assert(num_outputs == output_units.size());
    for (unsigned i=0; i<num_outputs; ++i)
    {
        NdArray<double> array(outputs_shape);
        AbstractValuePtr p_value = boost::make_shared<ArrayValue>(array);
        p_value->SetUnits(output_units[i]);
        pResults->DefineName(output_names[i], p_value, GetLocationInfo());
    }

    // Create arrays for vector outputs
    const std::vector<std::string>& r_vector_output_names = p_system->rGetVectorOutputNames();
    const unsigned num_vector_outputs = r_vector_output_names.size();
    const std::vector<unsigned> vector_output_lengths = p_system->GetVectorOutputLengths();
    for (unsigned i=0; i<num_vector_outputs; ++i)
    {
        NdArray<double>::Extents shape(outputs_shape);
        // Last dimension varies over the vector elements
        shape.push_back(vector_output_lengths[i]);
        NdArray<double> array(shape);
        AbstractValuePtr p_value = boost::make_shared<ArrayValue>(array);
        pResults->DefineName(r_vector_output_names[i], p_value, GetLocationInfo());
    }
}

void AbstractSimulation::ResizeOutputs(EnvironmentPtr pResults)
{
    assert(!GetOutputsPrefix().empty());

    if (!mpStepper->IsEndFixed())
    {
        const std::vector<std::string> output_names = pResults->GetDefinedNames();
        BOOST_FOREACH(const std::string& r_name, output_names)
        {
            NdArray<double> array = GET_ARRAY(pResults->Lookup(r_name, GetLocationInfo()));
            NdArray<double>::Extents shape = array.GetShape();
            shape[0] = mpStepper->GetNumberOfOutputPoints();
            array.Resize(shape);
        }
    }
}

void AbstractSimulation::CreateResultViews(EnvironmentPtr pResults)
{
    assert(!mpStepper->IsEndFixed());
    const std::string prefix = GetOutputsPrefix();

    if (!prefix.empty())
    {
        const unsigned view_size = mpStepper->GetCurrentOutputNumber() + 1;
        const std::vector<std::string> output_names = pResults->GetDefinedNames();
        EnvironmentPtr p_view_env = boost::const_pointer_cast<Environment>(rGetEnvironment().GetDelegateeEnvironment(prefix));
        DEFINE_TUPLE(dim_default, EXPR_LIST(NULL_EXPR)(NULL_EXPR)(CONST(1))(NULL_EXPR));
        DEFINE_TUPLE(dim0, EXPR_LIST(CONST(0))(CONST(0))(CONST(1))(CONST(view_size)));
        std::vector<AbstractExpressionPtr> view_args = boost::assign::list_of(dim0)(dim_default);
        BOOST_FOREACH(const std::string& r_name, output_names)
        {
            DEFINE(view, boost::make_shared<View>(LOOKUP(r_name), view_args));
            if (view_size == 1)
            {
                p_view_env->DefineName(r_name, (*view)(*pResults), mpStepper->GetLocationInfo());
            }
            else
            {
                p_view_env->OverwriteDefinition(r_name, (*view)(*pResults), mpStepper->GetLocationInfo());
            }
        }
   }
}
