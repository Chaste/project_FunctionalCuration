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
