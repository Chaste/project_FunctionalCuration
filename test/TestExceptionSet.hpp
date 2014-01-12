/*

Copyright (c) 2005-2014, University of Oxford.
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

#ifndef TESTEXCEPTIONSET_HPP_
#define TESTEXCEPTIONSET_HPP_

#include <cxxtest/TestSuite.h>

#include "ExceptionSet.hpp"

class TestExceptionSet : public CxxTest::TestSuite
{
public:
    void TestMultipleExceptions() throw (Exception)
    {
        std::vector<Exception> errors;
        for (unsigned i=0; i<3; ++i)
        {
            try
            {
                EXCEPTION("Error " << i);
            }
            catch (const Exception& e)
            {
                errors.push_back(e);
            }
        }
        ExceptionSet overall_error(errors, "file", 0u);

        TS_ASSERT_THROWS_CONTAINS(throw overall_error, "Summary of errors that occurred (see earlier for details):\n");
        TS_ASSERT_THROWS_CONTAINS(throw overall_error, "Error 0\n");
        TS_ASSERT_THROWS_CONTAINS(throw overall_error, "Error 1\n");
        TS_ASSERT_THROWS_CONTAINS(throw overall_error, "Error 2");

        TS_ASSERT_THROWS_CONTAINS(THROW_EXCEPTIONS(errors), "Summary of errors that occurred (see earlier for details):\n");
        TS_ASSERT_THROWS_CONTAINS(THROW_EXCEPTIONS(errors), "Error 0\n");
        TS_ASSERT_THROWS_CONTAINS(THROW_EXCEPTIONS(errors), "Error 1\n");
        TS_ASSERT_THROWS_CONTAINS(THROW_EXCEPTIONS(errors), "Error 2");

        // It's a summary even for 1 error
        errors.resize(1u, errors.front());
        TS_ASSERT_THROWS_CONTAINS(THROW_REPORTED_EXCEPTIONS(errors), "Summary of errors that occurred (see earlier for details):\n");
    }
};

#endif // TESTEXCEPTIONSET_HPP_
