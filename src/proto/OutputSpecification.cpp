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

#include "OutputSpecification.hpp"

#include "BacktraceException.hpp"

OutputSpecification::OutputSpecification(const std::string& rRef,
                                         const std::string& rName,
                                         const std::string& rDescription,
                                         const std::string& rUnits,
                                         const std::string& rType)
    : mRef(rRef),
      mName(rName),
      mDescription(rDescription),
      mUnits(rUnits),
      mType(rType)
{
    PROTO_ASSERT(mType == "Raw" || mType == "Post-processed",
                 "Incorrect output type '" << rType << "' specified; must be 'Raw' or 'Post-processed'.");
}

const std::string& OutputSpecification::rGetOutputRef() const
{
    return mRef;
}

const std::string& OutputSpecification::rGetOutputName() const
{
    return mName;
}

const std::string& OutputSpecification::rGetOutputDescription() const
{
    return mDescription;
}

const std::string& OutputSpecification::rGetOutputUnits() const
{
    return mUnits;
}

const std::string& OutputSpecification::rGetOutputType() const
{
    return mType;
}
