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

#include "AbstractExpression.hpp"

AbstractExpression::AbstractExpression(const std::vector<AbstractExpressionPtr>& rChildren)
    : mChildren(rChildren)
{}

AbstractExpression::AbstractExpression()
{}

AbstractExpression::~AbstractExpression()
{}

std::vector<AbstractValuePtr> AbstractExpression::EvaluateChildren(const Environment& rEnv) const
{
    std::vector<AbstractValuePtr> values;
    values.reserve(mChildren.size());
    for (std::vector<AbstractExpressionPtr>::const_iterator it = mChildren.begin();
         it != mChildren.end();
         ++it)
    {
        values.push_back((**it)(rEnv));
    }
    return values;
}

#include "DebugProto.hpp"

AbstractValuePtr AbstractExpression::TraceResult(AbstractValuePtr pResult) const
{
    if (GetTrace())
    {
        std::cout << "Result: ";
        PrintValue(pResult);
        std::cout << " at " << GetLocationInfo() << std::endl;
    }
    return pResult;
}
