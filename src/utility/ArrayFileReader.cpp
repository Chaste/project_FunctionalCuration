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

#include "ArrayFileReader.hpp"

#include <fstream>
#include <sstream>
#include <vector>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

#include "Exception.hpp"

NdArray<double> ArrayFileReader::ReadFile(const FileFinder& rDataFile)
{
    std::ifstream file(rDataFile.GetAbsolutePath().c_str());
    if (!file)
    {
        EXCEPTION("Failed to open data file: " << rDataFile.GetAbsolutePath());
    }
    // Read the first line to determine the number of columns
    std::string line;
    getline(file, line);
    std::istringstream line_stream(line);
    std::vector<double> data;
    double datum;
    bool csv = false;
    while (line_stream.good())
    {
        line_stream >> datum;
        if (!line_stream.fail())
        {
            data.push_back(datum);
        }
        if (line_stream.peek() == ',')
        {
            // CSV data
            csv = true;
            line_stream.get(); // discard the comma
        }
    }
    const unsigned num_cols = data.size();
    EXCEPT_IF_NOT(num_cols > 0u);
    unsigned num_rows = 1000u; // Guess a value; we'll adjust as we read
    unsigned num_rows_read = 1u;
    // Create the result array
    NdArray<double>::Extents shape = boost::assign::list_of(num_rows)(num_cols);
    NdArray<double> array(shape);
    // Fill in the first row (read above)
    NdArray<double>::Iterator array_it = array.Begin();
    BOOST_FOREACH(datum, data)
    {
        *array_it++ = datum;
    }
    // Read the rest of the file
    while (file.good())
    {
        if (num_rows_read == shape[0])
        {
            // Record where the iterator will need to point to after resizing
            NdArray<double>::Indices idxs = shape;
            idxs[1] = 0u;
            // Extend array
            shape[0] += 1000u;
            array.Resize(shape);
            // Adjust the iterator to point at the new array location
            array_it = NdArray<double>::Iterator(idxs, array);
        }
        getline(file, line);
        std::istringstream line_stream(line);
        unsigned items_read = 0u;
        while (line_stream.good())
        {
            line_stream >> datum;
            if (!line_stream.fail())
            {
                EXCEPT_IF_NOT(items_read < num_cols);
                *array_it++ = datum;
                items_read++;
            }
            if (csv)
            {
                line_stream.get(); // discard comma
            }
        }
        EXCEPT_IF_NOT(items_read == num_cols || items_read == 0u);
        if (items_read > 0u)
        {
            num_rows_read++;
        }
    }
    // Adjust final result size
    shape[0] = num_rows_read;
    array.Resize(shape);
    return array;
}
