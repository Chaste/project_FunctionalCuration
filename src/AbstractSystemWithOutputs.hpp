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
#ifndef ABSTRACTSYSTEMWITHOUTPUTS_HPP_
#define ABSTRACTSYSTEMWITHOUTPUTS_HPP_

#include <vector>
#include <string>

/**
 * An additional base class for cells that have outputs specified via a protocol.
 * 
 * The template parameter is the type used by the cell for vectors of real numbers.
 * It will be either std::vector<double> (for AbstractCardiacCell subclasses) or
 * N_Vector (for AbstractCvodeCell subclasses).
 */
template<typename VECTOR>
class AbstractSystemWithOutputs
{
public:
    /**
     * Get the number of system outputs.
     */
    unsigned GetNumberOfOutputs() const;
    
    /**
     * Compute the system outputs from the given system state.
     * Uses the current values for the parameters.
     * 
     * \todo  Is using current param values safe for all protocols?  Will need to watch out for this.
     * 
     * @param time  the time at which to compute the outputs
     * @param rState  values for the state variables
     */
    VECTOR ComputeOutputs(double time,
                          const VECTOR& rState);
    
    /**
     * Get the vector of output names.
     */
    std::vector<std::string> GetOutputNames() const;

    /**
     * Get the vector of output units.
     */
    std::vector<std::string> GetOutputUnits() const;
    
    /**
     * Get the index of an output, given its name.
     *
     * @param rName  the name of an output.
     */
    unsigned GetOutputIndex(const std::string& rName) const;
    
    /**
     * What types of variable can be system outputs.
     */
    enum OutputTypes
    {
        FREE,      ///< The free variable (usually time)
        STATE,     ///< A state variable
        PARAMETER, ///< A modifiable parameter
        DERIVED    ///< A derived quantity
    };
    
    /** Virtual destructor to force this class to be polymorphic. */
    virtual ~AbstractSystemWithOutputs();
    
protected:
    /**
     * Information encoding which variables in the system are outputs.
     * Their order matters.
     * This must be set up by subclass constructors, if the system has outputs.
     */
    std::vector<std::pair<unsigned, OutputTypes> > mOutputsInfo;
};

#endif /*ABSTRACTSYSTEMWITHOUTPUTS_HPP_*/
