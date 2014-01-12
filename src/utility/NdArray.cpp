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

#include "NdArray.hpp"

#include <algorithm>
#include <cstdlib> // For abs()
#include <cmath> // For ceil
#include <boost/numeric/conversion/bounds.hpp>

#include "Exception.hpp"
#include "VectorStreaming.hpp"

#define ASSERT_MSG(test, msg) if (!(test)) EXCEPTION(msg)

template<typename DATA>
NdArray<DATA>::InternalData::InternalData(const Extents& rExtents)
    : mExtents(rExtents)
{
    const Index num_dims = mExtents.size();
    mNumElements = 1;
    for (Index i=0; i<num_dims; ++i)
    {
        mNumElements *= mExtents[i];
    }
    mpData = new DATA[mNumElements];
    mpDataEnd = mpData + mNumElements;
    mIndicesMultipliers.resize(num_dims, 1);
    for (Index i=0; i<num_dims; ++i)
    {
        Index j = num_dims - i;
        if (j != num_dims)
        {
            mIndicesMultipliers[j-1] = mIndicesMultipliers[j] * mExtents[j];
        }
    }
}


template<typename DATA>
NdArray<DATA>::InternalData::InternalData(const boost::shared_ptr<InternalData> pSource,
                                          const Indices& rBeginOffsets,
                                          const std::vector<RangeIndex>& rSteps,
                                          const Extents& rExtents)
    : mExtents(rExtents),
      mpSourceArray(pSource)
{
    const Index our_num_dims = mExtents.size();
    const Index source_num_dims = pSource->mExtents.size();
    assert(our_num_dims <= source_num_dims);
    assert(rBeginOffsets.size() == source_num_dims);
    assert(rSteps.size() == source_num_dims);
    mNumElements = 1;
    for (Index i=0; i<our_num_dims; ++i)
    {
        mNumElements *= mExtents[i];
    }
    mpData = pSource->mpData;
    for (Index i=0; i<source_num_dims; ++i)
    {
        mpData += rBeginOffsets[i] * pSource->mIndicesMultipliers[i];
    }
    mIndicesMultipliers.resize(our_num_dims);
    for (Index i=0, j=0; i<our_num_dims; ++i, ++j)
    {
        while (rSteps[j] == 0)
        {
            ++j;
        }
        assert(j < source_num_dims);
        mIndicesMultipliers[i] = pSource->mIndicesMultipliers[j] * rSteps[j];
    }
    if (our_num_dims > 0)
    {
        mpDataEnd = mpData + mExtents[0] * mIndicesMultipliers[0];
    }
    else
    {
        mpDataEnd = mpData + 1;
    }
}


template<typename DATA>
NdArray<DATA>::InternalData::~InternalData()
{
    if (!mpSourceArray)
    {
        delete[] mpData;
    }
}


template<typename DATA>
NdArray<DATA>::NdArray(const Extents& rExtents)
{
    mpInternalData.reset(new InternalData(rExtents));
}


template<typename DATA>
NdArray<DATA>::NdArray(const NdArray<DATA>& rSource,
                       const Indices& rBeginOffsets,
                       const std::vector<RangeIndex>& rSteps,
                       const Extents& rExtents)
{
    mpInternalData.reset(new InternalData(rSource.mpInternalData, rBeginOffsets, rSteps, rExtents));
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
    RangeIndex actual_index = 0;
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
    RangeIndex actual_index = 0;
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
    ASSERT_MSG(rRanges.size() == num_dims, "Wrong number of ranges supplied; received " << rRanges.size()
               << " but array has " << num_dims << " dimensions.");
    // Work out the extents of the view, and determine how view indices map to ours
    Extents extents;
    Indices begins(num_dims);
    std::vector<RangeIndex> steps;
    for (Index dim=0; dim<num_dims; ++dim)
    {
        const Range& r = rRanges[dim];
        steps.push_back(r.mStep);
        RangeIndex begin = r.mBegin;
        if (begin == Range::END)
        {
            if (r.mStep < 0) begin = mpInternalData->mExtents[dim] - 1;
            else begin = 0u;
        }
        else if (begin < 0)
        {
            begin = mpInternalData->mExtents[dim] + begin;
            ASSERT_MSG(begin >= 0, "Cannot begin a range at a negative index; got "
                       << begin << " even after subtracting from dimension " << dim << " size"
                       << mpInternalData->mExtents[dim] << ".");
        }
        RangeIndex end = r.mEnd;
        if (end == Range::END)
        {
            if (r.mStep < 0) end = -1; // NB: Half-open interval so end point not included
            else end = mpInternalData->mExtents[dim];
        }
        else if (end < 0)
        {
            end = mpInternalData->mExtents[dim] + end;
            ASSERT_MSG(end >= 0 || (end == -1 && r.mStep < 0),
                       "Cannot end a range at a negative index; got " << end <<
                       " even after subtracting from dimension " << dim <<
                       " size" << mpInternalData->mExtents[dim] << ".");
        }

        if (r.mStep != 0) // Not a degenerate range
        {
            ASSERT_MSG(begin != end, "Start and end of range for dimension " << dim
                       << " are equal but the step is not zero.");
            ASSERT_MSG(begin <= (RangeIndex)mpInternalData->mExtents[dim], "Range start " << begin << " for dimension "
                       << dim << " is after the end (" << mpInternalData->mExtents[dim] << ") of the dimension.");
            ASSERT_MSG(end <= (RangeIndex)mpInternalData->mExtents[dim], "Range end " << end << " for dimension "
                       << dim << " is after the end (" << mpInternalData->mExtents[dim] << ") of the dimension.");
            ASSERT_MSG(r.mStep * (end-begin) > 0, "Range for dimension " << dim << " is inconsistent: "
                       << "step is " << r.mStep << " but (end-begin) is " << end-begin << ".");
            extents.push_back((Index)ceil((end - begin) / (double)r.mStep));
        }
        else
        {
            // We'll always index at the given location in this dimension. Check it's valid.
            ASSERT_MSG(begin < (RangeIndex)mpInternalData->mExtents[dim], "Index " << begin << " selected for dimension "
                       << dim << " is after the end (" << mpInternalData->mExtents[dim] << ") of the dimension.");
            ASSERT_MSG(end == begin, "When the step is zero the start and end of a range must be equal; "
                       << end << " != " << begin << " for dimension " << dim << ".");
        }
        assert(begin >= 0); // This is guaranteed by tests above, but let's be explicit
        begins[dim] = begin;
    }
    // Create the view array
    NdArray<DATA> view(*this, begins, steps, extents);
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
    // Create a new array to put the shared data in, then swap contents of internal pointers
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
    delete[] mpInternalData->mpData;
    (*mpInternalData) = (*new_array.mpInternalData);
    new_array.mpInternalData->mpData = NULL;
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
