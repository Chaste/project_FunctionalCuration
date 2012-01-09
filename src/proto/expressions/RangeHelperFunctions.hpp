/*

Copyright (C) University of Oxford, 2005-2012

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

#ifndef RANGEHELPERFUNCTIONS_HPP_
#define RANGEHELPERFUNCTIONS_HPP_

#include <string>
#include <map>
#include <vector>

#include "AbstractValue.hpp"
#include "NdArray.hpp"

// These utility functions are used by ArrayCreate and View.

/**
 * Check the start, end, or dimension of a range specification.
 *
 * @param pValue  the value specifying the start/end/dimension
 * @param nullOk  whether the null value is allowed here
 * @param specNum  which range specification this is
 * @param rLocationInfo  source program location to use in error messages
 * @param shouldBeIndex  whether the return value should fit within the NdArray<double>::Index type too
 */
NdArray<double>::RangeIndex CheckRangeElement(const AbstractValuePtr pValue,
                                              bool nullOk,
                                              NdArray<double>::Index specNum,
                                              const std::string& rLocationInfo,
                                              bool shouldBeIndex=false);

/**
 * Extract range specifications from a vector of tuple values.
 *
 * The tuple values should contain (dimension, start, stride, end, name) if pRangeNames
 * is supplied.  If not, they may contain either the first 3 or 4 items, but must all have
 * the same length.  If only 3 items are used, the dimension is taken to be the index into
 * the vector.
 *
 * @param rRangeSpecs  to be filled in with the range specifications
 * @param rTuples  the tuple values
 * @param rLocationInfo  source program location to use in error messages
 * @param pRangeNames  to be filled in with the names to use for index variables, if given
 */
void ExtractRangeSpecs(std::map<NdArray<double>::Index, NdArray<double>::Range>& rRangeSpecs,
                       const std::vector<AbstractValuePtr>& rTuples,
                       const std::string& rLocationInfo,
                       std::map<NdArray<double>::Index, std::string>* pRangeNames=NULL);

#endif // RANGEHELPERFUNCTIONS_HPP_
