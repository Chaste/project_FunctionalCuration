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

#include "AbstractSimulation.hpp"

#include <boost/foreach.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/make_shared.hpp>
#include <boost/assign/list_of.hpp>

#include "AbstractSystemWithOutputs.hpp"
#include "AbstractParameterisedSystem.hpp"
#include "BacktraceException.hpp"
#include "NdArray.hpp"
#include "ValueTypes.hpp"
#include "ProtoHelperMacros.hpp"
#include "View.hpp"
#include "NameLookup.hpp"
#include "TupleExpression.hpp"

#include "VectorHelperFunctions.hpp"

AbstractSimulation::AbstractSimulation(boost::shared_ptr<AbstractCardiacCellInterface> pCell,
                                       boost::shared_ptr<AbstractStepper> pStepper,
                                       boost::shared_ptr<ModifierCollection> pModifiers,
                                       boost::shared_ptr<std::vector<boost::shared_ptr<AbstractStepper> > > pSteppers)
    : mpCell(pCell),
      mpStepper(pStepper),
      mpModifiers(pModifiers),
      mpSteppers(pSteppers)
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
    mpStepper->SetEnvironment(mEnvironment);
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
    return mEnvironment;
}


void AbstractSimulation::SetOutputsPrefix(const std::string& rPrefix)
{
    mOutputsPrefix = rPrefix;
    if (!mpStepper->IsEndFixed() && !rPrefix.empty())
    {
        // Create an environment to contain views of the simulation outputs thus far, for
        // use in the loop condition test if needed.
        EnvironmentPtr p_view_env(new Environment(true/* allow overwrite */));
        mEnvironment.SetDelegateeEnvironment(p_view_env, rPrefix);
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
        CreateOutputArrays(p_results);
    }
    Run(p_results);
    if (store_results)
    {
        ResizeOutputs(p_results);
    }
    return p_results;
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

void AbstractSimulation::AddOutputData(EnvironmentPtr pResults)
{
    if (!pResults)
    {
        // Not storing output
        return;
    }
    boost::shared_ptr<AbstractSystemWithOutputs<N_Vector> > p_system
        = boost::dynamic_pointer_cast<AbstractSystemWithOutputs<N_Vector> >(mpCell);
    if (p_system)
    {
        AddOutputDataTemplated<N_Vector>(pResults);
    }
    else
    {
        boost::shared_ptr<AbstractSystemWithOutputs<std::vector<double> > > p_system2
            = boost::dynamic_pointer_cast<AbstractSystemWithOutputs<std::vector<double> > >(mpCell);
        if (p_system2)
        {
            AddOutputDataTemplated<std::vector<double> >(pResults);
        }
        else
        {
            PROTO_EXCEPTION("Unexpected template parameter for model.");
        }
    }
}


template<typename VECTOR>
void AbstractSimulation::AddOutputDataTemplated(EnvironmentPtr pResults)
{
    // Get the model information
    boost::shared_ptr<AbstractSystemWithOutputs<VECTOR> > p_system
        = boost::dynamic_pointer_cast<AbstractSystemWithOutputs<VECTOR> >(mpCell);
    assert(p_system);
    const VECTOR& r_state = boost::dynamic_pointer_cast<AbstractParameterisedSystem<VECTOR> >(mpCell)->rGetStateVariables();
    const bool is_while_loop = !mpStepper->IsEndFixed();
    std::vector<boost::shared_ptr<AbstractStepper> >& r_steppers = rGetSteppers();

    // Figure out which part of the output arrays to fill
    const unsigned nesting_depth = r_steppers.size();
    NdArray<double>::Indices indices(nesting_depth);
    bool about_to_outer_loop = true;
    for (unsigned dim=0; dim<nesting_depth; ++dim)
    {
        indices[dim] = r_steppers[dim]->GetCurrentOutputNumber();
        if (dim > 0 && indices[dim] < r_steppers[dim]->GetNumberOfOutputPoints())
        {
            about_to_outer_loop = false;
        }
    }

    // Fill in outputs
    const std::vector<std::string> output_names = p_system->GetOutputNames();
    const unsigned num_outputs = output_names.size();
    const double time = rGetSteppers().back()->GetCurrentOutputPoint();
    VECTOR outputs = p_system->ComputeOutputs(time, r_state);
    assert(GetVectorSize(outputs) == num_outputs);
    for (unsigned i=0; i<num_outputs; ++i)
    {
        NdArray<double> array = GET_ARRAY(pResults->Lookup(output_names[i], GetLocationInfo()));
        // Resize all outputs if a while loop has gone beyond the current allocation
        if (i == 0 && is_while_loop && indices[0] >= array.GetShape()[0])
        {
            ResizeOutputs(pResults);
        }
        // Assign value
        array[indices] = GetVectorComponent(outputs, i);
    }
    DeleteVector(outputs);

    // Fill in vector outputs
    const std::vector<std::string>& r_vector_output_names = p_system->rGetVectorOutputNames();
    const unsigned num_vector_outputs = r_vector_output_names.size();
    std::vector<VECTOR> vector_outputs = p_system->ComputeVectorOutputs(time, r_state);
    assert(vector_outputs.size() == num_vector_outputs);
    for (unsigned i=0; i<num_vector_outputs; ++i)
    {
        NdArray<double>::Indices our_indices(indices);
        our_indices.push_back(0u);
        NdArray<double> array = GET_ARRAY(pResults->Lookup(r_vector_output_names[i], GetLocationInfo()));
        // Fill in the vector
        const unsigned vector_length = GetVectorSize(vector_outputs[i]);
        assert(vector_length == array.GetShape().back());
        for (unsigned j=0; j<vector_length; ++j)
        {
            array[our_indices] = GetVectorComponent(vector_outputs[i], j);
            our_indices.back()++;
        }
    }

    if (about_to_outer_loop && is_while_loop)
    {
        CreateResultViews(pResults);
    }
}
