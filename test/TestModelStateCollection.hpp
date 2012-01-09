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

#ifndef TESTMODELSTATECOLLECTION_HPP_
#define TESTMODELSTATECOLLECTION_HPP_

#include <cxxtest/TestSuite.h>

#include <boost/pointer_cast.hpp> // NB: Not available on Boost 1.33.1

#include "ModelStateCollection.hpp"

#include "LuoRudy1991.hpp"
#include "SimpleStimulus.hpp"
#include "EulerIvpOdeSolver.hpp"

class TestModelStateCollection : public CxxTest::TestSuite
{
public:
    void TestHandlingStates() throw (Exception)
    {
        ModelStateCollection collection;

        // We need a model to work with...
        boost::shared_ptr<SimpleStimulus> p_stimulus(new SimpleStimulus(-25.5, 2.0, 0.0));
        boost::shared_ptr<EulerIvpOdeSolver> p_solver(new EulerIvpOdeSolver);
        boost::shared_ptr<AbstractCardiacCell> p_model(new CellLuoRudy1991FromCellML(p_solver, p_stimulus));
        boost::shared_ptr<AbstractParameterisedSystem<std::vector<double> > > p_system(boost::dynamic_pointer_cast<AbstractParameterisedSystem<std::vector<double> > >(p_model));

        collection.SaveState("initial_state", p_system);
        const double new_value = -1000.0;
        p_model->SetStateVariable(0, new_value);
        collection.SaveState("modified_state", p_system);

        collection.SetModelState(p_system, "initial_state");
        std::vector<double>& r_state = p_system->rGetStateVariables();
        std::vector<double> inits = p_system->GetInitialConditions();
        TS_ASSERT_EQUALS(inits.size(), r_state.size());
        for (unsigned i=0; i<inits.size(); ++i)
        {
            TS_ASSERT_EQUALS(inits[i], r_state[i]);
        }

        TS_ASSERT_DIFFERS(r_state[0], new_value);
        collection.SetModelState(p_system, "modified_state");
        TS_ASSERT_EQUALS(r_state[0], new_value);
        TS_ASSERT_EQUALS(inits.size(), r_state.size());
        for (unsigned i=1; i<inits.size(); ++i)
        {
            TS_ASSERT_EQUALS(inits[i], r_state[i]);
        }
    }
};

#endif // TESTMODELSTATECOLLECTION_HPP_
