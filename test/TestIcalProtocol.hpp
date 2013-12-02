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

#ifndef TESTICALPROTOCOL_HPP_
#define TESTICALPROTOCOL_HPP_

#include <string>
#include <cxxtest/TestSuite.h>

#include "ProtocolRunner.hpp"
#include "ProtocolLanguage.hpp"
#include "ProtoHelperMacros.hpp"

#include "FileFinder.hpp"
#include "OutputFileHandler.hpp"

#include "FileComparison.hpp"

class TestIcalProtocol : public CxxTest::TestSuite
{
public:
    void TestXmlSyntax() throw (Exception)
    {
        std::string dirname = "TestICaLProtocolOutputs";
        ProtocolFileFinder proto_xml_file("projects/FunctionalCuration/test/protocols/ICaL.xml", RelativeTo::ChasteSourceRoot);
        DoTestShortIcal(dirname, proto_xml_file, "fox_mcharg_gilmour_2002");
    }

    void TestCompactSyntax() throw (Exception)
    {
        std::string dirname = "TestICaLProtocolOutputs_Compact";
        ProtocolFileFinder proto_xml_file("projects/FunctionalCuration/test/protocols/compact/ICaL.txt", RelativeTo::ChasteSourceRoot);
        DoTestShortIcal(dirname, proto_xml_file, "fox_mcharg_gilmour_2002");

        // Check the default plots file (see e.g. #2475)
        FileFinder ref_plots("projects/FunctionalCuration/test/data/TestIcalProtocol-default-plots.csv", RelativeTo::ChasteSourceRoot);
        FileFinder out_dir(dirname, RelativeTo::ChasteTestOutput);
        FileFinder test_file("outputs-default-plots.csv", out_dir);
        FileComparison comparer(test_file, ref_plots);
        TS_ASSERT( comparer.CompareFiles() );
    }

    void TestImporting() throw (Exception)
    {
        std::string dirname = "TestICaLProtocolOutputs_Import";
        ProtocolFileFinder proto_xml_file("projects/FunctionalCuration/test/protocols/test_ical.xml", RelativeTo::ChasteSourceRoot);
        DoTestShortIcal(dirname, proto_xml_file, "fox_mcharg_gilmour_2002", false);
    }

    void TestPaciModel() throw (Exception)
    {
        std::string dirname = "TestICaLProtocolOutputs_Paci";
        ProtocolFileFinder proto_xml_file("projects/FunctionalCuration/test/protocols/test_ical.xml", RelativeTo::ChasteSourceRoot);
        DoTestShortIcal(dirname, proto_xml_file, "paci_hyttinen_aaltosetala_severi_ventricularVersion");
    }

private:
    void DoTestShortIcal(const std::string& rDirName, const ProtocolFileFinder& rProtoFile,
                         const std::string& rModelName, bool setInputs=true) throw (Exception)
    {
        FileFinder cellml_file("projects/FunctionalCuration/cellml/" + rModelName + ".cellml", RelativeTo::ChasteSourceRoot);

        ProtocolRunner runner(cellml_file, rProtoFile, rDirName);

        // Don't do too many runs
        if (setInputs)
        {
            std::vector<AbstractExpressionPtr> test_potentials
                = EXPR_LIST(CONST(-45.01))(CONST(-25.01))(CONST(0.01))(CONST(15.01))(CONST(40.01))(CONST(79.99));
            DEFINE(test_potentials_expr, boost::make_shared<ArrayCreate>(test_potentials));
            runner.GetProtocol()->SetInput("test_potentials", test_potentials_expr);
            runner.GetProtocol()->SetInput("steady_state_time", CONST(1000));
        }

        runner.RunProtocol();
        FileFinder success_file(rDirName + "/success", RelativeTo::ChasteTestOutput);
        TS_ASSERT(success_file.Exists());

        // Check the key outputs haven't changed.
        const Environment& r_outputs = runner.GetProtocol()->rGetOutputsCollection();
        NdArray<double> min_LCC = GET_ARRAY(r_outputs.Lookup("min_LCC"));
        TS_ASSERT_EQUALS(min_LCC.GetNumElements(), 18u);
        TS_ASSERT_EQUALS(min_LCC.GetNumDimensions(), 2u);
        TS_ASSERT_EQUALS(min_LCC.GetShape()[0], 3u);
        if (rModelName.substr(0,4) == "paci")
        {
            const double expected[] = {-0.1026, -1.0193, -3.4105, -2.4263, -0.7405, -0.0640,
                                       -0.2053, -2.0382, -6.8208, -4.8535, -1.4830, -0.1320,
                                       -0.3080, -3.0568, -10.2307, -7.2804, -2.2254, -0.2001};
            NdArray<double>::ConstIterator it = min_LCC.Begin();
            for (unsigned i=0; i<18u; i++)
            {
                TS_ASSERT_DELTA(*it++, expected[i], 1e-3);
            }
        }
        else
        {
            const double expected[] = {-0.0144, -0.1105, -0.4718, -0.3094, -0.0929, -0.0066,
                                       -0.0273, -0.2159, -0.9366, -0.6148, -0.1848, -0.0143,
                                       -0.0398, -0.3192, -1.3979, -0.9178, -0.2758, -0.0219};
            NdArray<double>::ConstIterator it = min_LCC.Begin();
            for (unsigned i=0; i<18u; i++)
            {
                TS_ASSERT_DELTA(*it++, expected[i], 1e-3);
            }
        }
    }
};

#endif // TESTICALPROTOCOL_HPP_
