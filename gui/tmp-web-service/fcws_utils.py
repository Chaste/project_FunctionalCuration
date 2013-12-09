"""
Utility module for Functional Curation web services.

It handles unpacking COMBINE archives containing CellML models or Functional Curation protocols,
and determining which file within is the primary model/protocol.

It also contains configuration settings (e.g. the path to Chaste) and a method for determining
whether a model and protocol are compatible.
"""

import os
import re
import sys
import xml.etree.ElementTree as ET
import zipfile

CHASTE_ROOT = '/home/tom/eclipse/workspace/Chaste'
FC_ROOT = os.path.join(CHASTE_ROOT, 'projects', 'FunctionalCuration')

EXPECTED_EXTENSIONS = {'model': ['.cellml'],
                       'proto': ['.txt', '.xml']}

MANIFEST = 'manifest.xml'

sys.path[0:0] = [os.path.join(FC_ROOT, 'src/proto/parsing'),
                 os.path.join(FC_ROOT, 'src/python'),
                 os.path.join(CHASTE_ROOT, 'python/pycml')]
import CompactSyntaxParser as CSP
import cellml_metadata
import pycml


def UnpackArchive(archivePath, tempPath, contentType):
    """Unpack a COMBINE archive, and return the path to the primary unpacked file.
    
    :param archivePath:  path to the archive
    :param tempPath:  path to a temporary folder under which to unpack
    :param contentType:  whether the archive contains a model ('model') or protocol ('proto')
    
    Files will be unpacked into the path tempPath/contentType.
    """
    assert contentType in ['model', 'proto']
    archive = zipfile.ZipFile(archivePath)
    output_path = os.path.join(tempPath, contentType)
    archive.extractall(output_path)
    # Check if the archive manifest specifies the primary file
    primary_file = None
    manifest_path = os.path.join(output_path, MANIFEST)
    if os.path.exists(manifest_path):
        manifest = ET.parse(manifest_path)
        for item in manifest.iter('content'):
            if item.get('master', 'false'):
                primary_file = item.get('location')
                break
    if not primary_file:
        # No manifest or no master listed, so try to figure it out ourselves: find the first item with expected extension
        for item in archive.infolist():
            if item.filename != MANIFEST and os.path.splitext(item.filename)[1] in EXPECTED_EXTENSIONS[contentType]:
                primary_file = item.filename
                break
    if not primary_file:
        raise ValueError('No suitable primary file detected in COMBINE archive')
    primary_path = os.path.join(output_path, primary_file)
    if not os.path.exists(primary_path):
        raise ValueError('Declared primary file not present in archive')
    return primary_path

def GetProtoInterface(protoPath):
    """Get the set of ontology terms used by the given protocol, recursively processing imports."""
    nested_proto = CSP.MakeKw('nests') + CSP.MakeKw('protocol') - CSP.CompactSyntaxParser.quotedUri
    ns_maps = {}
    terms = set()
    def AddTerm(qname):
        prefix, name = qname.split(':')
        nsuri = ns_maps[prefix]
        terms.add(nsuri + name)
    def ProcessInput(res):
        qname = res[0].tokens['name']
        if not 'units' in res[0].tokens or not 'initial_value' in res[0].tokens:
            # Input is not optional, so record as part of the interface
            AddTerm(qname)
    def ProcessOutput(res):
        AddTerm(res[0].tokens['name'])
    def ProcessNsDecl(res):
        ns_maps[res[0]['prefix']] = res[0]['uri']
    def ProcessImport(source_uri):
        # Relative URIs must be resolved relative to this protocol file, or the library folder if not found
        base = os.path.dirname(protoPath)
        source = source_uri
        if not os.path.isabs(source):
            source = os.path.join(base, source)
        if not os.path.exists(source):
            # Try resolving relative to the library folder
            library = os.path.join(FC_ROOT, 'src', 'proto', 'library')
            source = os.path.join(library, source_uri)
        terms.update(GetProtoInterface(source))
    grammars = {CSP.CompactSyntaxParser.nsDecl: ProcessNsDecl,
                CSP.CompactSyntaxParser.importStmt: (lambda res: ProcessImport(res[0][1])),
                CSP.CompactSyntaxParser.inputVariable: ProcessInput,
                CSP.CompactSyntaxParser.outputVariable: ProcessOutput,
                nested_proto: (lambda res: ProcessImport(res[0]))}
    for line in open(protoPath, 'rU'):
        for grammar, processor in grammars.items():
            try:
                match = grammar.parseString(line)
            except CSP.p.ParseException:
                continue
            processor(match)
            break
    return terms

def DetermineCompatibility(protoPath, modelPath):
    """Determine whether the given protocol and model are compatible.
    
    This checks whether the ontology terms accessed by the protocol are present in the model.
    It returns a list of terms required but not present, so the pair are compatible if this list is empty.
    
    NB: Only works with textual syntax protocols at present; assumes OK otherwise.
    """
    if not protoPath.endswith('.txt'):
        return []
    proto_terms = GetProtoInterface(protoPath)
    # Get the terms defined by the model
    model_doc = pycml.amara_parse_cellml(modelPath)
    named_uris = cellml_metadata.get_targets(model_doc.model, None, cellml_metadata.create_rdf_node(('bqbiol:is', pycml.NSS['bqbiol'])))
    category_uris = cellml_metadata.get_targets(model_doc.model, None, cellml_metadata.create_rdf_node(('bqbiol:isVersionOf', pycml.NSS['bqbiol'])))
    model_terms = set(str(uri) for uri in named_uris + category_uris) # TODO: Check!
    # Return the mismatch, if any, as a sorted list
    needed_terms = list(proto_terms - model_terms)
    needed_terms.sort()
    return needed_terms
