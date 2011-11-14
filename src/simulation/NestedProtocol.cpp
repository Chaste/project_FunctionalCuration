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

#include "NestedProtocol.hpp"

#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include "BacktraceException.hpp"
#include "ProtoHelperMacros.hpp"
#include "VectorStreaming.hpp"

/**
 * A fake stepper that takes no steps.
 */
class FakeStepper : public AbstractStepper
{
public:
    /**
     * Default constructor.
     */
    FakeStepper()
        : AbstractStepper("", "")
    {
        this->mCurrentStep = 0u;
        SetCurrentOutputPoint(0.0);
    }

    /**
     * This stepper always has size 0.
     */
    unsigned GetNumberOfOutputPoints() const
    {
        return 0u;
    }

    /**
     * The end point is fixed.
     */
    bool IsEndFixed() const
    {
        return true;
    }

    /**
     * No-op.
     */
    void Initialise()
    {}

    /**
     * Reset this stepper to its starting point.
     */
    virtual void Reset()
    {
        this->mCurrentStep = 0u;
    }

    /**
     * Does nothing and returns zero.
     */
    double Step()
    {
//        this->mCurrentStep++;
        return 0.0;
    }

};


NestedProtocol::NestedProtocol(ProtocolPtr pProtocol,
                               const std::map<std::string, AbstractExpressionPtr>& rInputSpecifications,
                               const std::vector<std::string>& rOutputSpecifications)
    : AbstractSimulation(boost::shared_ptr<AbstractCardiacCellInterface>(),
                         AbstractStepperPtr(new FakeStepper)),
      mpProtocol(pProtocol),
      mInputSpecifications(rInputSpecifications),
      mOutputSpecifications(rOutputSpecifications)
{
    this->mpStepper->SetLocationInfo(GetLocationInfo());
}


void NestedProtocol::SetCell(boost::shared_ptr<AbstractCardiacCellInterface> pCell)
{
    PROTO_ASSERT(!mOutputSpecifications.empty(), "No results are being retained from the nested protocol.");
    AbstractSimulation::SetCell(pCell);
    mpProtocol->SetModel(pCell);
}


void NestedProtocol::Run(EnvironmentPtr pResults)
{
    // Set protocol inputs, temporarily making the environment in which they are evaluated
    // delegate to our environment
    typedef std::pair<const std::string, AbstractExpressionPtr> StringExprPair;
    mpProtocol->rGetInputsEnvironment().SetDelegateeEnvironment(rGetEnvironment().GetAsDelegatee());
    BOOST_FOREACH(StringExprPair& r_input, mInputSpecifications)
    {
        mpProtocol->SetInput(r_input.first, r_input.second);
    }
    mpProtocol->rGetInputsEnvironment().SetDelegateeEnvironment(EnvironmentCPtr());
    // Run the nested protocol
    mpProtocol->Run();
    if (pResults)
    {
        bool first_run = (pResults->GetNumberOfDefinitions() == 0u);
        const Environment& r_outputs = mpProtocol->rGetOutputsCollection();
        BOOST_FOREACH(const std::string& r_output_name, mOutputSpecifications)
        {
            AbstractValuePtr p_output = r_outputs.Lookup(r_output_name, GetLocationInfo());
            PROTO_ASSERT(p_output->IsArray(),
                         "Nested protocol produced non-array output " << r_output_name << ".");
            const NdArray<double> output_array = GET_ARRAY(p_output);
            const NdArray<double>::Extents output_shape = output_array.GetShape();
            const unsigned num_local_dims = this->rGetSteppers().size() - 1u;
            NdArray<double> result_array;
            // Create output array, if not already done
            if (first_run)
            {
                mOutputShapes[r_output_name] = output_shape;
                NdArray<double>::Extents shape(num_local_dims + output_shape.size());
                for (unsigned i=0; i<num_local_dims; i++)
                {
                    shape[i] = this->rGetSteppers()[i]->GetNumberOfOutputPoints();
                }
                std::copy(output_shape.begin(), output_shape.end(), shape.begin() + num_local_dims);
                NdArray<double> result(shape);
                result_array = result;
                pResults->DefineName(r_output_name, boost::make_shared<ArrayValue>(result), GetLocationInfo());
            }
            else
            {
                // Check the sub array shape matches the original run
                PROTO_ASSERT(output_shape == mOutputShapes[r_output_name],
                             "All runs of a nested protocol must produce outputs with the same shape; output "
                             << r_output_name << " with shape now " << output_shape
                             << " does not match the original run of shape " << mOutputShapes[r_output_name] << ".");
                result_array = GET_ARRAY(pResults->Lookup(r_output_name, GetLocationInfo()));
            }
            // Add protocol output into result array
            NdArray<double>::Indices idxs = result_array.GetIndices();
            for (unsigned i=0; i<num_local_dims; i++)
            {
                idxs[i] = rGetSteppers()[i]->GetCurrentOutputNumber();
            }
            NdArray<double>::Iterator result_it(idxs, result_array);
            std::copy(output_array.Begin(), output_array.End(), result_it);
        }
    }
}
