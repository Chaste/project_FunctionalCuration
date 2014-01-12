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

#include "RangeHelperFunctions.hpp"

#include "BacktraceException.hpp"
#include "ValueTypes.hpp"
#include "NdArray.hpp"
#include "ProtoHelperMacros.hpp"

// Save typing...
typedef NdArray<double>::Range R;
typedef NdArray<double>::Index Index;
typedef NdArray<double>::RangeIndex RangeIndex;

RangeIndex CheckRangeElement(const AbstractValuePtr pValue, bool nullOk, Index specNum,
                             const std::string& rLocationInfo, bool shouldBeIndex)
{
    RangeIndex index;
    if (pValue->IsDouble())
    {
        index = GET_SIMPLE_VALUE(pValue);
    }
    else if (pValue->IsNull() && nullOk)
    {
        index = R::END;
    }
    else
    {
        PROTO_EXCEPTION2("The start, stride, end, and dimension of the range specification " << specNum
                         << " must be an integer" << (nullOk ? " or Null" : "") << ".", rLocationInfo);
    }
    if (shouldBeIndex)
    {
        PROTO_ASSERT2(index <= R::END,
                      "The dimension " << index << " in range specification " << specNum << " is too large.", rLocationInfo);
    }
    return index;
}


void ExtractRangeSpecs(std::map<Index, R>& rRangeSpecs,
                       const std::vector<AbstractValuePtr>& rTuples,
                       const std::string& rLocationInfo,
                       std::map<Index, std::string>* pRangeNames)
{
    const Index num_tuples = rTuples.size();
    for (Index i=0; i<num_tuples; ++i)
    {
        PROTO_ASSERT2(rTuples[i]->IsTuple(), "Range specification " << i << " is not a tuple.", rLocationInfo);
    }
    bool nulls_ok = pRangeNames ? false : true;
    unsigned num_tuple_items = 0u;
    bool implicit_dimension = true;
    for (Index i=0; i<num_tuples; ++i)
    {
        TupleValue* p_range = static_cast<TupleValue*>(rTuples[i].get());
        const unsigned num_items = p_range->GetNumItems();
        if (num_tuple_items == 0u)
        {
            if (pRangeNames)
            {
                PROTO_ASSERT2(num_items == 5 || num_items == 4,
                              "Range specification " << i << " has " << num_items << " not 4 or 5 items.", rLocationInfo);
                num_tuple_items = num_items;
                implicit_dimension = (num_items == 4);
            }
            else
            {
                PROTO_ASSERT2(num_items == 3 || num_items == 4,
                              "Range specifications must be 3-tuples or 4-tuples; specification " << i
                              << " has " << num_items << " items.", rLocationInfo);
                num_tuple_items = num_items;
                implicit_dimension = (num_items == 3);
            }
        }
        else
        {
            PROTO_ASSERT2(num_tuple_items == num_items,
                          "All range specifications must have the same number of items; specification "
                          << i << " has " << num_items << " not " << num_tuple_items << ".", rLocationInfo);
        }
        // Get & check dimension
        Index dimension = (Index)(implicit_dimension ? i : CheckRangeElement(p_range->GetItem(0), nulls_ok, i, rLocationInfo, true));
        PROTO_ASSERT2(rRangeSpecs.find(dimension) == rRangeSpecs.end(),
                      "Multiple range specifications given for dimension " << dimension << ".", rLocationInfo);
        // Get & check stride, start & end
        unsigned ioff = implicit_dimension ? 0 : 1;
        RangeIndex stride = CheckRangeElement(p_range->GetItem(1+ioff), false, i, rLocationInfo);
        RangeIndex start = CheckRangeElement(p_range->GetItem(0+ioff), nulls_ok && stride != 0, i, rLocationInfo);
        RangeIndex end = CheckRangeElement(p_range->GetItem(2+ioff), nulls_ok && stride != 0, i, rLocationInfo);
        // Get & check index variable name
        if (pRangeNames)
        {
            AbstractValuePtr p_name_item = p_range->GetItem(3 + ioff);
            PROTO_ASSERT2(p_name_item->IsString(),
                          "The last item in range specification " << i << " is not a string", rLocationInfo);
            std::string name = static_cast<StringValue*>(p_name_item.get())->GetString();
            pRangeNames->insert(std::pair<Index, std::string>(dimension, name));
        }
        // Store the range spec
        rRangeSpecs.insert(std::pair<Index, R>(dimension, R(start, stride, end)));
    }
}
