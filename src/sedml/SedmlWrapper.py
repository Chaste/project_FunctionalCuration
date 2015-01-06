"""Copyright (c) 2005-2015, University of Oxford.
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
"""

import new
import os

import protocol
import pycml
import translators
from pycml import *

pycml.NSS[u'sedml'] = u'http://sed-ml.org/'

def fix_amara():
    """Fix the Amara xpath_attr_wrapper class."""
    def __init__(self, qname, ns, value, parent):
        self.parentNode = parent
        self.namespaceURI = ns
        self.prefix, self.localName = SplitQName(qname)
        self.value = value
        self.nodeValue = value
        self.name = qname
        self.nodeName = qname
        self.rootNode = parent._rootNode()

    meth = new.instancemethod(locals()['__init__'], None, amara.binderyxpath.xpath_attr_wrapper)
    setattr(amara.binderyxpath.xpath_attr_wrapper, '__init__', meth)
fix_amara()

# TODO: relative models and MIRIAM URNs
# http://www.ebi.ac.uk/miriamws/main/rest/resolve/%s  for a URN gives XML like
# <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
#  <uris><uri type="URL">http://www.ebi.ac.uk/biomodels-main/BIOMD0000000139</uri>
#        <uri type="URL">http://biomodels.caltech.edu/BIOMD0000000139</uri></uris>


class SedmlParser(object):
    """This class parses a SED-ML file and applies the model modifications defined therein."""
    def __init__(self, cellml_doc):
        """Create a new parser for applying modifications to the supplied CellML model."""
        self.cellml_doc = cellml_doc
        model = self.model = cellml_doc.model
        # Parse the SED-ML document
        if model.get_option('protocol_options')[0] in ['"', "'"]:
            model.get_config().options.protocol_options = model.get_option('protocol_options')[1:-1]
        self.sedml_path, self.requested_model, self.model_source = model.get_option('protocol_options').split()
        self.sedml_doc = amara_parse_cellml(self.sedml_path)
        self.model_ids = map(unicode, self.sedml_doc.sedML.listOfModels.xml_xpath(u'sedml:model/@id'))
        # CellML models that have been parsed and loaded
        self.cellml_docs = {}
        self.cellml_docs[self.requested_model] = cellml_doc
        # CellML models that have had SED-ML changes applied
        self.ready_models = set()
    
    def prepare_model(self, model_id):
        """Apply SED-ML changes to the model with given id."""
        if model_id not in self.ready_models:
            list_of_models = self.sedml_doc.sedML.listOfModels
            model_defns = list_of_models.xml_xpath(u'sedml:model[@id="%s"]' % model_id)
            while unicode(model_defns[-1].source) in self.model_ids:
                model_defns.append(list_of_models.xml_xpath(u'sedml:model[@id="%s"]' % model_defns[-1].source)[0])
            if model_id not in self.cellml_docs:
                # Load the ultimate source model from file (TODO: assumes absolute path)
                    self.cellml_docs[model_id] = translators.load_model(model_defns[-1].source,
                                                                        self.model.get_config().options)
            # Apply changes from this and intermediate model definitions
            for model_defn in reversed(model_defns):
                self.apply_changes(self.cellml_docs[model_id], model_defn)
            self.ready_models.add(model_id)
    
    def apply_protocol(self):
        """Apply the changes in the parsed SED-ML."""
        p = protocol.Protocol(self.model)
        # Find the requested model definition, and those it extends, back to the source document
        self.prepare_model(self.requested_model)
        # Figure out what model variables are referenced by the SED-ML data generators
        used_var_xpaths = set()
        list_of_tasks = self.sedml_doc.sedML.listOfTasks
        relevant_task_ids = []
        for task in list_of_tasks.task:
            if task.modelReference == self.requested_model:
                relevant_task_ids.append(task.id)
        data_generators = self.sedml_doc.sedML.listOfDataGenerators
        for datagen in data_generators.dataGenerator:
            if hasattr(datagen, u'listOfVariables'):
                for var in datagen.listOfVariables.variable:
                    xpath = getattr(var, u'target', u'')
                    if xpath:
                        used_var_xpaths.add(xpath)
        # Make any variables referenced by repeatedTask setValue changes into model inputs,
        # and ensure those referenced in the calculations are available.
        # TODO: Only do this for repeatedTasks applied to the requested model.
        for task in self.sedml_doc.xml_xpath('/sedml:sedML/sedml:listOfTasks/sedml:repeatedTask'):
            for set_value in task.xml_xpath('sedml:listOfChanges/sedml:setValue'):
                assert hasattr(set_value, u'target'), "We only support setValue with target attribute at present"
                target = self.get_var_elt(self.cellml_doc, set_value.target)
                p.inputs.add(target)
                p.add_alias(target, set_value.target)
                for variable_decl in set_value.xml_xpath('sedml:listOfVariables/sedml:variable'):
                    xpath = getattr(variable_decl, u'target', u'')
                    if xpath:
                        used_var_xpaths.add(xpath)
        # Set up our protocol ready for code generation
        for var_xpath in used_var_xpaths:
            var = self.get_var_elt(self.cellml_doc, var_xpath)
            p.outputs.add(var)
            p.add_alias(var, var_xpath)
        p.modify_model()
        # Reflect the actual protocol used in the generated class name
        self.model.get_config().options.protocol = '_'.join([
            'SEDML', os.path.splitext(os.path.basename(self.sedml_path))[0].replace('-', '_'), self.requested_model])
    
    def get_var_elt(self, doc, xpath):
        var = doc.xml_xpath(xpath)
        if len(var) != 1:
            raise ValueError("Unable to find variable matching " + xpath)
        var = var[0]
        if not isinstance(var, cellml_variable):
            raise ValueError("Variable reference %s doesn't select a cellml:variable element" % xpath)
        return var

    def apply_changes(self, cellml_doc, model_defn):
        """Apply to cellml_doc the changes defined in the SED-ML model_defn."""
        if hasattr(model_defn, u'listOfChanges'):
            for change in model_defn.listOfChanges.xml_element_children():
#                print "Applying change id", getattr(change, u'id', '<>'), "type", change.localName
                assert hasattr(change, u'target')
                target = cellml_doc.xml_xpath(change.target)
                if len(target) != 1:
                    raise ValueError("Unable to find change target " + change.target)
                target = target[0]
                if change.localName == u'addXml':
                    assert hasattr(change, u'newXml')
                    for new_child in change.newXml.xml_children[:]:
                        change.newXml.safe_remove_child(new_child)
                        target.xml_append_child(new_child)
                elif change.localName == u'changeXml':
                    assert hasattr(change, u'newXml')
                    for new_xml in change.newXml.xml_children[:]:
                        change.newXml.safe_remove_child(new_xml)
                        target.xml_parent.xml_insert_before(target, new_xml)
                        target.xml_parent.safe_remove_child(target)
                elif change.localName == u'removeXml':
                    target.xml_parent.safe_remove_child(target)
                elif change.localName == u'changeAttribute':
                    assert hasattr(change, u'newValue')
                    assert isinstance(target, amara.binderyxpath.xpath_attr_wrapper)
                    setattr(target.parentNode, target.localName, change.newValue)
                elif change.localName == u'computeChange':
                    assert hasattr(change, u'math')
                    expr = change.math.xml_element_children().next()
                    if hasattr(change, u'listOfParameters'):
                        for param in change.listOfParameters.parameter:
                            self.set_variable(expr, param.id, param.value)
                    if hasattr(change, u'listOfVariables'):
                        for var in change.listOfVariables.variable:
                            assert hasattr(var, u'target')
                            assert hasattr(var, u'modelReference')
#                            print "V", var.id, var.target, var.modelReference
                            value = self.get_variable(var.modelReference, var.target)
                            self.set_variable(expr, var.id, value)
                    new_value = unicode(expr.evaluate())
                    if isinstance(target, amara.binderyxpath.xpath_attr_wrapper):
                        setattr(target.parentNode, target.localName, new_value)
                    elif isinstance(target, amara.binderyxpath.text_wrapper):
                        parent = target.parentNode
                        parent.xml_remove_child(unicode(target))
                        parent.xml_append(new_value)
                    elif isinstance(target, amara.bindery.element_base):
                        target.xml_remove_child(unicode(target))
                        target.xml_append(new_value)
                    else:
                        raise ValueError("Unsupported target type: ", target)
                else:
                    raise ValueError("Unrecognised change type: " + change.localName)

    def get_variable(self, model_id, var_xpath):
        """Get the value of the given variable in the given model."""
        if model_id not in self.ready_models:
            if model_id in self.cellml_docs:
                raise ValueError("Model loop in computeChange definitions.")
            self.prepare_model(model_id)
        var = self.cellml_docs[model_id].xml_xpath(var_xpath)
        if len(var) != 1:
            raise ValueError("Unable to find variable target " + var_xpath + " in model " + model_id)
        var = var[0]
        if isinstance(var, cellml_variable):
            value = unicode(var.get_value())
        else:
            value = unicode(var)
        return value
    
    def set_variable(self, expr, var_name, value):
        """Replace ci references to var_name within expr with cn elements containing value."""
        if isinstance(expr, mathml_ci):
            cn = mathml_cn.create_new(expr, value, u'dimensionless')
            expr.xml_parent.replace_child(expr, cn)
        else:
            for child in expr.xml_element_children():
                self.set_variable(child, var_name, value)

def apply_protocol(doc):
    """Do code generation required for a particular model in a SED-ML protocol."""
    parser = SedmlParser(doc)
    parser.apply_protocol()
