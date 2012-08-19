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

#include "SedmlParser.hpp"

#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>

#include "OutputFileHandler.hpp"
#include "AbstractCvodeCell.hpp"
#include "CellMLToSharedLibraryConverter.hpp"
#include "DynamicCellModelLoader.hpp"
#include "AbstractDynamicallyLoadableEntity.hpp"
#include "Warnings.hpp"

#include "TaggingDomParser.hpp"
#include "ModifierCollection.hpp"
#include "StateSaverModifier.hpp"
#include "ModelResetModifier.hpp"
#include "UniformStepper.hpp"
#include "VectorStepper.hpp"
#include "TimecourseSimulation.hpp"
#include "CombinedSimulation.hpp"
#include "NestedSimulation.hpp"
#include "ProtoHelperMacros.hpp"
#include "AssignmentStatement.hpp"
#include "NameLookup.hpp"
#include "OutputSpecification.hpp"
#include "ProtocolLanguage.hpp"
#include "ProtocolParser.hpp"
#include "BacktraceException.hpp"

using namespace xercesc;

SedmlParser::SedmlParser()
    : mSedmlNs("http://sed-ml.org/")
{
    SetUseImplicitMap();
}


ProtocolPtr SedmlParser::ParseSedml(const FileFinder& rSedmlFile,
                                    OutputFileHandler& rHandler)
{
    mSedmlFile = rSedmlFile;
    // Parse to DOM
    xsd::cxx::xml::auto_initializer init_fini(true, true);
    xsd::cxx::xml::dom::auto_ptr<DOMDocument> p_sedml_doc(TaggingDomParser::ParseFileToDom(rSedmlFile));
    DOMElement* p_root_elt = p_sedml_doc->getDocumentElement();
    SetContext(p_root_elt);
    PROTO_ASSERT(X2C(p_root_elt->getLocalName()) == "sedML", "Root element must be 'sedML'.");
    PROTO_ASSERT(X2C(p_root_elt->getNamespaceURI()) == "http://sed-ml.org/",
                 "Namespace must be 'http://sed-ml.org/'.");
    const std::string level(GetRequiredAttr(p_root_elt, "level"));
    const std::string version(GetRequiredAttr(p_root_elt, "version"));
    std::cout << "Parsing SED-ML L" << level << "V" << version << " document" << std::endl;
    PROTO_ASSERT(level == "1", "Only SED-ML L1V1 is supported.");
    PROTO_ASSERT(version == "1", "Only SED-ML L1V1 is supported.");

    // Convert DOM into protocol object
    CreateProtocol();
    ParseModels(p_root_elt, rHandler);
    ParseSimulations(p_root_elt);
    ParseTasks(p_root_elt);
    ParseDataGenerators(p_root_elt);
    ParseOutputs(p_root_elt);

    // Finish the setup
    mpProtocol->FinaliseSetup();
    mpProtocol->InitialiseLibrary();

    return mpProtocol;
}


// Subsidiary parsing methods

ProtocolPtr SedmlParser::CreateProtocol()
{
    mpProtocol.reset(new Protocol);
    ProtocolParser fc_parser;
    FileFinder lib_file("projects/FunctionalCuration/src/proto/library/BasicLibrary.xml", RelativeTo::ChasteSourceRoot);
    ProtocolPtr p_library_proto = fc_parser.ParseFile(lib_file);
    mpProtocol->AddImport("std", p_library_proto, "TestSedml");
    return mpProtocol;
}


void SedmlParser::ParseModels(const DOMElement* pRootElt,
                              OutputFileHandler& rHandler)
{
    SetContext(pRootElt);
    mModels.clear();
    std::vector<DOMElement*> model_defns = XmlTools::FindElements(pRootElt, "listOfModels/model");
    // Build map from models to the ultimate source document, checking they're CellML
    std::map<std::string, std::string> model_source_map;
    BOOST_FOREACH(DOMElement* p_model, model_defns)
    {
        const std::string id(GetRequiredAttr(p_model, "id"));
        const std::string source(GetRequiredAttr(p_model, "source"));
        model_source_map[id] = source;

        SetContext(p_model);
        const std::string lang(GetOptionalAttr(p_model, "language", "urn:sedml:language:xml"));
        PROTO_ASSERT(lang.substr(0,25) == "urn:sedml:language:cellml", "Only CellML models are supported.");
        PROTO_ASSERT(lang.substr(25,4) != ".1_1", "Only CellML 1.0 models are supported.");
    }
    BOOST_FOREACH(DOMElement* p_model, model_defns)
    {
        const std::string id(GetRequiredAttr(p_model, "id"));
        std::string source = model_source_map[id];
        std::map<std::string, std::string>::iterator it;
        while ((it = model_source_map.find(source)) != model_source_map.end())
        {
            source = it->second;
        }
        model_source_map[id] = source;
    }

    // Generate each model
    BOOST_FOREACH(DOMElement* p_model, model_defns)
    {
        const std::string id(GetRequiredAttr(p_model, "id"));
        mModels[id] = CreateModel(id, model_source_map[id], rHandler);
        // Make the model have a wrapper environment associated with the model's id.
        std::map<std::string, std::string> fake_nss;
        fake_nss[id] = "https://chaste.comlab.ox.ac.uk/cellml/ns/oxford-metadata#";
        mModels[id]->SetNamespaceBindings(fake_nss);
    }
}


void SedmlParser::ParseSimulations(const DOMElement* pRootElt)
{
    mSimulationDefinitions.clear();
    SetContext(pRootElt);
    std::vector<DOMElement*> list_of_sims = XmlTools::FindElements(pRootElt, "listOfSimulations");
    PROTO_ASSERT(list_of_sims.size() < 2u, "Only one listOfSimulations element may be present.");
    if (list_of_sims.size() == 1)
    {
        std::vector<DOMElement*> simulations = XmlTools::GetChildElements(list_of_sims.front());
        BOOST_FOREACH(const DOMElement* p_sim, simulations)
        {
            SetContext(p_sim);
            const std::string id(GetRequiredAttr(p_sim, "id"));
            PROTO_ASSERT(mSimulationDefinitions.find(id) == mSimulationDefinitions.end(),
                         "The simulation id '" << id << "' is used more than once.");
            mSimulationDefinitions[id] = p_sim;
        }
    }
}


AbstractSimulationPtr SedmlParser::ParseSimulation(const DOMElement* pSimElt,
                                                   boost::shared_ptr<AbstractSystemWithOutputs> pModel,
                                                   bool resetModel)
{
    AbstractSimulationPtr p_main_sim;
    SetContext(pSimElt);
    const std::string sim_type(X2C(pSimElt->getLocalName()));
    const std::string id(GetRequiredAttr(pSimElt, "id"));
    if (sim_type == "uniformTimeCourse")
    {
        // Extract and check time interval
        double t_start = String2Double(GetRequiredAttr(pSimElt, "initialTime"));
        double t_output_start = String2Double(GetRequiredAttr(pSimElt, "outputStartTime"));
        PROTO_ASSERT(t_start <= t_output_start,
                     "Time course start " << t_start << " is later than output start " << t_output_start);
        double t_end = String2Double(GetRequiredAttr(pSimElt, "outputEndTime"));
        PROTO_ASSERT(t_output_start <= t_end,
                     "Time course end " << t_end << " is before output start " << t_output_start);
        unsigned num_points = String2Unsigned(GetRequiredAttr(pSimElt, "numberOfPoints"));
        double t_step = num_points == 0u ? 0.0 : (t_end - t_output_start) / num_points;
        PROTO_ASSERT(num_points == 0u || 0.0 < t_step,
                     "Time course output interval is of zero duration.");

        // Create an initial reset modifier if needed
        AbstractSimulationModifierPtr p_initial_reset;
        if (resetModel)
        {
            p_initial_reset.reset(new ModelResetModifier<N_Vector>(
                    AbstractSimulationModifier::AT_START_ONLY, "", mpProtocol->GetStateCollection()));
        }

        if (t_output_start > t_start)
        {
            // Create an extra simulation for the portion that isn't recorded
            std::vector<double> values = boost::assign::list_of(t_start)(t_output_start);
            AbstractStepperPtr p_stepper(new VectorStepper(id + "_init", "", values));
            AbstractSimulationPtr p_init_sim(new TimecourseSimulation(pModel, p_stepper));
            if (p_initial_reset)
            {
                p_init_sim->AddModifier(p_initial_reset);
                p_initial_reset.reset();
            }
            TransferContext(pSimElt, p_stepper);
            TransferContext(pSimElt, p_init_sim);
            mpProtocol->AddSimulation(p_init_sim);
        }

        // Create the main part of the simulation
        AbstractStepperPtr p_stepper(new UniformStepper(id, "", t_output_start, t_end, t_step));
        p_main_sim.reset(new TimecourseSimulation(pModel, p_stepper));
        if (p_initial_reset)
        {
            p_main_sim->AddModifier(p_initial_reset);
        }
        TransferContext(pSimElt, p_stepper);
        TransferContext(pSimElt, p_main_sim);
    }
    else
    {
        PROTO_EXCEPTION("Unrecognised simulation type " << sim_type << "!");
    }
    // Check algorithm - we only support CVODE
    std::vector<DOMElement*> algorithm = XmlTools::FindElements(pSimElt, "algorithm");
    PROTO_ASSERT(algorithm.size() == 1u, "A single simulation algorithm must be specified.");
    const std::string kisao_id(GetRequiredAttr(algorithm.front(), "kisaoID"));
    PROTO_ASSERT(kisao_id.length() == 13u, "Incorrectly formatted KiSAO ID.");
    PROTO_ASSERT(kisao_id.substr(0u, 6u) == "KISAO:", "Incorrectly formatted KiSAO ID.");
    PROTO_ASSERT(kisao_id == "KISAO:0000019", "Only CVODE is supported.");

    return p_main_sim;
}


AbstractSimulationPtr SedmlParser::ParseTask(const xercesc::DOMElement* pDefnElt,
                                             bool resetModel)
{
    SetContext(pDefnElt);
    AbstractSimulationPtr p_sim;
    const std::string task_type(X2C(pDefnElt->getLocalName()));
    const std::string task_id(GetRequiredAttr(pDefnElt, "id"));
    if (task_type == "task")
    {
        const std::string model_ref(GetRequiredAttr(pDefnElt, "modelReference"));
        const std::string sim_ref(GetRequiredAttr(pDefnElt, "simulationReference"));
        PROTO_ASSERT(mModels.find(model_ref) != mModels.end(),
                     "Referenced model " << model_ref << " does not exist.");
        PROTO_ASSERT(mSimulationDefinitions.find(sim_ref) != mSimulationDefinitions.end(),
                     "Referenced simulation " << sim_ref << " does not exist.");

        boost::shared_ptr<AbstractSystemWithOutputs> p_model = mModels[model_ref];
        p_sim = ParseSimulation(mSimulationDefinitions[sim_ref], p_model, resetModel);
    }
    else if (task_type == "combinedTask")
    {
        std::vector<AbstractSimulationPtr> child_sims;
        BOOST_FOREACH(const DOMElement* p_subtask_list, XmlTools::FindElements(pDefnElt, "listOfSubTasks"))
        {
            BOOST_FOREACH(const DOMElement* p_subtask, XmlTools::GetChildElements(p_subtask_list))
            {
                const std::string subtask_id(GetRequiredAttr(p_subtask, "task"));
                bool reset_model = String2Bool(GetRequiredAttr(p_subtask, "resetModel"));
                child_sims.push_back(ParseTask(mTaskDefinitions[subtask_id], reset_model));
                if (reset_model)
                {
                    // Add a reset-at-end modifier to the subtask
                    child_sims.back()->AddModifier(boost::make_shared<ModelResetModifier<N_Vector> >(
                            AbstractSimulationModifier::AT_END, "", mpProtocol->GetStateCollection()));
                }
            }
        }
        CombinedSimulation::Scheduling scheduling;
        const std::string scheduling_attr(GetRequiredAttr(pDefnElt, "scheduling"));
        if (scheduling_attr == "sequential")
        {
            scheduling = CombinedSimulation::SEQUENTIAL;
        }
        else if (scheduling_attr == "parallel")
        {
            scheduling = CombinedSimulation::PARALLEL;
        }
        else
        {
            PROTO_EXCEPTION("The scheduling attribute must contain 'parallel' or 'sequential'; not '"
                            << scheduling_attr << "'.");
        }
        p_sim = boost::make_shared<CombinedSimulation>(child_sims, scheduling);
        // Pretend that we're simulating the same model as our first child, for now
        p_sim->SetModel(child_sims.front()->GetModel()); ///\todo do better
    }
    else if (task_type == "repeatedTask")
    {
        // Steppers / ranges
        const std::string range_attr(GetRequiredAttr(pDefnElt, "range"));
        std::map<std::string, AbstractStepperPtr> ranges = ParseRanges(pDefnElt);
        SetContext(pDefnElt);
        PROTO_ASSERT(ranges.size() == 1u, "Only a single range is supported at present.");
        AbstractStepperPtr p_stepper = ranges[range_attr];
        PROTO_ASSERT(p_stepper, "Primary task range '" << range_attr << "' is not defined!");
        // Modifiers implied by resetModel
        bool reset_model = String2Bool(GetRequiredAttr(pDefnElt, "resetModel"));
        std::vector<boost::shared_ptr<AbstractSimulationModifier> > modifiers;
        if (reset_model)
        {
            modifiers.push_back(boost::make_shared<ModelResetModifier<N_Vector> >(
                    AbstractSimulationModifier::EVERY_LOOP, "", mpProtocol->GetStateCollection()));
        }
        boost::shared_ptr<ModifierCollection> p_modifiers = boost::make_shared<ModifierCollection>(modifiers);
        // The sub-task
        std::vector<DOMElement*> subtasks = XmlTools::FindElements(pDefnElt, "subTask");
        PROTO_ASSERT(subtasks.size() == 1u, "A nestedTask must have exactly one subTask element.");
        SetContext(subtasks.front());
        const std::string subtask_id(GetRequiredAttr(subtasks.front(), "task"));
        AbstractSimulationPtr p_nested_sim = ParseTask(mTaskDefinitions[subtask_id]);
        p_sim = boost::make_shared<NestedSimulation>(p_nested_sim, p_stepper, p_modifiers);
        // We're simulating the same model as our nested task
        p_sim->SetModel(p_nested_sim->GetModel());
    }
    else
    {
        PROTO_EXCEPTION("Unknown task type '" << task_type << "'.");
    }
    // Add an initial reset if needed (for 'task' this is done in ParseSimulation)
    if (resetModel && task_type != "task")
    {
        AbstractSimulationModifierPtr p_initial_reset(new ModelResetModifier<N_Vector>(
                AbstractSimulationModifier::AT_START_ONLY, "", mpProtocol->GetStateCollection()));
        p_sim->AddModifier(p_initial_reset);
    }
    TransferContext(pDefnElt, p_sim);
    p_sim->SetOutputsPrefix(task_id);
    return p_sim;
}


std::map<std::string, AbstractStepperPtr> SedmlParser::ParseRanges(const xercesc::DOMElement* pTaskDefn)
{
    std::map<std::string, AbstractStepperPtr> ranges;
    BOOST_FOREACH(const DOMElement* p_range_list, XmlTools::FindElements(pTaskDefn, "listOfRanges"))
    {
        BOOST_FOREACH(const DOMElement* p_range, XmlTools::GetChildElements(p_range_list))
        {
            SetContext(p_range);
            const std::string range_type(X2C(p_range->getLocalName()));
            const std::string range_id(GetRequiredAttr(p_range, "id"));
            PROTO_ASSERT(ranges.find(range_id) == ranges.end(),
                         "Range id '" << range_id << "' is duplicated.");
            if (range_type == "vectorRange")
            {
                std::vector<double> values;
                BOOST_FOREACH(const DOMElement* p_value, XmlTools::GetChildElements(p_range))
                {
                    SetContext(p_value);
                    values.push_back(String2Double(X2C(p_value->getTextContent())));
                }
                ranges[range_id] = boost::make_shared<VectorStepper>(range_id, "unknown", values);
            }
            else if (range_type == "uniformRange")
            {
                //id="range_utc2" start="0.0" end="10.0" numberOfPoints="10"
                double t_start = String2Double(GetRequiredAttr(p_range, "start"));
                double t_end = String2Double(GetRequiredAttr(p_range, "end"));
                PROTO_ASSERT(t_start <= t_end,
                             "Time course end " << t_end << " is before start " << t_start);
                unsigned num_points = String2Unsigned(GetRequiredAttr(p_range, "numberOfPoints"));
                double t_step = num_points == 0u ? 0.0 : (t_end - t_start) / num_points;
                PROTO_ASSERT(num_points == 0u || 0.0 < t_step,
                             "Time course output interval is of zero duration.");
                ranges[range_id] = boost::make_shared<UniformStepper>(range_id, "unknown", t_start, t_end, t_step);
            }
            else
            {
                PROTO_EXCEPTION("Unrecognised range type '" << range_type << "'.");
            }
        }
    }
    return ranges;
}


void SedmlParser::ParseTasks(const DOMElement* pRootElt)
{
    mTasks.clear();
    mTaskDefinitions.clear();
    // Firstly set up the id->definition element mapping
    BOOST_FOREACH(const DOMElement* p_task_list, XmlTools::FindElements(pRootElt, "listOfTasks"))
    {
        BOOST_FOREACH(const DOMElement* p_task, XmlTools::GetChildElements(p_task_list))
        {
            const std::string task_id(GetRequiredAttr(p_task, "id"));
            mTaskDefinitions[task_id] = p_task;
        }
    }
    // Then actually parse each definition
    typedef std::pair<std::string, const DOMElement*> StringEltPair;
    BOOST_FOREACH(StringEltPair p_task_defn, mTaskDefinitions)
    {
        AbstractSimulationPtr p_sim = ParseTask(p_task_defn.second);
        mTasks[p_task_defn.first] = p_sim;
        mpProtocol->AddSimulation(p_sim);
    }
}


void SedmlParser::ParseDataGenerators(const DOMElement* pRootElt)
{
    BOOST_FOREACH(const DOMElement* p_data_gen, XmlTools::FindElements(pRootElt, "listOfDataGenerators/dataGenerator"))
    {
        SetContext(p_data_gen);
        const std::string data_gen_id(GetRequiredAttr(p_data_gen, "id"));
        std::vector<AbstractStatementPtr> body;
        std::vector<std::string> fps;

        // Constant parameters for the calculation
        BOOST_FOREACH(const DOMElement* p_param, XmlTools::FindElements(p_data_gen, "listOfParameters/parameter"))
        {
            SetContext(p_param);
            const std::string param_id(GetRequiredAttr(p_param, "id"));
            double value = String2Double(GetRequiredAttr(p_param, "value"));
            AbstractExpressionPtr p_value = CONST(value);
            TransferContext(p_param, p_value);
            AbstractStatementPtr p_assign = ASSIGN_STMT(param_id, p_value);
            TransferContext(p_param, p_assign);
            body.push_back(p_assign);
        }

        // Variables, which represent simulation results
        std::map<std::string, std::string> var_name_map;
        BOOST_FOREACH(const DOMElement* p_var, XmlTools::FindElements(p_data_gen, "listOfVariables/variable"))
        {
            ///\todo In order to support chaining data generators, we need to make taskReference optional,
            /// and add a new attribute (or extend target) to specify the dataGenerator id, which we can
            /// look up in var_name_map.
            SetContext(p_var);
            const std::string var_id(GetRequiredAttr(p_var, "id"));
            PROTO_ASSERT(var_name_map[var_id].empty(),
                         "The variable id " << var_id << " has been used twice.");
            const std::string target(GetOptionalAttr(p_var, "target"));
            PROTO_ASSERT(!target.empty(), "Variables defined by SED-ML symbols are not supported in CellML.");
            const std::string full_task_ref(GetRequiredAttr(p_var, "taskReference"));

            // If we're using a combinedTask, the task_ref may be prefixed.  Check all components exist,
            // and get the model from the last one.
            std::string model_task_ref(full_task_ref);
            size_t i = 0;
            while ((i = model_task_ref.find(':')) != std::string::npos)
            {
                std::string parent_task_ref = model_task_ref.substr(0, i);
                PROTO_ASSERT(mTasks.find(parent_task_ref) != mTasks.end(),
                             "Referenced task " << parent_task_ref << " does not exist.");
                model_task_ref = model_task_ref.substr(i+1);
            }
            PROTO_ASSERT(mTasks.find(model_task_ref) != mTasks.end(),
                         "Referenced task " << model_task_ref << " does not exist.");

            ///\todo Consider also using GetShortName (or similar) for generic ontology terms?
            boost::shared_ptr<AbstractSystemWithOutputs> p_model = mTasks[model_task_ref]->GetModel();
            std::string varname = p_model->rGetShortName(target);
            var_name_map[var_id] = full_task_ref + ":" + varname;
            fps.push_back(var_id);
        }

        // The actual equation to compute
        SetContext(p_data_gen);
        std::vector<DOMElement*> maths = XmlTools::FindElements(p_data_gen, "math");
        PROTO_ASSERT(maths.size() == 1u, "There must be exactly one math element.");
        SetContext(maths.front());
        std::vector<DOMElement*> math_children = XmlTools::GetChildElements(maths.front());
        PROTO_ASSERT(math_children.size() == 1u, "The math element must have exactly one child.");
        AbstractExpressionPtr p_result = ParseExpression(math_children.front());
        AbstractStatementPtr p_return = RETURN_STMT(p_result);
        TransferContext(p_data_gen, p_return);
        body.push_back(p_return);

        // Create a function call containing the data generator body
        AbstractExpressionPtr p_anon_lambda = boost::make_shared<LambdaExpression>(fps, body);
        TransferContext(p_data_gen, p_anon_lambda);
        std::vector<AbstractExpressionPtr> call_args;
        BOOST_FOREACH(const std::string& r_var_id, fps)
        {
            call_args.push_back(boost::make_shared<NameLookup>(var_name_map[r_var_id]));
            TransferContext(p_data_gen, call_args.back());
        }
        AbstractExpressionPtr p_call = boost::make_shared<FunctionCall>(p_anon_lambda, call_args);
        TransferContext(p_data_gen, p_call);

        // Assign the call result to the data generator id
        AbstractStatementPtr p_assign = ASSIGN_STMT(data_gen_id, p_call);
        TransferContext(p_data_gen, p_assign);
        std::vector<AbstractStatementPtr> post_proc = boost::assign::list_of(p_assign);
        mpProtocol->AddPostProcessing(post_proc);

        // Create an output specification for this data generator
        const std::string name(GetOptionalAttr(p_data_gen, "name"));
        OutputSpecificationPtr p_output_spec(
                new OutputSpecification(data_gen_id, data_gen_id, name.empty() ? data_gen_id : name,
                                        "", "Post-processed"));
        TransferContext(p_data_gen, p_output_spec);
        std::vector<OutputSpecificationPtr> output_specs = boost::assign::list_of(p_output_spec);
        mpProtocol->AddOutputSpecs(output_specs);
    }
}


void SedmlParser::ParseOutputs(const DOMElement* pRootElt)
{
    SetContext(pRootElt);
    std::vector<DOMElement*> list_of_outputs = XmlTools::FindElements(pRootElt, "listOfOutputs");
    PROTO_ASSERT(list_of_outputs.size() < 2u, "There must be at most one listOfOutputs element.");
    if (list_of_outputs.size() == 1)
    {
        BOOST_FOREACH(DOMElement* p_output, XmlTools::GetChildElements(list_of_outputs.front()))
        {
            SetContext(p_output);
            const std::string output_type(X2C(p_output->getLocalName()));
            const std::string output_id(GetRequiredAttr(p_output, "id"));
            const std::string output_name(GetOptionalAttr(p_output, "name"));
            if (output_type == "plot2D")
            {
                std::vector<PlotSpecificationPtr> plot_specs;
                BOOST_FOREACH(DOMElement* p_curve, XmlTools::FindElements(p_output, "listOfCurves/curve"))
                {
                    SetContext(p_curve);
                    const std::string curve_id(GetRequiredAttr(p_curve, "id"));
                    const std::string curve_name(GetOptionalAttr(p_curve, "name"));
                    const std::string xref(GetRequiredAttr(p_curve, "xDataReference"));
                    const std::string yref(GetRequiredAttr(p_curve, "yDataReference"));
                    if (String2Bool(GetRequiredAttr(p_curve, "logX")) || String2Bool(GetRequiredAttr(p_curve, "logY")))
                    {
                        WARNING("Log plots are unsupported; using linear axes.");
                    }
                    std::string plot_title = output_name + " - " + (curve_name.empty() ? curve_id : curve_name);
                    PlotSpecificationPtr p_spec(new PlotSpecification(plot_title, xref, yref));
                    TransferContext(p_curve, p_spec);
                    plot_specs.push_back(p_spec);
                }
                mpProtocol->AddDefaultPlots(plot_specs);
            }
            else if (output_type == "plot3D")
            {
                PROTO_EXCEPTION("3D plots are not supported.");
            }
            else if (output_type == "report")
            {
                BOOST_FOREACH(DOMElement* p_data, XmlTools::FindElements(p_output, "listOfDataSets/dataSet"))
                {
                    SetContext(p_data);
                    const std::string ref(GetRequiredAttr(p_data, "dataReference"));
                    const std::string label(GetRequiredAttr(p_data, "label"));
                    // Find the matching output spec and update its description to match label.
                    // Alternatively we could add another output spec named after p_data->id.
                    BOOST_FOREACH(OutputSpecificationPtr p_output_spec,
                                  mpProtocol->rGetOutputSpecifications())
                    {
                        if (p_output_spec->rGetOutputRef() == ref)
                        {
                            p_output_spec->SetOutputDescription(label);
                            break;
                        }
                    }
                }
            }
            else
            {
                std::cout << "  Unrecognised output type!" << std::endl;
            }
        }
    }
}


AbstractExpressionPtr SedmlParser::ParseCsymbolExpression(const DOMElement* pElement)
{
    SetContext(pElement);
    PROTO_EXCEPTION("SED-ML only supports the use of csymbols as operators.");
    // Need to return something to stop compilers complaining
    AbstractExpressionPtr p_expr;
    return p_expr;
}


AbstractExpressionPtr SedmlParser::ParseCsymbolApply(const DOMElement* pApplyElement,
                                                     const DOMElement* pOperator)
{
    SetContext(pApplyElement);
    AbstractExpressionPtr p_expr;
    std::string symbol = GetCsymbolName(pOperator);
    std::string fn_name;
    if (symbol == "min")
    {
        fn_name = "std:Min";
    }
    else if (symbol == "max")
    {
        fn_name = "std:Max";
    }
    else if (symbol == "sum")
    {
        fn_name = "std:Sum";
    }
    else if (symbol == "product")
    {
        fn_name = "std:Product";
    }
    else
    {
        PROTO_EXCEPTION("Application of csymbol " << symbol << " is not recognised.");
    }
    std::vector<AbstractExpressionPtr> operands = ParseOperands(pApplyElement);
    PROTO_ASSERT(operands.size() == 1, "The SED-ML " << symbol << " operator takes exactly one operand.");
    AbstractExpressionPtr p_inner_expr(new FunctionCall(fn_name, operands));
    std::vector<AbstractExpressionPtr> args(1, p_inner_expr);
    args.push_back(CONST(0));
    p_expr.reset(new FunctionCall("std:RemoveDim", args)); // We need a 0d result for SED-ML
    return p_expr; // Note: caller will transfer context for us
}


std::string SedmlParser::GetCsymbolName(const DOMElement* pElement)
{
    std::string definition_url = X2C(pElement->getAttribute(X("definitionURL")));
    PROTO_ASSERT(!definition_url.empty(), "All csymbol elements must have a definitionURL.");
    const size_t base_len = mSedmlNs.length() + 1;
    PROTO_ASSERT(definition_url.substr(0, base_len) == mSedmlNs + "#",
                 "All csymbol elements must have a definitionURL commencing with " << mSedmlNs
                 << "#; " << definition_url << " does not match.");
    return definition_url.substr(base_len);
}


std::string SedmlParser::GetOptionalAttr(const DOMElement* pElt,
                                         const std::string& rName,
                                         const std::string& rDefault)
{
    std::string value(rDefault);
    if (pElt->hasAttribute(X(rName)))
    {
        value = X2C(pElt->getAttribute(X(rName)));
    }
    return value;
}


std::string SedmlParser::GetRequiredAttr(const DOMElement* pElt, const std::string& rName)
{
    SetContext(pElt);
    PROTO_ASSERT(pElt->hasAttribute(X(rName)), "Required attribute " << rName << " is missing.");
    return X2C(pElt->getAttribute(X(rName)));
}


boost::shared_ptr<AbstractSystemWithOutputs> SedmlParser::CreateModel(const std::string& rModel,
                                                                      const std::string& rModelSource,
                                                                      OutputFileHandler& rHandler)
{
    // Copy CellML file into output dir and create conf file
    std::cout << "Generating code for model " << rModel << std::endl;
    FileFinder model_file;
    if (FileFinder::IsAbsolutePath(rModelSource))
    {
        model_file.SetPath(rModelSource, RelativeTo::Absolute);
    }
    else
    {
        model_file.SetPath(rModelSource, mSedmlFile);
    }
    OutputFileHandler handler(rHandler.FindFile(rModel));
    FileFinder copied_model = handler.CopyFileTo(model_file);
    std::string model_name = copied_model.GetLeafNameNoExtension();
    std::vector<std::string> options = boost::assign::list_of("--cvode");//("--expose-annotated-variables");
    FileFinder this_file(__FILE__, RelativeTo::ChasteSourceRoot);
    FileFinder proto_wrapper("SedmlWrapper.py", this_file);
    options.push_back("--protocol=" + proto_wrapper.GetAbsolutePath());
    options.push_back("--protocol-options=\"" + mSedmlFile.GetAbsolutePath() + " " + rModel + " "
                      + rModelSource + '"');

    // Do the conversion
    CellMLToSharedLibraryConverter converter(true, "projects/FunctionalCuration");
    converter.CreateOptionsFile(handler, model_name, options);
    DynamicCellModelLoaderPtr p_loader = converter.Convert(copied_model);
    boost::shared_ptr<AbstractStimulusFunction> p_stimulus;
    boost::shared_ptr<AbstractIvpOdeSolver> p_solver;
    boost::shared_ptr<AbstractSystemWithOutputs> p_cell(dynamic_cast<AbstractSystemWithOutputs*>(p_loader->CreateCell(p_solver, p_stimulus)));
    // Check we have the right bases
    assert(dynamic_cast<AbstractDynamicallyLoadableEntity*>(p_cell.get()));
    assert(dynamic_cast<AbstractCvodeCell*>(p_cell.get()));
    return p_cell;
}
