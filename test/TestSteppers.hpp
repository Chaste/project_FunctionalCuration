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
#include "MultipleStepper.hpp"
#include "FunctionalStepper.hpp"

#include "ProtocolLanguage.hpp"
#include "ProtoHelperMacros.hpp"

using boost::assign::list_of;
using boost::make_shared;

#define MAKE_PTR_A(ABS_TYPE, TYPE, NAME, ARGS) boost::shared_ptr<ABS_TYPE> NAME(new TYPE ARGS)
#define MAKE_PTR(TYPE, NAME, ARGS) MAKE_PTR_A(TYPE, TYPE, NAME, ARGS)
#define ASSIGN_PTR(NAME, TYPE, ARGS) NAME.reset(new TYPE ARGS)

class TestSteppers : public CxxTest::TestSuite
{
    void DoAbstractTest(boost::shared_ptr<AbstractStepper> pStepper,
                        const std::string& rName, const std::string& rUnits)
    {
        TS_ASSERT_EQUALS(pStepper->GetIndexName(), rName);
        TS_ASSERT_EQUALS(pStepper->GetUnits(), rUnits);
        TS_ASSERT_LESS_THAN(0u, pStepper->GetNumberOfOutputPoints());
        TS_ASSERT_EQUALS(pStepper->GetCurrentOutputNumber(), 0u);
        TS_ASSERT(!pStepper->AtEnd());
    }

    void DoUniformTestBody(boost::shared_ptr<AbstractStepper> pStepper, double start, double end, double step)
    {
        TS_ASSERT(pStepper->IsEndFixed());
        const double tol = fabs(step)/1000.0;

        TS_ASSERT_EQUALS(pStepper->GetNumberOfOutputPoints(), 1 + (unsigned)floor((end-start)/step + 0.5));
        TS_ASSERT_EQUALS(pStepper->GetCurrentOutputPoint(), start);
        for (unsigned i=0; i<pStepper->GetNumberOfOutputPoints(); i++)
        {
            TS_ASSERT(!pStepper->AtEnd());
            TS_ASSERT_EQUALS(pStepper->GetCurrentOutputNumber(), i);
            TS_ASSERT_DELTA(pStepper->Step(), start+(i+1)*step, tol);
            TS_ASSERT_DELTA(pStepper->GetCurrentOutputPoint(), start+(i+1)*step, tol);
        }
        TS_ASSERT(pStepper->AtEnd());
        pStepper->Reset();
        TS_ASSERT_EQUALS(pStepper->GetCurrentOutputNumber(), 0u);
        TS_ASSERT_EQUALS(pStepper->GetCurrentOutputPoint(), start);
        TS_ASSERT(!pStepper->AtEnd());
    }

    void DoUniformTest(const std::string& rName, const std::string& rUnits,
                       double start, double end, double step)
    {
        MAKE_PTR_A(AbstractStepper, UniformStepper, p_uniform, (rName, rUnits, start, end, step));
        DoAbstractTest(p_uniform, rName, rUnits);
        DoUniformTestBody(p_uniform, start, end, step);
    }

    void DoVectorTestBody(AbstractStepperPtr pStepper, const std::vector<double>& rVec, double tol=1e-6)
    {
        TS_ASSERT(pStepper->IsEndFixed());

        TS_ASSERT_EQUALS(pStepper->GetNumberOfOutputPoints(), rVec.size());
        for (unsigned i=0; i<pStepper->GetNumberOfOutputPoints(); i++)
        {
            TS_ASSERT(!pStepper->AtEnd());
            TS_ASSERT_EQUALS(pStepper->GetCurrentOutputNumber(), i);
            TS_ASSERT_EQUALS(pStepper->GetCurrentOutputPoint(), rVec[i]);
            if (i<rVec.size()-1)
            {
                const double next = rVec[i+1];
                TS_ASSERT_EQUALS(pStepper->Step(), next);
                TS_ASSERT_EQUALS(pStepper->GetCurrentOutputPoint(), next);
            }
            else
            {
                TS_ASSERT_EQUALS(pStepper->Step(), DOUBLE_UNSET);
            }
        }
        TS_ASSERT(pStepper->AtEnd());
        pStepper->Reset();
        TS_ASSERT_EQUALS(pStepper->GetCurrentOutputNumber(), 0u);
        TS_ASSERT_EQUALS(pStepper->GetCurrentOutputPoint(), rVec[0]);
        TS_ASSERT(!pStepper->AtEnd());
    }

    void DoVectorTest(const std::string& rName, const std::string& rUnits,
                      const std::vector<double>& rVec, double tol=1e-6)
    {
        MAKE_PTR_A(AbstractStepper, VectorStepper, ps, (rName, rUnits, rVec));
        DoAbstractTest(ps, rName, rUnits);
        DoVectorTestBody(ps, rVec, tol);
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

    void TestMultipleStepper() throw (Exception)
    {
        const std::string uname("units");
        {
            AbstractStepperPtr p_stepper1(new UniformStepper("uniform1", uname, 0, 4, 1));
            AbstractStepperPtr p_stepper2(new UniformStepper("uniform2", uname, 4, 12, 2));
            std::vector<AbstractStepperPtr> steppers = boost::assign::list_of(p_stepper1)(p_stepper2);
            AbstractStepperPtr p_stepper(new MultipleStepper(steppers));
            // The multiple stepper should behave like its first member for normal operation
            DoAbstractTest(p_stepper, "uniform1", uname);
            DoUniformTestBody(p_stepper, 0, 4, 1);
            // All stepper values should be in the environment
            EnvironmentPtr p_env(new Environment);
            p_stepper->SetEnvironment(p_env);
            TS_ASSERT_EQUALS(&(p_stepper->rGetEnvironment()), p_env.get());
            p_stepper->Reset();
            TS_ASSERT_EQUALS(p_env->GetNumberOfDefinitions(), 2u);
            std::vector<std::string> names = p_env->GetDefinedNames();
            TS_ASSERT_EQUALS(names[0], "uniform1");
            TS_ASSERT_EQUALS(names[1], "uniform2");
            TS_ASSERT_EQUALS(GET_SIMPLE_VALUE(p_env->Lookup("uniform1", "TestMultipleStepper")), 0.0);
            TS_ASSERT_EQUALS(GET_SIMPLE_VALUE(p_env->Lookup("uniform2", "TestMultipleStepper")), 4.0);
            // Subsidiary steppers should step every time the first one does
            TS_ASSERT_EQUALS(p_stepper1->GetNumberOfOutputPoints(), 5u);
            TS_ASSERT_EQUALS(p_stepper1->GetCurrentOutputNumber(), 0u);
            TS_ASSERT_EQUALS(p_stepper1->GetCurrentOutputPoint(), 0.0);
            TS_ASSERT_EQUALS(p_stepper2->GetNumberOfOutputPoints(), 5u);
            TS_ASSERT_EQUALS(p_stepper2->GetCurrentOutputNumber(), 0u);
            TS_ASSERT_EQUALS(p_stepper2->GetCurrentOutputPoint(), 4.0);

            for (unsigned i=0; i<5u; i++)
            {
                TS_ASSERT(!p_stepper1->AtEnd());
                TS_ASSERT(!p_stepper2->AtEnd());
                TS_ASSERT_EQUALS(p_stepper1->GetCurrentOutputNumber(), i);
                TS_ASSERT_EQUALS(p_stepper2->GetCurrentOutputNumber(), i);
                TS_ASSERT_DELTA(p_stepper->Step(), i+1, 1e-6);
                TS_ASSERT_DELTA(p_stepper1->GetCurrentOutputPoint(), 0.0+i+1, 1e-6);
                TS_ASSERT_DELTA(p_stepper2->GetCurrentOutputPoint(), 4.0+i*2+2, 1e-6);
                TS_ASSERT_DELTA(GET_SIMPLE_VALUE(p_env->Lookup("uniform1", "TestMultipleStepper")), 0.0+i+1, 1e-6);
                TS_ASSERT_DELTA(GET_SIMPLE_VALUE(p_env->Lookup("uniform2", "TestMultipleStepper")), 4.0+i*2+2, 1e-6);
            }
            TS_ASSERT(p_stepper1->AtEnd());
            TS_ASSERT(p_stepper2->AtEnd());
        }

        // Basic test with vector members
        {
            std::vector<double> values1 = boost::assign::list_of(5)(10)(20);
            std::vector<double> values2 = boost::assign::list_of(15)(10)(0);
            AbstractStepperPtr p_stepper1(new VectorStepper("vector1", uname, values1));
            AbstractStepperPtr p_stepper2(new VectorStepper("vector2", uname, values2));
            std::vector<AbstractStepperPtr> steppers = boost::assign::list_of(p_stepper1)(p_stepper2);
            AbstractStepperPtr p_stepper(new MultipleStepper(steppers));
            DoAbstractTest(p_stepper, "vector1", uname);
            DoVectorTestBody(p_stepper, values1);
        }

        // It's an error if a subsidiary stepper is exhausted too soon (but not v.v.)
        {
            std::vector<double> values1 = boost::assign::list_of(5)(10)(20)(40);
            std::vector<double> values2 = boost::assign::list_of(15)(10)(0);
            AbstractStepperPtr p_stepper1(new VectorStepper("vector1", uname, values1));
            AbstractStepperPtr p_stepper2(new VectorStepper("vector2", uname, values2));
            std::vector<AbstractStepperPtr> steppers = boost::assign::list_of(p_stepper1)(p_stepper2);
            AbstractStepperPtr p_stepper(new MultipleStepper(steppers));
            for (unsigned i=0; i<3u; i++)
            {
                p_stepper->Step();
            }
            TS_ASSERT(!p_stepper->AtEnd());
            TS_ASSERT(!p_stepper1->AtEnd());
            TS_ASSERT(p_stepper2->AtEnd());
            TS_ASSERT_THROWS_CONTAINS(p_stepper->Step(), "A subsidiary range for this task has been exhausted.");

            steppers = boost::assign::list_of(p_stepper2)(p_stepper1);
            AbstractStepperPtr p_stepper3(new MultipleStepper(steppers));
            for (p_stepper3->Reset(); !p_stepper3->AtEnd(); p_stepper3->Step())
            {
            }
            TS_ASSERT(p_stepper3->AtEnd());
            TS_ASSERT(!p_stepper1->AtEnd());
            TS_ASSERT(p_stepper2->AtEnd());
        }
    }

    void TestFunctionalStepper() throw (Exception)
    {
        // Create a "2*i" functional stepper
        std::vector<AbstractExpressionPtr> args = EXPR_LIST(LOOKUP("i"))(CONST(2));
        DEFINE(expr, make_shared<MathmlTimes>(args));
        AbstractStepperPtr p_fn_stepper(new FunctionalStepper("double", "ms", expr));
        TS_ASSERT_EQUALS(p_fn_stepper->GetIndexName(), "double");
        TS_ASSERT_EQUALS(p_fn_stepper->GetUnits(), "ms");
        TS_ASSERT_EQUALS(p_fn_stepper->GetCurrentOutputNumber(), 0u);
        TS_ASSERT(!p_fn_stepper->IsEndFixed());
        TS_ASSERT(!p_fn_stepper->AtEnd());
        TS_ASSERT_THROWS_CONTAINS(p_fn_stepper->GetNumberOfOutputPoints(),
                                  "The number of points in a functionalRange cannot be determined.");

        // Add it to a MultipleStepper so it can be used as in real life
        AbstractStepperPtr p_primary(new UniformStepper("i", "ms", 1, 5, 1));
        std::vector<AbstractStepperPtr> steppers = boost::assign::list_of(p_primary)(p_fn_stepper);
        AbstractStepperPtr p_multi_stepper(new MultipleStepper(steppers));
        EnvironmentPtr p_env(new Environment);
        p_multi_stepper->SetEnvironment(p_env);
        p_multi_stepper->Initialise();
        for (p_multi_stepper->Reset(); !p_multi_stepper->AtEnd(); p_multi_stepper->Step())
        {
            TS_ASSERT_EQUALS(p_fn_stepper->GetCurrentOutputNumber(), p_multi_stepper->GetCurrentOutputNumber());
            TS_ASSERT_EQUALS(p_fn_stepper->GetCurrentOutputNumber(), p_primary->GetCurrentOutputNumber());
            TS_ASSERT_EQUALS(p_fn_stepper->GetCurrentOutputPoint(), p_primary->GetCurrentOutputPoint() * 2);
        }
        TS_ASSERT(!p_fn_stepper->AtEnd());
        p_multi_stepper->Reset();
        TS_ASSERT_EQUALS(p_fn_stepper->GetCurrentOutputPoint(), 2.0);
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
        EnvironmentPtr p_env(new Environment);
        p_stst_runner->SetEnvironment(p_env);
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
        p_timecourse->SetEnvironment(p_env);
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
        unsigned N = 10;
        std::vector<AbstractExpressionPtr> args = EXPR_LIST(LOOKUP("i"))(CONST(N));
        DEFINE(cond, make_shared<MathmlLt>(args));
        MAKE_PTR_A(AbstractStepper, WhileStepper, p_stepper, ("i", "number", cond));
        EnvironmentPtr p_env(new Environment);
        p_stepper->SetEnvironment(p_env);
        p_stepper->Initialise();

        DoAbstractTest(p_stepper, "i", "number");
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

        // While i<2000 - test extending the range
        N = 2000;
        args = EXPR_LIST(LOOKUP("i2"))(CONST(N));
        DEFINE(cond2, make_shared<MathmlLt>(args));
        MAKE_PTR_A(AbstractStepper, WhileStepper, p_stepper2, ("i2", "number", cond2));
        p_stepper2->SetEnvironment(p_env);
        p_stepper2->Initialise();
        TS_ASSERT(!p_stepper2->IsEndFixed());
        TS_ASSERT_LESS_THAN(p_stepper2->GetNumberOfOutputPoints(), N);
        TS_ASSERT_EQUALS(p_stepper2->GetCurrentOutputNumber(), 0u);
        TS_ASSERT_EQUALS(p_stepper2->GetCurrentOutputPoint(), 0.0);

        for (unsigned i=0; i<N; i++)
        {
            TS_ASSERT(!p_stepper2->AtEnd());
            TS_ASSERT_EQUALS(p_stepper2->GetCurrentOutputPoint(), i);
            TS_ASSERT_EQUALS(p_stepper2->Step(), i+1);
        }
        TS_ASSERT(p_stepper2->AtEnd());
        TS_ASSERT_EQUALS(N, p_stepper2->GetNumberOfOutputPoints());

        // While i<0 - empty loop is impossble
        N = 0;
        args = EXPR_LIST(LOOKUP("i3"))(CONST(N));
        DEFINE(cond3, make_shared<MathmlLt>(args));
        MAKE_PTR_A(AbstractStepper, WhileStepper, p_stepper3, ("i3", "number", cond3));
        p_stepper3->SetEnvironment(p_env);
        p_stepper3->Initialise();
        TS_ASSERT(!p_stepper3->AtEnd());
        p_stepper3->Step();
        TS_ASSERT(p_stepper3->AtEnd());
    }

};

#endif // TESTSTEPPERS_HPP_
