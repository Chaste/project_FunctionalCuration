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

#ifndef TESTSTEPPERS_HPP_
#define TESTSTEPPERS_HPP_


#include <vector>
#include <string>
#include <cmath>

#include <boost/shared_ptr.hpp>
#include <boost/assign/list_of.hpp>

#include <cxxtest/TestSuite.h>

#include "AbstractStepper.hpp"
#include "UniformStepper.hpp"
#include "VectorStepper.hpp"
#include "WhileStepper.hpp"

#include "ProtocolLanguage.hpp"
#include "ProtoHelperMacros.hpp"

using boost::assign::list_of;
using boost::make_shared;

#define MAKE_PTR_A(ABS_TYPE, TYPE, NAME, ARGS) boost::shared_ptr<ABS_TYPE> NAME(new TYPE ARGS)
#define MAKE_PTR(TYPE, NAME, ARGS) MAKE_PTR_A(TYPE, TYPE, NAME, ARGS)
#define ASSIGN_PTR(NAME, TYPE, ARGS) NAME.reset(new TYPE ARGS)

class TestSteppers : public CxxTest::TestSuite
{
    void DoAbstractTest(boost::shared_ptr<AbstractStepper> ps, const std::string& rName)
    {
        TS_ASSERT_EQUALS(ps->GetIndexName(), rName);
        TS_ASSERT_LESS_THAN(0u, ps->GetNumberOfOutputPoints());
        TS_ASSERT_EQUALS(ps->GetCurrentOutputNumber(), 0u);
        TS_ASSERT(!ps->AtEnd());
    }

    void DoUniformTest(const std::string& rName, const std::string& rUnits,
                       double start, double end, double step)
    {
        MAKE_PTR_A(AbstractStepper, UniformStepper, p_uniform, (rName, rUnits, start, end, step));
        DoAbstractTest(p_uniform, rName);
        TS_ASSERT(p_uniform->IsEndFixed());
        const double tol = fabs(step)/1000.0;

        TS_ASSERT_EQUALS(p_uniform->GetNumberOfOutputPoints(), 1 + (unsigned)floor((end-start)/step + 0.5));
        TS_ASSERT_EQUALS(p_uniform->GetCurrentOutputPoint(), start);
        for (unsigned i=0; i<p_uniform->GetNumberOfOutputPoints(); i++)
        {
            TS_ASSERT(!p_uniform->AtEnd());
            TS_ASSERT_EQUALS(p_uniform->GetCurrentOutputNumber(), i);
            TS_ASSERT_DELTA(p_uniform->Step(), start+(i+1)*step, tol);
            TS_ASSERT_DELTA(p_uniform->GetCurrentOutputPoint(), start+(i+1)*step, tol);
        }
        TS_ASSERT(p_uniform->AtEnd());
        p_uniform->Reset();
        TS_ASSERT_EQUALS(p_uniform->GetCurrentOutputNumber(), 0u);
        TS_ASSERT_EQUALS(p_uniform->GetCurrentOutputPoint(), start);
        TS_ASSERT(!p_uniform->AtEnd());
    }

    void DoVectorTest(const std::string& rName, const std::string& rUnits,
                      const std::vector<double>& rVec, double tol=1e-6)
    {
        MAKE_PTR_A(AbstractStepper, VectorStepper, ps, (rName, rUnits, rVec));
        DoAbstractTest(ps, rName);
        TS_ASSERT(ps->IsEndFixed());

        TS_ASSERT_EQUALS(ps->GetNumberOfOutputPoints(), rVec.size());
        for (unsigned i=0; i<ps->GetNumberOfOutputPoints(); i++)
        {
            TS_ASSERT(!ps->AtEnd());
            TS_ASSERT_EQUALS(ps->GetCurrentOutputNumber(), i);
            TS_ASSERT_EQUALS(ps->GetCurrentOutputPoint(), rVec[i]);
            if (i<rVec.size()-1)
            {
                const double next = rVec[i+1];
                TS_ASSERT_EQUALS(ps->Step(), next);
                TS_ASSERT_EQUALS(ps->GetCurrentOutputPoint(), next);
            }
            else
            {
                TS_ASSERT_EQUALS(ps->Step(), DOUBLE_UNSET);
            }
        }
        TS_ASSERT(ps->AtEnd());
        ps->Reset();
        TS_ASSERT_EQUALS(ps->GetCurrentOutputNumber(), 0u);
        TS_ASSERT_EQUALS(ps->GetCurrentOutputPoint(), rVec[0]);
        TS_ASSERT(!ps->AtEnd());
    }

public:
    void TestUniformStepper() throw (Exception)
    {
        DoUniformTest("i", "dimensionless", 0, 4, 1);
        DoUniformTest("time", "ms", 2.0, 20.0, 1.0);
        DoUniformTest("back", "s", 15.5, 0.0, -0.1);
        DoUniformTest("one", "m", -12.1, -2.1, 10.0);
        DoUniformTest("none", "m", -12.1, -12.1, 10.0);

        TS_ASSERT_THROWS_CONTAINS(UniformStepper("x", "u", 1, 0, 1),
                                  "If and only if endPoint is before startPoint, stepInterval must be negative.");
        TS_ASSERT_THROWS_CONTAINS(UniformStepper("x", "u", 0, 2, 1.0001),
                                  "The step interval must divide the range.");
    }

    void TestVectorStepper() throw (Exception)
    {
        DoVectorTest("bcl", "ms", boost::assign::list_of(1000)(900)(750));
        DoVectorTest("odd", "num", boost::assign::list_of(999)(1.3)(-55)(79.97));

        std::vector<double> empty;
        TS_ASSERT_THROWS_CONTAINS(VectorStepper("x", "u", empty),
                                  "A VectorStepper must be given a non-empty vector.");
    }

    void TestS1S2Steppers() throw (Exception)
    {
        std::vector<double> s2_intervals = list_of(1000)(900)(800)(700)(600)(500);
        boost::shared_ptr<AbstractStepper> p_s2_stepper(new VectorStepper("S2_interval", "ms", s2_intervals));
        TS_ASSERT_EQUALS(p_s2_stepper->GetNumberOfOutputPoints(), 6u);
        TS_ASSERT_EQUALS(p_s2_stepper->GetCurrentOutputNumber(), 0u);
        TS_ASSERT_EQUALS(p_s2_stepper->GetCurrentOutputPoint(), 1000);
        TS_ASSERT(!p_s2_stepper->AtEnd());

        boost::shared_ptr<AbstractStepper> p_time_stepper(new UniformStepper("output_times","ms",0.0,2000.0,1.0));
        TS_ASSERT_EQUALS(p_time_stepper->GetNumberOfOutputPoints(), 2001u);
        TS_ASSERT_EQUALS(p_time_stepper->GetCurrentOutputNumber(), 0u);
        TS_ASSERT_EQUALS(p_time_stepper->GetCurrentOutputPoint(), 0.0);
        TS_ASSERT(!p_time_stepper->AtEnd());

        // New interface allows expressions for the arguments
        std::vector<AbstractExpressionPtr> args = EXPR_LIST(CONST(1000))(CONST(1000));
        DEFINE(stst_time_expr, make_shared<MathmlTimes>(args));
        args = EXPR_LIST(CONST(0))(stst_time_expr);
        boost::shared_ptr<AbstractStepper> p_stst_runner(new VectorStepper("time", "ms", args));
        TS_ASSERT_EQUALS(p_stst_runner->GetNumberOfOutputPoints(), 0u);
        TS_ASSERT_EQUALS(p_stst_runner->GetCurrentOutputNumber(), 0u);
        TS_ASSERT_EQUALS(p_stst_runner->GetCurrentOutputPoint(), DOUBLE_UNSET);
        TS_ASSERT(p_stst_runner->AtEnd());
        // Evaluate the expressions
        Environment env;
        p_stst_runner->SetEnvironment(env);
        p_stst_runner->Initialise();
        TS_ASSERT_EQUALS(p_stst_runner->GetNumberOfOutputPoints(), 2u);
        TS_ASSERT_EQUALS(p_stst_runner->GetCurrentOutputPoint(), 0);
        TS_ASSERT(!p_stst_runner->AtEnd());
        TS_ASSERT_EQUALS(p_stst_runner->Step(), 1000000.0);
        TS_ASSERT_EQUALS(p_stst_runner->Step(), DOUBLE_UNSET);
        TS_ASSERT(p_stst_runner->AtEnd());

        // Timecourse stepper
        args = EXPR_LIST(CONST(2))(CONST(1000));
        DEFINE(timecourse_end, make_shared<MathmlTimes>(args));
        MAKE_PTR_A(AbstractStepper, UniformStepper, p_timecourse, ("timecourse", "ms", CONST(0), timecourse_end, CONST(1)));
        TS_ASSERT_EQUALS(p_timecourse->GetNumberOfOutputPoints(), 0u);
        TS_ASSERT_EQUALS(p_timecourse->GetCurrentOutputNumber(), 0u);
        TS_ASSERT_EQUALS(p_timecourse->GetCurrentOutputPoint(), DOUBLE_UNSET);
        TS_ASSERT(p_timecourse->AtEnd());
        // Evaluate the expressions
        p_timecourse->SetEnvironment(env);
        p_timecourse->Initialise();
        TS_ASSERT_EQUALS(p_timecourse->GetNumberOfOutputPoints(), 2001u);
        for (unsigned i=0; i<=2000; i++)
        {
            TS_ASSERT(!p_timecourse->AtEnd());
            TS_ASSERT_EQUALS(p_timecourse->GetCurrentOutputPoint(), i);
            TS_ASSERT_EQUALS(p_timecourse->Step(), i+1);
        }
        TS_ASSERT(p_timecourse->AtEnd());
    }

    void TestWhileLoop() throw (Exception)
    {
        // While i<10
        const unsigned N=10;
        std::vector<AbstractExpressionPtr> args = EXPR_LIST(LOOKUP("i"))(CONST(N));
        DEFINE(cond, make_shared<MathmlLt>(args));
        MAKE_PTR_A(AbstractStepper, WhileStepper, p_stepper, ("i", "number", cond));
        Environment env;
        p_stepper->SetEnvironment(env);
        p_stepper->Initialise();

        DoAbstractTest(p_stepper, "i");
        TS_ASSERT(!p_stepper->IsEndFixed());
        TS_ASSERT_LESS_THAN(N, p_stepper->GetNumberOfOutputPoints()); // It should make a large initial guess!
        TS_ASSERT_EQUALS(p_stepper->GetCurrentOutputNumber(), 0u);
        TS_ASSERT_EQUALS(p_stepper->GetCurrentOutputPoint(), 0.0);

        for (unsigned i=0; i<N; i++)
        {
            TS_ASSERT(!p_stepper->AtEnd());
            TS_ASSERT_EQUALS(p_stepper->GetCurrentOutputPoint(), i);
            TS_ASSERT_EQUALS(p_stepper->Step(), i+1);
        }
        TS_ASSERT(p_stepper->AtEnd());
        TS_ASSERT_EQUALS(N, p_stepper->GetNumberOfOutputPoints());
    }

};

#endif // TESTSTEPPERS_HPP_
