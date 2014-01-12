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

#ifndef TESTPROTOCOLPARSER_HPP_
#define TESTPROTOCOLPARSER_HPP_

#include <cxxtest/TestSuite.h>
#include <boost/foreach.hpp>

#include "ProtocolLanguage.hpp"
#include "ProtocolParser.hpp"
#include "Protocol.hpp"
#include "ProtocolFileFinder.hpp"
#include "AbstractTemplatedSystemWithOutputs.hpp"

#include "OutputFileHandler.hpp"
#include "FileFinder.hpp"
#include "CellMLToSharedLibraryConverter.hpp"
#include "DynamicCellModelLoader.hpp"
#include "AbstractCardiacCellInterface.hpp"
#include "AbstractCvodeCell.hpp"
#include "RegularStimulus.hpp"

#include "UsefulFunctionsForProtocolTesting.hpp"
#include "FileComparison.hpp"
#include "PetscSetupAndFinalize.hpp"

using boost::assign::list_of;
using boost::make_shared;

class TestProtocolParser : public CxxTest::TestSuite
{
    NdArray<double> LookupArray(const Environment& rEnv, const std::string& rName)
    {
        AbstractValuePtr wrapped_array = rEnv.Lookup(rName);
        TS_ASSERT(wrapped_array->IsArray());
        return static_cast<ArrayValue*>(wrapped_array.get())->GetArray();
    }

    std::string ReplaceAll(std::string str, const std::string& rSearch, const std::string& rReplace)
    {
        size_t found = 0;
        while ((found = str.find(rSearch, found)) != std::string::npos)
        {
            str.replace(found, rSearch.length(), rReplace);
        }
        return str;
    }

    void DoCorePostproc(const ProtocolFileFinder& rProtoFile) throw (Exception)
    {
        ProtocolParser parser;
        ProtocolPtr p_proto = parser.ParseFile(rProtoFile);
        p_proto->InitialiseLibrary();

        // Test inputs
        const double input_value = 1.0;
        Environment& r_inputs = p_proto->rGetInputsEnvironment();
        TS_ASSERT(r_inputs.Lookup("example_input")->IsDouble());
        TS_ASSERT_DIFFERS(GET_SIMPLE_VALUE(r_inputs.Lookup("example_input")), input_value);
        r_inputs.OverwriteDefinition("example_input", CV(-input_value), "Test");
        TS_ASSERT_EQUALS(GET_SIMPLE_VALUE(r_inputs.Lookup("example_input")), -input_value);
        p_proto->SetInput("example_input", CONST(input_value));
        TS_ASSERT(r_inputs.Lookup("example_input")->IsDouble());
        TS_ASSERT_EQUALS(GET_SIMPLE_VALUE(r_inputs.Lookup("example_input")), input_value);
        TS_ASSERT_THROWS_CONTAINS(r_inputs.OverwriteDefinition("none", CV(0), ""),
                                  "Name none is not defined and may not be overwritten.");

        std::vector<AbstractStatementPtr>& r_program = p_proto->rGetPostProcessing();
        EnvironmentPtr p_env(new Environment);
        Environment& env = *p_env; // Save typing!
        TS_ASSERT_THROWS_CONTAINS(env.OverwriteDefinition("blah", CV(0), ""),
                                  "This environment does not support overwriting mappings.");
        env.SetDelegateeEnvironment(p_proto->rGetLibrary().GetAsDelegatee());
        env.ExecuteStatements(r_program);

        /* Extract and test results */
        NdArray<double> sum_array = LookupArray(env, "input_sum"); // Should be {55}
        TS_ASSERT_EQUALS(sum_array.GetNumDimensions(), 1u);
        TS_ASSERT_EQUALS(sum_array.GetNumElements(), 1u);
        NdArray<double>::Iterator p_result_elt = sum_array.Begin();
        TS_ASSERT_EQUALS(*p_result_elt, 55.0);

        NdArray<double> max_result_array = LookupArray(env, "input2d_max"); // Should be {{6,4,5}}
        TS_ASSERT_EQUALS(max_result_array.GetNumDimensions(), 2u);
        TS_ASSERT_EQUALS(max_result_array.GetShape()[0], 1u);
        TS_ASSERT_EQUALS(max_result_array.GetNumElements(), 3u);
        p_result_elt = max_result_array.Begin();
        TS_ASSERT_EQUALS(*p_result_elt++, 6.0);
        TS_ASSERT_EQUALS(*p_result_elt++, 4.0);
        TS_ASSERT_EQUALS(*p_result_elt, 5.0);

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

        // Array comprehensions
        {
            NdArray<double> counting1d = LookupArray(env, "counting1d");
            NdArray<double>::Iterator it = counting1d.Begin();
            for (NdArray<double>::Index i=0; i<10u; ++i)
            {
                TS_ASSERT_EQUALS(*it++, i);
            }
        }
        {
            NdArray<double> counting2d = LookupArray(env, "counting2d");
            NdArray<double>::Iterator it = counting2d.Begin();
            for (NdArray<double>::Index i=0; i<6u; ++i)
            {
                TS_ASSERT_EQUALS(*it++, i+3);
            }
        }
        {
            NdArray<double> blocks = LookupArray(env, "blocks");
            NdArray<double>::Iterator it = blocks.Begin();
            double values[] = {-10, 0, -9, 1, 10, 20, 11, 21};
            for (NdArray<double>::Index i=0; i<8u; ++i)
            {
                TS_ASSERT_EQUALS(*it++, values[i]);
            }
        }
    }

public:
    void TestCorePostprocXml() throw (Exception)
    {
        ProtocolFileFinder proto_file("projects/FunctionalCuration/test/protocols/test_core_postproc.xml", RelativeTo::ChasteSourceRoot);
        DoCorePostproc(proto_file);
    }

    void TestCorePostprocCompact() throw (Exception)
    {
        ProtocolFileFinder proto_file("projects/FunctionalCuration/test/protocols/compact/test_core_postproc.txt", RelativeTo::ChasteSourceRoot);
        DoCorePostproc(proto_file);

    }

    void TestSimpleError() throw (Exception)
    {
        ProtocolParser parser;
        ProtocolFileFinder proto_file("projects/FunctionalCuration/test/protocols/test_error1.xml", RelativeTo::ChasteSourceRoot);
        ProtocolPtr p_proto = parser.ParseFile(proto_file);

        std::vector<AbstractStatementPtr>& r_program = p_proto->rGetPostProcessing();
        EnvironmentPtr p_env(new Environment);
        Environment& env = *p_env; // Save typing!
        bool threw = false;
        try
        {
            env.ExecuteStatements(r_program);
        }
        catch (const BacktraceException& e)
        {
            threw = true;
            std::string expected_msg = "Protocol backtrace (most recent call last):\n"
                    "  <file>:27:20\tapply\n"
                    "  <file>:29:24\tapply\n"
                    "  <file>:9:25\tlambda\n"
                    "  (Implicit return statement)\n"
                    "  <file>:17:28\tapply\n"
                    "Cannot fold over dimension 4294967295 as the operand array only has 0 dimensions.\n";
            std::string actual_msg = ReplaceAll(e.GetShortMessage(), proto_file.GetAbsolutePath(), "<file>");
            TS_ASSERT_EQUALS(actual_msg, expected_msg);
            // Line/col numbers might easily change, but the strings below really shouldn't
            TS_ASSERT_EQUALS(e.CheckShortMessageContains(proto_file.GetAbsolutePath()), "");
            TS_ASSERT_EQUALS(e.CheckShortMessageContains("\tlambda"), "");
            TS_ASSERT_EQUALS(e.CheckShortMessageContains("Cannot fold over dimension 4294967295 as the operand array only has 0 dimensions."), "");
        }
        TS_ASSERT(threw);
    }

    void TestErrorInCompactSyntax() throw (Exception)
    {
        ProtocolParser parser;
        ProtocolFileFinder proto_file("projects/FunctionalCuration/test/protocols/compact/test_error_msg.txt", RelativeTo::ChasteSourceRoot);
        ProtocolPtr p_proto = parser.ParseFile(proto_file);

        std::vector<AbstractStatementPtr>& r_program = p_proto->rGetPostProcessing();
        EnvironmentPtr p_env(new Environment);
        bool threw = false;
        try
        {
            p_env->ExecuteStatements(r_program);
        }
        catch (const BacktraceException& e)
        {
            threw = true;
            std::string expected_msg = "Protocol backtrace (most recent call last):\n"
                    "  <file>:5:5\t    bad_sum = sum(1.0)\n"
                    "  <file>:5:15\t    bad_sum = sum(1.0)\n"
                    "  <file>:4:11\t    sum = lambda a, dim=default: fold(@2:+, a, 0, dim)\n"
                    "  (Implicit return statement)\n"
                    "  <file>:4:34\t    sum = lambda a, dim=default: fold(@2:+, a, 0, dim)\n"
                    "Cannot fold over dimension 4294967295 as the operand array only has 0 dimensions.\n";
            std::string actual_msg = ReplaceAll(e.GetShortMessage(), proto_file.rGetOriginalSource().GetAbsolutePath(), "<file>");
            TS_ASSERT_EQUALS(actual_msg, expected_msg);
            // Line/col numbers might easily change, but the strings below really shouldn't
            TS_ASSERT_EQUALS(e.CheckShortMessageContains(proto_file.rGetOriginalSource().GetAbsolutePath()), "");
            TS_ASSERT_EQUALS(e.CheckShortMessageContains("\t    sum = lambda a, dim=default: fold(@2:+, a, 0, dim)"), "");
            TS_ASSERT_EQUALS(e.CheckShortMessageContains("Cannot fold over dimension 4294967295 as the operand array only has 0 dimensions."), "");
        }
        TS_ASSERT(threw);
    }

    void TestFindAndIndexCompact() throw (Exception)
    {
        ProtocolParser parser;
        ProtocolFileFinder proto_file("projects/FunctionalCuration/test/protocols/compact/test_find_index.txt", RelativeTo::ChasteSourceRoot);
        ProtocolPtr p_proto = parser.ParseFile(proto_file);

        std::vector<AbstractStatementPtr>& r_library = p_proto->rGetLibraryStatements();
        std::vector<AbstractStatementPtr>& r_program = p_proto->rGetPostProcessing();
        EnvironmentPtr p_env(new Environment);
        Environment& env = *p_env; // Save typing!
        env.ExecuteStatements(r_library);
        env.ExecuteStatements(r_program);
    }

    void TestFindAndIndex() throw (Exception)
    {
        ProtocolParser parser;
        ProtocolFileFinder proto_file("projects/FunctionalCuration/test/protocols/test_find_index.xml", RelativeTo::ChasteSourceRoot);
        ProtocolPtr p_proto = parser.ParseFile(proto_file);

        std::vector<AbstractStatementPtr>& r_program = p_proto->rGetPostProcessing();
        EnvironmentPtr p_env(new Environment);
        Environment& env = *p_env; // Save typing!
        env.ExecuteStatements(r_program);

        NdArray<double> input = LookupArray(env, "input");
        {
            std::cout << " - All but one entry" << std::endl;
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
    }

    void TestCoreLibrary() throw (Exception)
    {
        ProtocolParser parser;
        ProtocolFileFinder proto_file("projects/FunctionalCuration/src/proto/library/BasicLibrary.xml", RelativeTo::ChasteSourceRoot);
        ProtocolPtr p_proto = parser.ParseFile(proto_file);
        p_proto->InitialiseLibrary();
        Environment& env = p_proto->rGetLibrary();

        {
            std::cout << " - max, min" << std::endl;
            std::vector<AbstractExpressionPtr> values = EXPR_LIST(CONST(1))(CONST(-2))(CONST(4))(CONST(2));
            DEFINE(input, boost::make_shared<ArrayCreate>(values));
            std::vector<AbstractExpressionPtr> args = EXPR_LIST(input)(DEFAULT_EXPR);

            AbstractValuePtr p_result = (*boost::make_shared<FunctionCall>("Max", args))(env);
            TS_ASSERT(p_result->IsArray());
            NdArray<double> array = GET_ARRAY(p_result);
            TS_ASSERT_EQUALS(array.GetNumDimensions(), 1u);
            TS_ASSERT_EQUALS(array.GetNumElements(), 1u);
            TS_ASSERT_EQUALS(*array.Begin(), 4);

            p_result = (*boost::make_shared<FunctionCall>("Min", args))(env);
            TS_ASSERT(p_result->IsArray());
            array = GET_ARRAY(p_result);
            TS_ASSERT_EQUALS(array.GetNumDimensions(), 1u);
            TS_ASSERT_EQUALS(array.GetNumElements(), 1u);
            TS_ASSERT_EQUALS(*array.Begin(), -2);
        }

        {
            std::cout << " - diff" << std::endl;
            std::vector<AbstractExpressionPtr> values = EXPR_LIST(CONST(1))(CONST(2))(CONST(4))(CONST(8));
            DEFINE(input, boost::make_shared<ArrayCreate>(values));
            std::vector<AbstractExpressionPtr> args = EXPR_LIST(input)(DEFAULT_EXPR);
            DEFINE(call, boost::make_shared<FunctionCall>("Diff", args));
            AbstractValuePtr p_result = (*call)(env);
            TS_ASSERT(p_result->IsArray());
            NdArray<double> array = GET_ARRAY(p_result);
            TS_ASSERT_EQUALS(array.GetNumDimensions(), 1u);
            TS_ASSERT_EQUALS(array.GetNumElements(), 3u);
            std::vector<double> result_values(array.Begin(), array.End());
            TS_ASSERT_EQUALS(result_values, boost::assign::list_of(1)(2)(4));
        }

        {
            std::cout << " - stretch" << std::endl;
            // Stretch a 1x3x1 array to 3x3x2
            NdArray<double>::Extents input_shape = list_of(1)(3)(1);
            NdArray<double>::Extents intermediate_shape = list_of(3)(3)(1);
            NdArray<double>::Extents result_shape = list_of(3)(3)(2);
            NdArray<double> input(input_shape);
            NdArray<double>::Iterator it = input.Begin();
            *it++ = 1; *it++ = 2; *it = 3;
            std::vector<AbstractExpressionPtr> args = EXPR_LIST(VALUE(ArrayValue, input))(CONST(3))(CONST(0));
            AbstractExpressionPtr p_call = make_shared<FunctionCall>("Stretch", args);
            AbstractValuePtr p_result = (*p_call)(env);

            TS_ASSERT(p_result->IsArray());
            NdArray<double> array = GET_ARRAY(p_result);
            TS_ASSERT_EQUALS(array.GetNumDimensions(), 3u);
            TS_ASSERT_EQUALS(array.GetNumElements(), 9u);
            TS_ASSERT_EQUALS(array.GetShape(), intermediate_shape);
            std::vector<double> result_values(array.Begin(), array.End());
            TS_ASSERT_EQUALS(result_values, list_of(1)(2)(3)(1)(2)(3)(1)(2)(3));

            args = EXPR_LIST(CONST(3))(CONST(3))(CONST(2));
            DEFINE(p_shape, make_shared<ArrayCreate>(args));
            args = EXPR_LIST(VALUE(ArrayValue, array))(CONST(2))(DEFAULT_EXPR);
            p_call = make_shared<FunctionCall>("Stretch", args);
            p_result = (*p_call)(env);

            TS_ASSERT(p_result->IsArray());
            array = GET_ARRAY(p_result);
            TS_ASSERT_EQUALS(array.GetNumDimensions(), 3u);
            TS_ASSERT_EQUALS(array.GetNumElements(), 18u);
            TS_ASSERT_EQUALS(array.GetShape(), result_shape);
            result_values.assign(array.Begin(), array.End());
            TS_ASSERT_EQUALS(result_values, list_of(1)(1)(2)(2)(3)(3)(1)(1)(2)(2)(3)(3)(1)(1)(2)(2)(3)(3));
        }

        {
            std::cout << " - window, shift, join" << std::endl;
            // Create a 2d array to operate on - comprehension of i*5+j for i=0:3, j=0:5
            const unsigned shape[] = {3, 5};
            DEFINE_TUPLE(i_range, EXPR_LIST(CONST(0))(CONST(0))(CONST(1))(CONST(shape[0]))(VALUE(StringValue, "i")));
            DEFINE_TUPLE(j_range, EXPR_LIST(CONST(1))(CONST(0))(CONST(1))(CONST(shape[1]))(VALUE(StringValue, "j")));
            std::vector<AbstractExpressionPtr> comp_args = list_of(i_range)(j_range);
            std::vector<AbstractExpressionPtr> args = EXPR_LIST(LOOKUP("i"))(CONST(shape[1]));
            DEFINE(times, make_shared<MathmlTimes>(args));
            args = EXPR_LIST(times)(LOOKUP("j"));
            DEFINE(plus, make_shared<MathmlPlus>(args));
            DEFINE(array_exp, make_shared<ArrayCreate>(plus, comp_args));
            AbstractValuePtr p_array = (*array_exp)(env);
            NdArray<double> input = GET_ARRAY(p_array);
            TS_ASSERT_EQUALS(input.GetShape()[0], 3u);
            TS_ASSERT_EQUALS(input.GetShape()[1], 5u);

            // Create a window of the "time" (j) dimension: window(input, 2, default)
            args = EXPR_LIST(VALUE(ArrayValue, input))(CONST(2))(DEFAULT_EXPR);
            DEFINE(window_call, make_shared<FunctionCall>(LOOKUP("Window"), args));
            AbstractValuePtr p_window = (*window_call)(env);
            NdArray<double> window = GET_ARRAY(p_window);
            /* It should look like:
             *  [ [ 2 3 4 4 4 ] [ 7 8 9 9 9 ] [ 12 13 14 14 14 ] ]
             *  [ [ 1 2 3 4 4 ] [ 6 7 8 9 9 ] [ 11 12 13 14 14 ] ]
             *  [ [ 0 1 2 3 4 ] [ 5 6 7 8 9 ] [ 10 11 12 13 14 ] ]
             *  [ [ 0 0 1 2 3 ] [ 5 5 6 7 8 ] [ 10 10 11 12 13 ] ]
             *  [ [ 0 0 0 1 2 ] [ 5 5 5 6 7 ] [ 10 10 10 11 12 ] ]
             */
            // Check shape is 5x3x5
            TS_ASSERT_EQUALS(window.GetNumDimensions(), 3u);
            TS_ASSERT_EQUALS(window.GetShape()[0], 5u);
            TS_ASSERT_EQUALS(window.GetShape()[1], 3u);
            TS_ASSERT_EQUALS(window.GetShape()[2], 5u);
            for (int shift = -2; shift <= 2; ++shift)
            {
                for (unsigned i=0; i<shape[0]; ++i)
                {
                    for (unsigned j=0; j<shape[1]; ++j)
                    {
                        int shifted_j = j - shift;
                        if (shifted_j < 0) shifted_j = 0;
                        if ((unsigned)shifted_j >= shape[1]) shifted_j = shape[1]-1;
                        NdArray<double>::Indices window_idx = boost::assign::list_of<unsigned>(shift+2)(i)(j);
                        NdArray<double>::Indices input_idx = boost::assign::list_of(i)(shifted_j);
                        TS_ASSERT_EQUALS(window[window_idx], input[input_idx]);
                    }
                }
            }
        }

        {
            std::cout << " - interp" << std::endl;
            // Create two 2d arrays (xs & ys) to operate on - we're going to interpolate multiple functions
            NdArray<double>::Extents shape = list_of(2u)(3u); // "Simulation number" x "Time"
            NdArray<double> xs(shape);  // [ [2, 3, 4], [2, 3, 4] ]
            NdArray<double> ys(shape);  // [ [ 0, 2, 4 ], [ 16, 14, 12 ] ]
            const double xs_values[] = {2, 3, 4, 2, 3, 4};
            std::copy(xs_values, xs_values+6, xs.Begin());
            const double ys_values[] = {0, 2, 4, 16, 14, 12};
            std::copy(ys_values, ys_values+6, ys.Begin());

            // Targets to find
            shape[1] = 1;
            NdArray<double> targets(shape);
            NdArray<double> starts(shape);
            for (unsigned i=0; i<shape[0]; ++i)
            {
                *(targets.Begin()+i) = 10.0*i + i + 1.3;  // [1.3, 12.3]
                *(starts.Begin()+i) = 2.0 + i;  // [2, 3]
            }

            // Search forwards
            std::vector<AbstractExpressionPtr> args
                = EXPR_LIST(VALUE(ArrayValue, xs))(VALUE(ArrayValue, ys))(VALUE(ArrayValue, targets))
                           (VALUE(ArrayValue, starts))(DEFAULT_EXPR)(DEFAULT_EXPR);
            AbstractValuePtr p_result = (*make_shared<FunctionCall>("Interp", args))(env);
            NdArray<double> result = GET_ARRAY(p_result);
            TS_ASSERT_EQUALS(result.GetNumDimensions(), 2u);
            TS_ASSERT_EQUALS(result.GetShape()[0], shape[0]);
            TS_ASSERT_EQUALS(result.GetShape()[1], 1u);
            NdArray<double>::Iterator it = result.Begin();
            TS_ASSERT_DELTA(*it++, 2.65, 1e-12);
            TS_ASSERT_DELTA(*it, 3.85, 1e-12);

            // Try searching backwards too
            for (unsigned i=0; i<shape[0]; ++i)
            {
                *(starts.Begin()+i) = 3.0 + i;  // [3, 4]
            }
            args = EXPR_LIST(VALUE(ArrayValue, xs))(VALUE(ArrayValue, ys))(VALUE(ArrayValue, targets))
                            (VALUE(ArrayValue, starts))(CONST(0))(DEFAULT_EXPR);
            p_result = (*make_shared<FunctionCall>("Interp", args))(env);
            result = GET_ARRAY(p_result);
            TS_ASSERT_EQUALS(result.GetNumDimensions(), 2u);
            TS_ASSERT_EQUALS(result.GetShape()[0], shape[0]);
            TS_ASSERT_EQUALS(result.GetShape()[1], 1u);
            it = result.Begin();
            TS_ASSERT_DELTA(*it++, 2.65, 1e-12);
            TS_ASSERT_DELTA(*it, 3.85, 1e-12);
        }
    }
};

#endif // TESTPROTOCOLPARSER_HPP_
