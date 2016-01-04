/*

Copyright (c) 2005-2016, University of Oxford.
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

#ifndef DEBUGPROTO_HPP_
#define DEBUGPROTO_HPP_

#include <iostream>

#include "AbstractValue.hpp"
#include "Environment.hpp"
#include "OutputFileHandler.hpp"

/**
 * Useful functions for debugging protocols.  Typically this will be used in
 * conjunction with the TRACE_PROTO macro defined below: controlling code will
 * call DebugProto::SetTraceFolder to enable full tracing, and low-level code
 * will call TRACE_PROTO at appropriate points.
 */
class DebugProto
{
public:
    /**
     * Set where tracing files should be written.
     *
     * @param rHandler  handler for the folder to write to
     */
    static void SetTraceFolder(const OutputFileHandler& rHandler);

    /**
     * Stop writing trace output to file.
     */
    static void StopTracing();

    /**
     * Whether we are currently tracing to file.
     */
    static bool IsTracing();

    /**
     * Trace the values in the given environment.
     *
     * @param rEnv  the environment to output
     */
    static void TraceEnv(const Environment& rEnv);

    /** The tracing file to write, if desired. */
    static out_stream mpTraceFile;
};


/**
 * Support for writing out arbitrary protocol values to output streams.
 * Unless writing to a file, will only write a synopsis of large values.
 *
 * @param rStream  the stream to write to
 * @param rpV  the value to write
 */
std::ostream& operator<< (std::ostream& rStream, const AbstractValuePtr& rpV);


/**
 * Stream some 'stuff' to std::out and a trace file, if set with DebugProto::SetTraceFolder.
 *
 * @param stuff  what to stream
 */
#define TRACE_PROTO(stuff)                       \
    do {                                         \
        std::cout << stuff;                      \
        if (DebugProto::IsTracing())             \
        {                                        \
            *(DebugProto::mpTraceFile) << stuff; \
        }                                        \
    } while (false)

#endif // DEBUGPROTO_HPP_
