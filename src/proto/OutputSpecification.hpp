/*

Copyright (c) 2005-2015, University of Oxford.
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

#ifndef OUTPUTSPECIFICATION_HPP_
#define OUTPUTSPECIFICATION_HPP_

#include <string>
#include <boost/shared_ptr.hpp>
#include "LocatableConstruct.hpp"

class OutputSpecification;
typedef boost::shared_ptr<OutputSpecification> OutputSpecificationPtr;

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

    /** Set the presentation name for this output. */
    void SetOutputDescription(const std::string& rDescription);

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
