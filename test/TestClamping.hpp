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

#ifndef TESTCLAMPING_HPP_
#define TESTCLAMPING_HPP_

#include <cxxtest/TestSuite.h>

#include <string>
#include <boost/assign/list_of.hpp>

#include "ArrayFileReader.hpp"
#include "ProtocolRunner.hpp"
#include "ProtocolFileFinder.hpp"
#include "ProtoHelperMacros.hpp"
#include "NumericFileComparison.hpp"

#include "FileFinder.hpp"
#include "Warnings.hpp"

#include "PetscSetupAndFinalize.hpp"

typedef NdArray<double>::Range R;
typedef std::vector<R> RangeSpec;

class TestClamping : public CxxTest::TestSuite
{
public:
    void TestClampingToInitialValue() throw (Exception)
    {
        std::string dirname = "TestClampingToInitialValue";
        ProtocolFileFinder proto_file("projects/FunctionalCuration/test/protocols/test_clamping1.txt",
                                      RelativeTo::ChasteSourceRoot);
        FileFinder cellml_file("projects/FunctionalCuration/cellml/beeler_reuter_model_1977.cellml",
                               RelativeTo::ChasteSourceRoot);
        ProtocolRunner runner(cellml_file, proto_file, dirname);
        runner.RunProtocol();

        FileFinder success_file(dirname + "/success", RelativeTo::ChasteTestOutput);
        TS_ASSERT(success_file.Exists());
    }

    void TestClampingToFixedValue() throw (Exception)
    {
        std::string dirname = "TestClampingToFixedValue";
        ProtocolFileFinder proto_file("projects/FunctionalCuration/test/protocols/test_clamping2.txt",
                                      RelativeTo::ChasteSourceRoot);
        FileFinder cellml_file("projects/FunctionalCuration/cellml/beeler_reuter_model_1977.cellml",
                               RelativeTo::ChasteSourceRoot);
        ProtocolRunner runner(cellml_file, proto_file, dirname);
        runner.RunProtocol();

        FileFinder success_file(dirname + "/success", RelativeTo::ChasteTestOutput);
        TS_ASSERT(success_file.Exists());
    }

    void TestClampingToTimecourse() throw(Exception)
    {
        std::string dirname = "TestClampingToTimecourse";

        // Load protocol
        ProtocolFileFinder proto_file("projects/FunctionalCuration/protocols/timecourse_voltage_clamp.txt",
                                      RelativeTo::ChasteSourceRoot);
        FileFinder cellml_file("projects/FunctionalCuration/cellml/ten_tusscher_model_2004_epi.cellml",
                               RelativeTo::ChasteSourceRoot);
        ProtocolRunner runner(cellml_file, proto_file, dirname);

        // Load voltage data from file with ArrayReader
        FileFinder this_test(__FILE__, RelativeTo::ChasteSourceRoot);
        FileFinder data_file("data/reference_traces/one_pace_voltage.csv", this_test);

        ArrayFileReader reader;
        NdArray<double> array = reader.ReadFile(data_file);

        RangeSpec view_indices = boost::assign::list_of(R(0, 1, R::END)) // All elements from dim 0
                                                       (R(0)); // Just the first entry in second dimension
        NdArray<double> times = array[view_indices];

        view_indices = boost::assign::list_of(R(0, 1, R::END)) // All elements from dim 0
                                             (R(1)); // Just the second entry in second dimension
        NdArray<double> voltages = array[view_indices];

        // Wrap in an ArrayValue & set as protocol input
        AbstractExpressionPtr p_times = VALUE(ArrayValue, times);
        AbstractExpressionPtr p_voltages = VALUE(ArrayValue, voltages);

        // Run protocol
        runner.GetProtocol()->SetInput("time_trace", p_times);
        runner.GetProtocol()->SetInput("voltage_trace", p_voltages);
        runner.RunProtocol();

        FileFinder success_file(dirname + "/success", RelativeTo::ChasteTestOutput);
        TS_ASSERT(success_file.Exists());

        FileFinder reference_trace("data/reference_traces/tt04_I_Kr_under_AP_clamp.csv", this_test);
        FileFinder generated_trace(dirname + "/outputs_I_Kr.csv", RelativeTo::ChasteTestOutput);

        NumericFileComparison comparer(reference_trace, generated_trace);
        TS_ASSERT(comparer.CompareFiles(1e-6)); // Abs tol to account for compiler variation
    }
};

#endif // TESTCLAMPING_HPP_
