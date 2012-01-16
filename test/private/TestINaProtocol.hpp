/*

Copyright (C) University of Oxford, 2005-2012

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

#ifndef TESTINAPROTOCOL_HPP_
#define TESTINAPROTOCOL_HPP_

#include <cxxtest/TestSuite.h>
#include <boost/assign/list_of.hpp>
using boost::assign::list_of;

#include "ProtocolRunner.hpp"
#include "ProtocolLanguage.hpp"
#include "ProtoHelperMacros.hpp"
#include "OutputFileHandler.hpp"

#include "UsefulFunctionsForProtocolTesting.hpp"
#include "PetscSetupAndFinalize.hpp"

class TestINaProtocol : public CxxTest::TestSuite
{
private:
    /**
     * Run the protocol on the given model.
     *
     * @param rCellMLFileBaseName  the cellml file to dynamically load and use.
     * @return  whether the protocol was completed successfully
     */
    void RunProtocol(std::string& rCellMLFileBaseName)
    {
        std::string protocol_name = "INa_IV_curve";
        std::string dirname = "FunctionalCuration/" + rCellMLFileBaseName + "/" + protocol_name;

        FileFinder cellml_file("projects/FunctionalCuration/cellml/" + rCellMLFileBaseName + ".cellml", RelativeTo::ChasteSourceRoot);
        FileFinder proto_xml_file("projects/FunctionalCuration/test/private/protocols/" + protocol_name + ".xml", RelativeTo::ChasteSourceRoot);

        try
        {
            ProtocolRunner runner(cellml_file, proto_xml_file, dirname);
            runner.RunProtocol();
        }
        catch (Exception& e)
        {
            OUR_WARN(e.GetMessage(), rCellMLFileBaseName, protocol_name);
        }
    }

public:

    void TestProtocolsForManyCellModels() throw(Exception, std::bad_alloc)
    {
        std::vector<std::string> cellml_files = GetAListOfCellMLFiles();
        if (CommandLineArguments::Instance()->OptionExists("--models"))
        {
            cellml_files = CommandLineArguments::Instance()->GetStringsCorrespondingToOption("--models");
        }

        for (unsigned i=0; i<cellml_files.size(); ++i)
        {
            std::cout << "\nRunning protocols for " << cellml_files[i] << std::endl << std::flush;

            // Protocols and graphs must be done together so that
            // mpHandler points to the correct protocol subfolder.
            RunProtocol(cellml_files[i]);
        }
    }
};

#endif // TESTINAPROTOCOL_HPP_
