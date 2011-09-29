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

#include "AssignmentStatement.hpp"

#include <boost/make_shared.hpp>
#include "ValueTypes.hpp"
#include "BacktraceException.hpp"
//#include "VectorStreaming.hpp"
//#include "ProtoHelperMacros.hpp"

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
//        if (p_rhs_value->IsArray())
//        {
//            NdArray<double> value = GET_ARRAY(p_rhs_value);
//            std::cout << "Assign " << mNamesToAssign.front() << " = {" << value.GetShape() << "}";
//            std::vector<double> values(value.Begin(), value.End());
//            if (values.size() < 100) std::cout << ": " << values;
//            std::cout << std::endl;
//        }
    }
    return boost::make_shared<NullValue>();
}
