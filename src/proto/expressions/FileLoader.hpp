/*

Copyright (c) 2005-2016, University of Oxford.
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

#ifndef PROJECTS_FUNCTIONALCURATION_SRC_PROTO_VALUES_FILELOADER_HPP_
#define PROJECTS_FUNCTIONALCURATION_SRC_PROTO_VALUES_FILELOADER_HPP_

// Forward declarations for stuff we don't really need; includes appear in the cpp file.
class Environment;

// Includes for stuff we really need: base classes, members that are objects, STL classes referenced here.
#include "AbstractExpression.hpp"
#include "AbstractValue.hpp"
#include "FileFinder.hpp"
#include <vector>

/**
 * A built-in function responsible for loading data files. It takes a single argument - the path
 * to the file to load, relative to the protocol file itself.  If the load succeeds, it returns
 * an n-d array containing the file contents.
 *
 * This is implemented as an expression with a helper function for constructing it into a
 * LambdaClosure, in a similar way to wrapping MathML operators in LambdaExpression::WrapMathml.
 */
class FileLoader : public AbstractExpression
{
public:
    /**
     * Create a new FileLoader instance relative to the given protocol file, wrapped in a
     * LambdaClosure.
     *
     * @param rProtoPath the protocol file with respect to which to resolve relative file paths
     * @param pDefiningEnv the Environment in which the wrapping LambdaClosure is defined (not
     *    really used in this instance, but required by the LambdaClosure class)
     */
    static AbstractValuePtr Create(const FileFinder& rProtoPath,
                                   EnvironmentCPtr pDefiningEnv);

    /**
     * Evaluate the expression in an environment.
     *
     * @param rEnv  the environment
     */
    AbstractValuePtr operator()(const Environment& rEnv) const;

private:
    /**
     * Create a new FileLoader instance relative to the given protocol file.
     *
     * @param rProtoPath the protocol file with respect to which to resolve relative file paths
     * @param rOperands single-element vector containing a NameLookup of the data file path
     */
    FileLoader(const FileFinder& rProtoPath,
               const std::vector<AbstractExpressionPtr>& rOperands);

    /** The path with respect to which to resolve relative file paths. */
    FileFinder mBasePath;
};

#endif // PROJECTS_FUNCTIONALCURATION_SRC_PROTO_VALUES_FILELOADER_HPP_
