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
