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

#ifndef SEDMLPARSER_HPP_
#define SEDMLPARSER_HPP_

#include <map>
#include <string>
#include <boost/shared_ptr.hpp>

#include "FileFinder.hpp"
#include "OutputFileHandler.hpp"
#include "XmlTools.hpp"

#include "AbstractExpression.hpp"
#include "AbstractSimulation.hpp"
#include "AbstractStepper.hpp"
#include "AbstractSystemWithOutputs.hpp"
#include "MathmlParser.hpp"
#include "Protocol.hpp"

class SedmlParser : public MathmlParser
{
public:
    /**
     * Main constructor.
     */
    SedmlParser();

    /**
     * Parse a SED-ML file into our protocol constructs.
     *
     * @param rSedmlFile  the SED-ML file
     * @param rHandler  where to put any generated model code
     */
    ProtocolPtr ParseSedml(const FileFinder& rSedmlFile,
                           OutputFileHandler& rHandler);

private:
    /** The SED-ML namespace URI. */
    const std::string mSedmlNs;

    /** The SED-ML being parsed. */
    FileFinder mSedmlFile;

    /** The protocol object we're parsing into. */
    ProtocolPtr mpProtocol;

    /** The models used by the SED-ML file. */
    std::map<std::string, boost::shared_ptr<AbstractSystemWithOutputs> > mModels;

    /** The simulation definitions present. */
    std::map<std::string, const xercesc::DOMElement*> mSimulationDefinitions;

    /** The task definitions present. */
    std::map<std::string, const xercesc::DOMElement*> mTaskDefinitions;

    /** The simulation tasks to perform, for which results are recorded for use in data generators. */
    std::map<std::string, AbstractSimulationPtr> mTasks;

    // Subsidiary parsing methods

    /**
     * Create a protocol object to be filled in from the SED-ML.
     */
    ProtocolPtr CreateProtocol();

    /**
     * Parse the model definitions, and call PyCml to generate code for each.
     * @param pRootElt  the SED-ML root element
     * @param rHandler  where to put the generated model code
     * @return  a map from model ID to the runnable model
     */
    void ParseModels(const xercesc::DOMElement* pRootElt,
                     OutputFileHandler& rHandler);

    /**
     * Parse the list of simulations to generate a mapping from their id to the definition element.
     * We don't parse individual simulations at this stage, because multiple tasks may reference the
     * same simulation, in which case we need multiple copies.
     *
     * @param pRootElt  the SED-ML root element
     */
    void ParseSimulations(const xercesc::DOMElement* pRootElt);

    /**
     * Parse a single simulation definition and associate it with a model to create one of our style simulation objects.
     *
     * @param pSimElt  the SED-ML simulation definition
     * @param pModel  the model on which the simulation should be run
     * @param resetModel  whether the model should be reset prior to running the simulation
     */
    AbstractSimulationPtr ParseSimulation(const xercesc::DOMElement* pSimElt,
                                          boost::shared_ptr<AbstractSystemWithOutputs> pModel,
                                          bool resetModel=true);

    /**
     * Parse a single task definition and create our corresponding simulation object
     * (since our "simulations" correspond to SED-ML "tasks").
     *
     * @param pDefnElt  the SED-ML task definition
     * @param resetModel  whether the model should be reset prior to running the simulation
     */
    AbstractSimulationPtr ParseTask(const xercesc::DOMElement* pDefnElt,
                                    bool resetModel=true);

    /**
     * Parse the task definitions and create corresponding simulation objects
     * (since our "simulations" correspond to SED-ML "tasks").
     *
     * @param pRootElt  the SED-ML root element
     */
    void ParseTasks(const xercesc::DOMElement* pRootElt);

    /**
     * Parse the dataGenerator definitions and create corresponding post-processing and output
     * specification constructs in the protocol.
     *
     * Implementation note: the contents of the math element within a SED-ML dataGenerator becomes the
     * return body in an anonymous lambda function, with the variables as function arguments.  Any
     * parameters are defined locally to the function.  Implicit maps are used to support SED-ML
     * semantics.  An assignment statement is added to the post-processing section of the protocol,
     * calling this function with the names used by the generated code, referencing the appropriate
     * simulation results, and assigning the result to the dataGenerator's id.
     *
     * The code generation step creates a map from XPath expression used in the data generators to short
     * name used by the generated code.
     *
     * @param pRootElt  the SED-ML root element
     */
    void ParseDataGenerators(const xercesc::DOMElement* pRootElt);

    /**
     * Parse a parameter definition into an assignment of the parameter's value to its id.
     *
     * @param pParamElt  the SED-ML parameter element
     */
    AbstractStatementPtr ParseParameter(const xercesc::DOMElement* pParamElt);

    /**
     * Parse a variable element to determine the name to look up to obtain the variable's value.
     *
     * @param pVariableElt  the SED-ML variable element
     * @param pTask  if given, the task in which to look up variables.  Used for setValue elements.
     */
    std::string DetermineVariableReferent(const xercesc::DOMElement* pVariableElt,
                                          AbstractSimulationPtr pTask=AbstractSimulationPtr());

    /**
     * Parse the MathML math element in a SED-ML element into a return statement.
     *
     * @param pParentElt  the SED-ML element containing a MathML math element child
     */
    AbstractStatementPtr ParseSedmlMathExpression(const xercesc::DOMElement* pParentElt);

    /**
     * Parse the contents of a SED-ML element that define a mathematical calculation.
     * This is used by data generators and functional ranges to parse the computation aspects
     * of these element.
     *
     * @param pDefnElt  the SED-ML element to parse
     * @param rVariableNameMap  as an input, contains any predefined mappings from variable names
     *     that may be used within the MathML to their fully-qualified names in the protocol
     *     implementation.  It may be modified by the method to include extra definitions from
     *     variable elements within pDefnElt.
     */
    AbstractExpressionPtr ParseSedmlMath(const xercesc::DOMElement* pDefnElt,
                                         std::map<std::string, std::string>& rVariableNameMap);

    /**
     * Parse the SED-ML output specifications and convert them to our output and plot specifications.
     *
     * Implementation notes: our current graphing functionality doesn't cover all of SED-ML.  In particular,
     * we only support 2d plots, we can only do multiple curves implicitly (i.e. when Y is a matrix), and
     * we don't support log-scale plots.
     *
     * A report is basically a 2d table of values, and only makes sense if the data is vectors.  However,
     * we could view it as essentially analogous to our output specifications: it labels variables
     * (dataGenerators) with human-readable names.  The only thing missing is units information, although
     * for SED-ML post-processing this could potentially come straight from the model.
     *
     * @param pRootElt  the SED-ML root element
     */
    void ParseOutputs(const xercesc::DOMElement* pRootElt);

    /**
     * Parse a csymbol element representing a whole expression (as opposed to an operator).
     * These do not occur in SED-ML, so we throw an exception.
     *
     * @param pElement  the csymbol element
     */
    AbstractExpressionPtr ParseCsymbolExpression(const xercesc::DOMElement* pElement);

    /**
     * Parse an application of a csymbol operator.  These provide the basic array handling
     * in SED-ML: min, max, sum, product.
     *
     * @param pApplyElement  the apply element
     * @param pOperator  the csymbol operator element
     */
    AbstractExpressionPtr ParseCsymbolApply(const xercesc::DOMElement* pApplyElement,
                                            const xercesc::DOMElement* pOperator);

    /**
     * Given a csymbol, check that it has a definitionURL with the correct base, and extract the
     * non-base portion.
     *
     * @param pElement  the csymbol element
     */
    std::string GetCsymbolName(const xercesc::DOMElement* pElement);

    /**
     * Get the value of an optional attribute of an element, or the given default if it's missing.
     * @param pElt  the element
     * @param rName  the attribute name
     * @param rDefault  the default value if the attribute is missing
     */
    std::string GetOptionalAttr(const xercesc::DOMElement* pElt,
                                const std::string& rName,
                                const std::string& rDefault="");

    /**
     * Check a required attribute exists, and return its value.
     * @param pElt  the element
     * @param rName  the attribute name
     */
    std::string GetRequiredAttr(const xercesc::DOMElement* pElt, const std::string& rName);

    /**
     * Generate code for the given model declared in the SED-ML.
     * @param rModel  the required model's id
     * @param rModelSource  the file that the model is ultimately derived from
     * @param rHandler  where to put the generated model code
     */
    boost::shared_ptr<AbstractSystemWithOutputs> CreateModel(const std::string& rModel,
                                                             const std::string& rModelSource,
                                                             OutputFileHandler& rHandler);

    //
    // Parsing explicitly of our extensions
    //

    /**
     * Parse the range specifications of a nestedTask.
     *
     * @param pTaskDefn  the task definition element
     */
    std::map<std::string, AbstractStepperPtr> ParseRanges(const xercesc::DOMElement* pTaskDefn);
};

#endif // SEDMLPARSER_HPP_
