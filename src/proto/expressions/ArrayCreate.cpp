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

#include "ArrayCreate.hpp"

#include <algorithm>
#include <cmath> // For ceil
#include <boost/assign/list_of.hpp>
#include <boost/make_shared.hpp>
#include <boost/numeric/conversion/bounds.hpp>
#include "ValueTypes.hpp"
#include "NdArray.hpp"
#include "BacktraceException.hpp"
#include "VectorStreaming.hpp"
#include "ProtoHelperMacros.hpp"
#include "RangeHelperFunctions.hpp"

// Save typing...
typedef NdArray<double>::Range R;
typedef NdArray<double>::Index Index;
typedef NdArray<double>::RangeIndex RangeIndex;

ArrayCreate::ArrayCreate(const std::vector<AbstractExpressionPtr>& rElements)
    : FunctionCall("~create~array", rElements)
{
    PROTO_ASSERT(!rElements.empty(), "You cannot define an empty array.");
}

/**
 * An array comprehension.
 *
 * @param pElementGenerator  the expression generating elements of the array
 * @param rRanges  range specifications for dimensions of the array being created
 */
ArrayCreate::ArrayCreate(const AbstractExpressionPtr pElementGenerator,
                         const std::vector<AbstractExpressionPtr>& rRanges)
    : FunctionCall("~create~array", rRanges),
      mpElementGenerator(pElementGenerator)
{
    PROTO_ASSERT(!rRanges.empty(), "You cannot define an empty array.");
    assert(mpElementGenerator);
}

/**
 * Define variables in the given environment representing the indices over specified dimensions.
 *
 * @param rSubEnv  the environment
 * @param rIndexNames  the variable names
 * @param rIndexCounts  how many steps along each dimension
 * @param rRanges  the ranges over which each dimension can vary
 * @param rLoc  location in the program
 */
void SetIndexValues(Environment& rSubEnv,
                    const std::vector<std::string>& rIndexNames,
                    const NdArray<double>::Indices& rIndexCounts,
                    const std::vector<R>& rRanges,
                    const std::string& rLoc)
{
    const unsigned N = rIndexNames.size();
    std::vector<AbstractValuePtr> values(N);
    for (unsigned i=0; i<N; ++i)
    {
        values[i] = boost::make_shared<SimpleValue>(rRanges[i].mBegin + rIndexCounts[i]*rRanges[i].mStep);
    }
    rSubEnv.DefineNames(rIndexNames, values, rLoc);
}

AbstractValuePtr ArrayCreate::operator()(const Environment& rEnv) const
{
    AbstractValuePtr p_result;
    std::vector<AbstractValuePtr> params = EvaluateChildren(rEnv);
    const unsigned num_params = params.size();
    if (mpElementGenerator)
    {
        // Array comprehension
        // Check range definitions
        std::map<Index, R> range_specs;
        std::map<Index, std::string> range_names;
        ExtractRangeSpecs(range_specs, params, GetLocationInfo(), &range_names);
        const Index num_range_specs = range_specs.size();
        // Construct view specifications
        std::vector<R> generation_ranges, view_ranges;
        std::vector<std::string> index_names;
        std::vector<Index> generator_dimensions;
        NdArray<double>::Extents generator_extents;
        Index last_dim = boost::numeric::bounds<Index>::highest();
        Index num_sub_arrays = 1u;
        Index num_gaps = 0u;
        NdArray<double>::Extents extents;
        for (std::map<Index, R>::const_iterator it = range_specs.begin(); it != range_specs.end(); ++it)
        {
            // Note: iteration over a map is in ascending key order
            PROTO_ASSERT(it->second.mStep * (int)(it->second.mEnd - it->second.mBegin) > 0,
                         "Stride and begin/end of range of dimension " << it->first << " are inconsistent.");
            generator_dimensions.push_back(it->first);
            generation_ranges.push_back(it->second);
            index_names.push_back(range_names.find(it->first)->second);
            // Missing dimensions in the range specs should be filled from sub-arrays
            while (++last_dim != it->first)
            {
                view_ranges.push_back(R(R::END, 1, R::END));
                extents.push_back(R::END);
                num_gaps++;
            }
            extents.push_back((Index)ceil((int)(it->second.mEnd - it->second.mBegin) / (double)it->second.mStep));
            generator_extents.push_back(extents.back());
            view_ranges.push_back(R(0, 1, extents.back()));
            num_sub_arrays *= extents.back();
        }
        // We figure out the shape of sub-arrays based on the first one
        NdArray<double>::Extents sub_array_shape;
        // Iterate over the generation range specifications to build up the new array
        boost::shared_ptr<NdArray<double> > p_array;
        NdArray<double>::Indices generator_indices(generator_extents.size(), 0u);
        for (Index i=0; i<num_sub_arrays; ++i)
        {
            EnvironmentPtr p_sub_env(new Environment(rEnv.GetAsDelegatee()));
            SetIndexValues(*p_sub_env, index_names, generator_indices, generation_ranges, GetLocationInfo());
            AbstractValuePtr p_sub_array = (*mpElementGenerator)(*p_sub_env);
            PROTO_ASSERT(p_sub_array->IsArray(),
                         "The generator expression in an array comprehension must yield arrays.");
            NdArray<double> sub_array = GET_ARRAY(p_sub_array);
            if (i == 0)
            {
                sub_array_shape = sub_array.GetShape();
//                std::cout << "Comp: sub-array shape: " << sub_array_shape << std::endl;
                // Check the sub-arrays are large enough to fill any gaps in the range specs
                Index num_sub_dims = sub_array.GetNumDimensions();
                PROTO_ASSERT(num_sub_dims >= num_gaps,
                             "The sub-arrays in this array comprehension have only " << num_sub_dims
                             << " dimensions; not enough to fill " << num_gaps
                             << " gaps in the range specifications.");
                Index sub_i = 0;
                for (Index j=0; j<extents.size(); ++j)
                {
                    if (extents[j] == R::END)
                    {
                        extents[j] = sub_array_shape[sub_i++];
                    }
                }
                // Any extra sub-array dimensions get put on the end
                for (; sub_i < num_sub_dims; ++sub_i)
                {
                    view_ranges.push_back(R(R::END, 1, R::END));
                    extents.push_back(sub_array_shape[sub_i]);
                }
                // Create the new array
//                std::cout << "Comp: Creating array shape: " << extents << std::endl;
                p_array.reset(new NdArray<double>(extents));
            }
            else
            {
                PROTO_ASSERT(sub_array_shape == sub_array.GetShape(),
                             "All sub-arrays in an array comprehension must have the same shape; arrays "
                             << i << " of shape " << sub_array.GetShape() << " and 0 of shape "
                             << sub_array_shape << "differ.");
            }
//            std::cout << "Sub array " << i << ": " << std::vector<double>(sub_array.Begin(), sub_array.End()) << std::endl;
            // Fill in the view of the new array corresponding to this sub-array
            for (Index j=0; j<num_range_specs; ++j)
            {
                Index index_j = generator_indices[j];
                view_ranges[generator_dimensions[j]] = R(index_j, 0, index_j);
            }
            NdArray<double> view = (*p_array)[view_ranges];
            std::copy(sub_array.Begin(), sub_array.End(), view.Begin());
            p_array->IncrementIndices(generator_indices, generator_extents);
        }
        p_result = boost::make_shared<ArrayValue>(*p_array);
    }
    else
    {
        // Array defined by listing elements
        // Check element shapes
        NdArray<double>::Extents sub_array_shape;
        for (unsigned i=0; i<num_params; ++i)
        {
            PROTO_ASSERT(params[i]->IsArray(),
                         "Elements of an array must be simple values or arrays; element " << i
                         << " is neither.");
            if (i == 0)
            {
                sub_array_shape = GET_ARRAY(params[i]).GetShape();
            }
            else
            {
                PROTO_ASSERT(sub_array_shape == GET_ARRAY(params[i]).GetShape(),
                             "All elements of an array must have the same shape; element " << i
                             << " with shape " << GET_ARRAY(params[i]).GetShape()
                             << " does not match element 0 of shape " << sub_array_shape << ".");
            }
        }
        // Create result array
        NdArray<double>::Extents shape = boost::assign::list_of(num_params).range(sub_array_shape);
        NdArray<double> result(shape);
        // Fill it in
        NdArray<double>::Iterator it = result.Begin();
        for (unsigned i=0; i<num_params; ++i)
        {
            NdArray<double> sub_array = GET_ARRAY(params[i]);
            it = std::copy(sub_array.Begin(), sub_array.End(), it);
        }
        p_result = boost::make_shared<ArrayValue>(result);
    }
    return p_result;
}
