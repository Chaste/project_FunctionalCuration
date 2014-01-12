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

#ifndef TESTRUNNINGALLCELLMODELTESTS_HPP_
#define TESTRUNNINGALLCELLMODELTESTS_HPP_

#include <cxxtest/TestSuite.h>

#include <vector>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

#include "FileFinder.hpp"

#include "ProtocolFileFinder.hpp"
#include "MultiProtocolRunner.hpp"

#include "PetscSetupAndFinalize.hpp"

/**
 * This test attempts to run all of the protocols listed for all of the cellml models in the FunctionalCuration/cellml folder.
 *
 * It collects warnings about the ones which failed (generally this is because of missing metadata tags).
 */
class TestRunningAllCellModelTests : public CxxTest::TestSuite
{
public:

    void TestProtocolsForManyCellModels() throw(Exception, std::bad_alloc)
    {
        MultiProtocolRunner runner;

        // Set up default protocols list
        {
            FileFinder proto_root("projects/FunctionalCuration/test", RelativeTo::ChasteSourceRoot);
            std::vector<ProtocolFileFinder> protocols;
            std::vector<std::string> proto_names = boost::assign::list_of(std::string("protocols/ICaL.xml"))
                                                                         ("protocols/S1S2.xml")
                                                                         ("protocols/SteadyPacing.xml")
                                                                         ("protocols/SteadyStateRunner.xml")
                                                                         ("private/protocols/INa_IV_curve.xml")
                                                                         ("private/protocols/Hypokalaemia.xml")
                                                                         ("private/protocols/IK1_IV_curve.xml")
                                                                         ("private/protocols/IKr_IV_curve.xml")
                                                                         ("private/protocols/IKs_IV_curve.xml");
            BOOST_FOREACH(const std::string& r_proto, proto_names)
            {
                protocols.push_back(ProtocolFileFinder(r_proto, proto_root));
            }
            runner.SetDefaultProtocols(protocols);
        }

        // Set default models list to be all CellML files in the project folder
        {
            FileFinder model_root("projects/FunctionalCuration/cellml", RelativeTo::ChasteSourceRoot);
            runner.SetDefaultModels(model_root.FindMatches("*.cellml"));
        }

        // Run protocols, in parallel if available
        runner.RunProtocols();
    }
};

#endif // TESTRUNNINGALLCELLMODELTESTS_HPP_
