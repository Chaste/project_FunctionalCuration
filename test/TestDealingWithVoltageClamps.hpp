/*

Copyright (C) University of Oxford, 2005-2011

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

#ifndef TESTDEALINGWITHVOLTAGECLAMPS_HPP_
#define TESTDEALINGWITHVOLTAGECLAMPS_HPP_

#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>


#include <cxxtest/TestSuite.h>

#include "AbstractCvodeCell.hpp"
#include "AbstractIvpOdeSolver.hpp"
#include "ZeroStimulus.hpp"
#include "VectorHelperFunctions.hpp"
#include "aslanidi_Purkinje_model_2009_NotLHopitaled.hpp"
#include "aslanidi_Purkinje_model_2009_LHopitaled.hpp"

#include "UsefulFunctionsForProtocolTesting.hpp" // Must come after any other CellModelTests headers
#include "PetscSetupAndFinalize.hpp"

class TestDealingWithVoltageClamps : public CxxTest::TestSuite
{
public:
    /**
     * It isn't very satisfying having cell models that will introduce NaNs
     * whenever the voltage hits certain (and very reasonable) values.
     *
     * Here Gary is playing with applying L'Hopital's Rule to get past some
     * voltage steps that this model failed on in the Cell Model Tests framework.
     *
     * http://en.wikipedia.org/wiki/L%27hopitals_rule
     *
     * Could something like this in Maple work:
     *
     *  * Look for all instances of f(V)/g(V).
     *  * Whenever V appears in g(V)
     *    * Try to solve g(V) = 0,
     *    * if you can for -100 < V_1 < 100 (say)
     *    * Then see if f(V_1) = 0
     *      * If it is then symbolically differentiate f(V) and g(V)
     *      * Put in
     *      if (V != V_1)
     *      {
     *        do normal thing
     *      }
     *      else
     *      {
     *        apply L'Hopital's Rule
     *      }
     *
     *  I've done this manually for the aslanidi 2009 Purkinje model (see below)
     *  and it works very nicely.
     */
    void TestApplyingLHopitalsRule() throw (Exception, std::bad_alloc)
    {
        // Solver and stimulus should be ignored...
        boost::shared_ptr<ZeroStimulus> p_stimulus(new ZeroStimulus());
        boost::shared_ptr<AbstractIvpOdeSolver> p_solver;

        // This is a cell model used for the ICaL calculation
        boost::shared_ptr<AbstractCvodeCell> p_cell1(new Aslanidi_Purkinje_model_2009FromCellMLCvodeNonLHopitaled(p_solver, p_stimulus));
        boost::shared_ptr<AbstractCvodeCell> p_cell2(new Aslanidi_Purkinje_model_2009FromCellMLCvodeLHopitaled(p_solver, p_stimulus));

        double holding_potential = -50.0;
        p_cell1->SetParameter("membrane_voltage_value", holding_potential);
        p_cell2->SetParameter("membrane_voltage_value", holding_potential);

        p_cell1->SetMaxSteps(2e7);
        p_cell2->SetMaxSteps(2e7);
        p_cell1->Solve(-10000,0,1); // Run for 1000ms, max step 1ms.
        p_cell2->Solve(-10000,0,1); // Run for 1000ms, max step 1ms.

        // Should both have the same state vars
        N_Vector solution_at_holding_potential = p_cell1->GetStateVariables();

        std::cout << "Holding potential reached\n" << std::flush;

        for (double test_potential = -45; test_potential <= 80; test_potential+=5)
        {
            // Reset model variables to holding potential
            p_cell1->SetStateVariables(solution_at_holding_potential);
            p_cell2->SetStateVariables(solution_at_holding_potential);

            std::cout << "Test potential = " << test_potential << "\t" << std::flush;

            // Original model
            p_cell1->SetParameter("membrane_voltage_value", test_potential);
            try
            {
                p_cell1->Solve(0,1,1); // Run for 1ms, max step 1ms.
                std::cout << "Cell 1 passed, " << std::flush;
            }
            catch (Exception &e)
            {
                std::cout << "Cell 1 failed, " << std::flush;
            }

            // Manually L'Hopitaled for V = 0 and V = 15.
            p_cell2->SetParameter("membrane_voltage_value", test_potential);
            p_cell2->Solve(0,1,1); // Run for 1ms, max step 1ms.
            std::cout << "Cell 2 passed\n" << std::flush;
        }

        // Clean up memory
        DeleteVector(solution_at_holding_potential);
    }
};

#endif // TESTDEALINGWITHVOLTAGECLAMPS_HPP_
