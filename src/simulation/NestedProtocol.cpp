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

#include <boost/foreach.hpp>
#include "OutputFileHandler.hpp"
#include "BacktraceException.hpp"


NestedProtocol::NestedProtocol(ProtocolPtr pProtocol,
                               const std::map<std::string, AbstractExpressionPtr>& rInputSpecifications,
                               const std::vector<std::string>& rOutputSpecifications)
    : AbstractSimulation(boost::shared_ptr<AbstractSystemWithOutputs>(), AbstractStepperPtr()),
      mpProtocol(pProtocol),
      mInputSpecifications(rInputSpecifications),
      mOutputSpecifications(rOutputSpecifications)
{
}


void NestedProtocol::SetModel(boost::shared_ptr<AbstractSystemWithOutputs> pModel)
{
    PROTO_ASSERT(!mOutputSpecifications.empty(), "No results are being retained from the nested protocol.");
    AbstractSimulation::SetModel(pModel);
    mpProtocol->SetModel(pModel);
}


void NestedProtocol::Run(EnvironmentPtr pResults)
{
    // Set protocol inputs, temporarily making the environment in which they are evaluated
    // delegate to our environment (bit of a hack really!)
    typedef std::pair<const std::string, AbstractExpressionPtr> StringExprPair;
    mpProtocol->rGetInputsEnvironment().SetDelegateeEnvironment(rGetEnvironment().GetAsDelegatee());
    BOOST_FOREACH(StringExprPair& r_input, mInputSpecifications)
    {
        mpProtocol->SetInput(r_input.first, r_input.second);
    }
    mpProtocol->rGetInputsEnvironment().SetDelegateeEnvironment(EnvironmentCPtr());
    // Re-run the imported protocol's library, since it may depend on protocol inputs
    mpProtocol->InitialiseLibrary(true);
    if (pResults && mpOutputHandler && GetTrace())
    {
        // Make the nested protocol save all its results each run
        unsigned run_number = 1u;
        const unsigned num_local_dims = this->rGetSteppers().size();
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
    // Add only the selected protocol outputs
    EnvironmentPtr p_selected_outputs(new Environment);
    const Environment& r_proto_outputs = mpProtocol->rGetOutputsCollection();
    BOOST_FOREACH(const std::string& r_output_name, mOutputSpecifications)
    {
        AbstractValuePtr p_value = r_proto_outputs.Lookup(r_output_name, GetLocationInfo());
        p_selected_outputs->DefineName(r_output_name, p_value, GetLocationInfo());
    }
    AddIterationOutputs(pResults, p_selected_outputs);
}
