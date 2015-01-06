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

#ifndef MULTIPROTOCOLRUNNER_HPP_
#define MULTIPROTOCOLRUNNER_HPP_

#include <vector>

#include "FileFinder.hpp"
#include "ProtocolFileFinder.hpp"

/**
 * This class wraps the ProtocolRunner to support easily running multiple protocols on multiple models.
 * It provides the functionality exposed by the main Functional Curation executable, and hence can be
 * configured using command-line arguments for convenience.  Default lists of models and protocols
 * may also be supplied by C++ code, and will be used if command-line arguments are not present.
 *
 * The arguments read are
 *  - --models <path/to/model/1.cellml> <path/to/model/2.cellml> ...
 *  - --protocols <path/to/proto/1.xml> <path/to/proto/2.txt> ...
 *    Relative paths for both models and protocols are interpreted relative to the current folder.
 *  - --png - if present, save figures as PNG format as well as EPS
 *  - --output-dir - base folder to save protocol outputs under.
 *    Results will be placed in a subfolder hierarchy named after the model and protocol leaf names.
 *    If the output-dir is a relative path, it will be treated relative to CHASTE_TEST_OUTPUT.
 */
class MultiProtocolRunner
{
public:
    /**
     * Set the models to process if none are specified on the command line.
     * @param rModelPaths  the default models to process
     */
    void SetDefaultModels(const std::vector<FileFinder>& rModelPaths);

    /**
     * Set the protocols to run if none are specified on the command line.
     * @param rProtocolPaths  the default protocols to run
     */
    void SetDefaultProtocols(const std::vector<ProtocolFileFinder>& rProtocolPaths);

    /**
     * Run all specified protocols on all specified models.
     *
     * If running on multiple processes, PetscTools::IsolateProcesses will be used to process
     * models in parallel.
     */
    void RunProtocols();

private:
    /** The default models to process. */
    std::vector<FileFinder> mDefaultModels;

    /** The default protocols to run. */
    std::vector<ProtocolFileFinder> mDefaultProtocols;
};

#endif // MULTIPROTOCOLRUNNER_HPP_
