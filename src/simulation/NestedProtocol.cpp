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
    if (pResults && mpOutputHandler && GetTrace())
    {
        // Make the nested protocol save all its results each run
        unsigned run_number = 1u;
        const unsigned num_local_dims = this->rGetSteppers().size() - 1u;
        for (unsigned i=0; i<num_local_dims; ++i)
        {
            run_number *= (rGetSteppers()[i]->GetCurrentOutputNumber() + 1u);
        }
        run_number--;
        std::stringstream run_dir;
        run_dir << "run_" << run_number;
        OutputFileHandler this_run(mpOutputHandler->FindFile(run_dir.str()));
        mpProtocol->SetOutputFolder(this_run);
    }
    // Run the nested protocol
    mpProtocol->Run();
    if (pResults && mpOutputHandler && GetTrace())
    {
        mpProtocol->WriteToFile("outputs");
    }
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
                AbstractValuePtr p_result = boost::make_shared<ArrayValue>(result);
                p_result->SetUnits(p_output->GetUnits());
                pResults->DefineName(r_output_name, p_result, GetLocationInfo());
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
