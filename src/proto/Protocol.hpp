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
#include "Manifest.hpp"

#include "OutputFileHandler.hpp"
#include "FileFinder.hpp"
#include "Exception.hpp"

// See below!
class Protocol;

/** Most users will access a protocol via this smart pointer type. */
typedef boost::shared_ptr<Protocol> ProtocolPtr;

/**
 * This class encapsulates a complete protocol description, at least as far as the C++ code is concerned.
 */
class Protocol : private boost::noncopyable
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
     * Run this protocol and, provided SetOutputFolder has been called, write the protocol
     * outputs to files upon completion.  If an error occurs while running the protocol,
     * this method will try to write out any requested outputs that have been generated.
     * Provenance information about the machine running the protocol will also be recorded.
     * If the protocol completes successfully, an empty file named 'success' will be created
     * in the output folder to indicate this.
     *
     * @param fileNameBase  base part of output file names; see WriteToFile for details
     */
    void RunAndWrite(const std::string fileNameBase="outputs");

    /**
     * Set the location to which protocol outputs, and any debugging/tracing information,
     * will be written.  If this is not called prior to running the protocol, then certain
     * debug/trace features will not be available.
     *
     * @param rHandler  output directory to write to
     */
    void SetOutputFolder(const OutputFileHandler& rHandler);

    /**
     * Write protocol outputs to a collection of files within our output folder.
     *
     * Each output will be written to a separate file, the name of which
     * will consist of the variable name appended to rFileNameBase, with
     * an extension of .csv.
     *
     * @param rFileNameBase  base part of output file names
     */
    void WriteToFile(const std::string& rFileNameBase);

    /**
     * Write protocol outputs to a collection of files in a given folder.
     * This just calls SetOutputFolder(rHandler) and WriteToFile(rFileNameBase).
     *
     * @param rHandler  output directory to write to
     * @param rFileNameBase  base part of output file names
     */
    void WriteToFile(const OutputFileHandler& rHandler,
                     const std::string& rFileNameBase);

    /**
     * Set the indentation level to use for status/error output written to the screen/terminal.
     * @param indent  the string (typically containing just spaces) to use for indentation
     */
    void SetIndent(std::string indent);

    /**
     * Set whether to write plots to PNG format as well as the default EPS.
     *
     * @param writePng  whether .png files should be created for plots
     */
    void SetPngOutput(bool writePng);

    /**
     * Set whether to use automatic parallelisation of nested simulation loops.
     *
     * @param paralleliseLoops  whether to parallelise whenever safe
     */
    void SetParalleliseLoops(bool paralleliseLoops=true);

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
     *
     * Note that the defining expression is evaluated in the context of the inputs environment.
     * However the old definition of the input is removed *before* it is evaluated.
     *
     * @param rName  the input name
     * @param pValue  an expression giving its new value
     */
    void SetInput(const std::string& rName, AbstractExpressionPtr pValue);

    /**
     * @return  the manifest of output files written by the protocol
     */
    Manifest& rGetManifest();

    /**
     * Write a short error message to an "errors.txt" file summarising any errors that occurred.
     *
     * @param rMessage  the error message
     * @param rHandler  where to write the errors file
     */
    static void WriteError(const std::string& rMessage, const OutputFileHandler& rHandler);

    /**
     * Write a short summary of the given error to the "errors.txt" file.
     *
     * @param rError  an error that has just occurred
     */
    void WriteError(const Exception& rError);

    /**
     * Write a short error message to the "errors.txt" file.
     *
     * @param rMessage  the error message
     */
    void WriteError(const std::string& rMessage);

    /**
     * Set where to write error summaries to (defaults to our main output location).
     *
     * @param pHandler  handler for writing error summaries
     */
    void SetErrorFolder(boost::shared_ptr<OutputFileHandler> pHandler);

    //
    // Get methods for the constituent parts of a protocol
    //

    /** Get the file the protocol was loaded from. */
    const FileFinder& rGetSourceFile() const;

    /** Get the imported protocol associated with the given prefix. */
    ProtocolPtr GetImportedProtocol(const std::string& rPrefix);

    /** Get all our (prefixed) imported protocols. */
    const std::map<std::string, ProtocolPtr> rGetImportedProtocols() const;

    /** Get the namespaces available for use in referencing model variables. */
    const std::map<std::string, std::string>& rGetNamespaceBindings() const;

    /** Get the protocol input definitions. */
    Environment& rGetInputsEnvironment();

    /** Get the statements defining the protocol inputs. */
    std::vector<AbstractStatementPtr>& rGetInputStatements();

    /** Get the library of definitions in this protocol. */
    Environment& rGetLibrary();

    /** Get the statements comprising the library part of the protocol. */
    std::vector<AbstractStatementPtr>& rGetLibraryStatements();

    /** Get the list of simulations to be performed. */
    std::vector<AbstractSimulationPtr>& rGetSimulations();

    /** Get the post-processing program part of the protocol. */
    std::vector<AbstractStatementPtr>& rGetPostProcessing();

    /** Get specifications of which variables should be considered as the outputs of this protocol. */
    std::vector<OutputSpecificationPtr>& rGetOutputSpecifications();

    /** Get specifications of what plots to produce by default. */
    std::vector<PlotSpecificationPtr>& rGetPlotSpecifications();

    /** Get the model being simulated in this protocol. */
    boost::shared_ptr<AbstractSystemWithOutputs> GetModel();

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
     *
     * @param reinit  whether to clear the library and re-run the library program
     */
    void InitialiseLibrary(bool reinit=false);

    /**
     * Set the model being simulated by this protocol.
     *
     * @param pModel  the model being simulated
     */
    void SetModel(boost::shared_ptr<AbstractSystemWithOutputs> pModel);

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
    boost::shared_ptr<AbstractSystemWithOutputs> mpModel;

    /** Handler for writing results, debug & tracing information to file. */
    boost::shared_ptr<OutputFileHandler> mpOutputHandler;

    /** Handler for writing error summaries to file. */
    boost::shared_ptr<OutputFileHandler> mpErrorHandler;

    /** The manifest of output files written by the protocol. */
    Manifest mManifest;

    /** The indentation string to use for status/error lines written to cout/cerr. */
    std::string mIndent;

    /** Whether to write plots to .png as well as .eps. */
    bool mWritePng;

    /** Whether to use automatic parallelisation of nested simulation loops. */
    bool mParalleliseLoops;

    /**
     * Check that the supplied model does have outputs, and cast it.
     *
     * @param pModel  the model
     */
    void CheckModel(boost::shared_ptr<AbstractSystemWithOutputs> pModel) const;

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

    /**
     * Generate and execute a Gnuplot script for the given specification, whose data should already exist in a file.
     *
     * \todo #1999 currently only works for two 1d arrays plotted against each other
     * \todo #1999 make the plot title include the model as well as protocol name
     *
     * @param pPlotSpec  the plot specification to get title, variable names etc.
     * @param rDataFileName  the name of the csv file to which Gnuplot data has been written
     * @param numTraces  how many traces are included in the data
     * @param numPointsInTrace  how many points are in the trace (only needed to work out whether to display as lines or points)
     * @param xLabel  label for the plot x axis
     * @param yLabel  label for the plot y axis
     * @param writePng  whether to write a PNG (true) or EPS (false) file
     */
    void PlotWithGnuplot(boost::shared_ptr<PlotSpecification> pPlotSpec,
                         const std::string& rDataFileName,
                         const unsigned numTraces,
                         const unsigned numPointsInTrace,
                         std::string xLabel,
                         std::string yLabel,
                         bool writePng);

    /**
     * Generate figures for requested plots, using Gnuplot.
     * Called from WriteToFile.
     * See also PlotWithGnuplot, which this method uses.
     *
     * @param rFileNameBase  the base name for output files (ignored for the final .eps files, which just use the plot title)
     */
    void GeneratePlots(const std::string& rFileNameBase);
};

#endif // PROTOCOL_HPP_
