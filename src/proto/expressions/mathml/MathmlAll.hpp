/*

Copyright (C) University of Oxford, 2005-2012

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
