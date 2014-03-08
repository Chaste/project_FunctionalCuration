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

#ifndef TESTACCESSINGSTATEVECTOR_HPP_
#define TESTACCESSINGSTATEVECTOR_HPP_

#include <cxxtest/TestSuite.h>
#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>

#include "ProtocolRunner.hpp"
#include "OutputFileHandler.hpp"

#include "ProtoHelperMacros.hpp"
#include "FileComparison.hpp"
#include "NumericFileComparison.hpp"

class TestAccessingStateVector : public CxxTest::TestSuite
{
public:
    void TestXmlSyntax() throw (Exception)
    {
        std::string dirname = "TestAccessingStateVector";
        ProtocolFileFinder proto_xml_file("projects/FunctionalCuration/protocols/xml/GraphState.xml", RelativeTo::ChasteSourceRoot);
        DoTestGraphState(dirname, proto_xml_file);
    }

    void TestCompactSyntax() throw (Exception)
    {
        std::string dirname = "TestAccessingStateVector_Compact";
        ProtocolFileFinder proto_xml_file("projects/FunctionalCuration/protocols/GraphState.txt", RelativeTo::ChasteSourceRoot);
        DoTestGraphState(dirname, proto_xml_file);
    }

private:
    void DoTestGraphState(const std::string& rDirName, const ProtocolFileFinder& rProtoFile) throw (Exception)
    {
        std::string model_name = "luo_rudy_1991";
        FileFinder cellml_file("projects/FunctionalCuration/cellml/" + model_name + ".cellml", RelativeTo::ChasteSourceRoot);

        ProtocolRunner runner(cellml_file, rProtoFile, rDirName);
        runner.RunProtocol();
        FileFinder success_file(rDirName + "/success", RelativeTo::ChasteTestOutput);
        TS_ASSERT(success_file.Exists());

        // Double-check transpose
        NdArray<double> state_trans = GET_ARRAY(runner.GetProtocol()->rGetOutputsCollection().Lookup("state", "test"));
        NdArray<double> state_raw = GET_ARRAY(runner.GetProtocol()->rGetOutputsCollection().Lookup("raw_state", "test"));
        for (NdArray<double>::ConstIterator it=state_raw.Begin(); it != state_raw.End(); ++it)
        {
            NdArray<double>::Indices idxs(2);
            idxs[0] = it.rGetIndices()[1];
            idxs[1] = it.rGetIndices()[0];
            TS_ASSERT_EQUALS(*it, state_trans[idxs]);
        }

        // Compare the results with original data
        FileFinder ref_dir("projects/FunctionalCuration/test/data/TestAccessingStateVector", RelativeTo::ChasteSourceRoot);
        FileFinder out_dir(rDirName, RelativeTo::ChasteTestOutput);

        // Test metadata files
        std::vector<std::string> filenames = boost::assign::list_of("-contents.csv")("-default-plots.csv");
        BOOST_FOREACH(std::string filename, filenames)
        {
            FileFinder ref_file("outputs" + filename, ref_dir);
            FileFinder test_file("outputs" + filename, out_dir);
            FileComparison comparer(test_file,ref_file);
            TS_ASSERT( comparer.CompareFiles() );
        }

        filenames = boost::assign::list_of("outputs_raw_state")("outputs_state");
        BOOST_FOREACH(std::string output_name, filenames)
        {
            FileFinder ref_output(output_name + ".csv", ref_dir);
            FileFinder test_output(output_name + ".csv", out_dir);
            NumericFileComparison comp(test_output.GetAbsolutePath(), ref_output.GetAbsolutePath());
            TS_ASSERT(comp.CompareFiles(1e-4, 0, false));
        }

    }
};

#endif // TESTACCESSINGSTATEVECTOR_HPP_
