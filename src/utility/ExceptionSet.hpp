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

#ifndef EXCEPTIONSET_HPP_
#define EXCEPTIONSET_HPP_

#include <vector>
#include "Exception.hpp"

/**
 * A exception subclass that collects multiple errors together into a single report.
 * Useful if errors aren't immediately fatal, but you still want them reported.
 */
class ExceptionSet : public Exception
{
public:
    /**
     * Create a new exception set.
     *
     * @param rComponentErrors  the errors making up this collection
     * @param rFileName  the source file throwing the collection
     * @param lineNumber  the line number where the collection is thrown
     * @param includeBacktraces  whether to include backtraces if present, or just give the actual error messages
     */
    ExceptionSet(const std::vector<Exception>& rComponentErrors,
                 const std::string& rFileName,
                 unsigned lineNumber,
                 bool includeBacktraces=true);

    /**
     * Extract the last non-empty line from an exception message.
     * Especially useful for getting the actual issue from a BacktraceException.
     *
     * @param rError  the error object
     * @return  the one-line error message
     */
    static std::string ExtractShortMessage(const Exception& rError);
};

/**
 * Convenience macro for reporting a collection of errors.
 * @param errors  the errors making up this collection
 */
#define THROW_EXCEPTIONS(errors)  throw ExceptionSet(errors, __FILE__, __LINE__)

/**
 * Convenience macro for reporting a collection of errors in brief (i.e. without backtraces).
 * @param errors  the errors making up this collection
 */
#define THROW_REPORTED_EXCEPTIONS(errors)  throw ExceptionSet(errors, __FILE__, __LINE__, false)

#endif // EXCEPTIONSET_HPP_
