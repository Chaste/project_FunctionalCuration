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
#ifndef TESTNDARRAY_HPP_
#define TESTNDARRAY_HPP_

#include <cxxtest/TestSuite.h>

#include <boost/assign/list_of.hpp>

#include "NdArray.hpp"

// Save typing...
typedef NdArray<double> Array;
typedef Array::Extents Extents;
typedef Array::Index Index;
typedef Array::Indices Indices;
typedef Array::Iterator Iterator;
typedef Array::ConstIterator ConstIterator;
typedef Array::Range R;
typedef std::vector<R> RangeSpec;

class TestNdArray : public CxxTest::TestSuite
{
public:
    void Test0dArray() throw (Exception)
    {
        Extents extents;
        Array arr0d(extents);
        TS_ASSERT_EQUALS(arr0d.GetNumDimensions(), 0u);
        TS_ASSERT_EQUALS(arr0d.GetNumElements(), 1u);
        TS_ASSERT_EQUALS(arr0d.GetShape().size(), 0u);
        TS_ASSERT(arr0d.GetShape().empty());
        TS_ASSERT(arr0d.Begin() != arr0d.End());
        double value = 1.0;
        *arr0d.Begin() = value;
        TS_ASSERT_EQUALS(*arr0d.Begin(), value);
        TS_ASSERT_EQUALS(++arr0d.Begin(), arr0d.End());

        Array arr0d2(value);
        TS_ASSERT_EQUALS(arr0d2.GetNumDimensions(), 0u);
        TS_ASSERT_EQUALS(arr0d2.GetNumElements(), 1u);
        TS_ASSERT_EQUALS(arr0d2.GetShape().size(), 0u);
        TS_ASSERT(arr0d2.GetShape().empty());
        TS_ASSERT(arr0d2.Begin() != arr0d2.End());
        TS_ASSERT_EQUALS(*arr0d2.Begin(), value);
        TS_ASSERT_EQUALS(++arr0d2.Begin(), arr0d2.End());
    }

    void TestBasicFunctionality() throw (Exception)
    {
        Extents extents = boost::assign::list_of(3)(4)(2);

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

        // We can also iterate over the whole array; useful for STL algorithms
        verify = 0;
        indices = arr.GetIndices();
        for (ConstIterator it=arr.Begin(); it != arr.End(); ++it)
        {
            TS_ASSERT_EQUALS(*it, verify);
            verify++;
            // We can get an indices collection from an iterator...
            for (unsigned i=0; i<3u; i++)
            {
                TS_ASSERT_EQUALS(it.rGetIndices()[i], indices[i]);
            }
            // ...and vice versa
            TS_ASSERT_EQUALS(it, ConstIterator(indices, arr));
            TS_ASSERT_EQUALS(it, Iterator(indices, arr));
            arr.IncrementIndices(indices);
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
        std::vector<double> expected = boost::assign::list_of(1)(5)(9)(13);
        unsigned i=0;
        for (ConstIterator it=view3d.Begin(); it != view3d.End(); ++it)
        {
            TS_ASSERT_EQUALS(*it, expected[i]);
            // Views should ideally alias the original data, not copy it
            TS_ASSERT_EQUALS(it, arr.Begin() + (ptrdiff_t)expected[i]);
            i++;
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
        for (ConstIterator it=view2d.Begin(), it3d=view3d.Begin(); it != view2d.End(); ++it, ++it3d)
        {
            TS_ASSERT_EQUALS(*it, *it3d);
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
        for (ConstIterator it=view_neg.Begin(), it3d=view3d.Begin(); it != view_neg.End(); ++it, ++it3d)
        {
            TS_ASSERT_EQUALS(*it, *it3d);
        }

        // We can do reverse views too
        view2d_indices = boost::assign::list_of(R(2))             // Last element from dim 0
                                               (R(R::END, -2, 0)) // Dim 1 reversed step 2 (elts 3, 1)
                                               (R(-1, -1, -3));   // Whole of dim 2 reversed
        view2d = arr[view2d_indices];
        TS_ASSERT_EQUALS(view2d.GetNumDimensions(), 2u);
        TS_ASSERT_EQUALS(view2d.GetNumElements(), 1u*2u*2u);
        TS_ASSERT_EQUALS(view2d.GetShape()[0], 2u);
        TS_ASSERT_EQUALS(view2d.GetShape()[1], 2u);
        expected = boost::assign::list_of(23)(22)(19)(18);
        i=0;
        for (ConstIterator it=view2d.Begin(); it != view2d.End(); ++it)
        {
            TS_ASSERT_EQUALS(*it, expected[i]);
            // Views should ideally alias the original data, not copy it
            TS_ASSERT_EQUALS(it, arr.Begin() + (ptrdiff_t)expected[i]);
            i++;
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
        for (Index idx=0; idx<num_elements; ++idx)
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
        Array copy_alias = copy; // Check any aliases are also resized
        copy.Resize(changed_extents);
        TS_ASSERT_EQUALS(copy.GetShape(), copy_alias.GetShape());
        indices = arr.GetIndices();
        for (Index idx=0; idx<num_elements; ++idx)
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
                TS_ASSERT_EQUALS(arr[indices], copy_alias[indices]);
            } // non-shared entries are unspecified
            arr.IncrementIndices(indices);
        }
    }

    void TestMoreIterationAndViews() throw (Exception)
    {
        Extents extents = boost::assign::list_of(3)(4)(2)(7);

        Array arr(extents);
        TS_ASSERT_EQUALS(arr.GetNumDimensions(), 4u);
        TS_ASSERT_EQUALS(arr.GetNumElements(), 3u*4u*2u*7u);

        // Fill in the array using iterators (and note that it++ works too)
        double value = 0.0;
        for (Iterator it = arr.Begin(); it != arr.End(); it++)
        {
            *it = (value--)/2.0;
        }

        // Check we can take a view missing 'internal' dimensions
        RangeSpec view_indices = boost::assign::list_of(R(0, 2, R::END))  // First & last elements from dim 0
                                                       (R(R::END, -2, 0)) // Dim 1 reversed step 2 (elts 3, 1)
                                                       (R(-1))            // Last element of dim 2
                                                       (R(1, -1, -8));    // First 2 elements of dim 3 reversed
        Array view = arr[view_indices];
        TS_ASSERT_EQUALS(view.GetNumDimensions(), 3u);
        TS_ASSERT_EQUALS(view.GetNumElements(), 2u*2u*2u);
        TS_ASSERT_EQUALS(view.GetShape()[0], 2u);
        TS_ASSERT_EQUALS(view.GetShape()[1], 2u);
        TS_ASSERT_EQUALS(view.GetShape()[2], 2u);
        // Original array multipliers are: 56, 14, 7, 1
        // So offsets into it are 50,49,22,21, 162,161,134,133
        std::vector<double> expected = boost::assign::list_of(-25.0)(-49.0/2)(-11.0)(-21.0/2)
                                                             (-81.0)(-161.0/2)(-67.0)(-133.0/2);
        unsigned i=0;
        for (ConstIterator it=view.Begin(); it != view.End(); ++it)
        {
            TS_ASSERT_EQUALS(*it, expected[i]);
            // Views should ideally alias the original data, not copy it
            TS_ASSERT_EQUALS(it, arr.Begin() + (ptrdiff_t)(-2*expected[i]));
            i++;
        }

        // Check that copying a view gives us a fresh array
        Array view_copy = view.Copy();
        TS_ASSERT_DIFFERS(view_copy.Begin(), view.Begin());
        for (ConstIterator it=view.Begin(), copy_it=view_copy.Begin(); it != view.End(); ++it, ++copy_it)
        {
            TS_ASSERT_EQUALS(*it, *copy_it);
        }

        // And check that a 0d view works
        view_indices = boost::assign::list_of(R(1))(R(2))(R(1))(R(3));
        view = arr[view_indices];
        TS_ASSERT_EQUALS(view.GetNumDimensions(), 0u);
        TS_ASSERT_EQUALS(view.GetNumElements(), 1u);
        TS_ASSERT_EQUALS(*view.Begin(), -0.5*(56+28+7+3));
        TS_ASSERT_EQUALS(++view.Begin(), view.End());
    }
};

#endif //TESTMULTIARRAY_HPP_
