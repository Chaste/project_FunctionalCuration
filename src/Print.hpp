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

#ifndef PRINT_HPP_
#define PRINT_HPP_

#include <iostream>
#include "ProtocolOutputs.hpp"

/**
 * For debugging, print a 1d array.
 * @param ra  array to output
 * @param size  the size of the array
 * @param msg  an optional message to print before the array
 */
template<typename ARR>
void Print(const ARR& ra, size_t size, const std::string msg=std::string())
{
    std::cout << msg << (msg.empty() ? "" : ": ");
    for (size_t i=0; i<size; ++i)
    {
        std::cout << ra[i] << " ";
    }
    std::cout << std::endl;
}

/**
 * For debugging, print a 2d array.
 * @param ra  array to print
 * @param msg  an optional message to print before the array
 */
void Print(const ARRAY_TYPE_NT(2)& ra,
           const std::string msg=std::string())
{
    std::cout << msg << " array(" << ra.shape()[0] << "," << ra.shape()[1] << ") = " << std::endl;
    for (unsigned i=0; i<ra.shape()[0]; ++i)
    {
        for (unsigned j=0; j<ra.shape()[1]; ++j)
        {
            std::cout << "\t" << ra[i][j];
        }
        std::cout << std::endl;
    }
}

#endif // PRINT_HPP_
