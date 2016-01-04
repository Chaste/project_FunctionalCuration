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

#ifndef MATHMLALL_HPP_
#define MATHMLALL_HPP_

/**
 * @file
 * Convenience header including all the MathML operations.
 *
 * It also defines macros useful for converting a run-time MathML operator
 * name into the appropriate type.
 */

#include "MathmlPlus.hpp"
#include "MathmlMinus.hpp"
#include "MathmlTimes.hpp"
#include "MathmlDivide.hpp"

#include "MathmlRelations.hpp"
#include "MathmlLogic.hpp"
#include "MathmlArithmetic.hpp"
#include "MathmlElementary.hpp"

/**
 * The first entry in a MathML operator table.  If the search operator
 * name matches this entry, "calls" the given macro with the right class.
 *
 * @param macro  the macro to call for a matching item
 * @param name  the operator name for this case
 * @param typesuffix  the suffix of the class name for this case
 * @param search  the operator name to look for
 */
#define MATHML_OPERATOR_TABLE_HEAD(macro, name, typesuffix, search) \
    if (search == #name) {                                          \
        macro(Mathml ## typesuffix)                                 \
    }

/**
 * A non-first entry in a MathML operator table.  Just does an else
 * MATHML_OPERATOR_TABLE_HEAD.
 *
 * @param macro  the macro to call for a matching item
 * @param name  the operator name for this case
 * @param typesuffix  the suffix of the class name for this case
 * @param search  the operator name to look for
 */
#define MATHML_OPERATOR_TABLE_ITEM(macro, name, typesuffix, search) \
    else MATHML_OPERATOR_TABLE_HEAD(macro, name, typesuffix, search)

/**
 * Instantiate this macro to produce code switching on MathML operator name and
 * executing a given macro with the class corresponding to the given name.
 *
 * @param macro  the macro to call, takes one argument - the operator class
 * @param opname  the operator name to look for
 */
#define MATHML_OPERATOR_TABLE(macro, opname)                      \
    MATHML_OPERATOR_TABLE_HEAD(macro, plus, Plus, opname)         \
    MATHML_OPERATOR_TABLE_ITEM(macro, minus, Minus, opname)       \
    MATHML_OPERATOR_TABLE_ITEM(macro, times, Times, opname)       \
    MATHML_OPERATOR_TABLE_ITEM(macro, divide, Divide, opname )    \
    MATHML_OPERATOR_TABLE_ITEM(macro, eq, Eq, opname)             \
    MATHML_OPERATOR_TABLE_ITEM(macro, neq, Neq, opname)           \
    MATHML_OPERATOR_TABLE_ITEM(macro, lt, Lt, opname)             \
    MATHML_OPERATOR_TABLE_ITEM(macro, gt, Gt, opname)             \
    MATHML_OPERATOR_TABLE_ITEM(macro, leq, Leq, opname)           \
    MATHML_OPERATOR_TABLE_ITEM(macro, geq, Geq, opname)           \
    MATHML_OPERATOR_TABLE_ITEM(macro, max, Max, opname)           \
    MATHML_OPERATOR_TABLE_ITEM(macro, min, Min, opname)           \
    MATHML_OPERATOR_TABLE_ITEM(macro, rem, Rem, opname)           \
    MATHML_OPERATOR_TABLE_ITEM(macro, quotient, Quotient, opname) \
    MATHML_OPERATOR_TABLE_ITEM(macro, power, Power, opname)       \
    MATHML_OPERATOR_TABLE_ITEM(macro, root, Root, opname)         \
    MATHML_OPERATOR_TABLE_ITEM(macro, abs, Abs, opname)           \
    MATHML_OPERATOR_TABLE_ITEM(macro, floor, Floor, opname)       \
    MATHML_OPERATOR_TABLE_ITEM(macro, ceiling, Ceiling, opname)   \
    MATHML_OPERATOR_TABLE_ITEM(macro, and, And, opname)           \
    MATHML_OPERATOR_TABLE_ITEM(macro, or, Or, opname)             \
    MATHML_OPERATOR_TABLE_ITEM(macro, xor, Xor, opname)           \
    MATHML_OPERATOR_TABLE_ITEM(macro, not, Not, opname)           \
    MATHML_OPERATOR_TABLE_ITEM(macro, exp, Exp, opname)           \
    MATHML_OPERATOR_TABLE_ITEM(macro, ln, Ln, opname)             \
    MATHML_OPERATOR_TABLE_ITEM(macro, log, Log, opname)           \
    MATHML_OPERATOR_TABLE_ITEM(macro, sin, Sin, opname)           \
    MATHML_OPERATOR_TABLE_ITEM(macro, cos, Cos, opname)           \
    MATHML_OPERATOR_TABLE_ITEM(macro, tan, Tan, opname)           \
    MATHML_OPERATOR_TABLE_ITEM(macro, sinh, Sinh, opname)          \
    MATHML_OPERATOR_TABLE_ITEM(macro, cosh, Cosh, opname)          \
    MATHML_OPERATOR_TABLE_ITEM(macro, tanh, Tanh, opname)          \
    MATHML_OPERATOR_TABLE_ITEM(macro, sec, Sec, opname)           \
    MATHML_OPERATOR_TABLE_ITEM(macro, csc, Csc, opname)           \
    MATHML_OPERATOR_TABLE_ITEM(macro, cot, Cot, opname)           \
    MATHML_OPERATOR_TABLE_ITEM(macro, sech, Sech, opname)          \
    MATHML_OPERATOR_TABLE_ITEM(macro, csch, Csch, opname)          \
    MATHML_OPERATOR_TABLE_ITEM(macro, coth, Coth, opname)          \
    MATHML_OPERATOR_TABLE_ITEM(macro, arcsin, ArcSin, opname)     \
    MATHML_OPERATOR_TABLE_ITEM(macro, arccos, ArcCos, opname)     \
    MATHML_OPERATOR_TABLE_ITEM(macro, arctan, ArcTan, opname)     \
    MATHML_OPERATOR_TABLE_ITEM(macro, arcsinh, ArcSinh, opname)    \
    MATHML_OPERATOR_TABLE_ITEM(macro, arccosh, ArcCosh, opname)    \
    MATHML_OPERATOR_TABLE_ITEM(macro, arctanh, ArcTanh, opname)    \
    MATHML_OPERATOR_TABLE_ITEM(macro, arcsec, ArcSec, opname)     \
    MATHML_OPERATOR_TABLE_ITEM(macro, arccsc, ArcCsc, opname)     \
    MATHML_OPERATOR_TABLE_ITEM(macro, arccot, ArcCot, opname)     \
    MATHML_OPERATOR_TABLE_ITEM(macro, arcsech, ArcSech, opname)    \
    MATHML_OPERATOR_TABLE_ITEM(macro, arccsch, ArcCsch, opname)    \
    MATHML_OPERATOR_TABLE_ITEM(macro, arccoth, ArcCoth, opname)    \
    else PROTO_EXCEPTION("Unrecognised MathML operator " << opname << ".");


#endif // MATHMLALL_HPP_
