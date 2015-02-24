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

#ifndef TESTREPRODUCIBILITY_HPP_
#define TESTREPRODUCIBILITY_HPP_

#include <cxxtest/TestSuite.h>

#include <vector>
#include <string>
#include <map>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

#include "CommandLineArguments.hpp"
#include "FileFinder.hpp"
#include "OutputFileHandler.hpp"
#include "PetscTools.hpp"

#include "ProtocolFileFinder.hpp"
#include "ProtocolRunner.hpp"

#include "UsefulFunctionsForProtocolTesting.hpp"
#include "ProtoHelperMacros.hpp"
#include "PetscSetupAndFinalize.hpp"

/**
 * This tests many of the model/protocol combinations seen on the Web Lab (https://chaste.cs.ox.ac.uk/FunctionalCuration)
 * to ensure that the results do not change as the implementation evolves.  It builds on TestFunctionalCurationLiteratePaper.hpp,
 * which does a similar thing for the ICaL and S1S2 protocols used in our 2011 publication.
 */
class TestReproducibility : public CxxTest::TestSuite
{
public:
    void TestWithWebLabExperiments() throw (Exception)
    {
        std::vector<std::string> models = boost::assign::list_of("aslanidi_atrial_model_2009")
                                                                ("aslanidi_Purkinje_model_2009")
                                                                ("beeler_reuter_model_1977")
//                                                                ("benson_epicardial_2008")
//                                                                ("bernus_wilders_zemlin_verschelde_panfilov_2002")
                                                                ("bondarenko_szigeti_bett_kim_rasmusson_2004_apical")
                                                                ("carro_2011_epi")
//                                                                ("clancy_rudy_2002")
//                                                                ("courtemanche_ramirez_nattel_1998") // -EPV
                                                                ("decker_2009")
                                                                ("difrancesco_noble_model_1985") // -EPV
//                                                                ("earm_noble_model_1990")
                                                                ("fink_noble_giles_model_2008")
                                                                ("grandi_pasqualini_bers_2010_ss")
                                                                ("iyer_2004") // EPV?
//                                                                ("iyer_model_2007")
                                                                ("li_mouse_2010") // -EPV
                                                                ("luo_rudy_1991")
                                                                ("mahajan_shiferaw_2008")
//                                                                ("maleckar_model_2009") // -EPV
//                                                                ("matsuoka_model_2003")
//                                                                ("noble_model_1991")
//                                                                ("noble_model_1998")
                                                                ("ohara_rudy_2011_epi")
                                                                ("priebe_beuckelmann_1998")
//                                                                ("sachse_moreno_abildskov_2008_b")
                                                                ("shannon_wang_puglisi_weber_bers_2004")
//                                                                ("ten_tusscher_model_2004_endo")
                                                                ("ten_tusscher_model_2006_epi")
//                                                                ("winslow_model_1999")
                                                                ;
        if (CommandLineArguments::Instance()->OptionExists("--models"))
        {
            models = CommandLineArguments::Instance()->GetStringsCorrespondingToOption("--models");
        }

        // Which outputs should we test for each protocol?
        typedef std::pair<const std::string, std::vector<std::string> > string_vec_pair;
        std::map<std::string, std::vector<std::string> > outputs_to_check;
        outputs_to_check["ExtracellularPotassiumVariation"] = boost::assign::list_of("scaled_APD90")("scaled_resting_potential")("detailed_voltage");
        outputs_to_check["GraphState"] = boost::assign::list_of("state");
        outputs_to_check["ICaL"] = boost::assign::list_of("min_LCC")("final_membrane_voltage");
//        outputs_to_check["ICaL_block"] = boost::assign::list_of("scaled_APD90")("detailed_voltage");
//        outputs_to_check["ICaL_IV_curve"] = boost::assign::list_of("normalised_peak_currents");
//        outputs_to_check["IK1_block"] = boost::assign::list_of("scaled_resting_potential")("scaled_APD90")("detailed_voltage");
        outputs_to_check["IK1_IV_curve"] = boost::assign::list_of("normalised_low_K1")("normalised_high_K1");
        outputs_to_check["IKr_block"] = boost::assign::list_of("scaled_APD90")("detailed_voltage");
        outputs_to_check["IKr_IV_curve"] = boost::assign::list_of("normalised_peak_Kr_tail");
//        outputs_to_check["IKs_block"] = boost::assign::list_of("scaled_APD90")("detailed_voltage");
//        outputs_to_check["IKs_IV_curve"] = boost::assign::list_of("normalised_peak_Ks_tail");
        outputs_to_check["INa_block"] = boost::assign::list_of("scaled_APD90")("detailed_voltage");
        outputs_to_check["INa_IV_curves"] = boost::assign::list_of("normalised_peak_currents")("current_activation");
//        outputs_to_check["Ito_block"] = boost::assign::list_of("scaled_resting_potential")("scaled_APD90")("detailed_voltage");
        outputs_to_check["NCX_block"] = boost::assign::list_of("scaled_resting_potential")("scaled_APD90")("detailed_voltage");
        outputs_to_check["RyR_block"] = boost::assign::list_of("scaled_APD90")("detailed_voltage");
        outputs_to_check["S1S2"] = boost::assign::list_of("S1S2_slope"); // NB: Must be different from outputs checked in TestFunctionalCurationLiteratePaper!
        outputs_to_check["SteadyStateRestitution"] = boost::assign::list_of("APD")("restitution_slope");
        outputs_to_check["SteadyStateRunner"] = boost::assign::list_of("num_paces")("detailed_voltage");
        outputs_to_check["SteadyStateRunner0_5Hz"] = boost::assign::list_of("num_paces")("detailed_voltage");
//        outputs_to_check["SteadyStateRunner2Hz"] = boost::assign::list_of("num_paces")("detailed_voltage");
//        outputs_to_check["SteadyStateRunner3Hz"] = boost::assign::list_of("num_paces")("detailed_voltage");
        outputs_to_check["SteadyStateRunner4Hz"] = boost::assign::list_of("num_paces")("detailed_voltage");

        std::vector<std::string> protocols;
        protocols.reserve(outputs_to_check.size());
        BOOST_FOREACH(string_vec_pair& r_proto_outputs, outputs_to_check)
        {
            protocols.push_back(r_proto_outputs.first);
        }
        if (CommandLineArguments::Instance()->OptionExists("--protocols"))
        {
            protocols = CommandLineArguments::Instance()->GetStringsCorrespondingToOption("--protocols");
        }

        /* We use Chaste's process isolation facility to process models in parallel, if running on multiple processes.
         * The main output folder needs to be created with a collective call (so we don't have multiple processes
         * trying to make the same folder) but thereafter each protocol run can be done completely independently.
         */
        OutputFileHandler base_handler("FunctionalCuration", false);
        BOOST_FOREACH(const std::string& r_model_name, models)
        {
            OutputFileHandler sub_handler(base_handler.FindFile(r_model_name), false);
        }
        PetscTools::IsolateProcesses(true);

        /* This utility class handles comparing virtual experiment results against stored reference data. */
        HistoricalResultTester result_tester;

        unsigned counter = 0u;
        BOOST_FOREACH(const std::string& r_model_name, models)
        {
            OutputFileHandler model_handler(base_handler.FindFile(r_model_name), false);
            BOOST_FOREACH(const std::string& r_proto_name, protocols)
            {
                if (counter++ % PetscTools::GetNumProcs() != PetscTools::GetMyRank())
                {
                    continue; // Let another process do this combination
                }

                OutputFileHandler handler(model_handler.FindFile(r_proto_name), false);

                try
                {
                    FileFinder cellml_file("projects/FunctionalCuration/cellml/" + r_model_name + ".cellml", RelativeTo::ChasteSourceRoot);
                    ProtocolFileFinder proto_file("projects/FunctionalCuration/protocols/" + r_proto_name + ".txt", RelativeTo::ChasteSourceRoot);
                    ProtocolRunner runner(cellml_file, proto_file, handler.GetRelativePath());
                    std::vector<std::string> input_names = boost::assign::list_of("max_paces")("max_steady_state_beats");
                    BOOST_FOREACH(std::string input, input_names)
                    {
                        try
                        {
                            runner.GetProtocol()->SetInput(input, CONST(1000));
                            std::cout << "Set " << input << " to 1000 for " << r_proto_name << std::endl;
                        }
                        catch (const Exception &) // Input doesn't exist, so do nothing
                        {}
                    }
                    // Re-run the protocol's library, since it may depend on protocol inputs
                    runner.GetProtocol()->InitialiseLibrary(true);
                    runner.RunProtocol();
                }
                catch (Exception& e)
                {
                    OUR_WARN(e.GetMessage(), r_model_name, r_proto_name);
                }
                std::vector<std::string>& r_outputs = outputs_to_check[r_proto_name];
                result_tester.CompareToHistoricalResults(handler, r_model_name, r_proto_name, r_outputs, 0.005, 1e-4); // 0.5% rel tol
            }
        }

        /* Turn off process isolation before communicating to summarise results. */
        PetscTools::IsolateProcesses(false);
        result_tester.ReportResults();
    }
};

#endif // TESTREPRODUCIBILITY_HPP_
