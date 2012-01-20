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

#ifndef LOCATABLECONSTRUCT_HPP_
#define LOCATABLECONSTRUCT_HPP_

#include <string>

/**
 * Mixin base class for objects that need to keep track of a source code location.
 */
class LocatableConstruct
{
public:
    /**
     * Constructor.  If no location is given, an anonymous location string is used; SetLocationInfo
     * should be called later.
     *
     * @param rLocationInfo  our location, formatted like "<file>:<line>:<col>  <source code line>"
     */
    LocatableConstruct(const std::string& rLocationInfo="<anon>");

    /**
     * Return information about where this construct is located within the protocol definition.
     *
     * Once parsing etc. is implemented, this will look something like
     *     "<file>:<line>:<col>  <source code line>"
     */
    const std::string& GetLocationInfo() const;

    /**
     * Get whether to trace this object.  If tracing has been requested, some sub-classes will output
     * a status message or similar when the object is used/evaluated.
     */
    bool GetTrace() const;

    /**
     * Specify the location of this construct within the source code.
     *
     * @param rLocationInfo  our location, formatted like "<file>:<line>:<col>  <source code line>"
     */
    void SetLocationInfo(const std::string& rLocationInfo);

    /**
     * Set whether to trace this object.
     * @param trace
     */
    void SetTrace(bool trace=true);

private:
    /** The location of this construct. */
    std::string mLocationInfo;

    /** Whether to trace this object. */
    bool mTrace;
};


#endif // LOCATABLECONSTRUCT_HPP_
