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
    return boost::make_shared<ArrayValue>(result);
}
