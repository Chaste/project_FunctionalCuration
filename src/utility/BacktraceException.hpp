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

#ifndef BACKTRACEEXCEPTION_HPP_
#define BACKTRACEEXCEPTION_HPP_

#include <boost/shared_ptr.hpp>

#include "Exception.hpp"

#include <sstream>

/**
 * A specialised form of exception for use in interpreters that supports "backtrace" functionality.
 *
 * The header also supplies several helper macros to aid in its use.  These all support specifying
 * the error message as a stream expression, so that callers can do, for example:
 * \code
 *   PROTO_EXCEPTION("Expecting " << i << " values; got " << j << " instead!");
 * \endcode
 */
class BacktraceException : public Exception
{
public:
    /**
     * Base constructor for use when an error is first found.
     *
     * @param rMessage  error message
     * @param rLocInfo  location in the interpreted code that caused the error
     * @param rChasteFileName  which Chaste source file created this error object
     * @param chasteLineNumber  line in the above file
     */
    BacktraceException(const std::string& rMessage, const std::string& rLocInfo,
                       const std::string& rChasteFileName, unsigned chasteLineNumber);

    /**
     * Propagating constructor for use when an exception has been thrown by called code.
     *
     * @param rParentException  the error thrown by the called code
     * @param rLocInfo  location in the interpreted code that called this code
     * @param rChasteFileName  which Chaste source file created this error object
     * @param chasteLineNumber  line in the above file
     */
    BacktraceException(const BacktraceException& rParentException, const std::string& rLocInfo,
                       const std::string& rChasteFileName, unsigned chasteLineNumber);

    /**
     * Get the detailed error backtrace.
     *
     * @param includeHeader  whether to include an initial header line
     */
    std::string GetBacktrace(bool includeHeader=true) const;

private:
    /** The exception which caused this to be thrown, if any. */
    boost::shared_ptr<BacktraceException> mpParentException;

    /** Information about where in the protocol the exception was triggered. */
    std::string mLocInfo;

    /** The actual error message. */
    std::string mErrorMessage;

    /** The header line for backtraces. */
    static std::string mHeader;
};

/**
 * Throw a BacktraceException.
 * @param msg  the error message to use, as a streamed expression
 * @param loc  the source location that caused the error
 */
#define PROTO_EXCEPTION2(msg, loc)               \
    do {                                         \
        std::stringstream msg_stream;            \
        msg_stream << msg;                       \
        throw BacktraceException(msg_stream.str(), loc, __FILE__, __LINE__); \
    } while (false)

/**
 * Throw a BacktraceException.  Calls the context's GetLocationInfo method to determine the error's source.
 * @param msg  the error message to use, as a streamed expression
 */
#define PROTO_EXCEPTION(msg)  PROTO_EXCEPTION2(msg, GetLocationInfo())

/**
 * Check some property holds, and throw a BacktraceException if not.
 * @param test  the property to check
 * @param msg  the error message to use if the test fails, as a streamed expression
 */
#define PROTO_ASSERT(test, msg)                  \
    if (!(test)) PROTO_EXCEPTION(msg)

/**
 * Check some property holds, and throw a BacktraceException if not.
 * @param test  the property to check
 * @param msg  the error message to use if the test fails, as a streamed expression
 * @param loc  the source location that caused the error
 */
#define PROTO_ASSERT2(test, msg, loc)            \
    if (!(test)) PROTO_EXCEPTION2(msg, loc)

/**
 * Propagate a BacktraceException.
 * @param call  the statement to execute that might throw an error
 * @param loc  the source location of the call
 */
#define PROPAGATE_BACKTRACE2(call, loc)          \
    do {                                         \
        try {                                    \
            call;                                \
        } catch (const BacktraceException& rE) { \
            throw BacktraceException(rE, loc, __FILE__, __LINE__); \
        }                                        \
    } while (false)

/**
 * Propagate a BacktraceException, with environment tracing.
 * @param call  the statement to execute that might throw an error
 * @param env  the caller's environment
 */
#define PROPAGATE_BACKTRACE_ENV(call, env)       \
    do {                                         \
        try {                                    \
            call;                                \
        } catch (const BacktraceException& rE) { \
            if (DebugProto::IsTracing()) {       \
                TRACE_PROTO("Variables defined at " << GetLocationInfo() << ":\n"); \
                DebugProto::TraceEnv(env);                                          \
            }                                                                       \
            throw BacktraceException(rE, GetLocationInfo(), __FILE__, __LINE__);    \
        }                                        \
    } while (false)

/**
 * Propagate a BacktraceException.
 * Calls the context's GetLocationInfo method to determine the error's source.
 * @param call  the statement to execute that might throw an error
 */
#define PROPAGATE_BACKTRACE(call)  PROPAGATE_BACKTRACE2(call, GetLocationInfo())

#endif // BACKTRACEEXCEPTION_HPP_
