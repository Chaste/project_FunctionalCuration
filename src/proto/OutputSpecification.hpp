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

#ifndef OUTPUTSPECIFICATION_HPP_
#define OUTPUTSPECIFICATION_HPP_

#include <string>
#include "LocatableConstruct.hpp"

/**
 * This little class describes an output of a protocol, giving details such as
 * its name, units, and whether it is a raw model output or post-processed.
 */
class OutputSpecification : public LocatableConstruct
{
public:
    /**
     * Main constructor.
     * @param rRef  the (possibly prefixed) name to lookup to find this output
     * @param rName  the (non-prefixed) internal name used for the output in the protocol
     * @param rDescription  a longer human-friendly version of the name
     * @param rUnits  the output's units
     * @param rType  its type - must be "Raw" or "Post-processed"
     */
    OutputSpecification(const std::string& rRef,
                        const std::string& rName,
                        const std::string& rDescription,
                        const std::string& rUnits,
                        const std::string& rType);

    /** Get the name to lookup to find this output. */
    const std::string& rGetOutputRef() const;

    /** Get the name of this output used in the protocol. */
    const std::string& rGetOutputName() const;

    /** Get the presentation name for this output. */
    const std::string& rGetOutputDescription() const;

    /** Get the units of this output. */
    const std::string& rGetOutputUnits() const;

    /** Get the type of this output: "Raw" or "Post-processed". */
    const std::string& rGetOutputType() const;

private:
    /** The output variable reference. */
    std::string mRef;

    /** The output name. */
    std::string mName;

    /** The output description. */
    std::string mDescription;

    /** The units of this output. */
    std::string mUnits;

    /** The type of this output. */
    std::string mType;
};

#endif // OUTPUTSPECIFICATION_HPP_
