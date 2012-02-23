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

#ifndef NESTEDPROTOCOL_HPP_
#define NESTEDPROTOCOL_HPP_

#include "AbstractSimulation.hpp"

#include <map>
#include <vector>
#include <string>

#include "Protocol.hpp"
#include "AbstractExpression.hpp"
#include "NdArray.hpp"

/**
 * A "simulation" class that wraps an entire protocol definition.
 * Really this is more akin to a model, but we don't have a framework for
 * multiple models yet.  We therefore have a 'fake' stepper, and just run
 * the wrapped protocol once when run.  We override most of the mechanism
 * for storing results to cope with nested protocol outputs that can be
 * arrays themselves.
 */
class NestedProtocol : public AbstractSimulation
{
public:
    /**
     * Create a new nested protocol.
     *
     * @param pProtocol  the protocol itself
     * @param rInputSpecifications  expressions setting the value of some protocol inputs
     * @param rOutputSpecifications  which protocol outputs are of interest
     */
    NestedProtocol(ProtocolPtr pProtocol,
                   const std::map<std::string, AbstractExpressionPtr>& rInputSpecifications,
                   const std::vector<std::string>& rOutputSpecifications);

    /**
     * Run a simulation, filling in the results if requested.
     *
     * We create the results arrays at the end of the first time this is called, since their
     * sizes won't be known until then.
     *
     * @param pResults  an Environment containing results arrays to fill in, or an empty pointer
     */
    void Run(EnvironmentPtr pResults);

    /**
     * Set the model being simulated by the nested protocol.
     *
     * @param pCell  the cell model the protocol is being run on
     */
    void SetCell(boost::shared_ptr<AbstractCardiacCellInterface> pCell);

private:
    /** The nested protocol itself. */
    ProtocolPtr mpProtocol;

    /** Expressions for setting some protocol inputs prior to each execution. */
    std::map<std::string, AbstractExpressionPtr> mInputSpecifications;

    /** Which protocol outputs are of interest. */
    std::vector<std::string> mOutputSpecifications;

    /** The shapes of the results of the nested protocol. */
    std::map<std::string, NdArray<double>::Extents> mOutputShapes;
};

#endif // NESTEDPROTOCOL_HPP_
