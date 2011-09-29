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

#ifndef PROTOCOLLANGUAGE_HPP_
#define PROTOCOLLANGUAGE_HPP_

/**
 * @file
 * A convenience header including all of the protocol language functionality.
 *
 * \todo Finish documenting some classes.
 *
 * \todo Document pointer typedefs.  Do we need const versions (i.e. pointer-to-const)?
 */

#include <boost/make_shared.hpp> // Requires Boost 1.39 (available in Lucid and newer)
#include <boost/assign/list_of.hpp> // Requires Boost 1.34

#include "BacktraceException.hpp"
#include "VectorStreaming.hpp"
#include "PetscTools.hpp"

#include "NdArray.hpp"

#include "LocatableConstruct.hpp"
#include "AbstractValue.hpp"
#include "AbstractStatement.hpp"
#include "AbstractExpression.hpp"
#include "Environment.hpp"

#include "ValueTypes.hpp"

#include "AssignmentStatement.hpp"
#include "ReturnStatement.hpp"
#include "AssertStatement.hpp"

#include "FunctionCall.hpp"
#include "LambdaExpression.hpp"
#include "NameLookup.hpp"
#include "TupleExpression.hpp"
#include "ValueExpression.hpp"
#include "Accessor.hpp"
#include "If.hpp"
#include "Find.hpp"
#include "Fold.hpp"
#include "Index.hpp"
#include "Map.hpp"
#include "View.hpp"
#include "ArrayCreate.hpp"
#include "MathmlAll.hpp"

#include "ProtoHelperMacros.hpp"


#endif // PROTOCOLLANGUAGE_HPP_
