/*

Copyright (c) 2005-2015, University of Oxford.
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

#include "AssignmentStatement.hpp"

#include <boost/make_shared.hpp>
#include "ValueTypes.hpp"
#include "BacktraceException.hpp"

#include "DebugProto.hpp"

AssignmentStatement::AssignmentStatement(const std::string& rNameToAssign,
                                         const AbstractExpressionPtr pRhs)
    : mNamesToAssign(1, rNameToAssign),
      mpRhs(pRhs)
{}

AssignmentStatement::AssignmentStatement(const std::vector<std::string>& rNamesToAssign,
                                         const AbstractExpressionPtr pRhs)
    : mNamesToAssign(rNamesToAssign),
      mpRhs(pRhs)
{}

AbstractValuePtr AssignmentStatement::operator()(Environment& rEnv) const
{
    AbstractValuePtr p_rhs_value = (*mpRhs)(rEnv);
    const unsigned num_names = mNamesToAssign.size();
    if (num_names > 1)
    {
        PROTO_ASSERT(p_rhs_value->IsTuple(),
                     "When assigning multiple names the value to assign must be a tuple.");
        TupleValue* p_tuple = static_cast<TupleValue*>(p_rhs_value.get());
        PROTO_ASSERT(p_tuple->GetNumItems() == num_names,
                     "Cannot assign " << p_tuple->GetNumItems() << " values to " << num_names << " names.");
        for (unsigned i=0; i<num_names; ++i)
        {
            rEnv.DefineName(mNamesToAssign[i], p_tuple->GetItem(i), GetLocationInfo());
        }
    }
    else
    {
        rEnv.DefineName(mNamesToAssign.front(), p_rhs_value, GetLocationInfo());
        if (GetTrace())
        {
            TRACE_PROTO("Assign " << mNamesToAssign.front() << " <- " << p_rhs_value
                        << " at " << GetLocationInfo() << std::endl);
        }
    }
    return boost::make_shared<NullValue>();
}

const std::vector<std::string>& AssignmentStatement::rGetNamesToAssign() const
{
    return mNamesToAssign;
}
