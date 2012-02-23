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
