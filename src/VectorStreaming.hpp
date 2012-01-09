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

#ifndef VECTORSTREAMING_HPP_
#define VECTORSTREAMING_HPP_

#include <iostream>
#include <vector>

/**
 * Allow std::vectors to be written directly to an output stream, without
 * needing special-case handling.  The vector will be formatted like:
 * \code
 *   {item0, item1, item2, ..., itemn}
 * \endcode
 */
template <typename T>
std::ostream& operator<< (std::ostream& out, const std::vector<T>& v)
{
    out << '{';
    for (typename std::vector<T>::const_iterator it = v.begin(); it != v.end(); ++it)
    {
        if (it != v.begin()) out << ", ";
        out << (*it);
    }
    out << '}';
    return out;
}


#endif // VECTORSTREAMING_HPP_
