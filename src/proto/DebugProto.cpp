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

#include "DebugProto.hpp"

#include "VectorStreaming.hpp"
#include "ProtoHelperMacros.hpp"

void PrintValue(AbstractValuePtr pValue)
{
    if (pValue->IsArray())
    {
        NdArray<double> value = GET_ARRAY(pValue);
        std::cout << "shape " << value.GetShape();
        std::vector<double> values(value.Begin(), value.End());
        if (values.size() < 100) std::cout << ": " << values;
    }
    else if (pValue->IsDefault())
    {
        std::cout << "default-param";
    }
    else if (pValue->IsNull())
    {
        std::cout << "null";
    }
    else if (pValue->IsLambda())
    {
        std::cout << "function";
    }
    else if (pValue->IsString())
    {
        std::cout << "string(" << boost::dynamic_pointer_cast<StringValue>(pValue)->GetString() << ")";
    }
    else if (pValue->IsTuple())
    {
        boost::shared_ptr<TupleValue> p_value = boost::dynamic_pointer_cast<TupleValue>(pValue);
        std::cout << "tuple{" << p_value->GetNumItems() << "}:";
        for (unsigned i=0; i<p_value->GetNumItems(); ++i)
        {
            std::cout << " " << p_value->GetItem(i);
        }
    }
}
