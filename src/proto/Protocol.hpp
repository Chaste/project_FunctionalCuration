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

#ifndef PROTOCOL_HPP_
#define PROTOCOL_HPP_

#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>

#include "AbstractProtocolOutputs.hpp"
#include "AbstractSystemWithOutputs.hpp"
#include "AbstractStatement.hpp"
#include "Environment.hpp"
#include "AbstractSimulation.hpp"
#include "ModelStateCollection.hpp"
#include "OutputSpecification.hpp"
#include "PlotSpecification.hpp"

#include "OutputFileHandler.hpp"
#include "AbstractCardiacCellInterface.hpp"
#include "AbstractParameterisedSystem.hpp"

/**
 * This class encapsulates a complete protocol description, at least as far as the C++ code is concerned.
 * It defines the interface, but omits part of the implementation - that is delegated to a derived class
 * templated over the underlying model state vector type.
 */
class AbstractProtocol : public AbstractProtocolOutputs
{
public:
    /**
     * Default constructor.
     */
    AbstractProtocol();

    /**
     * Set the model being simulated by this protocol.
     * This method then sets up empty arrays to store the direct model outputs.
     *
     * @param pModel  the model being simulated
     */
    virtual void SetModel(boost::shared_ptr<AbstractCardiacCellInterface> pModel)=0;

    /**
     * Run this protocol.
     */
    void Run();

    /**
     * Add some output data from the model being simulated.
     *
     * @param rSteppers  the steppers controlling the simulation, from which
     *   we can determine which portion of the output to fill
     */
    virtual void AddOutputData(const std::vector<boost::shared_ptr<AbstractStepper> >& rSteppers)=0;

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
     * Set the value of a protocol input.
     * @param rName  the input name
     * @param pValue  an expression giving its new value
     */
    void SetInput(const std::string& rName, AbstractExpressionPtr pValue);

    /**
     * Get the protocol input definitions.
     */
    Environment& rGetInputsEnvironment();

    /**
     * Get the library of definitions in this protocol.
     */
    Environment& rGetLibrary();

    /**
     * Get the post-processing program part of the protocol.
     */
    std::vector<AbstractStatementPtr>& rGetPostProcessing();

    /**
     * Set the namespaces available for use in referencing model variables.
     *
     * @param rBindings  mapping from prefix to namespace URI
     */
    void SetNamespaceBindings(const std::map<std::string, std::string>& rBindings);

    /**
     * Add a simulation to run.  Simulations will be run in the order in which they are added.
     *
     * @param pSimulation  the simulation
     */
    void AddSimulation(boost::shared_ptr<AbstractSimulation> pSimulation);

    /**
     * Add some post-processing commands to the library available for this protocol.
     *
     * @param rStatements  the statements to append
     */
    void AddLibrary(const std::vector<AbstractStatementPtr>& rStatements);

    /**
     * Add some additional post-processing commands to the program.
     *
     * @param rStatements  the statements to append
     */
    void AddPostProcessing(const std::vector<AbstractStatementPtr>& rStatements);

    /**
     * Set which variables should be considered as the outputs of this protocol.
     *
     * @param rSpecifications  the output variable specifications
     */
    void SetProtocolOutputs(const std::vector<boost::shared_ptr<OutputSpecification> >& rSpecifications);

    /**
     * Set what plots to produce by default.
     *
     * @param rSpecifications  the plot specifications
     */
    void SetDefaultPlots(const std::vector<boost::shared_ptr<PlotSpecification> >& rSpecifications);

    /**
     * Get the model state collection associated with this protocol.
     */
    boost::shared_ptr<ModelStateCollection> GetStateCollection();

    /**
     * Get the model being simulated in this protocol.
     */
    virtual boost::shared_ptr<AbstractUntemplatedParameterisedSystem> GetModel() =0;

protected:
    /** Which simulation is currently being run. */
    unsigned mSimulationNumber;

    /** The namespaces available for use in referencing model variables. */
    std::map<std::string, std::string> mOntologyNamespaceBindings;

    /**
     * Environments containing the outputs of interest for this protocol.
     * The overall protocol outputs are found under the empty string; outputs
     * for specific simulations are indexed by their defined prefix.
     */
    std::map<std::string, EnvironmentPtr> mOutputs;

    /** The environment containing the protocol inputs. */
    Environment mInputs;

    /** The environment containing the library of functionality available for use. */
    Environment mLibrary;

    /** The simulations to run. */
    std::vector<boost::shared_ptr<AbstractSimulation> > mSimulations;

    /** Our collection of saved model states. */
    boost::shared_ptr<ModelStateCollection> mpModelStateCollection;

    /** The environment wrapping the model's state, parameters, etc. */
    boost::shared_ptr<Environment> mpModelEnvironment;

    /** The library program. */
    std::vector<AbstractStatementPtr> mLibraryStatements;

    /** The post-processing program. */
    std::vector<AbstractStatementPtr> mPostProcessing;

    /** Which variables should be considered as the outputs of this protocol. */
    std::vector<boost::shared_ptr<OutputSpecification> > mOutputSpecifications;

    /** What plots to produce by default. */
    std::vector<boost::shared_ptr<PlotSpecification> > mPlotSpecifications;
};

/** Most users will access a protocol via this smart pointer type. */
typedef boost::shared_ptr<AbstractProtocol> ProtocolPtr;

/**
 * This class encapsulates a complete protocol description, at least as far as the C++ code is concerned.
 */
template<typename VECTOR>
class Protocol : public AbstractProtocol
{
public:
    /**
     * Set the model being simulated by this protocol.
     * This method then sets up empty arrays to store the direct model outputs.
     *
     * @param pModel  the model being simulated
     */
    void SetModel(boost::shared_ptr<AbstractCardiacCellInterface> pModel);

    /**
     * Add some output data from the model being simulated.
     *
     * @param rSteppers  the steppers controlling the simulation, from which
     *   we can determine which portion of the output to fill
     */
    void AddOutputData(const std::vector<boost::shared_ptr<AbstractStepper> >& rSteppers);

    /** Get the model being simulated in this protocol. */
    boost::shared_ptr<AbstractUntemplatedParameterisedSystem> GetModel();

private:
    /** Type of a raw pointer to a model with outputs. */
    typedef AbstractSystemWithOutputs<VECTOR> RawModelPtr;

    /** Type of a pointer to a model with outputs. */
    typedef boost::shared_ptr<RawModelPtr> ModelPtr;

    /** The model being simulated by this protocol. */
    ModelPtr mpModel;

    /**
     * Check that the supplied model does have outputs, and cast it.
     *
     * @param pModel  the model
     */
    ModelPtr CheckModel(boost::shared_ptr<AbstractCardiacCellInterface> pModel) const;
};

#endif // PROTOCOL_HPP_
