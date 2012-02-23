/*

Copyright (c) 2005-2012, University of Oxford.
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
