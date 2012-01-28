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

#ifndef MAP_HPP_
#define MAP_HPP_

#include "FunctionCall.hpp"

/**
 * The fundamental "map" construct in the post-processing language, that maps a function
 * element-wise onto n-dimensional arrays.
 */
class Map : public FunctionCall
{
public:
    /**
     * Create a map operation.
     *
     * @param rParameters  the parameters for map, the first of which must be evaluate to
     *     a function, and the remainder to arrays of the same shape
     * @param allowImplicitArrays  whether to allow some parameters to be simple values, and
     *     treat them implicitly as arrays containing that value everywhere
     */
    Map(const std::vector<AbstractExpressionPtr>& rParameters,
        bool allowImplicitArrays=false);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;

private:
    /** Whether to treat 0d parameters as implicit arrays. */
    bool mAllowImplicitArrays;
};


#endif // MAP_HPP_
