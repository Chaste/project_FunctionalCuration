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

#ifndef TESTENVIRONMENT_HPP_
#define TESTENVIRONMENT_HPP_

#include <cxxtest/TestSuite.h>

#include <vector>
#include <string>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

#include "Environment.hpp"
#include "ValueTypes.hpp"
#include "ProtoHelperMacros.hpp"
#include "DebugProto.hpp"

#define NEW_ENV_A(name, args)  EnvironmentPtr _p_##name(new Environment args); Environment& name = *_p_##name;
#define NEW_ENV(name)  NEW_ENV_A(name, ())

class TestEnvironment : public CxxTest::TestSuite
{
public:
    void TestDefiningNames() throw (Exception)
    {
        NEW_ENV(env);
        AbstractValuePtr p_val = CV(123.4);
        const std::string name = "a_test_value";

        TS_ASSERT_THROWS_CONTAINS(env.Lookup(name), "Name " + name + " is not defined in this environment.");
        env.DefineName(name, p_val, "");
        TS_ASSERT_EQUALS(env.Lookup(name), p_val);
        TS_ASSERT_EQUALS(env.GetNumberOfDefinitions(), 1u);
        TS_ASSERT_EQUALS(env.GetDefinedNames(), boost::assign::list_of(name));
        TS_ASSERT_THROWS_CONTAINS(env.DefineName(name, p_val, ""),
                                  "Name " + name + " is already defined and may not be re-bound.");
        TS_ASSERT_THROWS_CONTAINS(env.OverwriteDefinition(name, p_val, ""),
                                  "This environment does not support overwriting mappings.");

        TS_ASSERT_THROWS_CONTAINS(env.Lookup(env.FreshIdent()), " is not defined in this environment.");

        std::vector<std::string> names = boost::assign::list_of("n1")("n2")("n3");
        std::vector<AbstractValuePtr> values = boost::assign::list_of(p_val)(p_val)(p_val);
        env.DefineNames(names, values, "");
        TS_ASSERT_EQUALS(env.GetNumberOfDefinitions(), 4u);
        TS_ASSERT_EQUALS(env.GetDefinedNames(), boost::assign::list_of(name).range(names));
        BOOST_FOREACH(const std::string& n, names)
        {
            TS_ASSERT_EQUALS(env.Lookup(n), p_val);
        }
        TS_ASSERT_THROWS_CONTAINS(env.DefineName(names[0], p_val, ""),
                                  "Name " + names[0] + " is already defined and may not be re-bound.");
        TS_ASSERT_THROWS_CONTAINS(env.DefineNames(names, values, ""),
                                  "Name " + names[0] + " is already defined and may not be re-bound.");

        NEW_ENV(env2);
        env2.Merge(env, "test");
        TS_ASSERT_EQUALS(env.GetDefinedNames(), env2.GetDefinedNames());

        // Test debug tracing of environments
        DebugProto::TraceEnv(env);
    }

    void TestOverwritingEnv() throw (Exception)
    {
        NEW_ENV_A(env, (true));
        const std::string name = "test_value";
        TS_ASSERT_THROWS_CONTAINS(env.Lookup(name), "Name " + name + " is not defined in this environment.");

        AbstractValuePtr p_val = CV(123.4);
        env.DefineName(name, p_val, "");
        TS_ASSERT_EQUALS(env.Lookup(name), p_val);

        AbstractValuePtr p_val2 = CV(432.1);
        TS_ASSERT_THROWS_CONTAINS(env.DefineName(name, p_val2, ""),
                                  "Name " + name + " is already defined and may not be re-bound.");
        env.OverwriteDefinition(name, p_val2, "");
        TS_ASSERT_EQUALS(env.Lookup(name), p_val2);
        TS_ASSERT_THROWS_CONTAINS(env.OverwriteDefinition("name2", p_val2, ""),
                                  "Name name2 is not defined and may not be overwritten.");
    }

    void TestDelegation() throw (Exception)
    {
        NEW_ENV(root_env);
        NEW_ENV_A(middle_env, (root_env.GetAsDelegatee()));
        NEW_ENV(top_env);
        top_env.SetDelegateeEnvironment(middle_env.GetAsDelegatee());

        TS_ASSERT(!root_env.GetDelegateeEnvironment());
        TS_ASSERT(middle_env.GetDelegateeEnvironment());
        TS_ASSERT(top_env.GetDelegateeEnvironment());
        TS_ASSERT(middle_env.GetDelegateeEnvironment().get() == root_env.GetAsDelegatee().get());
        TS_ASSERT(top_env.GetDelegateeEnvironment().get() == middle_env.GetAsDelegatee().get());

        const std::string name = "test_value";
        AbstractValuePtr p_val = CV(123.4);
        root_env.DefineName(name, p_val, "");
        TS_ASSERT_EQUALS(top_env.Lookup(name), p_val);

        AbstractValuePtr p_val2 = CV(432.1);
        middle_env.DefineName(name, p_val2, "");
        TS_ASSERT_EQUALS(top_env.Lookup(name), p_val2);

        AbstractValuePtr p_val3 = CV(6.5);
        top_env.DefineName(name, p_val3, "");
        TS_ASSERT_EQUALS(top_env.Lookup(name), p_val3);
    }

    void TestPrefixedDelegation() throw (Exception)
    {
        NEW_ENV(root_env);
        NEW_ENV_A(env, (root_env.GetAsDelegatee()));
        NEW_ENV(env_a);
        NEW_ENV(env_b);
        root_env.SetDelegateeEnvironment(env_a.GetAsDelegatee(), "a");
        root_env.SetDelegateeEnvironment(env_b.GetAsDelegatee(), "b");

        TS_ASSERT(root_env.GetDelegateeEnvironment("a").get() == env_a.GetAsDelegatee().get());
        TS_ASSERT(root_env.GetDelegateeEnvironment("b").get() == env_b.GetAsDelegatee().get());
        TS_ASSERT(!env.GetDelegateeEnvironment("a"));
        TS_ASSERT(!env.GetDelegateeEnvironment("b"));
        TS_ASSERT(!env.GetDelegateeEnvironment("c"));

        AbstractValuePtr p_one = CV(1);
        env_a.DefineName("a", p_one, "");
        AbstractValuePtr p_two = CV(2);
        env_b.DefineName("b", p_two, "");

        TS_ASSERT_THROWS_CONTAINS(env.DefineName("a:b", p_one, ""),
                                  "Names such as 'a:b' containing a colon are not allowed.");

        TS_ASSERT_EQUALS(root_env.Lookup("a:a"), p_one);
        TS_ASSERT_EQUALS(root_env.Lookup("b:b"), p_two);
        TS_ASSERT_EQUALS(env.Lookup("a:a"), p_one);
        TS_ASSERT_EQUALS(env.Lookup("b:b"), p_two);

        TS_ASSERT_THROWS_CONTAINS(env.Lookup("a:n"), "Name n is not defined in this environment.");
        TS_ASSERT_THROWS_CONTAINS(env.Lookup("c:c"), "No environment associated with the prefix 'c'.");
        TS_ASSERT_THROWS_CONTAINS(root_env.Lookup("a:n"), "Name n is not defined in this environment.");
        TS_ASSERT_THROWS_CONTAINS(root_env.Lookup("c:c"), "No environment associated with the prefix 'c'.");

        NEW_ENV(env_aa);
        env_a.SetDelegateeEnvironment(env_aa.GetAsDelegatee(), "a");
        AbstractValuePtr p_three = CV(3);
        env_aa.DefineName("a", p_three, "");
        TS_ASSERT_EQUALS(env_a.Lookup("a:a"), p_three);
        TS_ASSERT_EQUALS(env.Lookup("a:a:a"), p_three);
    }

    void TestOverwritingDelegation() throw (Exception)
    {
        NEW_ENV_A(root_env, (true));
        NEW_ENV_A(env1, (root_env.GetAsDelegatee()));
        NEW_ENV(env2);
        const std::string prefix("root");
        env2.SetDelegateeEnvironment(root_env.GetAsDelegatee(), prefix);

        const std::string name("name");
        AbstractValuePtr p_val = CV(5.5);
        root_env.DefineName(name, p_val, "");
        TS_ASSERT_EQUALS(env1.Lookup(name), p_val);
        TS_ASSERT_EQUALS(env2.Lookup(prefix + ":" + name), p_val);

        AbstractValuePtr p_val2 = CV(6.6);
        AbstractValuePtr p_val3 = CV(7.7);
        TS_ASSERT_THROWS_CONTAINS(root_env.DefineName(name, p_val2, ""),
                                  "Name " + name + " is already defined and may not be re-bound.");
        env1.OverwriteDefinition(name, p_val2, "");
        TS_ASSERT_EQUALS(root_env.Lookup(name), p_val2);
        env2.OverwriteDefinition(prefix + ":" + name, p_val3, "");
        TS_ASSERT_EQUALS(root_env.Lookup(name), p_val3);

        env1.DefineName(name, p_val, "");
        TS_ASSERT_THROWS_CONTAINS(env1.OverwriteDefinition(name, p_val2, ""),
                                  "This environment does not support overwriting mappings.");
        env2.DefineName(name, p_val, "");
        TS_ASSERT_THROWS_CONTAINS(env2.OverwriteDefinition(name, p_val2, ""),
                                  "This environment does not support overwriting mappings.");
    }
};

#endif // TESTENVIRONMENT_HPP_
