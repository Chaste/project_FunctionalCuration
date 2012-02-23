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

#ifndef PROTOCOLRUNNER_HPP_
#define PROTOCOLRUNNER_HPP_

#include <string>

#include "FileFinder.hpp"
#include "OutputFileHandler.hpp"
#include "Protocol.hpp"

/**
 * This class encapsulates the typical logic required to load a model and run a protocol on it.
 */
class ProtocolRunner
{
public:
    /**
     * Create a protocol runner, providing the paths to model and protocol definitions.
     * The constructor will perform the model modifications and code generation for the model.
     *
     * @param rModelFile  the model CellML file
     * @param rProtoXmlFile  the protocol definition file
     * @param rOutputFolder  where to put generated files and protocol outputs
     * @param optimiseModel  whether to apply PyCml optimisations to the model
     */
    ProtocolRunner(const FileFinder& rModelFile,
                   const FileFinder& rProtoXmlFile,
                   const std::string& rOutputFolder,
                   bool optimiseModel=false);

    /**
     * Run the protocol and write out results.
     */
    void RunProtocol();

    /**
     * Get the protocol object, to enable further setup, querying or analysis.
     */
    ProtocolPtr GetProtocol();

private:
    /** The handler for file output. */
    OutputFileHandler mHandler;

    /** The protocol object. */
    ProtocolPtr mpProtocol;
};

#endif // PROTOCOLRUNNER_HPP_
