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
#define PROTO_ASSERT2(test, msg, loc)                  \
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
 * Propagate a BacktraceException.
 * Calls the context's GetLocationInfo method to determine the error's source.
 * @param call  the statement to execute that might throw an error
 */
#define PROPAGATE_BACKTRACE(call)  PROPAGATE_BACKTRACE2(call, GetLocationInfo())

#endif // BACKTRACEEXCEPTION_HPP_
