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

#include "FileLoader.hpp"

// Includes for things we forward-declared in our header
#include "AbstractValue.hpp"
#include "Environment.hpp"

// Includes for other stuff only used here
#include <boost/make_shared.hpp>
#include "ArrayFileReader.hpp"
#include "BacktraceException.hpp"
#include "DebugProto.hpp"
#include "LambdaClosure.hpp"
#include "NameLookup.hpp"
#include "ReturnStatement.hpp"
#include "ValueTypes.hpp"

FileLoader::FileLoader(const FileFinder& rProtoPath, const std::vector<AbstractExpressionPtr>& rOperands)
    : AbstractExpression(rOperands),
      mBasePath(rProtoPath)
{
}


AbstractValuePtr FileLoader::Create(const FileFinder& rProtoPath, EnvironmentCPtr pDefiningEnv)
{
    std::vector<std::string> fps(1, "data_file_path");
    std::vector<AbstractExpressionPtr> operands(1, boost::make_shared<NameLookup>(fps.front()));
    std::vector<AbstractValuePtr> defaults;
    AbstractExpressionPtr p_loader(new FileLoader(rProtoPath, operands));
    std::vector<AbstractStatementPtr> body(1, boost::make_shared<ReturnStatement>(p_loader));
    body.back()->SetLocationInfo("(Implicit return statement)");
    return boost::make_shared<LambdaClosure>(pDefiningEnv, fps, body, defaults);
}


AbstractValuePtr FileLoader::operator ()(const Environment& rEnv) const
{
    std::vector<AbstractValuePtr> operands = EvaluateChildren(rEnv);
    assert(operands.size() == 1u);
    PROTO_ASSERT(operands.front()->IsString(), "The built-in load function requires a string argument.");
    std::string file_path = static_cast<StringValue*>((operands.front()).get())->GetString();
    FileFinder data_file(file_path, mBasePath);
    ArrayFileReader reader;
    NdArray<double> array = reader.ReadFile(data_file);
    AbstractValuePtr p_result;
    PROPAGATE_BACKTRACE_ENV(p_result = boost::make_shared<ArrayValue>(array), rEnv);
    return TraceResult(p_result);

}
