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

#ifndef ABSTRACTSTATEMENT_HPP_
#define ABSTRACTSTATEMENT_HPP_

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include "LocatableConstruct.hpp"
#include "AbstractValue.hpp"

class AbstractStatement;
typedef boost::shared_ptr<AbstractStatement> AbstractStatementPtr;

class Environment;  // Avoid circular includes

/**
 * Base class for statements in the protocol post-processing language.
 *
 * There are only three kinds of statement: assignments, function returns, and assertions.
 * Most computation is done by expressions.
 */
class AbstractStatement : boost::noncopyable, public LocatableConstruct
{
public:
    /**
     * Evaluate the RHS of this statement within a given environment.
     *
     * @param rEnv  the environment
     */
    virtual AbstractValuePtr operator()(Environment& rEnv) const =0;

    /** Needed since we have virtual methods */
    virtual ~AbstractStatement()
    {}
};


#endif // ABSTRACTSTATEMENT_HPP_
