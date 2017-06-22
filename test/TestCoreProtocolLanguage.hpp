/*

Copyright (c) 2005-2016, University of Oxford.
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

#ifndef TESTCOREPROTOCOLLANGUAGE_HPP_
#define TESTCOREPROTOCOLLANGUAGE_HPP_

#include <cxxtest/TestSuite.h>

#include <boost/assign/list_of.hpp>
#include <boost/make_shared.hpp> // Requires Boost 1.39 (available in Lucid and newer)

#include "ProtocolLanguage.hpp"

#include "OutputFileHandler.hpp"
#include "FileFinder.hpp"
#include "DebugProto.hpp"

#include "FakePetscSetup.hpp"

class TestCoreProtocolLanguage : public CxxTest::TestSuite
{
    NdArray<double> LookupArray(const Environment& rEnv, const std::string& rName)
    {
        AbstractValuePtr wrapped_array = rEnv.Lookup(rName);
        TS_ASSERT(wrapped_array->IsArray());
        return static_cast<ArrayValue*>(wrapped_array.get())->GetArray();
    }

public:
    void TestBasics() throw (Exception)
    {
        /*
         * Create the language fragment to be interpreted
         */
        std::vector<AbstractStatementPtr> program;
        // sum = lambda a, dim=default: fold(mathml.plus, a, 0, dim)
        {
            DEFINE(plus, LambdaExpression::WrapMathml<MathmlPlus>(2));
            DEFINE(a, LOOKUP("a"));
            DEFINE(dim, LOOKUP("dim"));
            DEFINE(zero, CONST(0.0));
            DEFINE(fold, boost::make_shared<Fold>(plus, a, zero, dim));
            std::vector<std::string> fps = {"a", "dim"};
            std::vector<AbstractValuePtr> defaults(2u);
            defaults.back() = boost::make_shared<DefaultParameter>();
            DEFINE(sum, boost::make_shared<LambdaExpression>(fps, fold, defaults)); // Second constructor - implicit return stmt
            program.push_back(ASSIGN_STMT("sum", sum)); LOC(program.back());
        }
        // max = lambda a, dim: fold(mathml.max, a, DOUBLE_UNSET<default>, dim)
        {
            DEFINE(mathml_max, LambdaExpression::WrapMathml<MathmlMax>(2));
            DEFINE(a, LOOKUP("a"));
            DEFINE(dim, LOOKUP("dim"));
            DEFINE(default_, DEFAULT_EXPR);
            DEFINE(fold, boost::make_shared<Fold>(mathml_max, a, default_, dim));
            std::vector<std::string> fps = {"a", "dim"};
            DEFINE(max, boost::make_shared<LambdaExpression>(fps, fold)); // Second constructor - implicit return stmt
            program.push_back(ASSIGN_STMT("max", max)); LOC(program.back());
        }
        // input = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }
        {
            std::vector<AbstractExpressionPtr> elements;
            for (unsigned i=0; i<10; ++i)
            {
                elements.push_back(CONST(i+1)); LOC(elements.back());
            }
            DEFINE(array, boost::make_shared<ArrayCreate>(elements));
            program.push_back(ASSIGN_STMT("input", array)); LOC(program.back());
        }
        // input_sum = sum(input)  --> 55
        {
            std::vector<AbstractExpressionPtr> inputs;
            inputs.push_back(LOOKUP("input")); LOC(inputs.back());
            DEFINE(call, boost::make_shared<FunctionCall>("sum", inputs));
            program.push_back(ASSIGN_STMT("input_sum", call)); LOC(program.back());
        }
        // input2d = { { 1, 3, 5 }, { 6, 4, 2 } }
        {
            std::vector<AbstractExpressionPtr> elements;
            for (unsigned i=0; i<3; ++i)
            {
                elements.push_back(CONST(i*2+1)); LOC(elements.back());
            }
            DEFINE(array1, boost::make_shared<ArrayCreate>(elements));
            elements.clear();
            for (unsigned i=0; i<3; ++i)
            {
                elements.push_back(CONST(6 - i*2)); LOC(elements.back());
            }
            DEFINE(array2, boost::make_shared<ArrayCreate>(elements));
            elements = {array1, array2};
            DEFINE(array, boost::make_shared<ArrayCreate>(elements));
            program.push_back(ASSIGN_STMT("input2d", array)); LOC(program.back());
        }
        // input2d_max = max(input2d, 0)  --> {{ 6, 4, 5 }}
        {
            std::vector<AbstractExpressionPtr> inputs;
            inputs.push_back(LOOKUP("input2d")); LOC(inputs.back());
            inputs.push_back(VALUE(SimpleValue, 0.0)); LOC(inputs.back());
            AbstractExpressionPtr call = boost::make_shared<FunctionCall>("max", inputs); LOC(call);
            program.push_back(ASSIGN_STMT("input2d_max", call)); LOC(program.back());
        }
        // input2d_slice = input2d[<1>1:2]  --> {{3}, {4}}
        {
            DEFINE_TUPLE(dim_default, EXPR_LIST(NULL_EXPR)(NULL_EXPR)(CONST(1))(NULL_EXPR));
            DEFINE_TUPLE(dim1, EXPR_LIST(CONST(1))(CONST(1))(CONST(1))(CONST(2)));
            std::vector<AbstractExpressionPtr> view_args = {dim1, dim_default};
            AbstractExpressionPtr view = boost::make_shared<View>(LOOKUP("input2d"), view_args); LOC(view);
            program.push_back(ASSIGN_STMT("input2d_slice", view)); LOC(program.back());
        }
        // input2 = { 1, 2, 3 }
        {
            NdArray<double>::Extents extents = {3u};
            NdArray<double> input2(extents);
            int i=1;
            for (NdArray<double>::Iterator it = input2.Begin(); it != input2.End(); ++it)
            {
                *it = i++;
            }
            program.push_back(ASSIGN_STMT("input2", VALUE(ArrayValue, input2))); LOC(program.back());
        }
        // map_result = map(mathml.plus, input2d_max[0][:], input2)  --> { 7, 6, 8 }
        {
            DEFINE_TUPLE(dim0, EXPR_LIST(CONST(0))(CONST(0))(CONST(0)));
            DEFINE_TUPLE(dim1, EXPR_LIST(NULL_EXPR)(CONST(1))(NULL_EXPR));
            std::vector<AbstractExpressionPtr> view_args = {dim0, dim1};
            AbstractExpressionPtr view = boost::make_shared<View>(LOOKUP("input2d_max"), view_args); LOC(view);
            std::vector<AbstractExpressionPtr> map_args;
            map_args.push_back(LambdaExpression::WrapMathml<MathmlPlus>(2)); LOC(map_args.back());
            map_args.push_back(view);
            map_args.push_back(LOOKUP("input2")); LOC(map_args.back());
            AbstractExpressionPtr map = boost::make_shared<Map>(map_args); LOC(map);
            program.push_back(ASSIGN_STMT("map_result", map)); LOC(program.back());
        }
        // adder = lambda input: map(mathml.plus, input, input2)
        {
            std::vector<AbstractExpressionPtr> map_args;
            map_args.push_back(LambdaExpression::WrapMathml<MathmlPlus>(2));
            map_args.push_back(LOOKUP("input")); LOC(map_args.back());
            map_args.push_back(LOOKUP("input2")); LOC(map_args.back());
            AbstractExpressionPtr map = boost::make_shared<Map>(map_args); LOC(map);
            std::vector<std::string> fps = {"input"};
            AbstractExpressionPtr lambda = boost::make_shared<LambdaExpression>(fps, map); LOC(lambda); // Second constructor - implicit return stmt
            program.push_back(ASSIGN_STMT("adder", lambda)); LOC(program.back());
        }
        // double_input2 = adder(input2)  --> {2,4,6}
        {
            std::vector<AbstractExpressionPtr> args = {LOOKUP("input2")}; LOC(args.back());
            AbstractExpressionPtr call = boost::make_shared<FunctionCall>("adder", args); LOC(call);
            program.push_back(ASSIGN_STMT("double_input2", call)); LOC(program.back());
        }

        /*
         * Do the interpretation
         */
        EnvironmentPtr p_env(new Environment);
        Environment& env = *p_env; // Save typing!
        env.ExecuteStatements(program);

        /* Extract and test results */
        NdArray<double> sum_array = LookupArray(env, "input_sum"); // Should be {55}
        TS_ASSERT(GET_SIMPLE_VALUE(Accessor(LOOKUP("input_sum"), Accessor::IS_ARRAY)(env)));
        TS_ASSERT(!GET_SIMPLE_VALUE(Accessor(LOOKUP("input_sum"), Accessor::IS_SIMPLE_VALUE)(env)));
        TS_ASSERT_EQUALS(sum_array.GetNumDimensions(), 1u);
        TS_ASSERT_EQUALS(GET_SIMPLE_VALUE(Accessor(LOOKUP("input_sum"), Accessor::NUM_DIMS)(env)), 1);
        TS_ASSERT_EQUALS(sum_array.GetNumElements(), 1u);
        TS_ASSERT_EQUALS(GET_SIMPLE_VALUE(Accessor(LOOKUP("input_sum"), Accessor::NUM_ELEMENTS)(env)), 1);
        NdArray<double>::ConstIterator p_result_elt = sum_array.Begin();
        TS_ASSERT_EQUALS(*p_result_elt, 55.0);

        NdArray<double> max_result_array = LookupArray(env, "input2d_max"); // Should be {{6,4,5}}
        TS_ASSERT_EQUALS(max_result_array.GetNumDimensions(), 2u);
        TS_ASSERT_EQUALS(max_result_array.GetShape()[0], 1u);
        TS_ASSERT_EQUALS(max_result_array.GetNumElements(), 3u);
        p_result_elt = max_result_array.Begin();
        TS_ASSERT_EQUALS(*p_result_elt++, 6.0);
        TS_ASSERT_EQUALS(*p_result_elt++, 4.0);
        TS_ASSERT_EQUALS(*p_result_elt, 5.0);
        NdArray<double> max_result_shape = GET_ARRAY(Accessor(LOOKUP("input2d_max"), Accessor::SHAPE)(env));
        TS_ASSERT_EQUALS(max_result_shape.GetNumDimensions(), 1u);
        TS_ASSERT_EQUALS(max_result_shape.GetNumElements(), 2u);
        p_result_elt = max_result_shape.Begin();
        TS_ASSERT_EQUALS(*p_result_elt++, 1.0);
        TS_ASSERT_EQUALS(*p_result_elt, 3.0);

        NdArray<double> slice_result_array = LookupArray(env, "input2d_slice"); // Should be {{3}, {4}}
        TS_ASSERT_EQUALS(slice_result_array.GetNumDimensions(), 2u);
        TS_ASSERT_EQUALS(slice_result_array.GetShape()[0], 2u);
        TS_ASSERT_EQUALS(slice_result_array.GetNumElements(), 2u);
        p_result_elt = slice_result_array.Begin();
        TS_ASSERT_EQUALS(*p_result_elt++, 3.0);
        TS_ASSERT_EQUALS(*p_result_elt, 4.0);

        NdArray<double> input2_array = LookupArray(env, "input2"); // Should be {1,2,3}
        TS_ASSERT_EQUALS(input2_array.GetNumDimensions(), 1u);
        TS_ASSERT_EQUALS(input2_array.GetShape()[0], 3u);
        TS_ASSERT_EQUALS(input2_array.GetNumElements(), 3u);
        p_result_elt = input2_array.Begin();
        TS_ASSERT_EQUALS(*p_result_elt++, 1.0);
        TS_ASSERT_EQUALS(*p_result_elt++, 2.0);
        TS_ASSERT_EQUALS(*p_result_elt, 3.0);

        NdArray<double> map_result_array = LookupArray(env, "map_result"); // Should be {7,6,8}
        TS_ASSERT_EQUALS(map_result_array.GetNumDimensions(), 1u);
        TS_ASSERT_EQUALS(map_result_array.GetShape()[0], 3u);
        TS_ASSERT_EQUALS(map_result_array.GetNumElements(), 3u);
        p_result_elt = map_result_array.Begin();
        TS_ASSERT_EQUALS(*p_result_elt++, 7.0);
        TS_ASSERT_EQUALS(*p_result_elt++, 6.0);
        TS_ASSERT_EQUALS(*p_result_elt, 8.0);

        NdArray<double> double_input2_array = LookupArray(env, "double_input2"); // Should be {2,4,6}
        TS_ASSERT_EQUALS(double_input2_array.GetNumDimensions(), 1u);
        TS_ASSERT_EQUALS(double_input2_array.GetShape()[0], 3u);
        TS_ASSERT_EQUALS(double_input2_array.GetNumElements(), 3u);
        p_result_elt = double_input2_array.Begin();
        TS_ASSERT_EQUALS(*p_result_elt++, 2.0);
        TS_ASSERT_EQUALS(*p_result_elt++, 4.0);
        TS_ASSERT_EQUALS(*p_result_elt, 6.0);

        /*
         * Test some exceptions / backtraces
         */
        {
            // sum(null)
            std::vector<AbstractExpressionPtr> inputs;
            inputs.push_back(NULL_EXPR); LOC(inputs.back());
            inputs.push_back(DEFAULT_EXPR); LOC(inputs.back());
            AbstractExpressionPtr call = boost::make_shared<FunctionCall>("sum", inputs); LOC(call);
            AbstractStatementPtr stmt = boost::make_shared<AssignmentStatement>("bad_sum", call); LOC(stmt);
            std::vector<AbstractStatementPtr> stmts = {stmt};
            std::string error_msg(
                    "Protocol backtrace (most recent call last):\n"
                    "  ./projects/FunctionalCuration/test/TestCoreProtocolLanguage.hpp:236:0     stmt\n"
                    "  ./projects/FunctionalCuration/test/TestCoreProtocolLanguage.hpp:235:0     call\n"
                    "  ./projects/FunctionalCuration/test/TestCoreProtocolLanguage.hpp:68:0      sum\n"
                    "  <anon>\n"
                    "  ./projects/FunctionalCuration/test/TestCoreProtocolLanguage.hpp:64:0      fold\n"
                    "Second argument to fold should be an array.");
            TS_ASSERT_THROWS_CONTAINS(env.ExecuteStatements(stmts), "fold\nSecond argument to fold should be an array.");
            // And again but with tracing
            OutputFileHandler handler("TestCoreProtocolLanguage_Tracing");
            FileFinder trace = handler.FindFile("trace.txt");
            DebugProto::SetTraceFolder(handler);
            TS_ASSERT(DebugProto::IsTracing());
            TS_ASSERT(trace.Exists());
            TS_ASSERT(trace.IsEmpty());
            TS_ASSERT_THROWS_CONTAINS(env.ExecuteStatements(stmts), "fold\nSecond argument to fold should be an array.");
            DebugProto::StopTracing();
            TS_ASSERT(trace.Exists());
            TS_ASSERT(!trace.IsEmpty());
        }
        {
            DEFINE_STMT(assert0, ASSERT_STMT(CONST(0)));
            TS_ASSERT_THROWS_CONTAINS(env.ExecuteStatement(assert0), "Assertion failed");
        }
    }

    void TestFindAndIndex() throw (Exception)
    {
        EnvironmentPtr p_env(new Environment);
        Environment& env = *p_env; // Save typing!

        // Create a 2d array to operate on
        NdArray<double>::Extents input_shape = {3, 5};
        NdArray<double> input(input_shape);
        for (unsigned i=0; i<input_shape[0]; ++i)
        {
            for (unsigned j=0; j<input_shape[1]; ++j)
            {
                NdArray<double>::Indices ij = {i, j};
                input[ij] = i*input_shape[1] + j;
            }
        }
        env.ExecuteStatement(ASSIGN_STMT("input", VALUE(ArrayValue, input)));
        const double input_max = 14;
        DEFINE(zero, CONST(0));
        DEFINE(one, CONST(1));
        DEFINE(two, CONST(2));
        DEFINE(minus_one, CONST(-1));
        std::vector<std::string> single_anon_param = {"_1"};
        AbstractExpressionPtr lookup_anon_param = LOOKUP("_1");

        {
            std::cout << " - All but one entry" << std::endl;
            std::vector<AbstractStatementPtr> program;
            DEFINE(largest_number, CONST(input_max));
            std::vector<AbstractExpressionPtr> lt_args = {lookup_anon_param, largest_number};
            AbstractExpressionPtr f = boost::make_shared<LambdaExpression>(single_anon_param, boost::make_shared<MathmlLt>(lt_args)); LOC(f);
            std::vector<AbstractExpressionPtr> map_args = {f, LOOKUP("input")};
            AbstractExpressionPtr map = boost::make_shared<Map>(map_args); LOC(map);
            program.push_back(boost::make_shared<AssignmentStatement>("all_bar_largest", map)); LOC(program.back());
            AbstractExpressionPtr find = boost::make_shared<Find>(LOOKUP("all_bar_largest")); LOC(find);
            program.push_back(ASSIGN_STMT("all_bar_largest_idxs", find)); LOC(program.back());
            DEFINE(replace_with, CONST(-1));
            AbstractExpressionPtr index = boost::make_shared<Index>(LOOKUP("input"), LOOKUP("all_bar_largest_idxs"),
                                                             zero, DEFAULT_EXPR, one, replace_with); LOC(index);
            program.push_back(ASSIGN_STMT("all_bar_largest_indexed", index)); LOC(program.back());
            env.ExecuteStatements(program);

            // Check that we got back all the original entries (except the largest number which is replaced by -1.
            NdArray<double> result_array = LookupArray(env, "all_bar_largest_indexed");
            TS_ASSERT_EQUALS(result_array.GetNumDimensions(), 2u);
            TS_ASSERT_EQUALS(result_array.GetNumElements(), input.GetNumElements());
            NdArray<double>::Indices idxs = input.GetIndices();
            for (NdArray<double>::Index i=0; i<input.GetNumElements(); ++i)
            {
                if (i == input.GetNumElements() - 1)
                {   // Largest number
                    TS_ASSERT_EQUALS(result_array[idxs], -1);
                }
                else
                {
                    TS_ASSERT_EQUALS(result_array[idxs], input[idxs]);
                }
                input.IncrementIndices(idxs);
            }
        }

        {
            std::cout << " - Whole array" << std::endl;
            std::vector<AbstractStatementPtr> program;
            DEFINE(largest_number, CONST(input_max+1));
            std::vector<AbstractExpressionPtr> lt_args = {lookup_anon_param, largest_number};
            AbstractExpressionPtr f = boost::make_shared<LambdaExpression>(single_anon_param, boost::make_shared<MathmlLt>(lt_args)); LOC(f);
            std::vector<AbstractExpressionPtr> map_args = {f, LOOKUP("input")};
            AbstractExpressionPtr map = boost::make_shared<Map>(map_args); LOC(map);
            program.push_back(boost::make_shared<AssignmentStatement>("all_entries", map)); LOC(program.back());
            AbstractExpressionPtr find = boost::make_shared<Find>(LOOKUP("all_entries")); LOC(find);
            program.push_back(ASSIGN_STMT("all_entries_idxs", find)); LOC(program.back());
            // Shouldn't need padding this time
            AbstractExpressionPtr index = boost::make_shared<Index>(LOOKUP("input"), LOOKUP("all_entries_idxs"), zero); LOC(index);
            program.push_back(ASSIGN_STMT("all_entries_indexed", index)); LOC(program.back());
            env.ExecuteStatements(program);

            // Check that we got back all the original entries
            NdArray<double> result_array = LookupArray(env, "all_entries_indexed");
            TS_ASSERT_EQUALS(result_array.GetNumDimensions(), 2u);
            TS_ASSERT_EQUALS(result_array.GetNumElements(), input.GetNumElements());
            NdArray<double>::Indices idxs = input.GetIndices();
            for (NdArray<double>::Index i=0; i<input.GetNumElements(); ++i)
            {
                TS_ASSERT_EQUALS(result_array[idxs], input[idxs]);
                input.IncrementIndices(idxs);
            }
        }

        {
            std::cout << " - Odd entries" << std::endl;
            // Find odd entries
            std::vector<AbstractStatementPtr> program;
            std::vector<AbstractExpressionPtr> rem_args = {lookup_anon_param, two};
            AbstractExpressionPtr f = boost::make_shared<LambdaExpression>(single_anon_param, boost::make_shared<MathmlRem>(rem_args)); LOC(f);
            std::vector<AbstractExpressionPtr> map_args = {f, LOOKUP("input")};
            AbstractExpressionPtr map = boost::make_shared<Map>(map_args); LOC(map);
            program.push_back(ASSIGN_STMT("odd_entries", map)); LOC(program.back());
            AbstractExpressionPtr find = boost::make_shared<Find>(LOOKUP("odd_entries")); LOC(find);
            program.push_back(ASSIGN_STMT("odd_indices", find)); LOC(program.back());
            env.ExecuteStatements(program);

            // Some basic testing
            NdArray<double> is_odd = LookupArray(env, "odd_entries");
            NdArray<double>::ConstIterator ptr = is_odd.Begin();
            TS_ASSERT_EQUALS(*ptr++, 0); // Entry [0,0] = 0 even
            TS_ASSERT_EQUALS(*ptr++, 1); // Entry [0,1] = 1 odd
            TS_ASSERT_EQUALS(*ptr++, 0); // Entry [0,2] = 2 even

            NdArray<double> odd_indices = LookupArray(env, "odd_indices");
            TS_ASSERT_EQUALS(odd_indices.GetNumDimensions(), 2u);
            TS_ASSERT_EQUALS(odd_indices.GetShape()[0], 7u);
            TS_ASSERT_EQUALS(odd_indices.GetShape()[1], 2u);
            ptr = odd_indices.Begin();
            TS_ASSERT_EQUALS(*ptr++, 0);
            TS_ASSERT_EQUALS(*ptr++, 1); // [0,1] is odd
            TS_ASSERT_EQUALS(*ptr++, 0);
            TS_ASSERT_EQUALS(*ptr++, 3); // [0,3] is odd

            // Basic exceptions
            //TS_ASSERT_THROWS_CONTAINS((*boost::make_shared<Index>(not_2d_array, odd_indices))(env), "Indices are the wrong size for this operand");
            //TS_ASSERT_THROWS_CONTAINS((*boost::make_shared<Index>(not_array, odd_indices))(env), "First argument to index must be an array");
            // Indexing using odd_indices should fail because it would give an irregular array
            TS_ASSERT_THROWS_CONTAINS((*boost::make_shared<Index>(LOOKUP("input"), LOOKUP("odd_indices")))(env),
                                      "Cannot index if the result is irregular");
            // However, if we tell it to shrink the output, we're ok...
            env.ExecuteStatement(ASSIGN_STMT("shrink_right", boost::make_shared<Index>(LOOKUP("input"), LOOKUP("odd_indices"), one, one)));
            env.ExecuteStatement(ASSIGN_STMT("shrink_left", boost::make_shared<Index>(LOOKUP("input"), LOOKUP("odd_indices"), one, minus_one)));
            NdArray<double> shrink_right = LookupArray(env, "shrink_right");
            TS_ASSERT_EQUALS(shrink_right.GetShape()[0], 3u);
            TS_ASSERT_EQUALS(shrink_right.GetShape()[1], 2u);
            NdArray<double> shrink_left = LookupArray(env, "shrink_left");
            TS_ASSERT_EQUALS(shrink_left.GetShape()[0], 3u);
            TS_ASSERT_EQUALS(shrink_left.GetShape()[1], 2u);

            // Explicitly construct a regular index
            NdArray<double>::Extents shape = {6, 2};
            NdArray<double> indices(shape);
            std::vector<unsigned> indices_2 = ({1},3,0,2,1,3);
            NdArray<double>::Indices idxs = indices.GetIndices();
            for (unsigned i=0; i<shape[0]; ++i)
            {
                indices[idxs] = i/2;
                indices.IncrementIndices(idxs);
                indices[idxs] = indices_2[i];
                indices.IncrementIndices(idxs);
            }
            env.ExecuteStatement(ASSIGN_STMT("some_odd_entries", boost::make_shared<Index>(LOOKUP("input"), VALUE(ArrayValue, indices), one)));
            NdArray<double> some_odd_entries = LookupArray(env, "some_odd_entries");
            TS_ASSERT_EQUALS(some_odd_entries.GetShape()[0], 3u);
            TS_ASSERT_EQUALS(some_odd_entries.GetShape()[1], 2u);

            /* Check results.
             * Input was:
             *   0  1  2  3  4
             *   5  6  7  8  9
             *   10 11 12 13 14
             * So irregular output would be:
             *   1  3
             *   5  7  9
             *   11 13
             * some_odd_entries and shrink_right drop the 9; shrink_left drops the 5.
             */
            for (unsigned i=0; i<3u; ++i)
            {
                for (unsigned j=0; j<2u; ++j)
                {
                    NdArray<double>::Indices ij = {i, j};
                    TS_ASSERT_EQUALS(fmod(some_odd_entries[ij], 2), 1.0);
                    TS_ASSERT_EQUALS(fmod(shrink_right[ij], 2), 1.0);
                    TS_ASSERT_EQUALS(fmod(shrink_left[ij], 2), 1.0);

                    const double input_entry = i*input_shape[1] + indices_2[i*2+j];
                    TS_ASSERT_EQUALS(some_odd_entries[ij], input_entry);
                    TS_ASSERT_EQUALS(shrink_right[ij], input_entry);
                    if (i == 1)
                    {
                        TS_ASSERT_EQUALS(shrink_left[ij], 2+input_entry);
                    }
                    else
                    {
                        TS_ASSERT_EQUALS(shrink_left[ij], input_entry);
                    }
                }
            }

            /* Now try with padding instead.  Result should be:
             *   1  3  P
             *   5  7  9
             *   11 13 P
             * for right padding, and
             *   P  1  3
             *   5  7  9
             *   P  11 13
             * for left padding.
             */
            TS_ASSERT_THROWS_CONTAINS((*boost::make_shared<Index>(LOOKUP("input"), LOOKUP("odd_indices"), one, one, one))(env),
                                      "You cannot both pad and shrink!");
            env.ExecuteStatement(ASSIGN_STMT("pad_right", boost::make_shared<Index>(LOOKUP("input"), LOOKUP("odd_indices"), one, zero, one)));
            double padding = 55.55;
            DEFINE(pad_value, CONST(padding));
            env.ExecuteStatement(ASSIGN_STMT("pad_left", boost::make_shared<Index>(LOOKUP("input"), LOOKUP("odd_indices"), one, zero, minus_one, pad_value)));
            NdArray<double> pad_right = LookupArray(env, "pad_right");
            NdArray<double> pad_left = LookupArray(env, "pad_left");
            TS_ASSERT_EQUALS(pad_right.GetShape()[0], 3u);
            TS_ASSERT_EQUALS(pad_right.GetShape()[1], 3u);
            TS_ASSERT_EQUALS(pad_left.GetShape()[0], 3u);
            TS_ASSERT_EQUALS(pad_left.GetShape()[1], 3u);

            for (unsigned i=0; i<3; ++i)
            {
                for (unsigned j=0; j<3; ++j)
                {
                    NdArray<double>::Indices ij = {i, j};
                    double r = pad_right[ij];
                    double l = pad_left[ij];
                    if (i == 1)
                    {
                        TS_ASSERT_EQUALS(fmod(l, 2), 1.0);
                        TS_ASSERT_EQUALS(fmod(r, 2), 1.0);
                        TS_ASSERT_EQUALS(l, 5 + j*2);
                        TS_ASSERT_EQUALS(r, 5 + j*2);
                    }
                    else
                    {
                        if (l != padding)
                        {
                            TS_ASSERT_EQUALS(fmod(l, 2), 1.0);
                            TS_ASSERT_EQUALS(l, i*input_shape[1] + indices_2[i*2+j-1]);
                        }
                        else
                        {
                            TS_ASSERT(j == 0);
                        }
                        if (r != DBL_MAX)
                        {
                            TS_ASSERT_EQUALS(fmod(r, 2), 1.0);
                            TS_ASSERT_EQUALS(r, i*input_shape[1] + indices_2[i*2+j]);
                        }
                        else
                        {
                            TS_ASSERT(j == 2);
                        }
                    }
                }
            }
        }
    }

    void TestIf() throw (Exception)
    {
        EnvironmentPtr p_env(new Environment);
        // if (1, 2, 3)  --> 2
        {
            DEFINE(if_t, boost::make_shared<If>(CONST(1), CONST(2), CONST(3)));
            AbstractValuePtr p_result = (*if_t)(*p_env);
            TS_ASSERT(p_result->IsDouble());
            TS_ASSERT_EQUALS(GET_SIMPLE_VALUE(p_result), 2.0);
        }
        // if (0, 2, 3)  --> 3
        {
            DEFINE(if_t, boost::make_shared<If>(CONST(0), CONST(2), CONST(3)));
            AbstractValuePtr p_result = (*if_t)(*p_env);
            TS_ASSERT(p_result->IsDouble());
            TS_ASSERT_EQUALS(GET_SIMPLE_VALUE(p_result), 3.0);
        }
    }

    void TestArrayComprehensions() throw (Exception)
    {
        EnvironmentPtr p_env(new Environment);
        // counting1d = { i for #0#i=0:10 }  --> {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
        {
            DEFINE_TUPLE(i_range, EXPR_LIST(CONST(0))(CONST(0))(CONST(1))(CONST(10))(VALUE(StringValue, "i")));
            std::vector<AbstractExpressionPtr> comp_args = {i_range};
            DEFINE(array_exp, boost::make_shared<ArrayCreate>(LOOKUP("i"), comp_args));
            AbstractValuePtr p_array = (*array_exp)(*p_env);
            TS_ASSERT(p_array->IsArray());
            NdArray<double> array = static_cast<ArrayValue*>(p_array.get())->GetArray();
            TS_ASSERT_EQUALS(array.GetNumDimensions(), 1u);
            TS_ASSERT_EQUALS(array.GetNumElements(), 10u);
            TS_ASSERT_EQUALS(array.GetShape()[0], 10u);
            NdArray<double>::Iterator it = array.Begin();
            for (NdArray<double>::Index i=0; i<10u; ++i)
            {
                TS_ASSERT_EQUALS(*it++, i);
            }
        }

        // counting2d = { i * 3 + j for #0#i=1:3, #1#j=0:3 }  --> { {3,4,5}, {6,7,8} }
        {
            DEFINE_TUPLE(i_range, EXPR_LIST(CONST(0))(CONST(1))(CONST(1))(CONST(3))(VALUE(StringValue, "i")));
            DEFINE_TUPLE(j_range, EXPR_LIST(CONST(1))(CONST(0))(CONST(1))(CONST(3))(VALUE(StringValue, "j")));
            std::vector<AbstractExpressionPtr> comp_args = {i_range, j_range};
            std::vector<AbstractExpressionPtr> args = boost::assign::list_of<AbstractExpressionPtr>(LOOKUP("i"))(CONST(3));
            DEFINE(times, boost::make_shared<MathmlTimes>(args));
            args = boost::assign::list_of<AbstractExpressionPtr>(times)(LOOKUP("j"));
            DEFINE(plus, boost::make_shared<MathmlPlus>(args));
            DEFINE(array_exp, boost::make_shared<ArrayCreate>(plus, comp_args));
            AbstractValuePtr p_array = (*array_exp)(*p_env);
            TS_ASSERT(p_array->IsArray());
            NdArray<double> array = static_cast<ArrayValue*>(p_array.get())->GetArray();
            TS_ASSERT_EQUALS(array.GetNumDimensions(), 2u);
            TS_ASSERT_EQUALS(array.GetNumElements(), 6u);
            TS_ASSERT_EQUALS(array.GetShape()[0], 2u);
            TS_ASSERT_EQUALS(array.GetShape()[1], 3u);
            NdArray<double>::Iterator it = array.Begin();
            for (NdArray<double>::Index i=0; i<6u; ++i)
            {
                TS_ASSERT_EQUALS(*it++, i+3);
            }
        }

        // blocks = { {{-10+j,j},{10+j,20+j}} for #1#j=0:2 }  --> {{{-10,0}, {-9,1}},  {{10,20}, {11,21}}}
        {
            DEFINE_TUPLE(j_range, EXPR_LIST(CONST(1))(CONST(0))(CONST(1))(CONST(2))(VALUE(StringValue, "j")));
            std::vector<AbstractExpressionPtr> comp_args = {j_range};

            std::vector<AbstractExpressionPtr> args = boost::assign::list_of<AbstractExpressionPtr>(CONST(-10))(LOOKUP("j"));
            DEFINE(elt1, boost::make_shared<MathmlPlus>(args));
            DEFINE(elt2, LOOKUP("j"));
            args = boost::assign::list_of<AbstractExpressionPtr>(CONST(10))(LOOKUP("j"));
            std::vector<AbstractExpressionPtr> sub_array_elts = {elt1, elt2};
            DEFINE(subarray1, boost::make_shared<ArrayCreate>(sub_array_elts));
            DEFINE(elt3, boost::make_shared<MathmlPlus>(args));
            args = boost::assign:list_of<AbstractExpressionPtr>(CONST(20))(LOOKUP("j"));
            DEFINE(elt4, boost::make_shared<MathmlPlus>(args));
            sub_array_elts = {elt3, elt4};
            DEFINE(subarray2, boost::make_shared<ArrayCreate>(sub_array_elts));
            sub_array_elts = {subarray1, subarray2};
            DEFINE(subarray, boost::make_shared<ArrayCreate>(sub_array_elts));

            DEFINE(array_exp, boost::make_shared<ArrayCreate>(subarray, comp_args));
            AbstractValuePtr p_array = (*array_exp)(*p_env);
            TS_ASSERT(p_array->IsArray());
            NdArray<double> array = static_cast<ArrayValue*>(p_array.get())->GetArray();
            TS_ASSERT_EQUALS(array.GetNumDimensions(), 3u);
            TS_ASSERT_EQUALS(array.GetNumElements(), 8u);
            TS_ASSERT_EQUALS(array.GetShape()[0], 2u);
            TS_ASSERT_EQUALS(array.GetShape()[1], 2u);
            TS_ASSERT_EQUALS(array.GetShape()[2], 2u);
            NdArray<double>::Iterator it = array.Begin();
            double values[] = {-10, 0, -9, 1, 10, 20, 11, 21};
            for (NdArray<double>::Index i=0; i<8u; ++i)
            {
                TS_ASSERT_EQUALS(*it++, values[i]);
            }
        }
    }

    void TestMultipleReturns() throw (Exception)
    {
        EnvironmentPtr p_env(new Environment);
        Environment& env = *p_env; // Save typing!

        // swap = lambda a, b: return b, a
        {
            std::vector<std::string> fps = {"a", "b"};
            std::vector<AbstractExpressionPtr> ret_args = EXPR_LIST(LOOKUP("b"))(LOOKUP("a"));
            DEFINE_STMT(ret, RETURN_STMT(ret_args));
            std::vector<AbstractStatementPtr> fn_body = {ret};
            DEFINE(swap, boost::make_shared<LambdaExpression>(fps, fn_body));
            env.ExecuteStatement(ASSIGN_STMT("swap", swap));
        }
        // one, two = swap(2, 1)
        {
            std::vector<AbstractExpressionPtr> args = EXPR_LIST(CONST(2))(CONST(1));
            DEFINE(call, boost::make_shared<FunctionCall>("swap", args));
            std::vector<std::string> dests = {"one", "two"};
            env.ExecuteStatement(ASSIGN_STMT(dests, call));
        }

        TS_ASSERT_EQUALS(env.GetDefinedNames().size(), 3u);
        TS_ASSERT(env.Lookup("swap")->IsLambda());
        TS_ASSERT(env.Lookup("one")->IsDouble());
        TS_ASSERT(env.Lookup("two")->IsDouble());
        TS_ASSERT_EQUALS(GET_SIMPLE_VALUE(env.Lookup("one")), 1.0);
        TS_ASSERT_EQUALS(GET_SIMPLE_VALUE(env.Lookup("two")), 2.0);
    }
};

#endif // TESTCOREPROTOCOLLANGUAGE_HPP_
