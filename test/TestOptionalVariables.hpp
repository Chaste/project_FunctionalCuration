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

#ifndef TESTOPTIONALVARIABLES_HPP_
#define TESTOPTIONALVARIABLES_HPP_

#include <string>
#include <cxxtest/TestSuite.h>

#include "ProtocolRunner.hpp"
#include "ProtocolLanguage.hpp"
#include "ProtoHelperMacros.hpp"

#include "FileFinder.hpp"
#include "OutputFileHandler.hpp"

#include "FileComparison.hpp"

class TestOptionalVariables : public CxxTest::TestSuite
{
public:
    void TestClampingWithOldModel() throw (Exception)
    {
        std::string dirname = "TestOptionalVariables_TestClampingWithOldModel";
        ProtocolFileFinder proto_file("projects/FunctionalCuration/test/protocols/compact/test_INa_IV_curves.txt", RelativeTo::ChasteSourceRoot);
        FileFinder cellml_file("projects/FunctionalCuration/cellml/beeler_reuter_model_1977.cellml", RelativeTo::ChasteSourceRoot);
        ProtocolRunner runner(cellml_file, proto_file, dirname);
        runner.RunProtocol();
        FileFinder success_file(dirname + "/success", RelativeTo::ChasteTestOutput);
        TS_ASSERT(success_file.Exists());

        // Check the key outputs haven't changed.
        const Environment& r_outputs = runner.GetProtocol()->rGetOutputsCollection();
        NdArray<double> arr_to_check = GET_ARRAY(r_outputs.Lookup("normalised_peak_currents"));
        TS_ASSERT_EQUALS(arr_to_check.GetNumElements(), 19u);
        TS_ASSERT_EQUALS(arr_to_check.GetNumDimensions(), 1u);
        const double expected[] = {1.0,0.999998,0.999981,0.999906,0.999564,0.999062,0.99795,0.995344,0.988466,0.966859,
                                   0.887918,0.624599,0.192671,0.0186169,0.00193235,0.00116954,0.00114086,0.00113976,0.00113976};
        NdArray<double>::ConstIterator it = arr_to_check.Begin();
        for (unsigned i=0; i<19u; i++)
        {
            TS_ASSERT_DELTA(*it++, expected[i], 1e-3);
        }
    }

    void TestDefaultExpressions() throw (Exception)
    {
        std::string dirname = "TestOptionalVariables_TestDefaultExpressions";
        ProtocolFileFinder proto_file("projects/FunctionalCuration/test/protocols/compact/INa_IV_curves.txt", RelativeTo::ChasteSourceRoot);
        FileFinder cellml_file("projects/FunctionalCuration/cellml/beeler_reuter_model_1977.cellml", RelativeTo::ChasteSourceRoot);
        ProtocolRunner runner(cellml_file, proto_file, dirname);
        runner.RunProtocol();
        FileFinder success_file(dirname + "/success", RelativeTo::ChasteTestOutput);
        TS_ASSERT(success_file.Exists());

        // Check the key outputs haven't changed.
        const Environment& r_outputs = runner.GetProtocol()->rGetOutputsCollection();
        NdArray<double> arr_to_check = GET_ARRAY(r_outputs.Lookup("normalised_peak_currents"));
        TS_ASSERT_EQUALS(arr_to_check.GetNumElements(), 19u);
        TS_ASSERT_EQUALS(arr_to_check.GetNumDimensions(), 1u);
        const double expected[] = {1.0,0.999998,0.999981,0.999906,0.999564,0.999062,0.99795,0.995344,0.988466,0.966859,
                                   0.887918,0.624599,0.192671,0.0186169,0.00193235,0.00116954,0.00114086,0.00113976,0.00113976};
        NdArray<double>::ConstIterator it = arr_to_check.Begin();
        for (unsigned i=0; i<19u; i++)
        {
            TS_ASSERT_DELTA(*it++, expected[i], 1e-3);
        }
    }
};

#endif // TESTOPTIONALVARIABLES_HPP_
