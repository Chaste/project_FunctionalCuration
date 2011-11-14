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
