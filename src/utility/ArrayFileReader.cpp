/*

Copyright (c) 2005-2016, University of Oxford.
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
#include <boost/foreach.hpp>

#include "Exception.hpp"

NdArray<double> ArrayFileReader::ReadFile(const FileFinder& rDataFile)
{
    std::ifstream file(rDataFile.GetAbsolutePath().c_str());
    if (!file)
    {
        EXCEPTION("Failed to open data file: " << rDataFile.GetAbsolutePath());
    }
    std::string line;
    std::istringstream line_stream;
    // Read the first non-header line to determine the number of columns
    while (true)
    {
        getline(file, line);
        line_stream.str(line);
        line_stream >> std::ws; // Ignore leading spaces
        if (line_stream.peek() != '#' || !line_stream.good() || !file.good())
        {
            break; // Line is not a header, or file is only headers!
        }
    }
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
    NdArray<double>::Extents shape {num_cols, num_rows};
    NdArray<double> array(shape);
    NdArray<double>::Indices idxs = array.GetIndices();
    // Fill in the first row (read above)
    for (unsigned i=0; i<num_cols; i++)
    {
        idxs[0] = i;
        array[idxs] = data[i];
    }
    // Read the rest of the file
    while (file.good())
    {
        if (num_rows_read == shape[1])
        {
            // Extend array
            shape[1] += 1000u;
            array.Resize(shape);
        }
        getline(file, line);
        std::istringstream line_stream(line);
        idxs[0] = 0u;
        idxs[1] = num_rows_read;
        while (line_stream.good())
        {
            line_stream >> datum;
            if (!line_stream.fail())
            {
                EXCEPT_IF_NOT(idxs[0] < num_cols);
                array[idxs] = datum;
                idxs[0]++;
            }
            if (csv)
            {
                line_stream.get(); // discard comma
            }
        }
        EXCEPT_IF_NOT(idxs[0] == num_cols || idxs[0] == 0u);
        if (idxs[0] > 0u)
        {
            num_rows_read++;
        }
    }
    // Adjust final result size
    shape[1] = num_rows_read;
    array.Resize(shape);
    return array;
}
