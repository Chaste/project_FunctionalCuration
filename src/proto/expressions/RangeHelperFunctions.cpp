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
    for (Index i=0; i<num_tuples; ++i)
    {
        TupleValue* p_range = static_cast<TupleValue*>(rTuples[i].get());
        const unsigned num_items = p_range->GetNumItems();
        if (num_tuple_items == 0u)
        {
            if (pRangeNames)
            {
                PROTO_ASSERT2(num_items == 5,
                              "Range specification " << i << " has " << num_items << " not 5 items.", rLocationInfo);
                num_tuple_items = 5u;
            }
            else
            {
                PROTO_ASSERT2(num_items == 3 || num_items == 4,
                              "Range specifications must be 3-tuples or 4-tuples; specification " << i
                              << " has " << num_items << " items.", rLocationInfo);
                num_tuple_items = num_items;
            }
        }
        else
        {
            PROTO_ASSERT2(num_tuple_items == num_items,
                          "All range specifications must have the same number of items; specification "
                          << i << " has " << num_items << " not " << num_tuple_items << ".", rLocationInfo);
        }
        // Get & check dimension
        Index dimension = (Index)((num_items == 3) ? i : CheckRangeElement(p_range->GetItem(0), nulls_ok, i, rLocationInfo, true));
        PROTO_ASSERT2(rRangeSpecs.find(dimension) == rRangeSpecs.end(),
                      "Multiple range specifications given for dimension " << dimension << ".", rLocationInfo);
        // Get & check stride, start & end
        unsigned ioff = (num_items == 3) ? 0 : 1;
        RangeIndex stride = CheckRangeElement(p_range->GetItem(1+ioff), false, i, rLocationInfo);
        RangeIndex start = CheckRangeElement(p_range->GetItem(0+ioff), nulls_ok && stride != 0, i, rLocationInfo);
        RangeIndex end = CheckRangeElement(p_range->GetItem(2+ioff), nulls_ok && stride != 0, i, rLocationInfo);
        // Get & check index variable name
        if (num_tuple_items == 5)
        {
            AbstractValuePtr p_name_item = p_range->GetItem(4);
            PROTO_ASSERT2(p_name_item->IsString(),
                          "The fifth item in range specification " << i << " is not a string", rLocationInfo);
            std::string name = static_cast<StringValue*>(p_name_item.get())->GetString();
            pRangeNames->insert(std::pair<Index, std::string>(dimension, name));
        }
        // Store the range spec
        rRangeSpecs.insert(std::pair<Index, R>(dimension, R(start, stride, end)));
    }
}
