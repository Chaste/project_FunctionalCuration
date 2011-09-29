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
#ifndef TESTNDARRAY_HPP_
#define TESTNDARRAY_HPP_

#include <cxxtest/TestSuite.h>

#include <boost/assign/list_of.hpp>

#include "NdArray.hpp"

typedef NdArray<double> Array;
typedef Array::Index Index;
typedef Array::Indices Indices;
typedef Array::Range R;
typedef std::vector<R> RangeSpec;

class TestNdArray : public CxxTest::TestSuite
{
public:
    void Test0dArray() throw (Exception)
    {
        Array::Extents extents;
        Array arr0d(extents);
        TS_ASSERT_EQUALS(arr0d.GetNumDimensions(), 0u);
        TS_ASSERT_EQUALS(arr0d.GetNumElements(), 1u);
        TS_ASSERT_EQUALS(arr0d.GetShape().size(), 0u);
        TS_ASSERT(arr0d.GetShape().empty());
        TS_ASSERT(arr0d.Begin() != arr0d.End());
        double value = 1.0;
        *arr0d.Begin() = value;
        TS_ASSERT_EQUALS(*arr0d.Begin(), value);

        Array arr0d2(value);
        TS_ASSERT_EQUALS(arr0d2.GetNumDimensions(), 0u);
        TS_ASSERT_EQUALS(arr0d2.GetNumElements(), 1u);
        TS_ASSERT_EQUALS(arr0d2.GetShape().size(), 0u);
        TS_ASSERT(arr0d2.GetShape().empty());
        TS_ASSERT(arr0d2.Begin() != arr0d2.End());
        TS_ASSERT_EQUALS(*arr0d2.Begin(), value);
    }

    void TestBasicFunctionality() throw (Exception)
    {
        Array::Extents extents = boost::assign::list_of(3)(4)(2);

        Array arr(extents);
        const Index num_elements = extents[0]*extents[1]*extents[2];
        TS_ASSERT_EQUALS(arr.GetNumDimensions(), 3u);
        TS_ASSERT_EQUALS(arr.GetNumElements(), num_elements);
        TS_ASSERT_EQUALS(arr.GetShape()[0], extents[0]);
        TS_ASSERT_EQUALS(arr.GetShape()[1], extents[1]);
        TS_ASSERT_EQUALS(arr.GetShape()[2], extents[2]);

        // Assign values to the elements
        Indices indices = arr.GetIndices();
        int values = 0;
        for (Index i=0; i<num_elements; ++i)
        {
            arr[indices] = values++;
            arr.IncrementIndices(indices);
        }

        // Verify values
        int verify = 0;
        indices = arr.GetIndices();
        for (Index i=0; i<num_elements; ++i)
        {
            TS_ASSERT_EQUALS(arr[indices], verify);
            verify++;
            arr.IncrementIndices(indices);
        }

        // For STL algorithms it's nice to be able to iterate over the data
        verify = 0;
        for (Array::Iterator p_data=arr.Begin(); p_data != arr.End(); ++p_data)
        {
            TS_ASSERT_EQUALS(*p_data, verify);
            verify++;
        }

        // Look at views
        RangeSpec view3d_indices = boost::assign::list_of(R(0,2))        // Take elements 0,1 from dim 0 ext 3
                                                         (R(0,2,R::END)) // Take elements 0,2 from dim 1 ext 4
                                                         (R(1,R::END));  // Take element 1 from dim 2 ext 2
        Array view3d = arr[view3d_indices];
        TS_ASSERT_EQUALS(view3d.GetNumDimensions(), 3u);
        TS_ASSERT_EQUALS(view3d.GetNumElements(), 2u*2u*1u);
        TS_ASSERT_DIFFERS(view3d.GetShape(), arr.GetShape());
        TS_ASSERT_EQUALS(view3d.GetShape()[0], 2u);
        TS_ASSERT_EQUALS(view3d.GetShape()[1], 2u);
        TS_ASSERT_EQUALS(view3d.GetShape()[2], 1u);
        indices = view3d.GetIndices();
        std::vector<double> expected = boost::assign::list_of(1)(5)(9)(13);
        for (Index i=0; i<4u; ++i)
        {
            TS_ASSERT_EQUALS(view3d[indices], expected[i]);
            view3d.IncrementIndices(indices);
        }

        // Now a 2d view of the same data
        RangeSpec view2d_indices = boost::assign::list_of(R(0,2))   // Take elements 0,1 from dim 0 ext 3
                                                         (R(0,2,4)) // Take elements 0,2 from dim 1 ext 4
                                                         (R(1));    // Take element 1 from dim 2 ext 2
        Array view2d = arr[view2d_indices];
        TS_ASSERT_EQUALS(view2d.GetNumDimensions(), 2u);
        TS_ASSERT_EQUALS(view2d.GetNumElements(), 2u*2u*1u);
        TS_ASSERT_EQUALS(view2d.GetShape()[0], 2u);
        TS_ASSERT_EQUALS(view2d.GetShape()[1], 2u);
        indices = view3d.GetIndices();
        Indices indices2 = view2d.GetIndices();
        for (Index i=0; i<4u; ++i)
        {
            TS_ASSERT_EQUALS(view3d[indices], view2d[indices2]);
            view3d.IncrementIndices(indices);
            view2d.IncrementIndices(indices2);
        }

        // We can use negative indices to count from the end; same data as above
        RangeSpec view_neg_indices = boost::assign::list_of(R(R::END, -1))
                                                           (R(-4, 2, -1))
                                                           (R(-1));
        Array view_neg = arr[view_neg_indices];
        TS_ASSERT_EQUALS(view_neg.GetNumDimensions(), 2u);
        TS_ASSERT_EQUALS(view_neg.GetNumElements(), 2u*2u*1u);
        TS_ASSERT_EQUALS(view_neg.GetShape()[0], 2u);
        TS_ASSERT_EQUALS(view_neg.GetShape()[1], 2u);
        indices = view3d.GetIndices();
        indices2 = view_neg.GetIndices();
        for (Index i=0; i<4u; ++i)
        {
            TS_ASSERT_EQUALS(view3d[indices], view_neg[indices2]);
            view3d.IncrementIndices(indices);
            view_neg.IncrementIndices(indices2);
        }

        // We can do reverse views too
        view2d_indices = boost::assign::list_of(R(2))             // Last element from dim 0
                                               (R(R::END, -2, 1)) // Dim 1 reversed step 2 (elts 3, 1)
                                               (R(2, -1, 0));     // Whole of dim 2 reversed
        view2d = arr[view2d_indices];
        TS_ASSERT_EQUALS(view2d.GetNumDimensions(), 2u);
        TS_ASSERT_EQUALS(view2d.GetNumElements(), 1u*2u*2u);
        TS_ASSERT_EQUALS(view2d.GetShape()[0], 2u);
        TS_ASSERT_EQUALS(view2d.GetShape()[1], 2u);
        indices = view2d.GetIndices();
        expected = boost::assign::list_of(23)(22)(19)(18);
        for (Index i=0; i<4u; ++i)
        {
            TS_ASSERT_EQUALS(view2d[indices], expected[i]);
            view2d.IncrementIndices(indices);
        }

        // Assignment just aliases, so we need a 'real' copy operation too
        Array alias = arr;
        TS_ASSERT_EQUALS(arr.Begin(), alias.Begin());
        TS_ASSERT_EQUALS(arr.End(), alias.End());
        Array copy = arr.Copy();
        TS_ASSERT_DIFFERS(arr.Begin(), copy.Begin());
        TS_ASSERT_DIFFERS(arr.End(), copy.End());
        TS_ASSERT_EQUALS(arr.GetShape(), copy.GetShape());
        indices = arr.GetIndices();
        for (Index i=0; i<num_elements; ++i)
        {
            TS_ASSERT_EQUALS(arr[indices], alias[indices]);
            TS_ASSERT_EQUALS(arr[indices], copy[indices]);
            arr.IncrementIndices(indices);
        }
        double value = -1.0;
        *alias.Begin() = value;
        TS_ASSERT_EQUALS(*arr.Begin(), value);
        TS_ASSERT_DIFFERS(*copy.Begin(), value);
        *arr.Begin() = 0.0;

        // PostProcFind uses a resize operation
        Array::Extents changed_extents = boost::assign::list_of(3)(3)(3); // Was 3x4x2
        Array::Extents min_extents = boost::assign::list_of(3)(3)(2);
        copy.Resize(changed_extents);
        indices = arr.GetIndices();
        for (Index i=0; i<num_elements; ++i)
        {
            bool shared = true;
            for (unsigned dim=0; dim<indices.size(); dim++)
            {
                if (indices[dim] >= min_extents[dim])
                {
                    shared = false;
                    break;
                }
            }
            if (shared)
            {
                TS_ASSERT_EQUALS(arr[indices], copy[indices]);
            } // non-shared entries are unspecified
            arr.IncrementIndices(indices);
        }
    }
};

#endif //TESTMULTIARRAY_HPP_
