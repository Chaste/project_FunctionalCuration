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

class TestEnvironment : public CxxTest::TestSuite
{
public:
    void TestDefiningNames() throw (Exception)
    {
        Environment env;
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
    }

    void TestOverwritingEnv() throw (Exception)
    {
        Environment env(true);
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
        Environment root_env;
        Environment middle_env(root_env.GetAsDelegatee());
        Environment top_env;
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
        Environment root_env;
        Environment env(root_env.GetAsDelegatee());
        Environment env_a;
        Environment env_b;
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

        Environment env_aa;
        env_a.SetDelegateeEnvironment(env_aa.GetAsDelegatee(), "a");
        AbstractValuePtr p_three = CV(3);
        env_aa.DefineName("a", p_three, "");
        TS_ASSERT_EQUALS(env_a.Lookup("a:a"), p_three);
        TS_ASSERT_EQUALS(env.Lookup("a:a:a"), p_three);
    }

    void TestOverwritingDelegation() throw (Exception)
    {
        Environment root_env(true);
        Environment env1(root_env.GetAsDelegatee());
        Environment env2;
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
