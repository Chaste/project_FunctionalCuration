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

#ifndef TESTNESTEDPROTOCOLS_HPP_
#define TESTNESTEDPROTOCOLS_HPP_

#include <cxxtest/TestSuite.h>

#include "ProtocolRunner.hpp"
#include "OutputFileHandler.hpp"
#include "ProtoHelperMacros.hpp"

class TestNestedProtocols : public CxxTest::TestSuite
{
public:
    void TestSimpleExample() throw (Exception)
    {
        std::string dirname = "TestNestedProtocols";
        std::string model_name = "luo_rudy_1991";
        FileFinder cellml_file("projects/FunctionalCuration/cellml/" + model_name + ".cellml", RelativeTo::ChasteSourceRoot);
        FileFinder proto_xml_file("projects/FunctionalCuration/test/protocols/test_nested_protocol.xml", RelativeTo::ChasteSourceRoot);

        ProtocolRunner runner(cellml_file, proto_xml_file, dirname);
        runner.RunProtocol();
        FileFinder success_file(dirname + "/success", RelativeTo::ChasteTestOutput);
        TS_ASSERT(success_file.Exists());

        // Check results
        const Environment& r_outputs = runner.GetProtocol()->rGetOutputsCollection();
        NdArray<double> peak_V = GET_ARRAY(r_outputs.Lookup("peak_voltage", "test"));
        NdArray<double> apd90 = GET_ARRAY(r_outputs.Lookup("apd90", "test"));
        TS_ASSERT_EQUALS(peak_V.GetNumDimensions(), 1u);
        TS_ASSERT_EQUALS(peak_V.GetShape()[0], 2u);
        TS_ASSERT_EQUALS(peak_V.GetNumElements(), 2u);
        TS_ASSERT_EQUALS(apd90.GetNumDimensions(), 1u);
        TS_ASSERT_EQUALS(apd90.GetShape()[0], 2u);
        TS_ASSERT_EQUALS(apd90.GetNumElements(), 2u);
        NdArray<double>::Indices idxs = peak_V.GetIndices();
        TS_ASSERT_DELTA(peak_V[idxs], 46.959, 1e-3);
        TS_ASSERT_DELTA(apd90[idxs], 359.015, 1e-3);
        peak_V.IncrementIndices(idxs);
        TS_ASSERT_DELTA(peak_V[idxs], 46.939, 1e-3);
        TS_ASSERT_DELTA(apd90[idxs], 361.847, 1e-3);
    }
};

#endif // TESTNESTEDPROTOCOLS_HPP_
