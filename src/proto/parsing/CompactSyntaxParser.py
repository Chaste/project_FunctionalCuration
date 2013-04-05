
"""Copyright (c) 2005-2012, University of Oxford.
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

from __future__ import division

import os
import sys

pycml_dir = os.path.normpath(os.path.join(__file__, os.path.pardir, os.path.pardir, os.path.pardir,
                                          os.path.pardir, os.path.pardir, os.path.pardir, 'python', 'pycml'))
sys.path[0:0] = [pycml_dir]
import pyparsing as p

__all__ = ['CompactSyntaxParser']

# Necessary for reasonable speed when using operatorPrecedences
p.ParserElement.enablePackrat()

################################################################################
# Parse actions that can generate the XML syntax
################################################################################
import lxml.builder
import lxml.etree as ET

PROTO_NS = "https://chaste.cs.ox.ac.uk/nss/protocol/0.1#"
MATHML_NS = "http://www.w3.org/1998/Math/MathML"
CELLML_NS = "http://www.cellml.org/cellml/1.0#"
PROTO_CSYM_BASE = "https://chaste.cs.ox.ac.uk/nss/protocol/"
P = lxml.builder.ElementMaker(namespace=PROTO_NS)
M = lxml.builder.ElementMaker(namespace=MATHML_NS)
CELLML = lxml.builder.ElementMaker(namespace=CELLML_NS,
                                   nsmap={'cellml': CELLML_NS})

class Actions(object):
    """Container for parse actions."""
    source_file = "" # Should be filled in by main parse method
    units_map = {}   # Will be cleared by main parse method

    class BaseAction(object):
        """Base parse action.
        
        This contains the code for allowing parsed protocol elements to be compared to lists in the test code.
        """
        def __init__(self, s, loc, tokens):
            self.tokens = tokens
            if isinstance(loc, str):
                # This instance is being created manually to implement syntactic sugar.
                self.source_location = loc
            else:
                self.source_location = "%s:%d:%d\t%s" % (Actions.source_file, p.lineno(loc, s), p.col(loc, s), p.line(loc, s))
#            print 'Creating', self.__class__.__name__, self.tokens
        
        def __eq__(self, other):
            """Comparison of these parse results to another instance or a list."""
            if type(other) == type(self):
                return self.tokens == other.tokens
            elif isinstance(other, list):
                return self.tokens == other
            elif isinstance(other, str):
                return str(self.tokens) == other
            else:
                return False
        
        def __len__(self):
            """Get the length of the encapsulated token list."""
            if isinstance(self.tokens, str):
                length = 1
            else:
                length = len(self.tokens)
            return length
        
        def __getitem__(self, i):
            """Get the i'th encapsulated token."""
            assert not isinstance(self.tokens, str)
            return self.tokens[i]
        
        def __str__(self):
            if isinstance(self.tokens, str):
                detail = '[%s]' % self.tokens
            else:
                detail = str(self.tokens)
            return self.__class__.__name__ + detail
        def __repr__(self):
            return str(self)
        
        def GetChildrenXml(self):
            """Convert all sub-tokens to XML and return the list of elements."""
            return map(lambda tok: tok.xml(), self.tokens)
        
        def TransferAttrs(self, *attrNames):
            """Create an attribute dictionary for use in generating XML from named parse results."""
            attrs = {}
            for key in attrNames:
                if key in self.tokens:
                    value = self.tokens[key]
                    if not isinstance(value, str):
                        value = value[0]
                    attrs[key] = value
            return attrs
        
        def Delegate(self, action, tokens):
            """Create another parse action to process the given tokens for us."""
            if isinstance(action, str):
                action = getattr(Actions, action)
            return action('', self.source_location, tokens)
        
        def DelegateSymbol(self, symbol, content=None):
            """Create a csymbol parse action for producing part of our XML output."""
            if content is None:
                content = list()
            return self.Delegate(Actions.Symbol(symbol), [content])
        
        def AddLoc(self, elt):
            """Add our location information to the given element."""
            elt.set('{%s}loc' % PROTO_NS, self.source_location)
            return elt
        
        def xml(self):
            """Main method to generate the XML syntax.
            Will add an attribute containing source location information where appropriate.
            """
            result = self._xml()
            if ET.iselement(result):
                self.AddLoc(result)
            return result
        
        def _xml(self):
            """Subclasses must implement this method to generate their specific XML."""
            raise NotImplementedError
    
    class BaseGroupAction(BaseAction):
        """Base class for parse actions associated with a Group.
        This strips the extra nesting level in its __init__.
        """
        def __init__(self, s, loc, tokens):
            super(Actions.BaseGroupAction, self).__init__(s, loc, tokens[0])
    
    class Trace(BaseGroupAction):
        """This wrapping action turns on tracing of the enclosed expression or nested protocol."""
        def _xml(self):
            wrapped_xml = self.tokens[0].xml()
            wrapped_xml.set('{%s}trace' % PROTO_NS, '1')
            return wrapped_xml
    
    ######################################################################
    # Post-processing language expressions
    ######################################################################
    
    class Number(BaseGroupAction):
        """Parse action for numbers."""
        def __init__(self, s, loc, tokens):
            super(Actions.Number, self).__init__(s, loc, tokens)
            if len(tokens) == 2:
                # We have a units annotation
                self._units = tokens[1]
            else:
                self._units = None

        def _xml(self):
            elt = M.cn(self.tokens)
            if self._units:
                elt.set('{%s}units' % CELLML_NS, str(self._units))
            return elt
    
    class Variable(BaseGroupAction):
        """Parse action for variable references (identifiers)."""
        def _xml(self):
            var_name = self.tokens
            if var_name.startswith('MathML:'):
                result = getattr(M, var_name[7:])
            else:
                result = M.ci(self.tokens)
            return result
    
    class Operator(BaseGroupAction):
        """Parse action for most MathML operators that are represented as operators in the syntax."""
        # Map from operator symbols used to MathML element names
        OP_MAP = {'+': 'plus', '-': 'minus', '*': 'times', '/': 'divide', '^': 'power',
                  '==': 'eq', '!=': 'neq', '<': 'lt', '>': 'gt', '<=': 'leq', '>=': 'geq',
                  'not': 'not', '&&': 'and', '||': 'or'}
        def __init__(self, *args, **kwargs):
            super(Actions.Operator, self).__init__(*args)
            self.rightAssoc = kwargs.get('rightAssoc', False)
        
        def OperatorOperands(self):
            """Generator over (operator, operand) pairs."""
            it = iter(self.tokens[1:])
            while 1:
                try:
                    operator = next(it)
                    operand = next(it)
                    yield (operator, operand)
                except StopIteration:
                    break
        
        def Operator(self, operator):
            """Get the MathML element for the given operator."""
            return getattr(M, self.OP_MAP[operator])
        
        def _xml(self):
            if self.rightAssoc:
                # The only right-associative operators are also unary
                result = self.tokens[-1].xml()
                for operator in self.tokens[-2:-1:]:
                    result = M.apply(self.Operator(operator), result)
            else:
                result = self.tokens[0].xml()
                for operator, operand in self.OperatorOperands():
                    result = M.apply(self.Operator(operator), result, operand.xml())
            return result
    
    class Wrap(BaseGroupAction):
        """Parse action for wrapped MathML operators."""
        def _xml(self):
            assert len(self.tokens) == 2
            num_operands = self.tokens[0]
            operator = self.tokens[1]
            if operator.startswith('MathML:'):
                operator = operator[7:]
            else:
                operator = Actions.Operator.OP_MAP[operator]
            return self.DelegateSymbol('wrap/' + num_operands, operator).xml()
    
    class Piecewise(BaseGroupAction):
        """Parse action for if-then-else."""
        def _xml(self):
            if_, then_, else_ = self.GetChildrenXml()
            return M.piecewise(M.piece(then_, if_), M.otherwise(else_))
    
    class MaybeTuple(BaseGroupAction):
        """Parse action for elements that may be grouped into a tuple, or might be a single item."""
        def _xml(self):
            assert len(self.tokens) > 0
            if len(self.tokens) > 1:
                # Tuple
                return self.Delegate('Tuple', [self.tokens]).xml()
            else:
                # Single item
                return self.tokens[0].xml()
    
    class Tuple(BaseGroupAction):
        """Parse action for tuples."""
        def _xml(self):
            child_xml = self.GetChildrenXml()
            return M.apply(M.csymbol(definitionURL=PROTO_CSYM_BASE+"tuple"), *child_xml)
    
    class Lambda(BaseGroupAction):
        """Parse action for lambda expressions."""
        def _xml(self):
            assert len(self.tokens) == 2
            param_list = self.tokens[0]
            body = self.tokens[1].xml()
            children = []
            for param_decl in param_list:
                param_bvar = M.bvar(param_decl[0].xml())
                if len(param_decl) == 1: # No default given
                    children.append(param_bvar)
                else: # Default value case
                    children.append(M.semantics(param_bvar, getattr(M, 'annotation-xml')(param_decl[1].xml())))
            children.append(body)
            return getattr(M, 'lambda')(*children)
    
    class FunctionCall(BaseGroupAction):
        """Parse action for function calls."""
        def _xml(self):
            assert len(self.tokens) == 2
            func_name = str(self.tokens[0].tokens)
            if func_name in ['map', 'fold', 'find']:
                func = self.DelegateSymbol(func_name).xml()
            else:
                func = self.tokens[0].xml()
            args = map(lambda t: t.xml(), self.tokens[1])
            return M.apply(func, *args)
    
    class _Symbol(BaseGroupAction):
        """Parse action for csymbols."""
        def __init__(self, s, loc, tokens, symbol):
            super(Actions._Symbol, self).__init__(s, loc, tokens)
            self.symbol = symbol
        def _xml(self):
            if isinstance(self.tokens, str):
                return M.csymbol(self.tokens, definitionURL=PROTO_CSYM_BASE+self.symbol)
            else:
                return M.csymbol(definitionURL=PROTO_CSYM_BASE+self.symbol)
    
    @staticmethod
    def Symbol(symbol):
        """Wrapper around the _Symbol class."""
        def parse_action(s, loc, tokens):
            return Actions._Symbol(s, loc, tokens, symbol)
        return parse_action
    
    class Accessor(BaseGroupAction):
        """Parse action for accessors."""
        def _xml(self):
            if len(self.tokens) > 2:
                # Chained accessors, e.g. A.SHAPE.IS_ARRAY
                return self.Delegate('Accessor', [[self.Delegate('Accessor', [self.tokens[:-1]]), self.tokens[-1]]]).xml()
            assert len(self.tokens) == 2
            object = self.tokens[0].xml()
            property = self.tokens[1]
            return M.apply(self.DelegateSymbol('accessor', property).xml(), object)
    
    class Comprehension(BaseGroupAction):
        """Parse action for the comprehensions with array definitions."""
        def _xml(self):
            assert 2 <= len(self.tokens) <= 3
            parts = []
            if len(self.tokens) == 3:
                # There's an explicit dimension
                parts.append(self.tokens[0])
            range = self.tokens[-1]
            if len(range) == 2:
                # Add a step of 1
                range = [range[0], self.Delegate('Number', ['1']), range[-1]]
            parts.extend(range)
            parts.append(self.DelegateSymbol('string', self.tokens[-2])) # The variable name
            return self.Delegate('Tuple', [parts]).xml()
    
    class Array(BaseGroupAction):
        """Parse action for creating arrays."""
        def _xml(self):
            entries = self.GetChildrenXml()
            if len(entries) > 1 and isinstance(self.tokens[1], Actions.Comprehension):
                # Array comprehension
                entries = [M.domainofapplication(*entries[1:]), entries[0]]
            return M.apply(M.csymbol(definitionURL=PROTO_CSYM_BASE+'newArray'), *entries)
    
    class View(BaseGroupAction):
        """Parse action for array views."""
        def _xml(self):
            assert 2 <= len(self.tokens)
            apply_content = [self.DelegateSymbol('view').xml(), self.tokens[0].xml()]
            seen_generic_dim = False
            null_token = self.DelegateSymbol('null')
            next_dimension = 0
            for viewspec in self.tokens[1:]:
                tuple_tokens = []
                if 'dimspec' in viewspec:
                    dimspec = viewspec['dimspec'][0]
                    if dimspec == '*':
                        seen_generic_dim = True
                        dimspec = null_token
                    tuple_tokens.append(dimspec)
                    viewspec = viewspec[1:]
                else:
                    # Since we will provide a generic specification, all tuples need 4 elements for the XML parser :(
                    # This is very fragile!
                    tuple_tokens.append(self.Delegate('Number', [str(next_dimension)]))
                    next_dimension += 1
                if len(viewspec) == 1:
                    # Take single value (in this dimension)
                    tuple_tokens.extend([viewspec[0], self.Delegate('Number', ['0']), viewspec[0]])
                elif len(viewspec) == 2:
                    # Range with step 1
                    tuple_tokens.extend([viewspec[0], self.Delegate('Number', ['1']), viewspec[1]])
                else:
                    # Fully specified range
                    tuple_tokens.extend(viewspec)
                # Replace unspecified elements with csymbol-null
                for i, token in enumerate(tuple_tokens):
                    if token == '':
                        tuple_tokens[i] = null_token
                apply_content.append(self.Delegate('Tuple', [tuple_tokens]).xml())
            if not seen_generic_dim:
                # Add a 'take everything from other dimensions' view specification
                tuple_tokens = [null_token, null_token, self.Delegate('Number', ['1']), null_token]
                apply_content.append(self.Delegate('Tuple', [tuple_tokens]).xml())
            return M.apply(*apply_content)
    
    class Index(BaseGroupAction):
        """Parse action for index expressions."""
        def _xml(self):
            """Construct apply(csymbol-index, indexee, indices, dim, shrink, pad, padValue).
            shrink and pad both default to 0 (false).
            """
            assert len(self.tokens) == 2
            index_tokens = self.tokens[1]
            assert 1 <= len(index_tokens) <= 3
            apply_content = [self.DelegateSymbol('index'), self.tokens[0], index_tokens[0]]
            if len(index_tokens) == 2:
                # We're shrinking
                apply_content.append(index_tokens[1]) # Dimension to shrink along
                apply_content.append(self.Delegate('Number', ['1'])) # shrink=true
            elif len(index_tokens) == 3:
                # We're padding
                apply_content.append(index_tokens[1]) # Dimension to shrink along
                apply_content.append(self.DelegateSymbol('defaultParameter')) # shrink=default (false)
                apply_content.append(self.Delegate('Number', ['1'])) # pad=true
                apply_content.append(index_tokens[2]) # Pad value
            return M.apply(*map(lambda t: t.xml(), apply_content))
    
    ######################################################################
    # Post-processing language statements
    ######################################################################
    
    class Assignment(BaseGroupAction):
        """Parse action for both simple and tuple assignments."""
        def _xml(self):
            assignee, value = self.GetChildrenXml()
            return M.apply(M.eq, assignee, value)
    
    class Return(BaseGroupAction):
        """Parse action for return statements."""
        def _xml(self):
            return M.apply(self.DelegateSymbol('return').xml(), *self.GetChildrenXml())
    
    class Assert(BaseGroupAction):
        """Parse action for assert statements."""
        def _xml(self):
            return M.apply(self.DelegateSymbol('assert').xml(), *self.GetChildrenXml())
    
    class FunctionDef(BaseGroupAction):
        """Parse action for function definitions, which are sugar for assignment of a lambda."""
        def _xml(self):
            assert len(self.tokens) == 3
            lambda_ = self.Delegate('Lambda', [self.tokens[1:]])
            assign = self.Delegate('Assignment', [[self.tokens[0], lambda_]])
            return assign.xml()

    class StatementList(BaseGroupAction):
        """Parse action for lists of post-processing language statements."""
        def _xml(self):
            statements = self.GetChildrenXml()
            return M.apply(self.DelegateSymbol('statementList').xml(), *statements)
    
    ######################################################################
    # Model interface section
    ######################################################################
    
    class SetTimeUnits(BaseAction):
        def _xml(self):
            return P.setIndependentVariableUnits(**self.TransferAttrs('units'))
    
    class InputVariable(BaseGroupAction):
        def _xml(self):
            return P.specifyInputVariable(**self.TransferAttrs('name', 'units', 'initial_value'))
    
    class OutputVariable(BaseGroupAction):
        def _xml(self):
            return P.specifyOutputVariable(**self.TransferAttrs('name', 'units'))
    
    class DeclareVariable(BaseGroupAction):
        def _xml(self):
            return P.declareNewVariable(**self.TransferAttrs('name', 'units', 'initial_value'))
    
    class ModelEquation(BaseGroupAction):
        def _xml(self):
            assert len(self.tokens) == 2
            if isinstance(self.tokens[0], Actions.Variable):
                lhs = self.tokens[0].xml()
            else:
                # Assigning an ODE
                assert len(self.tokens[0]) == 2
                bvar = M.bvar(self.tokens[0][1].xml())
                lhs = self.AddLoc(M.apply(M.diff, bvar, self.tokens[0][0].xml()))
            rhs = self.tokens[1].xml()
            return P.addOrReplaceEquation(self.AddLoc(M.apply(M.eq, lhs, rhs)))
    
    class UnitsConversion(BaseGroupAction):
        def _xml(self):
            attrs = self.TransferAttrs('desiredDimensions', 'actualDimensions')
            rule = self.tokens[-1].xml()
            return P.unitsConversionRule(rule, **attrs)
    
    class ModelInterface(BaseGroupAction):
        def _xml(self):
            if len(self.tokens) > 0:
                return P.modelInterface(*self.GetChildrenXml())
    
    ######################################################################
    # Simulation tasks section
    ######################################################################
    
    class Range(BaseGroupAction):
        """Parse action for all the kinds of range supported."""
        def _xml(self):
            attrs = self.TransferAttrs('name', 'units')
            if 'uniform' in self.tokens:
                tokens = self.tokens['uniform'][0]
                start = P.start(tokens[0].xml())
                stop = P.stop(tokens[-1].xml())
                if len(tokens) == 3:
                    step = P.step(tokens[1].xml())
                else:
                    step = P.step(self.Delegate('Number', '1').xml())
                range = P.uniformStepper(start, stop, step, **attrs)
            elif 'vector' in self.tokens:
                range = P.vectorStepper(self.tokens['vector'][0].xml(), **attrs)
            elif 'while' in self.tokens:
                cond = self.AddLoc(P.condition(self.tokens['while'][0].xml()))
                range = P.whileStepper(cond, **attrs)
            return range
    
    class ModifierWhen(BaseGroupAction):
        """Parse action for the when part of modifiers."""
        def _xml(self):
            when = {'start': 'AT_START_ONLY', 'each': 'EVERY_LOOP', 'end': 'AT_END'}[self.tokens]
            return P.when(when)
    
    class Modifier(BaseGroupAction):
        """Parse action that generates all kinds of modifier."""
        def _xml(self):
            args = [self.tokens[0].xml()]
            detail = self.tokens[1]
            if 'set' in self.tokens[1]:
                modifier = P.setVariable
                args.append(P.name(detail[0]))
                args.append(P.value(detail[1].xml()))
            elif 'save' in self.tokens[1]:
                modifier = P.saveState
                args.append(P.name(detail[0]))
            elif 'reset' in self.tokens[1]:
                modifier = P.resetState
                if len(detail) > 0:
                    args.append(P.state(detail[0]))
            return modifier(*args)
    
    class Modifiers(BaseGroupAction):
        """Parse action for the modifiers collection."""
        def _xml(self):
            return P.modifiers(*self.GetChildrenXml())
    
    class TimecourseSimulation(BaseGroupAction):
        def _xml(self):
            args = self.GetChildrenXml()
            if len(args) == 1:
                # Add an empty modifiers element
                args.append(self.Delegate('Modifiers', [[]]).xml())
            return P.timecourseSimulation(*args)
        
    class NestedSimulation(BaseGroupAction):
        def _xml(self):
            args = map(lambda t: t.xml(), self.tokens[0:-1])
            if len(args) == 1:
                # Add an empty modifiers element
                args.append(self.Delegate('Modifiers', [[]]).xml())
            nested = self.tokens[-1][0]
            if isinstance(nested, (Actions.Simulation, Actions.NestedProtocol)):
                # Inline definition
                args.append(nested.xml())
            else:
                # Reference to named task
                nested = P.subTask(task=str(nested))
                args.append(self.AddLoc(nested))
            return P.nestedSimulation(*args)
    
    class OneStepSimulation(BaseGroupAction):
        def _xml(self):
            attrs = {}
            args = []
            if 'step' in self.tokens:
                attrs['step'] = str(self.tokens['step'][0])
            if 'modifiers' in self.tokens:
                args.append(self.tokens['modifiers'][0].xml())
            return P.oneStep(*args, **attrs)
    
    class NestedProtocol(BaseGroupAction):
        def __init__(self, s, loc, tokens):
            self.trace = (tokens[0][-1] == '?')
            if self.trace:
                tokens[0] = tokens[0][:-1]
            super(Actions.NestedProtocol, self).__init__(s, loc, tokens)

        def _xml(self):
            attrs = {'source': str(self.tokens[0])}
            args = []
            # TODO: consider doing setInput more like the inputs section (requires change to XML syntax)
            assert isinstance(self.tokens[1], Actions.StatementList)
            for assignment in self.tokens[1]:
                input_name = assignment.tokens[0].tokens
                input_value = assignment.tokens[1].xml()
                args.append(self.AddLoc(P.setInput(input_value, name=input_name)))
            for output in self.tokens[2:]:
                args.append(self.AddLoc(P.selectOutput(name=output)))
            result = P.nestedProtocol(*args, **attrs)
            if self.trace:
                result.set('{%s}trace' % PROTO_NS, '1')
            return result
    
    class Simulation(BaseGroupAction):
        """Parse action for all kinds of simulation."""
        def _xml(self):
            sim_elt = self.tokens[1].xml()
            prefix = str(self.tokens[0])
            if prefix:
                sim_elt.set('prefix', prefix)
            return sim_elt
    
    class Tasks(BaseGroupAction):
        """Parse action for a collection of simulation tasks."""
        def _xml(self):
            if len(self.tokens) > 0:
                return P.simulations(*self.GetChildrenXml())
    
    ######################################################################
    # Other protocol language constructs
    ######################################################################
    
    class Inputs(BaseAction):
        """Parse action for the inputs section of a protocol."""
        def _xml(self):
            assert len(self.tokens) <= 1
            if len(self.tokens) == 1: # Don't create an empty element
                return P.inputs(self.tokens[0].xml())
    
    class Import(BaseGroupAction):
        """Parse action for protocol imports."""
        def _xml(self):
            assert len(self.tokens) >= 2
            attrs = {'source': self.tokens[1]}
            if self.tokens[0]:
                attrs['prefix'] = self.tokens[0]
            else:
                attrs['mergeDefinitions'] = 'true'
            children = []
            if len(self.tokens) == 3:
                for set_input in self.tokens[2].tokens:
                    children.append(P.setInput(set_input.tokens[1].xml(), name=set_input.tokens[0].tokens))
            return getattr(P, 'import')(*children, **attrs)
    
    class UseImports(BaseGroupAction):
        """Parse action for 'use imports' constructs."""
        def _xml(self):
            return P.useImports(prefix=self.tokens[0])
    
    class UnitRef(BaseGroupAction):
        """Parse action for unit references within units definitions."""
        def GetValue(self, token, negate=False):
            """Get a decent string representation of the value of the given numeric token.
            It may be a plain number, or it may be a simple expression which we have to evaluate.
            """
            format = "%.17g"
            result = str(token).strip()
            try:
                value = float(result)
            except ValueError:
                # Evaluation required; somewhat risky!
                value = eval(result)
                if negate:
                    value = -value
                result = format % value
            else:
                # Just use the string representation in the protocol
                if negate:
                    if result[0] == '-':
                        result = result[1:]
                    else:
                        result = '-' + result
            return result
        
        def _xml(self):
            attrs = self.TransferAttrs('prefix', 'units', 'exponent')
            if 'multiplier' in self.tokens:
                attrs['multiplier'] = self.GetValue(self.tokens['multiplier'][0])
            if 'offset' in self.tokens:
                attrs['offset'] = self.GetValue(self.tokens['offset'][0][1], self.tokens['offset'][0][0] == '-')
            return CELLML.unit(**attrs)
    
    class UnitsDef(BaseGroupAction):
        """Parse action for units definitions."""
        def _xml(self):
            name = str(self.tokens[0])
            if 'description' in self.tokens:
                Actions.units_map[name] = str(self.tokens['description'])
            unit_refs = [t.xml() for t in self.tokens if isinstance(t, Actions.UnitRef)]
            return CELLML.units(*unit_refs, name=name)
    
    class Units(BaseAction):
        """Parse action for the units definitions section."""
        def _xml(self):
            if len(self.tokens) > 0:
                return P.units(*self.GetChildrenXml())
    
    class Library(BaseAction):
        """Parse action for the library section."""
        def _xml(self):
            if len(self.tokens) > 0:
                assert len(self.tokens) == 1
                return P.library(*self.GetChildrenXml())
    
    class PostProcessing(BaseAction):
        """Parse action for the post-processing section."""
        def _xml(self):
            if len(self.tokens) > 0:
                # We need to group non useImports elements into statement lists, but useImports may occur anywhere
                children, statements = [], []
                for token in self.tokens:
                    if isinstance(token, Actions.UseImports):
                        if statements:
                            children.append(self.Delegate('StatementList', [statements]).xml())
                            statements = []
                        children.append(self.Delegate('UseImports', [token]).xml())
                    else:
                        statements.append(token)
                if statements:
                    children.append(self.Delegate('StatementList', [statements]).xml())
                return getattr(P, 'post-processing')(*children)
    
    class Output(BaseGroupAction):
        """Parse action for an output specification."""
        def _xml(self):
            if not 'units' in self.tokens:
                # It has to be a raw model output
                elt = P.raw
            else:
                # It's a post-processed output
                elt = P.postprocessed
                # Check if the units referenced have a description which should be used instead of their name
                units = self.tokens['units']
                if units[0] in Actions.units_map:
                    units[0] = Actions.units_map[units[0]]
            return elt(**self.TransferAttrs('name', 'ref', 'units', 'description'))
    
    class Outputs(BaseGroupAction):
        """Parse action for the plots section."""
        def _xml(self):
            if len(self.tokens) > 0:
                return P.outputVariables(*self.GetChildrenXml())
    
    class Plot(BaseGroupAction):
        """Parse action for simple plot specifications."""
        def _xml(self):
            assert len(self.tokens) == 2, "Only a single plot curve is currently supported in XML"
            curve = self.tokens[1]
            key = curve.get('key', '')
            if key:
                curve = curve[:-1]
            assert len(curve) == 2, "Only a single y variable is currently supported in XML"
            title = str(self.tokens[0])
            y, x = map(str, curve)
            args = [P.title(title), P.x(x), P.y(y)]
            if key:
                args.append(P.key(key))
            return P.plot(*args)
            
    class Plots(BaseGroupAction):
        """Parse action for the plots section."""
        def _xml(self):
            if len(self.tokens) > 0:
                return P.plots(*self.GetChildrenXml())
    
    class Protocol(BaseAction):
        """Parse action for a full protocol."""
        def _xml(self):
            # Build namespace map based on bindings in the protocol
            nsmap = {'proto': PROTO_NS, 'm': MATHML_NS}
            if 'namespace' in self.tokens:
                for prefix, uri in self.tokens['namespace']:
                    nsmap[prefix] = uri
            # Create root element, then add children
            root = ET.Element('{%s}protocol' % PROTO_NS, nsmap=nsmap)
            for token in self.tokens:
                if isinstance(token, Actions.BaseAction):
                    xml = token.xml()
                    if xml is not None:
                        root.append(xml)
            return root
    

################################################################################
# Helper methods for defining parsers
################################################################################
def MakeKw(keyword, suppress=True):
    """Helper function to create a parser for the given keyword."""
    kw = p.Keyword(keyword)
    if suppress:
        kw = kw.suppress()
    return kw

def Adjacent(parser):
    """Create a copy of the given parser that doesn't permit whitespace to occur before it."""
    adj = parser.copy()
    adj.setWhitespaceChars('')
    return adj

class Optional(p.Optional):
    """An Optional pattern that doesn't consume whitespace if the contents don't match."""
    def __init__(self, *args, **kwargs):
        super(Optional, self).__init__(*args, **kwargs)
        self.callPreparse = False
        self._optionalNotMatched = p.Optional(p.Empty()).defaultValue
    
    def parseImpl( self, instring, loc, doActions=True ):
        try:
            loc, tokens = self.expr._parse( instring, loc, doActions, callPreParse=True )
        except (p.ParseException,IndexError):
            if self.defaultValue is not self._optionalNotMatched:
                if self.expr.resultsName:
                    tokens = p.ParseResults([ self.defaultValue ])
                    tokens[self.expr.resultsName] = self.defaultValue
                else:
                    tokens = [ self.defaultValue ]
            else:
                tokens = []
        return loc, tokens

def OptionalDelimitedList(expr, delim):
    """Like delimitedList, but the list may be empty."""
    return p.delimitedList(expr, delim) | p.Empty()

def DelimitedMultiList(elements, delimiter):
    """Like delimitedList, but allows for a sequence of constituent element expressions.
    
    elements should be a sequence of tuples (expr, unbounded), where expr is a ParserElement,
    and unbounded is True iff zero or more occurrences are allowed; otherwise the expr is
    considered to be optional (i.e. 0 or 1 occurrences).  The delimiter parameter must occur
    in between each matched token, and is suppressed from the output.
    """
    if len(elements) == 0:
        return p.Empty()
    # If we have an optional expr, we need (expr + delimiter + rest) | expr | rest
    # If we have an unbounded expr, we need (expr + delimiter + this) | expr | rest, i.e. allow expr to recur
    expr, unbounded = elements[0]
    if not isinstance(delimiter, p.Suppress):
        delimiter = p.Suppress(delimiter)
    rest = DelimitedMultiList(elements[1:], delimiter)
    if unbounded:
        result = p.Forward()
        result << ((expr + delimiter + result) | expr | rest)
    else:
        if isinstance(rest, p.Empty):
            result = expr | rest
        else:
            result = (expr + delimiter + rest) | expr | rest
    return result

def UnIgnore(parser):
    """Stop ignoring things in the given parser (and its children)."""
    for child in getattr(parser, 'exprs', []):
        UnIgnore(child)
    if hasattr(parser, 'expr'):
        UnIgnore(parser.expr)
    parser.ignoreExprs = []

def MonkeyPatch():
    """Monkey-patch some pyparsing methods to behave slightly differently."""
    def ignore( self, other ):
        """Improved ignore that avoids ignoring self by accident."""
        if isinstance( other, p.Suppress ):
            if other not in self.ignoreExprs and other != self:
                self.ignoreExprs.append( other.copy() )
        else:
            self.ignoreExprs.append( p.Suppress( other.copy() ) )
        return self

    import new
    setattr(p.ParserElement, 'ignore', new.instancemethod(locals()['ignore'], None, p.ParserElement))

MonkeyPatch()


class CompactSyntaxParser(object):
    """A parser that converts a compact textual syntax for protocols into XML."""
    # Newlines are significant most of the time for us
    p.ParserElement.setDefaultWhitespaceChars(' \t\r')
    # Single-line Python-style comments
    comment = p.Regex(r'#.*').suppress().setName('Comment')

    # Punctuation etc.
    eq = p.Suppress('=')
    colon = p.Suppress(':')
    comma = p.Suppress(',')
    oparen = p.Suppress('(')
    cparen = p.Suppress(')')
    osquare = p.Suppress('[')
    csquare = p.Suppress(']')
    dollar = p.Suppress('$')
    nl = p.Suppress(p.OneOrMore(Optional(comment) + p.LineEnd())).setName('Newline(s)') # Any line can end with a comment
    obrace = (Optional(nl) + p.Suppress('{') + Optional(nl)).setName('{')
    cbrace = (Optional(nl) + p.Suppress('}') + Optional(nl)).setName('}')
    embedded_cbrace = (Optional(nl) + p.Suppress('}')).setName('}')
    
    # Identifiers
    ncIdent = p.Regex('[_a-zA-Z][_0-9a-zA-Z]*').setName('ncIdent')
    ident = p.Regex('[_a-zA-Z][_0-9a-zA-Z]*(:[_a-zA-Z][_0-9a-zA-Z]*)*').setName('Ident')
    ncIdentAsVar = ncIdent.copy().setParseAction(Actions.Variable)
    identAsVar = ident.copy().setParseAction(Actions.Variable)
    
    # Numbers can be given in scientific notation, with an optional leading minus sign.
    # They may optionally have units specified.
    unitsAnnotation = p.Suppress('::') - ncIdent("units")
    number = (p.Regex(r'-?[0-9]+((\.[0-9]+)?(e[-+]?[0-9]+)?)?') + Optional(unitsAnnotation)).setName('Number')

    # Used for descriptive text
    quotedString = (p.QuotedString('"', escChar="\\") | p.QuotedString("'", escChar="\\")).setName('QuotedString')
    # This may become more specific in future
    quotedUri = quotedString.copy().setName('QuotedUri')
    
    # Expressions from the "post-processing" language
    #################################################
    
    # Expressions and statements must be constructed recursively
    expr = p.Forward().setName('Expression')
    stmtList = p.Forward().setName('StatementList')
    
    # A vector written like 1:2:5 or 1:5 or A:B:C
    numericRange = p.Group(expr + colon + expr + Optional(colon + expr))

    # Creating arrays
    dimSpec = Optional(expr + Adjacent(dollar)) + ncIdent
    comprehension = p.Group(MakeKw('for') - dimSpec + MakeKw('in') - numericRange).setParseAction(Actions.Comprehension)
    array = p.Group(osquare - expr + (p.OneOrMore(comprehension) | p.ZeroOrMore(comma - expr)) + csquare
                    ).setName('Array').setParseAction(Actions.Array)
    
    # Array views
    optExpr = Optional(expr, default='')
    viewSpec = p.Group(Adjacent(osquare) - Optional(('*' | expr) + Adjacent(dollar))('dimspec') +
                       optExpr + Optional(colon - optExpr + Optional(colon - optExpr)) + csquare).setName('ViewSpec')
    
    # If-then-else
    ifExpr = p.Group(MakeKw('if') - expr + MakeKw('then') - expr +
                     MakeKw('else') - expr).setName('IfThenElse').setParseAction(Actions.Piecewise)
    
    # Lambda definitions
    paramDecl = p.Group(ncIdentAsVar + Optional(eq + expr)) # TODO: check we can write XML for a full expr as default value
    paramList = p.Group(OptionalDelimitedList(paramDecl, comma))
    lambdaExpr = p.Group(MakeKw('lambda') - paramList + ((colon - expr) | (obrace - stmtList + embedded_cbrace))
                         ).setName('Lambda').setParseAction(Actions.Lambda)
    
    # Function calls
    # TODO: allow lambdas, not just ident?
    argList = p.Group(OptionalDelimitedList(expr, comma))
    functionCall = p.Group(identAsVar + Adjacent(oparen) - argList + cparen).setName('FnCall').setParseAction(Actions.FunctionCall)
    
    # Tuples
    tuple = p.Group(oparen + expr + comma - OptionalDelimitedList(expr, comma) + cparen).setName('Tuple').setParseAction(Actions.Tuple)
    
    # Accessors
    accessor = p.Combine(Adjacent(p.Suppress('.')) -
                         p.oneOf('IS_SIMPLE_VALUE IS_ARRAY IS_STRING IS_TUPLE IS_FUNCTION IS_NULL IS_DEFAULT '
                                 'NUM_DIMS NUM_ELEMENTS SHAPE')).setName('Accessor')

    # Indexing
    pad = MakeKw('pad') + Adjacent(colon) - expr + eq + expr
    shrink = MakeKw('shrink') + Adjacent(colon) - expr
    index = p.Group(Adjacent(p.Suppress('{')) - expr + Optional(comma + (pad|shrink)) + p.Suppress('}')).setName('Index')

    # Special values
    nullValue = p.Group(MakeKw('null')).setName('Null').setParseAction(Actions.Symbol('null'))
    defaultValue = p.Group(MakeKw('default')).setName('Default').setParseAction(Actions.Symbol('defaultParameter'))
    
    # Recognised MathML operators
    mathmlOperators = set('''quotient rem max min root xor abs floor ceiling exp ln log
                             sin cos tan   sec csc cot   sinh cosh tanh   sech csch coth
                             arcsin arccos arctan   arccosh arccot arccoth
                             arccsc arccsch arcsec   arcsech arcsinh arctanh'''.split())

    # Wrapping MathML operators into lambdas
    mathmlOperator = (p.oneOf('^ * / + - not == != <= >= < > && ||') |
                      p.Combine('MathML:' + p.oneOf(' '.join(mathmlOperators))))
    wrap = p.Group(p.Suppress('@') - Adjacent(p.Word(p.nums)) + Adjacent(colon) + mathmlOperator
                   ).setName('WrapMathML').setParseAction(Actions.Wrap)
    
    # Turning on tracing for debugging protocols
    trace = Adjacent(p.Suppress('?'))
    
    # The main expression grammar.  Atoms are ordered according to rough speed of detecting mis-match.
    atom = (array | wrap | number.copy().setParseAction(Actions.Number) |
            ifExpr | nullValue | defaultValue | lambdaExpr | functionCall | identAsVar | tuple).setName('Atom')
    expr << p.operatorPrecedence(atom, [(trace, 1, p.opAssoc.LEFT, Actions.Trace),
                                        (accessor, 1, p.opAssoc.LEFT, Actions.Accessor),
                                        (viewSpec, 1, p.opAssoc.LEFT, Actions.View),
                                        (index, 1, p.opAssoc.LEFT, Actions.Index),
                                        ('^', 2, p.opAssoc.LEFT, Actions.Operator),
                                        ('-', 1, p.opAssoc.RIGHT, lambda *args: Actions.Operator(*args, rightAssoc=True)),
                                        (p.oneOf('* /'), 2, p.opAssoc.LEFT, Actions.Operator),
                                        (p.oneOf('+ -'), 2, p.opAssoc.LEFT, Actions.Operator),
                                        ('not', 1, p.opAssoc.RIGHT, lambda *args: Actions.Operator(*args, rightAssoc=True)),
                                        (p.oneOf('== != <= >= < >'), 2, p.opAssoc.LEFT, Actions.Operator),
                                        (p.oneOf('&& ||'), 2, p.opAssoc.LEFT, Actions.Operator)
                                       ])

    # Newlines in expressions may be escaped with a backslash
    expr.ignore('\\' + p.LineEnd())
    # Bare newlines are OK provided we started with a bracket.
    # However, it's quite hard to enforce that restriction.
    expr.ignore(p.Literal('\n'))
    # Embedded comments are also OK
    expr.ignore(comment)
    # Avoid mayhem
    UnIgnore(nl)
    
    # Statements from the "post-processing" language
    ################################################
    
    # Simple assignment (i.e. not to a tuple)
    simpleAssign = p.Group(ncIdentAsVar + eq + expr).setParseAction(Actions.Assignment)
    simpleAssignList = p.Group(OptionalDelimitedList(simpleAssign, nl)).setParseAction(Actions.StatementList)
    
    # Assertions and function returns
    assertStmt = p.Group(MakeKw('assert') - expr).setName('AssertStmt').setParseAction(Actions.Assert)
    returnStmt = p.Group(MakeKw('return') - p.delimitedList(expr)).setName('ReturnStmt').setParseAction(Actions.Return)
    
    # Full assignment, to a tuple of names or single name
    assignStmt = p.Group(p.Group(p.delimitedList(ncIdentAsVar)).setParseAction(Actions.MaybeTuple) + eq +
                         p.Group(p.delimitedList(expr)).setParseAction(Actions.MaybeTuple))   \
                 .setName('AssignStmt').setParseAction(Actions.Assignment)
    
    # Function definition
    functionDefn = p.Group(MakeKw('def') - ncIdentAsVar + oparen + paramList + cparen +
                           ((colon + expr) | (obrace + stmtList + Optional(nl) + p.Suppress('}')))
                           ).setName('FunctionDef').setParseAction(Actions.FunctionDef)
    
    stmtList << p.Group(p.delimitedList(assertStmt | returnStmt | functionDefn | assignStmt, nl))
    stmtList.setParseAction(Actions.StatementList)

    # Miscellaneous constructs making up protocols
    ##############################################
    
    # Namespace declarations
    nsDecl = p.Group(MakeKw('namespace') - ncIdent("prefix") + eq + quotedUri("uri")).setName('NamespaceDecl')
    nsDecls = OptionalDelimitedList(nsDecl("namespace*"), nl)
    
    # Protocol input declarations, with default values
    inputs = (MakeKw('inputs') + obrace - simpleAssignList + cbrace).setName('Inputs').setParseAction(Actions.Inputs)

    # Import statements & use-imports
    importStmt = p.Group(MakeKw('import') - Optional(ncIdent + eq, default='') + quotedUri +
                         Optional(obrace - simpleAssignList + embedded_cbrace)).setName('Import').setParseAction(Actions.Import)
    imports = OptionalDelimitedList(importStmt, nl).setName('Imports')
    useImports = p.Group(MakeKw('use') + MakeKw('imports') - ncIdent).setName('UseImports').setParseAction(Actions.UseImports)
    
    # Library, globals defined using post-processing language.
    # Strictly speaking returns aren't allowed, but that gets picked up later.
    library = (MakeKw('library') + obrace - Optional(stmtList) + cbrace).setName('Library').setParseAction(Actions.Library)
    
    # Post-processing
    postProcessing = (MakeKw('post-processing') + obrace - 
                      OptionalDelimitedList(useImports | assertStmt | returnStmt | functionDefn | assignStmt, nl) +
                      cbrace).setName('PostProc').setParseAction(Actions.PostProcessing)
    
    # Units definitions
    siPrefix = p.oneOf('deka hecto kilo mega giga tera peta exa zetta yotta'
                       'deci centi milli micro nano pico femto atto zepto yocto')
    _num_or_expr = p.originalTextFor(number | (oparen + expr + cparen))
    unitRef = p.Group(Optional(_num_or_expr)("multiplier") + Optional(siPrefix)("prefix") + ncIdent("units")
                      + Optional(p.Suppress('^') + number)("exponent")
                      + Optional(p.Group(p.oneOf('- +') + _num_or_expr))("offset")).setParseAction(Actions.UnitRef)
    unitsDef = p.Group(ncIdent + eq + p.delimitedList(unitRef, '.') + Optional(quotedString)("description")
                       ).setName('UnitsDefinition').setParseAction(Actions.UnitsDef)
    units = (MakeKw('units') + obrace - OptionalDelimitedList(useImports | unitsDef, nl) + cbrace
             ).setName('Units').setParseAction(Actions.Units)
    
    # Model interface section
    #########################
    unitsRef = MakeKw('units') + ncIdent
    
    # Setting the units for the independent variable
    setTimeUnits = (MakeKw('independent') + MakeKw('var') - unitsRef("units")).setParseAction(Actions.SetTimeUnits)
    # Input variables, with optional units and initial value
    inputVariable = p.Group(MakeKw('input') - ident("name") + Optional(unitsRef)("units")
                            + Optional(eq + number)("initial_value")).setName('InputVariable').setParseAction(Actions.InputVariable)
    # Model outputs of interest, with optional units
    outputVariable = p.Group(MakeKw('output') - ident("name") + Optional(unitsRef)("units")
                             ).setName('OutputVariable').setParseAction(Actions.OutputVariable)
    # New variables added to the model, with optional initial value
    newVariable = p.Group(MakeKw('var') - ncIdent("name") + unitsRef("units") + Optional(eq + number)("initial_value")
                          ).setName('NewVariable').setParseAction(Actions.DeclareVariable)
    # Adding or replacing equations in the model
    modelEquation = p.Group(MakeKw('define')
                            - (p.Group(MakeKw('diff') + Adjacent(oparen) - identAsVar + p.Suppress(';') + identAsVar + cparen)
                               | identAsVar) + eq + expr).setName('AddOrReplaceEquation').setParseAction(Actions.ModelEquation)
    # Units conversion rules
    unitsConversion = p.Group(MakeKw('convert') - ncIdent("actualDimensions") + MakeKw('to') + ncIdent("desiredDimensions") +
                              MakeKw('by') - lambdaExpr).setName('UnitsConversion').setParseAction(Actions.UnitsConversion)
    
    modelInterface = p.Group(MakeKw('model') + MakeKw('interface') + obrace -
                             DelimitedMultiList([(useImports, True),
                                                 (setTimeUnits, False),
                                                 (inputVariable, True),
                                                 (outputVariable, True),
                                                 (newVariable, True),
                                                 (modelEquation, True),
                                                 (unitsConversion, True)], nl)
                             + cbrace).setName('ModelInterface').setParseAction(Actions.ModelInterface)
    
    # Simulation definitions
    ########################
    
    # Ranges
    uniformRange = MakeKw('uniform') + numericRange
    vectorRange = MakeKw('vector') + expr
    whileRange = MakeKw('while') + expr
    range = p.Group(MakeKw('range') + ncIdent("name") + unitsRef("units")
                    + (uniformRange("uniform") | vectorRange("vector") | whileRange("while"))
                    ).setName('Range').setParseAction(Actions.Range)
    
    # Modifiers
    modifierWhen = MakeKw('at') - (MakeKw('start', False) |
                                   (MakeKw('each', False) + MakeKw('loop')) |
                                   MakeKw('end', False)).setParseAction(Actions.ModifierWhen)
    setVariable = MakeKw('set') - ident + eq + expr
    saveState = MakeKw('save') + MakeKw('as') - ncIdent
    resetState = MakeKw('reset') - Optional(MakeKw('to') + ncIdent)
    modifier = p.Group(modifierWhen + p.Group(setVariable("set") | saveState("save") | resetState("reset"))
                       ).setName('Modifier').setParseAction(Actions.Modifier)
    modifiers = p.Group(MakeKw('modifiers') + obrace - OptionalDelimitedList(modifier, nl) + cbrace
                        ).setName('Modifiers').setParseAction(Actions.Modifiers)
    
    # The simulations themselves
    simulation = p.Forward().setName('Simulation')
    _selectOutput = (MakeKw('select') + MakeKw('output') - ncIdent).setName('SelectOutput')
    nestedProtocol = p.Group(MakeKw('protocol') - quotedUri + obrace +
                             simpleAssignList + Optional(nl) + OptionalDelimitedList(_selectOutput, nl) +
                             cbrace + Optional('?')).setName('NestedProtocol').setParseAction(Actions.NestedProtocol)
    timecourseSim = p.Group(MakeKw('timecourse') + obrace - range + Optional(nl + modifiers) + cbrace
                            ).setName('TimecourseSim').setParseAction(Actions.TimecourseSimulation)
    nestedSim = p.Group(MakeKw('nested') + obrace - range + nl + Optional(modifiers)
                        + p.Group(MakeKw('nests') + (simulation | nestedProtocol | ident))
                        + cbrace).setName('NestedSim').setParseAction(Actions.NestedSimulation)
    oneStepSim = p.Group(MakeKw('oneStep') - Optional(p.originalTextFor(expr))("step")
                         + Optional(obrace - modifiers + cbrace)("modifiers")).setParseAction(Actions.OneStepSimulation)
    simulation << p.Group(MakeKw('simulation') - Optional(ncIdent + eq, default='')
                          + (timecourseSim | nestedSim | oneStepSim)).setParseAction(Actions.Simulation)

    tasks = p.Group(MakeKw('tasks') + obrace - p.ZeroOrMore(simulation) + cbrace).setName('Tasks').setParseAction(Actions.Tasks)

    # Output specifications
    #######################
    
    outputDesc = Optional(quotedString)("description")
    outputSpec = p.Group(ncIdent("name") + ((unitsRef("units") + outputDesc) |
                                    (eq + ident("ref") + Optional(unitsRef)("units") + outputDesc))).setName('Output').setParseAction(Actions.Output)
    outputs = p.Group(MakeKw('outputs') + obrace - OptionalDelimitedList(useImports | outputSpec, nl) + cbrace).setName('Outputs').setParseAction(Actions.Outputs)

    # Plot specifications
    #####################
    
    plotCurve = p.Group(p.delimitedList(ncIdent, ',')
                        + MakeKw('against') - ncIdent
                        + Optional(MakeKw('key') - ncIdent("key"))).setName('Curve')
    plotSpec = p.Group(MakeKw('plot') - quotedString + obrace +
                       plotCurve + p.ZeroOrMore(nl + plotCurve) + cbrace).setName('Plot').setParseAction(Actions.Plot)
    plots = p.Group(MakeKw('plots') + obrace - p.ZeroOrMore(useImports | plotSpec) + cbrace).setName('Plots').setParseAction(Actions.Plots)
    
    # Parsing a full protocol
    #########################
    
    protocol = p.And(map(Optional, [nl, nsDecls + nl, inputs, imports + nl, library, units, modelInterface,
                                    tasks, postProcessing, outputs, plots])).setName('Protocol').setParseAction(Actions.Protocol)
    
    def __init__(self):
        """Initialise the parser."""
        # We just store the original stack limit here, so we can increase
        # it for the lifetime of this object if needed for parsing, on the
        # basis that if one expression needs to, several are likely to.
        self._stack_depth_factor = 1
        self._original_stack_limit = sys.getrecursionlimit()
        
    def __del__(self):
        """Reset the stack limit if it changed."""
        sys.setrecursionlimit(self._original_stack_limit)
    
    def _Try(self, callable, *args, **kwargs):
        """Try calling the given parse command, increasing the stack depth limit if needed."""
        r = None # Result
        while self._stack_depth_factor < 3:
            try:
                r = callable(*args, **kwargs)
            except RuntimeError, msg:
                print >> sys.stderr, "Got RuntimeError:", msg
                self._stack_depth_factor += 0.5
                new_limit = int(self._stack_depth_factor * self._original_stack_limit)
                print >> sys.stderr, "Increasing recursion limit to", new_limit
                sys.setrecursionlimit(new_limit)
            else:
                break # Parsed OK
        if not r:
            raise RuntimeError("Failed to parse expression even with a recursion limit of %d; giving up!"
                               % (int(self._stack_depth_factor * self._original_stack_limit),))
        return r
    
    def ParseFile(self, filename):
        """Main entry point for parsing a complete protocol file; returns an ElementTree."""
        Actions.source_file = filename
        Actions.units_map = {}
        xml_generator = self._Try(self.protocol.parseFile, filename, parseAll=True)[0]
        xml = xml_generator.xml()
        xml.base = filename
        return ET.ElementTree(xml)
    
    def _ConvertSource(self, referringElt, referringProtoPath, outputDir):
        """Possibly convert a protocol referred to by another."""
        source_path = referringElt.attrib['source']
        if source_path.endswith('.txt'):
            # We'll need to convert.  Figure out the full path to the referent.
            if not os.path.isabs(source_path):
                source_path = os.path.join(os.path.dirname(referringProtoPath), source_path)
            new_path = self.ConvertProtocol(source_path, outputDir)
            referringElt.attrib['source'] = new_path
    
    def ConvertProtocol(self, sourcePath, outputDir):
        """Convert a protocol from textual syntax to XML in a temporary file.
        
        Recursively converts imported/nested textual protocols too.
        """
        import tempfile
        xml = self.ParseFile(sourcePath)
        # Find imported/nested textual protocols, and convert them first, updating our references to them
        subst = {'ns': '{%s}' % PROTO_NS}
        for import_elt in xml.iterfind('%(ns)simport' % subst):
            self._ConvertSource(import_elt, sourcePath, outputDir)
        for nested_proto in xml.iterfind('%(ns)ssimulations//%(ns)snestedProtocol' % subst):
            self._ConvertSource(nested_proto, sourcePath, outputDir)
        # Write this protocol to file
        handle, output_path = tempfile.mkstemp(dir=outputDir, text=True, suffix='.xml')
        output_file = os.fdopen(handle, 'w')
        xml.write(output_file, pretty_print=True, xml_declaration=True)
        output_file.close()
        return output_path


################################################################################
# Parser debugging support
################################################################################

def GetNamedGrammars(obj=CompactSyntaxParser):
    """Get a list of all the named grammars in the given object."""
    grammars = []
    for parser in dir(obj):
        parser = getattr(obj, parser)
        if isinstance(parser, p.ParserElement):
            grammars.append(parser)
    return grammars

def EnableDebug(grammars=None):
    """Enable debugging of our (named) grammars."""
    def DisplayLoc(instring, loc):
        return " at loc " + str(loc) + "(%d,%d)" % ( p.lineno(loc,instring), p.col(loc,instring) )
    
    def SuccessDebugAction( instring, startloc, endloc, expr, toks ):
        print ("Matched " + str(expr) + " -> " + str(toks.asList()) + DisplayLoc(instring, endloc))
    
    def ExceptionDebugAction( instring, loc, expr, exc ):
        print ("Exception raised:" + str(exc) + DisplayLoc(instring, loc))

    for parser in grammars or GetNamedGrammars():
        parser.setDebugActions(None, SuccessDebugAction, ExceptionDebugAction)

def DisableDebug(grammars=None):
    """Stop debugging our (named) grammars."""
    for parser in grammars or GetNamedGrammars():
        parser.setDebug(False)

class Debug(object):
    """A Python 2.6+ context manager that enables debugging just for the enclosed block."""
    def __init__(self, grammars=None):
        self._grammars = list(grammars or GetNamedGrammars())   
    def __enter__(self):
        EnableDebug(self._grammars)
    def __exit__(self, type, value, traceback):
        DisableDebug(self._grammars)

################################################################################
# Using this as a script from C++
################################################################################

if __name__ == '__main__':
    assert len(sys.argv) >= 3
    source_path = sys.argv[1]
    output_dir = sys.argv[2]
    try:
        parser = CompactSyntaxParser()
        output_path = parser.ConvertProtocol(source_path, output_dir)
        print output_path
    except:
        if len(sys.argv) == 3:
            raise
        # Otherwise we swallow the error
