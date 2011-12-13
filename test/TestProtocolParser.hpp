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

#ifndef TESTPROTOCOLPARSER_HPP_
#define TESTPROTOCOLPARSER_HPP_

#include <cxxtest/TestSuite.h>
#include <boost/foreach.hpp>

#include "ProtocolLanguage.hpp"
#include "ProtocolParser.hpp"
#include "Protocol.hpp"

#include "OutputFileHandler.hpp"
#include "FileFinder.hpp"
#include "CellMLToSharedLibraryConverter.hpp"
#include "DynamicCellModelLoader.hpp"
#include "AbstractCardiacCellInterface.hpp"
#include "AbstractCvodeCell.hpp"
#include "RegularStimulus.hpp"

#include "UsefulFunctionsForProtocolTesting.hpp"
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

public:
    void TestWithCorePostproc() throw (Exception)
    {
        ProtocolParser parser;
        FileFinder proto_file("projects/FunctionalCuration/test/protocols/test_core_postproc.xml", RelativeTo::ChasteSourceRoot);
        ProtocolPtr p_proto = parser.ParseFile(proto_file);

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

    void TestSimpleError() throw (Exception)
    {
        ProtocolParser parser;
        FileFinder proto_file("projects/FunctionalCuration/test/protocols/test_error1.xml", RelativeTo::ChasteSourceRoot);
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

    void TestFindAndIndex() throw (Exception)
    {
        ProtocolParser parser;
        FileFinder proto_file("projects/FunctionalCuration/test/protocols/test_find_index.xml", RelativeTo::ChasteSourceRoot);
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
        FileFinder proto_file("projects/FunctionalCuration/src/proto/library/BasicLibrary.xml", RelativeTo::ChasteSourceRoot);
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

    void TestBasicSimulationAndApd() throw (Exception)
    {
        typedef N_Vector VECTOR;
        typedef AbstractCvodeCell CELL_TYPE;
//        typedef std::vector<double> VECTOR;
//        typedef AbstractCardiacCell CELL_TYPE;

        // Copy CellML file into output dir and create conf file
        std::string dirname = "TestBasicSimulationAndApd";
        OutputFileHandler handler(dirname);
        FileFinder cellml_file("heart/src/odes/cellml/LuoRudy1991.cellml", RelativeTo::ChasteSourceRoot);
        FileFinder copied_cellml = handler.CopyFileTo(cellml_file);
        FileFinder proto_py_file("projects/FunctionalCuration/test/protocols/SimpleProtocol.py", RelativeTo::ChasteSourceRoot);
        std::vector<std::string> options = list_of("--opt")("--cvode")("--no-use-chaste-stimulus")("--expose-annotated-variables");
        CreateOptionsFile(handler, proto_py_file, "LuoRudy1991", options);

        // Do the conversion
        CellMLToSharedLibraryConverter converter(true, "projects/FunctionalCuration");
        DynamicCellModelLoader* p_loader = converter.Convert(copied_cellml);
        boost::shared_ptr<AbstractCardiacCellInterface> p_abs_cell(CreateCellFromLoader(*p_loader));
        boost::shared_ptr<CELL_TYPE> p_cell = boost::dynamic_pointer_cast<CELL_TYPE>(p_abs_cell);
        assert(p_cell);
        // Check it has the right base classes
        TS_ASSERT(boost::dynamic_pointer_cast<AbstractSystemWithOutputs<VECTOR> >(p_cell));
        TS_ASSERT(boost::dynamic_pointer_cast<AbstractParameterisedSystem<VECTOR> >(p_cell));

        // Test with simulation & post-processing loaded from file
        ProtocolParser parser;
        FileFinder proto_file("projects/FunctionalCuration/test/protocols/test_basic_simulation_and_apd.xml", RelativeTo::ChasteSourceRoot);
        ProtocolPtr p_proto = parser.ParseFile(proto_file);
        p_proto->SetModel(p_cell);
        p_proto->Run();
        p_proto->WriteToFile(handler, "TestBasicSimulationAndApd");

        // Test metadata files
        std::vector<std::string> filenames = list_of("-contents.csv")("-steppers.csv")("-default-plots.csv");
        BOOST_FOREACH(std::string filename, filenames)
        {
            FileFinder ref_file("projects/FunctionalCuration/test/data/test_basic_simulation/TestBasicSimulationAndApd" + filename, RelativeTo::ChasteSourceRoot);
            FileFinder test_file = handler.FindFile("TestBasicSimulationAndApd" + filename);
            TS_ASSERT_THROWS_NOTHING(EXPECT0(system, "diff -u " + test_file.GetAbsolutePath() + " " + ref_file.GetAbsolutePath()));
        }

        // Check base outputs
        const unsigned num_runs = 5u;
        const unsigned num_timesteps = 1201u;
        TS_ASSERT_EQUALS(p_proto->GetNumberOfOutputs(), 3u); // V from model plus APD & DI
        const Environment& r_sim2_outputs = p_proto->rGetOutputsCollection("sim2");
        NdArray<double> time = GET_ARRAY(r_sim2_outputs.Lookup("time"));
        TS_ASSERT_EQUALS(time.GetNumDimensions(), 2u);
        TS_ASSERT_EQUALS(time.GetShape()[0], num_runs);
        TS_ASSERT_EQUALS(time.GetShape()[1], num_timesteps);
        NdArray<double> V = GET_ARRAY(r_sim2_outputs.Lookup("fast_sodium_current__V"));
        TS_ASSERT_EQUALS(V.GetNumDimensions(), 2u);
        TS_ASSERT_EQUALS(V.GetShape()[0], num_runs);
        TS_ASSERT_EQUALS(V.GetShape()[1], num_timesteps);

        // Check apd(fast_sodium_current__V, time)
        {
            const Environment& r_outputs = p_proto->rGetOutputsCollection();
            AbstractValuePtr p_apd = r_outputs.Lookup("apd90");
            AbstractValuePtr p_di = r_outputs.Lookup("di");
            TS_ASSERT(p_apd->IsArray());
            TS_ASSERT(p_di->IsArray());
            NdArray<double> apd = GET_ARRAY(p_apd);
            NdArray<double> di = GET_ARRAY(p_di);
            TS_ASSERT_EQUALS(apd.GetShape()[0], num_runs);
            TS_ASSERT_EQUALS(apd.GetShape()[1], 2u);
            TS_ASSERT_EQUALS(di.GetShape()[0], num_runs);
            TS_ASSERT_EQUALS(di.GetShape()[1], 1u);
            for (unsigned i=0; i<num_runs; ++i)
            {
                TS_ASSERT_DELTA(*(apd.Begin()+i*2), 337.1248, 0.001);
                TS_ASSERT_DELTA(*(apd.Begin()+i*2+1), 337.1246, 0.0013);
                TS_ASSERT_DELTA(*(di.Begin()+i), 262.8751, 0.001);
            }
        }

        // Test the 3d simulation that occurs in the same protocol, run before that above.
        TS_ASSERT_EQUALS(p_proto->GetNumberOfOutputs(0u), 5u);
        TS_ASSERT_EQUALS(p_proto->GetNumberOfOutputs("sim1"), 5u);
        const Environment& r_outputs_3d = p_proto->rGetOutputsCollection(0u);
        TS_ASSERT_EQUALS(&r_outputs_3d, &(p_proto->rGetOutputsCollection("sim1")));
        NdArray<double> time_3d = GET_ARRAY(r_outputs_3d.Lookup("time"));
        TS_ASSERT_EQUALS(time_3d.GetNumDimensions(), 3u);
        TS_ASSERT_EQUALS(time_3d.GetShape()[0], 3u);
        TS_ASSERT_EQUALS(time_3d.GetShape()[1], 3u);
        TS_ASSERT_EQUALS(time_3d.GetShape()[2], num_timesteps);
        NdArray<double>::ConstIterator p_time_3d = time_3d.Begin();
        NdArray<double>::ConstIterator p_V_3d = GET_ARRAY(r_outputs_3d.Lookup("fast_sodium_current__V")).Begin();
        for (unsigned i=0; i<3u; ++i)
        {
            for (unsigned j=0; j<3u; ++j)
            {
                NdArray<double>::ConstIterator p_time = time.Begin();
                NdArray<double>::ConstIterator p_V = V.Begin();
                for (unsigned k=0; k<1201u; ++k)
                {
                    TS_ASSERT_EQUALS(*p_time_3d++, *p_time++);
                    TS_ASSERT_EQUALS(*p_V_3d++, *p_V++);
                }
            }
        }

        // Check that AT_END works
        boost::shared_ptr<ModelStateCollection> p_states = p_proto->GetStateCollection();
        p_states->SetModelState(boost::dynamic_pointer_cast<AbstractParameterisedSystem<VECTOR> >(p_cell), "final_state");
    }
};

#endif // TESTPROTOCOLPARSER_HPP_
