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

#include "NdArray.hpp"

#include <algorithm>
#include <cstdlib> // For abs()
#include <cmath> // For ceil
#include <boost/numeric/conversion/bounds.hpp>

#include "Exception.hpp"
#include <sstream>

#define ASSERT_MSG(test, msg)                    \
    if (!(test))                                   \
    do {                                         \
        std::stringstream msg_stream;            \
        msg_stream << msg;                       \
        EXCEPTION(msg_stream.str());             \
    } while (false)


template<typename DATA>
NdArray<DATA>::InternalData::InternalData(const Extents& rExtents)
{
    const Index num_dims = rExtents.size();
    mExtents = rExtents;
    mNumElements = 1;
    for (Index i=0; i<num_dims; ++i)
    {
        mNumElements *= rExtents[i];
    }
    mpData = new DATA[mNumElements];
    mIndicesMultipliers.resize(num_dims, 1);
    for (Index i=0; i<num_dims; ++i)
    {
        Index j = num_dims - i;
        if (j != num_dims)
        {
            mIndicesMultipliers[j-1] = mIndicesMultipliers[j] * rExtents[j];
        }
    }
}



template<typename DATA>
NdArray<DATA>::InternalData::~InternalData()
{
    delete[] mpData;
}


template<typename DATA>
NdArray<DATA>::NdArray(const Extents& rExtents)
{
    mpInternalData.reset(new InternalData(rExtents));
}


template<typename DATA>
NdArray<DATA>::NdArray()
{
}


template<typename DATA>
NdArray<DATA>::NdArray(const NdArray<DATA>& rOther)
    : mpInternalData(rOther.mpInternalData)
{
}


template<typename DATA>
NdArray<DATA>::NdArray(const DATA& rValue)
{
    Extents empty_extents;
    mpInternalData.reset(new InternalData(empty_extents));
    mpInternalData->mpData[0] = rValue;
}


template<typename DATA>
NdArray<DATA>& NdArray<DATA>::operator=(const NdArray<DATA>& rOther)
{
    mpInternalData = rOther.mpInternalData;
    return *this;
}


template<typename DATA>
DATA& NdArray<DATA>::operator[](const Indices& rIndices)
{
    const Index num_dims = rIndices.size();
    assert(num_dims == mpInternalData->mExtents.size());
    Index actual_index = 0;
    for (Index i=0; i<num_dims; ++i)
    {
        actual_index += mpInternalData->mIndicesMultipliers[i] * rIndices[i];
    }
    return mpInternalData->mpData[actual_index];
}


template<typename DATA>
const DATA& NdArray<DATA>::operator[](const Indices& rIndices) const
{
    const Index num_dims = rIndices.size();
    assert(num_dims == mpInternalData->mExtents.size());
    Index actual_index = 0;
    for (Index i=0; i<num_dims; ++i)
    {
        actual_index += mpInternalData->mIndicesMultipliers[i] * rIndices[i];
    }
    return mpInternalData->mpData[actual_index];
}


template <typename DATA>
NdArray<DATA> NdArray<DATA>::operator[](const std::vector<Range>& rRanges)
{
    const Index num_dims = mpInternalData->mExtents.size();
    const Index no_dim = boost::numeric::bounds<Index>::highest();
    ASSERT_MSG(num_dims < no_dim, "Array has too many dimensions to view!!!");
    ASSERT_MSG(rRanges.size() == num_dims, "Wrong number of ranges supplied; received " << rRanges.size()
               << " but array has " << num_dims << " dimensions.");
    // Work out the extents of the view, and determine how view indices map to ours
    Extents extents;
    Indices our_idxs = GetIndices();
    Extents begins(num_dims);
    std::vector<Index> dim_map(num_dims);
    for (Index dim=0; dim<num_dims; ++dim)
    {
        const Range& r = rRanges[dim];
        RangeIndex begin = r.mBegin;
        if (r.mStep != 0) // Not a degenerate range
        {
            dim_map[dim] = extents.size();
            RangeIndex end = r.mEnd;
            if (end == Range::END)
            {
                if (r.mStep < 0) end = 0u;
                else end = mpInternalData->mExtents[dim];
            }
            else if (end < 0)
            {
                end = mpInternalData->mExtents[dim] + end;
                assert(end >= 0);
            }
            if (begin == Range::END)
            {
                if (r.mStep < 0) begin = mpInternalData->mExtents[dim];
                else begin = 0u;
            }
            else if (begin < 0)
            {
                begin = mpInternalData->mExtents[dim] + begin;
                ASSERT_MSG(begin >= 0, "Cannot begin a range at a negative index; got "
                           << begin << " even after subtracting from dimension " << dim << " size"
                           << mpInternalData->mExtents[dim] << ".");
            }
            ASSERT_MSG(begin != end, "Start and end of range for dimension " << dim
                       << " are equal but the step is not zero.");
            ASSERT_MSG(begin <= mpInternalData->mExtents[dim], "Range start " << begin << " for dimension "
                       << dim << " is after the end (" << mpInternalData->mExtents[dim] << ") of the dimension.");
            ASSERT_MSG(end <= mpInternalData->mExtents[dim], "Range end " << end << " for dimension "
                       << dim << " is after the end (" << mpInternalData->mExtents[dim] << ") of the dimension.");
            ASSERT_MSG(r.mStep * (end-begin) > 0, "Range for dimension " << dim << " is inconsistent: "
                       << "step is " << r.mStep << " but (end-begin) is " << end-begin << ".");
            extents.push_back((Index)ceil((end - begin) / (double)r.mStep));
            if (r.mStep < 0)
            {
                assert(begin > 0u); // This is covered by the assertions above, but let's be explicit
                begin--; // Avoid having to have different behaviour when indexing below
            }
        }
        else
        {
            // We'll always index at the given location in this dimension. Check it's valid.
            RangeIndex begin = r.mBegin;
            if (begin < 0)
            {
                begin = mpInternalData->mExtents[dim] + begin;
                ASSERT_MSG(begin >= 0, "Cannot begin a range at a negative index; got " << begin
                           << " even after subtracting from dimension " << dim << " size "
                           << mpInternalData->mExtents[dim] << ".");
            }
            ASSERT_MSG(begin < mpInternalData->mExtents[dim], "Index " << begin << " selected for dimension "
                       << dim << " is after the end (" << mpInternalData->mExtents[dim] << ") of the dimension.");
            ASSERT_MSG(r.mEnd == r.mBegin, "When the step is zero the start and end of a range must be equal; "
                       << r.mEnd << " != " << r.mBegin << " for dimension " << dim << ".");
            our_idxs[dim] = begin;
            dim_map[dim] = no_dim;
        }
        begins[dim] = begin;
    }
    // Create the view array
    NdArray<DATA> view(extents);
    Indices view_idxs = view.GetIndices();
    const Index num_elements = view.GetNumElements();
    for (Index i=0; i<num_elements; ++i)
    {
        for (Index dim=0; dim<num_dims; ++dim)
        {
            Index view_dim = dim_map[dim];
            if (view_dim != no_dim)
            {
                const Range& r = rRanges[dim];
                our_idxs[dim] = view_idxs[view_dim] * r.mStep + begins[dim];
            }
        }
        view[view_idxs] = (*this)[our_idxs];
        view.IncrementIndices(view_idxs);
    }
    return view;
}


template<typename DATA>
void NdArray<DATA>::Resize(const Extents& rExtents)
{
    const Index num_dims = mpInternalData->mExtents.size();
    assert(num_dims == rExtents.size());

    // Figure out the smallest extent of each dimension in the old & new shapes
    Extents min_extents(num_dims);
    const Index& (*min)(const Index&, const Index&) = std::min;
    std::transform(rExtents.begin(), rExtents.end(),
                   mpInternalData->mExtents.begin(),
                   min_extents.begin(),
                   min);
    Index num_shared_elts = 1;
    for (Index i=0; i<num_dims; ++i)
    {
        num_shared_elts *= min_extents[i];
    }
    // Create a new array to put the shared data in, then swap internal pointers
    NdArray<DATA> new_array(rExtents);
    Indices old_idxs = GetIndices();
    Indices new_idxs = new_array.GetIndices();
    for (Index i=0; i<num_shared_elts; ++i)
    {
        new_array[new_idxs] = (*this)[old_idxs];
        IncrementIndices(old_idxs, min_extents);
        IncrementIndices(new_idxs, min_extents);
    }
    // ... the swap
    std::swap(mpInternalData, new_array.mpInternalData);
}


template<typename DATA>
NdArray<DATA> NdArray<DATA>::Copy() const
{
    NdArray<DATA> copy(mpInternalData->mExtents);
    std::copy(Begin(), End(), copy.Begin());
    return copy;
}


template<typename DATA>
void NdArray<DATA>::IncrementIndices(Indices& rIndices, const Extents& rExtents) const
{
    // Last dimension varies fastest
    const unsigned num_dims = rExtents.size();
    for (unsigned dim = num_dims; dim-- != 0; )
    {
        rIndices[dim] = (1+rIndices[dim]) % rExtents[dim];
        if (rIndices[dim] != 0)
        {
            break;
        }
    }
}


///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                              Range class                              //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

template<typename DATA>
NdArray<DATA>::Range::Range(RangeIndex begin, RangeIndex end)
    : mBegin(begin),
      mEnd(end),
      mStep(1)
{}


template<typename DATA>
NdArray<DATA>::Range::Range(RangeIndex begin, RangeIndex step, RangeIndex end)
    : mBegin(begin),
      mEnd(end),
      mStep(step)
{}


template<typename DATA>
NdArray<DATA>::Range::Range(RangeIndex item)
    : mBegin(item),
      mEnd(item),
      mStep(0)
{}

template<typename DATA>
typename NdArray<DATA>::RangeIndex NdArray<DATA>::Range::END = boost::numeric::bounds<Index>::highest();


///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                        Explicit instantiation                         //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

template class NdArray<double>;
