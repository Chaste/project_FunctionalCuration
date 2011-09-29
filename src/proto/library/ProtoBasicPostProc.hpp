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

#ifndef PROTOBASICPOSTPROC_HPP_
#define PROTOBASICPOSTPROC_HPP_

#include "Environment.hpp"

/**
 * This class contains methods to define various "standard library" post-processing functions
 * in an environment.
 */
class ProtoBasicPostProc
{
public:
    /**
     * Add all the post-processing functions defined in this class to the given
     * environment.
     *
     * @param rEnv  the environment to contain the function definitions
     */
    void DefineAll(Environment& rEnv);

    /**
     * \code
     * def diff(array, dimension=array.num_dims-1):
     *     map(-, array[<dimension>1:], array[<dimension>:-1])
     * \endcode
     *
     * @param rEnv  the environment to contain the function definition
     */
    void DefineDiff(Environment& rEnv);
};

#endif // PROTOBASICPOSTPROC_HPP_
