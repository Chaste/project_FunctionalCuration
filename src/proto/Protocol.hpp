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

#ifndef PROTOCOL_HPP_
#define PROTOCOL_HPP_

#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include "AbstractSystemWithOutputs.hpp"
#include "AbstractStatement.hpp"
#include "Environment.hpp"
#include "AbstractSimulation.hpp"
#include "ModelStateCollection.hpp"
#include "OutputSpecification.hpp"
#include "PlotSpecification.hpp"

#include "OutputFileHandler.hpp"
#include "FileFinder.hpp"
#include "AbstractCardiacCellInterface.hpp"

// See below!
class Protocol;

/** Most users will access a protocol via this smart pointer type. */
typedef boost::shared_ptr<Protocol> ProtocolPtr;

/**
 * This class encapsulates a complete protocol description, at least as far as the C++ code is concerned.
 */
class Protocol : boost::noncopyable
{
public:
    /**
     * Default constructor.
     */
    Protocol();

    /**
     * Run this protocol.
     */
    void Run();

    /**
     * Write protocol outputs to a collection of files.
     *
     * Each output will be written to a separate file, the name of which
     * will consist of the variable name appended to rFileNameBase, with
     * an extension of .dat.
     *
     * @param rHandler  output directory to write to
     * @param rFileNameBase  base part of output file names
     */
    void WriteToFile(const OutputFileHandler& rHandler,
                     const std::string& rFileNameBase) const;

    /**
     * Get the number of protocol outputs defined.
     *
     * @param simulation  which simulation to get the number of outputs of, by index
     */
    unsigned GetNumberOfOutputs(unsigned simulation) const;

    /**
     * Get the environment containing the outputs of interest for this protocol.
     *
     * @param simulation  which simulation to get the outputs of, by index
     */
    const Environment& rGetOutputsCollection(unsigned simulation) const;

    /**
     * Get the number of protocol outputs defined.
     *
     * @param rPrefix  which simulation to get the number of outputs of (defaults to the overall protocol outputs)
     */
    unsigned GetNumberOfOutputs(const std::string& rPrefix="") const;

    /**
     * Get the environment containing the outputs of interest for this protocol.
     *
     * @param rPrefix  which simulation to get the outputs of (defaults to the overall protocol outputs)
     */
    const Environment& rGetOutputsCollection(const std::string& rPrefix="") const;

    /**
     * Get the model state collection associated with this protocol.
     */
    boost::shared_ptr<ModelStateCollection> GetStateCollection();

    /**
     * Set the value of a protocol input.
     * @param rName  the input name
     * @param pValue  an expression giving its new value
     */
    void SetInput(const std::string& rName, AbstractExpressionPtr pValue);

    //
    // Get methods for the constituent parts of a protocol
    //

    /** Get the file the protocol was loaded from. */
    const FileFinder& rGetSourceFile() const;

    /** Get the imported protocol associated with the given prefix. */
    ProtocolPtr GetImportedProtocol(const std::string& rPrefix);

    /** Get the namespaces available for use in referencing model variables. */
    const std::map<std::string, std::string>& rGetNamespaceBindings() const;

    /** Get the protocol input definitions. */
    Environment& rGetInputsEnvironment();

    /** Get the statements defining the protocol inputs. */
    std::vector<AbstractStatementPtr>& rGetInputStatements();

    /**Get the library of definitions in this protocol. */
    Environment& rGetLibrary();

    /** Get the statements comprising the library part of the protocol. */
    std::vector<AbstractStatementPtr>& rGetLibraryStatements();

    /** Get the list of simulations to be performed. */
    std::vector<AbstractSimulationPtr>& rGetSimulations();

    /** Get the post-processing program part of the protocol. */
    std::vector<AbstractStatementPtr>& rGetPostProcessing();

    /** Get specifications of which variables should be considered as the outputs of this protocol. */
    std::vector<boost::shared_ptr<OutputSpecification> >& rGetOutputSpecifications();

    /** Get specifications of what plots to produce by default. */
    std::vector<boost::shared_ptr<PlotSpecification> >& rGetPlotSpecifications();

    /** Get the model being simulated in this protocol. */
    boost::shared_ptr<AbstractUntemplatedParameterisedSystem> GetModel();

    //
    // Methods for setting up a protocol, used by the parser
    //

    /** Record the file the protocol was loaded from. */
    void SetSourceFile(const FileFinder& rSourceFile);

    /**
     * Add an imported protocol that can be referred to by a prefix.
     *
     * @param rPrefix  the prefix to use
     * @param pImport  the imported protocol
     * @param rLoc  the location of the import element, for error messages
     */
    void AddImport(const std::string& rPrefix, ProtocolPtr pImport, const std::string& rLoc);

    /**
     * Add to the namespaces available for use in referencing model variables.
     *
     * @param rBindings  mapping from prefix to namespace URI
     */
    void AddNamespaceBindings(const std::map<std::string, std::string>& rBindings);

    /**
     * Add default definitions for this protocol's inputs.
     *
     * @param rStatements  the statements to append
     */
    void AddInputDefinitions(const std::vector<AbstractStatementPtr>& rStatements);

    /**
     * Add some post-processing commands to the library available for this protocol.
     *
     * @param rStatements  the statements to append
     */
    void AddLibrary(const std::vector<AbstractStatementPtr>& rStatements);

    /**
     * Add a simulation to run.  Simulations will be run in the order in which they are added.
     *
     * @param pSimulation  the simulation
     */
    void AddSimulation(AbstractSimulationPtr pSimulation);

    /**
     * Add a list of simulations to run.  Simulations will be run in the order in which they are added.
     *
     * @param rSimulations  the simulations
     */
    void AddSimulations(const std::vector<AbstractSimulationPtr>& rSimulations);

    /**
     * Add some additional post-processing commands to the program.
     *
     * @param rStatements  the statements to append
     */
    void AddPostProcessing(const std::vector<AbstractStatementPtr>& rStatements);

    /**
     * Add specifications for which variables should be considered as the outputs of this protocol.
     *
     * @param rSpecifications  the output variable specifications
     */
    void AddOutputSpecs(const std::vector<boost::shared_ptr<OutputSpecification> >& rSpecifications);

    /**
     * Add some plots to produce by default.
     *
     * @param rSpecifications  the plot specifications
     */
    void AddDefaultPlots(const std::vector<boost::shared_ptr<PlotSpecification> >& rSpecifications);

    /**
     * Once all components are added to the protocol, this method should be called to finish
     * initialisation before the protocol may be run, or used as an import.
     */
    void FinaliseSetup();

    /**
     * This method runs the library program.  In normal usage it is run by SetModel, as the
     * library may reference model variables.  However, a separate method is provided so that
     * tests of protocol libraries don't need to provide a model.
     */
    void InitialiseLibrary();

    /**
     * Set the model being simulated by this protocol.
     *
     * \todo #1872 set model for imported protocols too
     *
     * @param pModel  the model being simulated
     */
    void SetModel(boost::shared_ptr<AbstractCardiacCellInterface> pModel);

    /**
     * Generate and execute a Gnuplot script for the given specification, whose data should already exist in a file.
     *
     * \todo #1999 currently only works for two 1d arrays plotted against each other
     * \todo #1999 make the plot title include the model as well as protocol name
     *
     * @param pPlotSpec  The plot specification to get title, variable names etc.
     * @param rHandler  The output file handler pointing to the current model/protocol output directory.
     * @param rDataFileName  The name of the csv file to which Gnuplot data has been written
     * @param numTraces  How many traces are included in the data
     * @param numPointsInTrace  How many points are in the trace (only needed to work out whether to display as lines or points)
     */
    void PlotWithGnuplot(boost::shared_ptr<PlotSpecification> pPlotSpec,
                         const OutputFileHandler& rHandler,
                         const std::string& rDataFileName,
                         const unsigned numTraces,
                         const unsigned numPointsInTrace) const;

    /**
     * Generate figures for requested plots, using Gnuplot.
     * Called from WriteToFile.
     * See also PlotWithGnuplot, which this method uses.
     *
     * @param rHandler  The output file handler pointing to the current model/protocol output directory
     * @param rFileNameBase  The base name for output files (ignored for the final .eps files, which just use the plot title)
     */
    void GeneratePlots(const OutputFileHandler& rHandler,
                       const std::string& rFileNameBase) const;

private:
    /** Where the protocol was loaded from, for resolving relative imports. */
    FileFinder mSourceFilePath;

    /** The namespaces available for use in referencing model variables. */
    std::map<std::string, std::string> mOntologyNamespaceBindings;

    /**
     * Environments containing the outputs of interest for this protocol.
     * The overall protocol outputs are found under the empty string; outputs
     * for specific simulations are indexed by their defined prefix.
     */
    std::map<std::string, EnvironmentPtr> mOutputs;

    /** Imported protocols associated with a prefix for access. */
    std::map<std::string, ProtocolPtr> mImports;

    /** The environment containing the protocol inputs. */
    EnvironmentPtr mpInputs;

    /** The environment containing the library of functionality available for use. */
    EnvironmentPtr mpLibrary;

    /** The simulations to run. */
    std::vector<AbstractSimulationPtr> mSimulations;

    /** Our collection of saved model states. */
    boost::shared_ptr<ModelStateCollection> mpModelStateCollection;

    /** The environment wrapping the model's state, parameters, etc. */
    EnvironmentPtr mpModelEnvironment;

    /** The protocol input default definitions. */
    std::vector<AbstractStatementPtr> mInputStatements;

    /** The library program. */
    std::vector<AbstractStatementPtr> mLibraryStatements;

    /** The post-processing program. */
    std::vector<AbstractStatementPtr> mPostProcessing;

    /** Which variables should be considered as the outputs of this protocol. */
    std::vector<boost::shared_ptr<OutputSpecification> > mOutputSpecifications;

    /** What plots to produce by default. */
    std::vector<boost::shared_ptr<PlotSpecification> > mPlotSpecifications;

    /** The model being simulated by this protocol. */
    boost::shared_ptr<AbstractUntemplatedSystemWithOutputs> mpModel;

    /**
     * Check that the supplied model does have outputs, and cast it.
     *
     * @param pModel  the model
     */
    boost::shared_ptr<AbstractUntemplatedSystemWithOutputs> CheckModel(boost::shared_ptr<AbstractCardiacCellInterface> pModel) const;

    /**
     * Ensure that this protocol, and any it imports, can access model variables through the
     * given environment wrappers.  We do this by making the inputs environment delegate to
     * model environment(s) for ontology prefixes.
     *
     * This method also executes the library program for this protocol and any inports, which
     * may need to reference model variables.
     *
     * @param rModelEnvs  environments wrapping the model
     */
    void SetModelEnvironments(const std::map<std::string, EnvironmentPtr>& rModelEnvs);
};

#endif // PROTOCOL_HPP_
