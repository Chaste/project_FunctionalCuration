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

#ifndef NDARRAY_HPP_
#define NDARRAY_HPP_

#include <vector>
#include <boost/shared_ptr.hpp>

/**
 * An n-dimensional array datatype, with the number of dimensions specifiable at run-time.
 * Once arrays have been filled with data, they should be considered immutable, although this
 * is not enforced in order to allow more flexibility in definition.
 *
 * \todo Most error conditions trigger asserts; they should probably be changed to PROTO_ASSERTs.
 */
template<typename DATA>
class NdArray
{
public:
    /** The type of an index into a single dimension of an array. */
    typedef unsigned Index;

    /**
     * The type of indices used to define ranges,
     * which can be negative to count from the end of the dimension.
     */
    typedef long RangeIndex;

    /** The type of objects defining the extents of an array. */
    typedef std::vector<Index> Extents;

    /** The type of objects used to index an array. */
    typedef std::vector<Index> Indices;

    /** The type of an iterator over elements of an array. */
    typedef DATA* Iterator;

    /**
     * Default constructor, that doesn't allocate any memory for the array.
     * This is only here so you can declare an array variable then assign a
     * real array to it later.
     */
    NdArray();

    /**
     * Create an empty n-dimensional array, ready to be filled with data.
     *
     * @param rExtents  gives the number and extent of our dimensions
     */
    NdArray(const Extents& rExtents);

    /**
     * Copy constructor.  This effectively makes us an alias for the other array.
     *
     * @param rOther  the array to copy
     */
    NdArray(const NdArray<DATA>& rOther);

    /**
     * Create a 0-dimensional array containing a single value.
     *
     * @param rValue  the contained value
     */
    NdArray(const DATA& rValue);

    /**
     * Assignment operator.  This effectively makes us an alias for the other array.
     *
     * @param rOther  the array to assign from
     */
    NdArray<DATA>& operator=(const NdArray<DATA>& rOther);

    /**
     * Index this array to reference a single element.
     *
     * @param rIndices  the index into each dimension
     */
    DATA& operator[](const Indices& rIndices);

    /**
     * Index this array to reference a single element.
     *
     * @param rIndices  the index into each dimension
     */
    const DATA& operator[](const Indices& rIndices) const;

    /** Return an iterator to the beginning of the array data. */
    inline Iterator Begin() const;

    /** Return an iterator to one past the end of the array data. */
    inline Iterator End() const;

    /**
     * Resize this array to a new shape.  It must retain the same number of dimensions, but
     * each dimension may grow or shrink.  Data that still fits in the new size will be retained.
     *
     * @param rExtents  the new shape
     */
    void Resize(const Extents& rExtents);

    /**
     * Get an Indices object which can be used to index this array, and references the first
     * element.  Use IncrementIndices() to iterate over the array elements.
     */
    Indices GetIndices() const;

    /**
     * Increment an Indices object to reference the next element of an array with the given shape.
     * It will wrap around to the beginning once it reaches the end of the array.
     *
     * @param rIndices  the Indices object
     * @param rExtents  the shape of the array to index
     */
    void IncrementIndices(Indices& rIndices, const Extents& rExtents) const;

    /**
     * Increment an Indices object to reference the next element of this array.
     * It will wrap around to the beginning once it reaches the end of the array.
     *
     * @param rIndices  the Indices object
     */
    void IncrementIndices(Indices& rIndices) const;

    /** Get the number of dimensions of this array. */
    unsigned GetNumDimensions() const;

    /** Get the total number of elements in this array. */
    inline unsigned GetNumElements() const;

    /** Get the shape of this array. */
    inline Extents GetShape() const;

    /**
     * Range objects are used to specify views into an array.  They define how much of each dimension
     * is included in the view.
     */
    class Range
    {
    public:
        /**
         * Create a range specifying the half-open interval from begin to end.
         *
         * @param begin  lower closed end of the range
         * @param end  upper open end of the range
         */
        Range(RangeIndex begin, RangeIndex end);

        /**
         * Create a range specifying the half-open interval from begin to end, taking every step'th element.
         * Note that step may be negative, in which case begin should be greater than end.
         *
         * @param begin  closed end of the range
         * @param step  interval between selected elements
         * @param end  open end of the range
         */
        Range(RangeIndex begin, RangeIndex step, RangeIndex end);

        /**
         * Create a 'range' specifying that the relevant dimension should not appear in the view (i.e.
         * the view will have a lower dimensionality than the array).
         *
         * @param item  where to slice along the omitted dimension
         */
        Range(RangeIndex item);

        /** Magic number that can be used for the end of a range to say 'continue to the end of this dimension'. */
        static RangeIndex END;

    public: // need to change this...
        RangeIndex mBegin; /**< Closed end of the range */
        RangeIndex mEnd; /**< Open end of the range */
        RangeIndex mStep; /**< Interval between selected elements */
    };

    /**
     * Index this array to obtain a view of a sub-array.
     *
     * @param rRanges  the part of each dimension to include
     */
    NdArray<DATA> operator[](const std::vector<Range>& rRanges);

    /**
     * Do a deep copy of this array, creating a fresh array containing the same data.
     */
    NdArray<DATA> Copy() const;

private:
    /**
     * The type of array internal data.  We don't contain these directly, but via a shared pointer,
     * so that copying arrays is just a quick aliasing operation.
     */
    struct InternalData
    {
        /**
         * Create a new array data structure
         * @param rExtents  gives the number and extent of our dimensions
         */
        InternalData(const Extents& rExtents);

        /** Free array memory */
        ~InternalData();

        /** The number and extents of our dimensions */
        Extents mExtents;

        /** The total number of elements contained in this array */
        Index mNumElements;

        /** The actual array data */
        DATA* mpData;

        /** The stride through #mpData used for each dimension */
        Indices mIndicesMultipliers;
    };

    /** Our internal data. */
    boost::shared_ptr<InternalData> mpInternalData;
};


// Implementation of small inline methods

template<typename DATA>
DATA* NdArray<DATA>::Begin() const
{
    return mpInternalData->mpData;
}


template<typename DATA>
DATA* NdArray<DATA>::End() const
{
    return mpInternalData->mpData + mpInternalData->mNumElements;
}


template<typename DATA>
typename NdArray<DATA>::Indices NdArray<DATA>::GetIndices() const
{
    Indices indices(GetNumDimensions(), 0u);
    return indices;
}


template<typename DATA>
void NdArray<DATA>::IncrementIndices(Indices& rIndices) const
{
    IncrementIndices(rIndices, mpInternalData->mExtents);
}


template<typename DATA>
unsigned NdArray<DATA>::GetNumDimensions() const
{
    return mpInternalData->mExtents.size();
}


template<typename DATA>
unsigned NdArray<DATA>::GetNumElements() const
{
    return mpInternalData->mNumElements;
}


template<typename DATA>
typename NdArray<DATA>::Extents NdArray<DATA>::GetShape() const
{
    return mpInternalData->mExtents;
}

#endif // NDARRAY_HPP_
