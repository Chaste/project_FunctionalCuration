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

#include <cassert>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/utility/enable_if.hpp>

/**
 * An n-dimensional array datatype, with the number of dimensions specifiable at run-time.
 * Once arrays have been filled with data, they should be considered immutable, although this
 * is not enforced in order to allow more flexibility in definition.
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
     * View constructor - make an array that's a sub-array of an existing array.
     * @param rSource  the array to make a view of
     * @param rBeginOffsets  where the view starts in each dimension
     * @param rSteps  the stride of the view along each dimension, with zero entries for omitted dimensions
     * @param rExtents  the shape of the view
     */
    NdArray(const NdArray<DATA>& rSource,
            const Indices& rBeginOffsets,
            const std::vector<RangeIndex>& rSteps,
            const Extents& rExtents);

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

    /** Iterators over elements of an array. */
    template<class VALUE>
    class IteratorImpl : public boost::iterator_facade<IteratorImpl<VALUE>, VALUE, boost::forward_traversal_tag>
    {
    private:
        /** Used to stop attempts to convert from ConstIterator to Iterator. */
        struct enabler {};

    public:
        /** Construct an iterator that doesn't point to anything. */
        IteratorImpl()
            : mPointer(NULL)
        {}

        /**
         * Constructor used by Begin and End to create iterators.
         * @param pEntry  the array entry pointed to
         * @param rIndices  the indices of the element pointed to within each dimension of the array
         * @param rStrides  how far the internal pointer should be incremented to progress along each dimension
         * @param rExtents  the shape of the array being iterated over
         */
        explicit IteratorImpl(VALUE* pEntry, const Indices& rIndices,
                              const std::vector<RangeIndex>& rStrides,
                              const Extents& rExtents)
            : mPointer(pEntry),
              mIndices(rIndices),
              mpStrides(&rStrides),
              mpExtents(&rExtents)
        {}

        /**
         * Converter constructor allowing us to pass an Iterator where a ConstIterator is expected.
         * @param rOther  the other iterator
         */
        template<class OTHER_VALUE>
        IteratorImpl(const IteratorImpl<OTHER_VALUE>& rOther,
                     typename boost::enable_if<boost::is_convertible<OTHER_VALUE*,VALUE*>, enabler>::type = enabler())
            : mPointer(rOther.mPointer),
              mIndices(rOther.mIndices),
              mpStrides(rOther.mpStrides),
              mpExtents(rOther.mpExtents)
        {}

        /**
         * Get the indices of the array element pointed at.
         */
        const Indices& rGetIndices() const
        {
            return mIndices;
        }

     private:
        friend class boost::iterator_core_access;
        template <class> friend class IteratorImpl;

        /** Deference this iterator to obtain a reference to the array entry pointed at. */
        VALUE& dereference() const
        {
            assert(mPointer);
            return *mPointer;
        }

        /**
         * Test this iterator for equality against another, i.e. whether they point at the same entry.
         * @param rOther  the other iterator
         */
        template<class OTHER_VALUE>
        bool equal(const IteratorImpl<OTHER_VALUE>& rOther) const
        {
            return mPointer == rOther.mPointer;
        }

        /** Increment this iterator to point at the next array entry. */
        void increment()
        {
            assert(mPointer);
            const unsigned num_dims = mIndices.size();
            if (num_dims == 0)
            {
                mPointer++;
            }
            else
            {
                for (unsigned dim = num_dims; dim-- != 0; )
                {
                    mIndices[dim] = (1+mIndices[dim]) % (*mpExtents)[dim];
                    mPointer += (*mpStrides)[dim];
                    if (mIndices[dim] != 0)
                    {
                        break;
                    }
                    else if (dim > 0) // Don't go back to the beginning when we reach the end!
                    {
                        mPointer -= (*mpStrides)[dim] * (*mpExtents)[dim];
                    }
                }
            }
        }

        /**
         * Advance this iterator n positions, by repeatedly incrementing it.
         * This isn't efficient, and is only here to support tests.
         * @param n  how far to advance
         */
        void advance(typename boost::iterator_facade<IteratorImpl<VALUE>, VALUE, boost::forward_traversal_tag>::difference_type n)
        {
            for (typename boost::iterator_facade<IteratorImpl<VALUE>, VALUE, boost::forward_traversal_tag>::difference_type i=0; i<n; i++)
            {
                increment();
            }
        }

        /** The array entry pointed to. */
        VALUE* mPointer;

        /** The indices of the element pointed to within each dimension of the array. */
        Indices mIndices;

        /** How far the internal pointer should be incremented to progress along each dimension. */
        const std::vector<RangeIndex>* mpStrides;

        /** The shape of the array being iterated over. */
        const Extents* mpExtents;
    };

    /** The type of an iterator over a mutable array. */
    typedef IteratorImpl<DATA> Iterator;

    /** The type of an iterator over a constant array. */
    typedef IteratorImpl<DATA const> ConstIterator;

    /** Return an iterator to the beginning of the array data. */
    inline Iterator Begin();

    /** Return an iterator to one past the end of the array data. */
    inline Iterator End();

    /** Return an iterator to the beginning of the array data. */
    inline ConstIterator Begin() const;

    /** Return an iterator to one past the end of the array data. */
    inline ConstIterator End() const;

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

    public: // should really change this...
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
         * Create a new array data structure.
         * @param rExtents  gives the number and extent of our dimensions
         */
        InternalData(const Extents& rExtents);

        /**
         * View constructor - make an array that's a sub-array of an existing array.
         * @param pSource  the data of the array to make a view of
         * @param rBeginOffsets  where the view starts in each dimension
         * @param rSteps  the stride of the view along each dimension, with zero entries for omitted dimensions
         * @param rExtents  the shape of the view
         */
        InternalData(const boost::shared_ptr<InternalData> pSource,
                     const Indices& rBeginOffsets,
                     const std::vector<RangeIndex>& rSteps,
                     const Extents& rExtents);

        /** Free array memory. */
        ~InternalData();

        /** The number and extents of our dimensions. */
        Extents mExtents;

        /** The total number of elements contained in this array. */
        Index mNumElements;

        /** The (start of the) actual array data. */
        DATA* mpData;

        /** What the End iterator points to. */
        DATA* mpDataEnd;

        /** The stride through #mpData used for each dimension. */
        std::vector<RangeIndex> mIndicesMultipliers;

        /** If this is a view, points to the original array's data; otherwise empty. */
        boost::shared_ptr<InternalData> mpSourceArray;
    };

    /** Our internal data. */
    boost::shared_ptr<InternalData> mpInternalData;
};


// Implementation of small inline methods

template<typename DATA>
typename NdArray<DATA>::Iterator NdArray<DATA>::Begin()
{
    return NdArray<DATA>::Iterator(mpInternalData->mpData, GetIndices(),
                                   mpInternalData->mIndicesMultipliers, mpInternalData->mExtents);
}


template<typename DATA>
typename NdArray<DATA>::Iterator NdArray<DATA>::End()
{
    return NdArray<DATA>::Iterator(mpInternalData->mpDataEnd, GetIndices(),
                                   mpInternalData->mIndicesMultipliers, mpInternalData->mExtents);
}

template<typename DATA>
typename NdArray<DATA>::ConstIterator NdArray<DATA>::Begin() const
{
    return NdArray<DATA>::ConstIterator(mpInternalData->mpData, GetIndices(),
                                        mpInternalData->mIndicesMultipliers, mpInternalData->mExtents);
}


template<typename DATA>
typename NdArray<DATA>::ConstIterator NdArray<DATA>::End() const
{
    return NdArray<DATA>::ConstIterator(mpInternalData->mpDataEnd, GetIndices(),
                                        mpInternalData->mIndicesMultipliers, mpInternalData->mExtents);
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
