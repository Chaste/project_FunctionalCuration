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

#include "Find.hpp"

#include <boost/assign/list_of.hpp>
#include <boost/make_shared.hpp>

#include "BacktraceException.hpp"
#include "NdArray.hpp"
#include "ValueTypes.hpp"
#include "ProtoHelperMacros.hpp"

Find::Find(const AbstractExpressionPtr pOperand)
    : FunctionCall("~find", boost::assign::list_of(pOperand))
{}

AbstractValuePtr Find::operator()(const Environment& rEnv) const
{
    // Get & check arguments
    std::vector<AbstractValuePtr> actual_params = EvaluateChildren(rEnv);
    const AbstractValuePtr p_operand = actual_params[0];
    PROTO_ASSERT(p_operand->IsArray(), "First argument to index should be an array.");
    NdArray<double> operand = GET_ARRAY(p_operand);

    // Create output array large enough if all entries are non-zero
    const unsigned N = operand.GetNumDimensions();
    NdArray<double>::Extents shape(2);
    shape[0] = operand.GetNumElements();
    shape[1] = N;
    NdArray<double> result(shape);

    // Fill it in
    NdArray<double>::Iterator result_iter = result.Begin();
    unsigned num_non_zeros = 0u;
    for (NdArray<double>::ConstIterator operand_iter = operand.Begin();
         operand_iter != operand.End();
         ++operand_iter)
    {
        if (*operand_iter != 0.0)
        {
            for (unsigned j=0; j<N; ++j)
            {
                *result_iter++ = operand_iter.rGetIndices()[j];
            }
            num_non_zeros++;
        }
    }

    // Re-size down to actual number of non-zeros
    if (num_non_zeros != shape[0])
    {
        shape[0] = num_non_zeros;
        result.Resize(shape);
    }
    return TraceResult(boost::make_shared<ArrayValue>(result));
}
