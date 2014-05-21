// Matrix_as.cpp:  ActionScript "Matrix" class, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#include "Matrix_as.h"

#include <cmath>
#include <boost/numeric/ublas/matrix.hpp> // boost matrix
#include <boost/numeric/ublas/io.hpp>
#include <boost/intrusive_ptr.hpp>
#include <sstream>

#include "as_function.h"
#include "as_object.h" 
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "GnashException.h" // for ActionException
#include "VM.h"
#include "namedStrings.h"
// According to senocular, Flash docs get this wrong (b and c swapped).
//
// A transformation matrix for affine transformations:
//    a  c  tx
//    b  d  ty
//    u  v  w
// The matrix can only operate in 2d space. The bottom row is immutable
// as 0  0  1.
// Most transformations only apply to a subset of the elements
// (partly because the bottom line is immutable), so it's unnecessary to
// use a full 3x3 matrix for every operation: particularly for invert(),
// where boost::ublas is overcomplicated and can easily fail its own
// consistency checks. For simpler multiplication, boost::ublas is very
// helpful for keep the code clear and tidy.
//
// Most of the methods can be faked: that is, applied to non-Matrices by
// setting an object's method to the corresponding Matrix prototype method.
// The following code successfully inverts o as if it were a Matrix:
//
// o = { a:3, b: 0, c: 1, d: 4, tx: 4, ty: 6};
// o.invert = flash.geom.Matrix.prototype.invert;
// o.invert();
//
// Methods that apparently only work on Matrices are rotate, scale and
// createGradientBox. The method createBox fills in two values, which
// suggests how the PP implements it.
//
// Define this to get verbose debugging messages for matrix calculations
//#define GNASH_DEBUG_GEOM_MATRIX 1

namespace gnash {


// Forward declarations
namespace {

    typedef boost::numeric::ublas::c_matrix<double, 3, 3> MatrixType;
    typedef boost::numeric::ublas::c_vector<double, 2> PointType;

    as_value matrix_clone(const fn_call& fn);
    as_value matrix_concat(const fn_call& fn);
    as_value matrix_createBox(const fn_call& fn);
    as_value matrix_createGradientBox(const fn_call& fn);
    as_value matrix_deltaTransformPoint(const fn_call& fn);
    as_value matrix_identity(const fn_call& fn);
    as_value matrix_invert(const fn_call& fn);
    as_value matrix_rotate(const fn_call& fn);
    as_value matrix_scale(const fn_call& fn);
    as_value matrix_toString(const fn_call& fn);
    as_value matrix_transformPoint(const fn_call& fn);
    as_value matrix_translate(const fn_call& fn);
    void fillMatrix(MatrixType& matrix, as_object& matrixObject);
    PointType transformPoint(as_object& pointObject, as_object& matrixObject);

    as_value get_flash_geom_matrix_constructor(const fn_call& fn);
    as_value matrix_ctor(const fn_call& fn);
    as_object* instanceOfMatrix(const fn_call& fn);
}

void
matrix_class_init(as_object& where, const ObjectURI& uri)
{
    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
    where.init_destructive_property(uri, get_flash_geom_matrix_constructor,
            flags);
}


namespace {

void
attachMatrixInterface(as_object& o)
{
    int fl = 0;

    Global_as& gl = getGlobal(o);
    o.init_member("clone", gl.createFunction(matrix_clone), fl);
    o.init_member("concat", gl.createFunction(matrix_concat), fl);
    o.init_member("createBox", gl.createFunction(matrix_createBox), fl);
    o.init_member("createGradientBox",
            gl.createFunction(matrix_createGradientBox), fl);
    o.init_member("deltaTransformPoint",
            gl.createFunction(matrix_deltaTransformPoint), fl);
    o.init_member("identity", gl.createFunction(matrix_identity), fl);
    o.init_member("invert", gl.createFunction(matrix_invert), fl);
    o.init_member("rotate", gl.createFunction(matrix_rotate), fl);
    o.init_member("scale", gl.createFunction(matrix_scale), fl);
    o.init_member("toString", gl.createFunction(matrix_toString), fl);
    o.init_member("transformPoint",
            gl.createFunction(matrix_transformPoint), fl);
    o.init_member("translate", gl.createFunction(matrix_translate), fl);
}

as_object*
instanceOfMatrix(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);

    as_function* ctor = getClassConstructor(fn, "flash.geom.Matrix");
    if (obj->instanceOf(ctor)) return obj;
    return nullptr;
}

/// Return an exact copy of the matrix.
as_value
matrix_clone(const fn_call& fn)
{
    // It doesn't matter whether it is a matrix or not; a new Matrix
    // is created using any Matrix properties the object may have.
    as_object* ptr = ensure<ValidThis>(fn);

    as_value a, b, c, d, tx, ty;
    ptr->get_member(NSV::PROP_A, &a);
    ptr->get_member(NSV::PROP_B, &b);
    ptr->get_member(NSV::PROP_C, &c);
    ptr->get_member(NSV::PROP_D, &d);
    ptr->get_member(NSV::PROP_TX, &tx);
    ptr->get_member(NSV::PROP_TY, &ty);

    fn_call::Args args;
    args += a, b, c, d, tx, ty;

    as_value matrixClass(findObject(fn.env(), "flash.geom.Matrix"));

    as_function* ctor = matrixClass.to_function();
    if (!ctor) {
        return as_value();
    }

    as_object* o = constructInstance(*ctor, fn.env(), args);

    return as_value(o);
}

// A full, normal concatenation, so use full 3x3 matrices.
as_value
matrix_concat(const fn_call& fn)
{
    // Doesn't have to be a Matrix.
    as_object* ptr = ensure<ValidThis>(fn);

    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("Matrix.concat(%s): needs one argument"), ss.str());
        );
        return as_value();
    }

    // Matrix passed as argument:    
    const as_value& arg = fn.arg(0);
    
    if ( ! arg.is_object() )
    {
        /// Isn't an object...
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("Matrix.concat(%s): needs a Matrix object"), ss.str());
        );
        return as_value();
    }

    // The object to concatenate doesn't have to be a matrix.    
    as_object* obj = toObject(arg, getVM(fn));
    assert(obj);

    MatrixType concatMatrix;
    fillMatrix(concatMatrix, *obj);

    // Current ('this') Matrix
    MatrixType currentMatrix;
    fillMatrix(currentMatrix, *ptr);

#ifdef GNASH_DEBUG_GEOM_MATRIX
    log_debug("(Matrix.concat) This matrix (pre-transform): %s",
              currentMatrix);
    log_debug("(Matrix.concat) Transform matrix: %s",
              concatMatrix);
#endif
    
    currentMatrix = boost::numeric::ublas::prod(concatMatrix, currentMatrix);
 
#ifdef GNASH_DEBUG_GEOM_MATRIX
    log_debug("(Matrix.concat) This matrix (post-transform): %s",
              currentMatrix);
#endif 
    
    // Set values of current matrix
    ptr->set_member(NSV::PROP_A, currentMatrix(0, 0));
    ptr->set_member(NSV::PROP_B, currentMatrix(1, 0));
    ptr->set_member(NSV::PROP_C, currentMatrix(0, 1));
    ptr->set_member(NSV::PROP_D, currentMatrix(1, 1));
    ptr->set_member(NSV::PROP_TX, currentMatrix(0, 2));
    ptr->set_member(NSV::PROP_TY, currentMatrix(1, 2));

    return as_value();
}


/// Creates a matrix from (X scale, Y scale, rotation, X translation, Y translation)
/// The translation values can be any as_value; the others (because mathematical
/// operations are applied to them), result in NaN if anything other than a number
/// is passed, so we treat them as doubles from the beginning.
as_value
matrix_createBox(const fn_call& fn)
{
    // Doesn't have to be a Matrix
    as_object* ptr = ensure<ValidThis>(fn);

    if (fn.nargs < 2)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("Matrix.createBox(%s): needs at least two arguments"), ss.str());
        );
        return as_value();
    }

    double scaleX, scaleY;

    // Default values for optional arguments    
    double rotation = 0;
    as_value tx, ty;
    tx.set_double(0);
    ty.set_double(0);
    
    switch (fn.nargs)
    {
        default:
            // Log as coding error
        case 5:
            ty = fn.arg(4);
        case 4:
            tx = fn.arg(3);
        case 3:
            rotation = toNumber(fn.arg(2), getVM(fn));
        case 2:
            // There must be a minimum of 2 arguments.
            scaleY = toNumber(fn.arg(1), getVM(fn));
            scaleX = toNumber(fn.arg(0), getVM(fn));
            break;
    }
    
    
    const double a = std::cos(rotation) * scaleX;
    const double b = std::sin(rotation) * scaleY;
    const double c = -std::sin(rotation) * scaleX;
    const double d = std::cos(rotation) * scaleY;
    
    ptr->set_member(NSV::PROP_A, as_value(a));
    ptr->set_member(NSV::PROP_B, as_value(b));
    ptr->set_member(NSV::PROP_C, as_value(c));
    ptr->set_member(NSV::PROP_D, as_value(d));
    ptr->set_member(NSV::PROP_TX, tx);
    ptr->set_member(NSV::PROP_TY, ty);
    
    return as_value();
}


// Like createBox, but with strange offsets applied.
as_value
matrix_createGradientBox(const fn_call& fn)
{
    as_object* ptr = instanceOfMatrix(fn);
    if (!ptr) return as_value();

    if (fn.nargs < 2)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("Matrix.createGradientBox(%s): needs at least "
                          "two arguments"), ss.str());
        );
        return as_value();
    }

    double widthX, widthY;

    // Default values for optional arguments    
    double rotation = 0;
    as_value tx, ty;
    tx.set_double(0);
    ty.set_double(0);
    
    switch (fn.nargs)
    {
        default:
            // Log as coding error
        case 5:
            ty = fn.arg(4);
        case 4:
            tx = fn.arg(3);
        case 3:
            rotation = toNumber(fn.arg(2), getVM(fn));
        case 2:
            // There must be a minimum of 2 arguments.
            widthY = toNumber(fn.arg(1), getVM(fn));
            widthX = toNumber(fn.arg(0), getVM(fn));
            break;
    }
    
    // A bit of a magic number: the maximum positive co-ordinate of
    // a gradient square.
    const double gradientSquareMax = 16384.0;
    
    const double a = std::cos(rotation) * widthX * 10 / gradientSquareMax;
    const double b = std::sin(rotation) * widthY * 10 / gradientSquareMax;
    const double c = -std::sin(rotation) * widthX * 10 / gradientSquareMax;
    const double d = std::cos(rotation) * widthY * 10 / gradientSquareMax;
    
    ptr->set_member(NSV::PROP_A, a);
    ptr->set_member(NSV::PROP_B, b);
    ptr->set_member(NSV::PROP_C, c);
    ptr->set_member(NSV::PROP_D, d);
    
    // The translation is offset by half the size of the corresponding
    // dimension. Or rather, half the dimension is added to the translation,
    // whether it's a number or not.
    VM& vm = getVM(fn);
    newAdd(tx, widthX / 2.0, vm);
    newAdd(ty, widthY / 2.0, vm);

    ptr->set_member(NSV::PROP_TX, tx);
    ptr->set_member(NSV::PROP_TY, ty);
    
    return as_value();
}


/// Transform the point using a Matrix. The translation
/// elements (tx, ty) do not have any effect.
///
/// Returns a new Point, leaving the object passed untouched.
as_value
matrix_deltaTransformPoint(const fn_call& fn)
{
    // Doesn't have to be a Matrix
    as_object* ptr = ensure<ValidThis>(fn);

    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("Matrix.deltaTransformPoint(%s): needs one argument"),
                ss.str());
        );
        return as_value();
    }

    const as_value& arg = fn.arg(0);
    
    if ( ! arg.is_object() )
    {
        /// Isn't an object...
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("Matrix.deltaTransformPoint(%s): needs an object"),
                ss.str());
        );
        return as_value();
    }

    // It doesn't have to be a point. If it has x and y
    // properties, they will be used.    
    as_object* obj = toObject(arg, getVM(fn));
    assert(obj);

    const PointType& point = transformPoint(*obj, *ptr);

    // Construct a Point and set its properties.
    as_value pointClass(findObject(fn.env(), "flash.geom.Point"));

    as_function* pointCtor = pointClass.to_function();

    if (!pointCtor) {
        log_error(_("Failed to construct flash.geom.Point!"));
        return as_value();
    }

    fn_call::Args args;
    args += point(0), point(1);

    as_value ret = constructInstance(*pointCtor, fn.env(), args);

    return as_value(ret);
}


/// Reset the matrix to a null-transformation
///
/// | 1   0   0 |
/// | 0   1   0 |
///(| 0   0   1 |)
/// Returns void.
as_value
matrix_identity(const fn_call& fn)
{
    // Doesn't have to be a Matrix
    as_object* ptr = ensure<ValidThis>(fn);

    ptr->set_member(NSV::PROP_A, 1.0);
    ptr->set_member(NSV::PROP_B, 0.0);
    ptr->set_member(NSV::PROP_C, 0.0);
    ptr->set_member(NSV::PROP_D, 1.0);
    ptr->set_member(NSV::PROP_TX, 0.0);
    ptr->set_member(NSV::PROP_TY, 0.0);

    return as_value();
}


inline double
getMinorDeterminant(const MatrixType& m)
{
    return m(0, 0) * m(1, 1) - m(0, 1) * m(1, 0);
}

as_value
matrix_invert(const fn_call& fn)
{

    // Doesn't have to be a Matrix
    as_object* ptr = ensure<ValidThis>(fn);

    MatrixType currentMatrix;
    
    // This just saves repeating code to get doubles for each
    // value.
    fillMatrix(currentMatrix, *ptr);

    const double determinant = getMinorDeterminant(currentMatrix);
    
    // This is a double, so it could be worth checking for the epsilon.
    if (determinant == 0)
    {
        // Return the identity matrix
        ptr->set_member(NSV::PROP_A, 1.0);
        ptr->set_member(NSV::PROP_B, 0.0);
        ptr->set_member(NSV::PROP_C, 0.0);
        ptr->set_member(NSV::PROP_D, 1.0);
        ptr->set_member(NSV::PROP_TX, 0.0);
        ptr->set_member(NSV::PROP_TY, 0.0); 
        return as_value();  
    }
    
    const double a = currentMatrix(1, 1) / determinant;
    const double c = - currentMatrix(0, 1) / determinant;
    const double b = - currentMatrix(1, 0) / determinant;
    const double d = currentMatrix(0, 0) / determinant;
    
    const double tx = - (a * currentMatrix(0, 2) + c * currentMatrix(1, 2));
    const double ty = - (b * currentMatrix(0, 2) + d * currentMatrix(1, 2));

    // Returns the identity matrix if unsuccessful.
    ptr->set_member(NSV::PROP_A, as_value(a));
    ptr->set_member(NSV::PROP_B, as_value(b));
    ptr->set_member(NSV::PROP_C, as_value(c));
    ptr->set_member(NSV::PROP_D, as_value(d));
    ptr->set_member(NSV::PROP_TX, as_value(tx));
    ptr->set_member(NSV::PROP_TY, as_value(ty));

    return as_value();
}


as_value
matrix_rotate(const fn_call& fn)
{
    // Apparently has to be a Matrix.
    as_object* ptr = instanceOfMatrix(fn);
    if (!ptr) return as_value();

    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("Matrix.rotate(%s): needs one argument"), ss.str());
        );
        return as_value();
    }

    // Make rotation matrix
    boost::numeric::ublas::c_matrix<double, 2, 2> transformMatrix;
    
    const double rot = toNumber(fn.arg(0), getVM(fn));
    
    transformMatrix(0, 0) = std::cos(rot);
    transformMatrix(0, 1) = std::sin(rot);
    transformMatrix(1, 0) = -std::sin(rot);
    transformMatrix(1, 1) = std::cos(rot);

    // Get current matrix
    boost::numeric::ublas::c_matrix<double, 2, 2> currentMatrix;

    as_value a, b, c, d, tx, ty;

    ptr->get_member(NSV::PROP_A, &a);
    ptr->get_member(NSV::PROP_B, &b);
    ptr->get_member(NSV::PROP_C, &c);
    ptr->get_member(NSV::PROP_D, &d);
    ptr->get_member(NSV::PROP_TX, &tx);
    ptr->get_member(NSV::PROP_TY, &ty);
            
    currentMatrix(0, 0) = toNumber(a, getVM(fn));
    currentMatrix(0, 1) = toNumber(b, getVM(fn));
    currentMatrix(1, 0) = toNumber(c, getVM(fn));
    currentMatrix(1, 1) = toNumber(d, getVM(fn));

#ifdef GNASH_DEBUG_GEOM_MATRIX
    log_debug("(Matrix.rotate) This matrix (pre-transform): %s",
              currentMatrix);
#endif

    // Apply rotation to current matrix.
    currentMatrix = boost::numeric::ublas::prod(currentMatrix, transformMatrix);

#ifdef GNASH_DEBUG_GEOM_MATRIX
    log_debug("(Matrix.rotate) Transformation matrix: %s", transformMatrix);
    log_debug("(Matrix.rotate) This matrix (post-transform): %s",
              currentMatrix);
#endif

    ptr->set_member(NSV::PROP_A, as_value(currentMatrix(0, 0)));
    ptr->set_member(NSV::PROP_B, as_value(currentMatrix(0, 1)));
    ptr->set_member(NSV::PROP_C, as_value(currentMatrix(1, 0)));
    ptr->set_member(NSV::PROP_D, as_value(currentMatrix(1, 1)));

    // Do rotation separately.
    PointType translation;
    translation(0) = toNumber(tx, getVM(fn));
    translation(1) = toNumber(ty, getVM(fn));
    
    translation = boost::numeric::ublas::prod(translation, transformMatrix);

    ptr->set_member(NSV::PROP_TX, translation(0));
    ptr->set_member(NSV::PROP_TY, translation(1));  

    return as_value();
}

as_value
matrix_scale(const fn_call& fn)
{
    // Apparently does have to be a Matrix.
    as_object* ptr = instanceOfMatrix(fn);
    if (!ptr) return as_value();

    if (fn.nargs < 2)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("Matrix.translate(%s): needs two arguments"), ss.str());
        );
        return as_value();
    }

    // Make scale matrix
    boost::numeric::ublas::c_matrix<double, 2, 2> transformMatrix;
    
    const double scaleX = toNumber(fn.arg(0), getVM(fn));
    const double scaleY = toNumber(fn.arg(1), getVM(fn));
    
    transformMatrix(0, 0) = scaleX;
    transformMatrix(0, 1) = 0.0;
    transformMatrix(1, 0) = 0.0;
    transformMatrix(1, 1) = scaleY;

    // Get current matrix
    boost::numeric::ublas::c_matrix<double, 2, 2> currentMatrix;
    
    as_value a, b, c, d, tx, ty;

    ptr->get_member(NSV::PROP_A, &a);
    ptr->get_member(NSV::PROP_B, &b);
    ptr->get_member(NSV::PROP_C, &c);
    ptr->get_member(NSV::PROP_D, &d);
    ptr->get_member(NSV::PROP_TX, &tx);
    ptr->get_member(NSV::PROP_TY, &ty);
            
    currentMatrix(0, 0) = toNumber(a, getVM(fn));
    currentMatrix(0, 1) = toNumber(b, getVM(fn));
    currentMatrix(1, 0) = toNumber(c, getVM(fn));
    currentMatrix(1, 1) = toNumber(d, getVM(fn));
    
#ifdef GNASH_DEBUG_GEOM_MATRIX
    log_debug("(Matrix.scale) This matrix (pre-transform): %s",
              currentMatrix);
#endif
    
    // Apply scale to current matrix.
    currentMatrix = boost::numeric::ublas::prod(currentMatrix, transformMatrix);

#ifdef GNASH_DEBUG_GEOM_MATRIX
    log_debug("(Matrix.scale) Transformation matrix: %s", transformMatrix);
    log_debug("(Matrix.scale) This matrix (post-transform): %s",
              currentMatrix);
#endif

    ptr->set_member(NSV::PROP_A, currentMatrix(0, 0));
    ptr->set_member(NSV::PROP_B, currentMatrix(0, 1));
    ptr->set_member(NSV::PROP_C, currentMatrix(1, 0));
    ptr->set_member(NSV::PROP_D, currentMatrix(1, 1));

    // This is just a simple multiplication, so do it separately.
    ptr->set_member(NSV::PROP_TX, as_value(toNumber(tx, getVM(fn)) * scaleX));
    ptr->set_member(NSV::PROP_TY, as_value(toNumber(ty, getVM(fn)) * scaleY));  

    return as_value();
}

as_value
matrix_toString(const fn_call& fn)
{
    // Doesn't have to be a Matrix
    as_object* ptr = ensure<ValidThis>(fn);

    as_value a, b, c, d, tx, ty;

    ptr->get_member(NSV::PROP_A, &a);
    ptr->get_member(NSV::PROP_B, &b);
    ptr->get_member(NSV::PROP_C, &c);
    ptr->get_member(NSV::PROP_D, &d);
    ptr->get_member(NSV::PROP_TX, &tx);
    ptr->get_member(NSV::PROP_TY, &ty);
    
    VM& vm = getVM(fn);

    as_value ret("(a=");
    newAdd(ret, a, vm);
    newAdd(ret, ", b=", vm);
    newAdd(ret, b, vm);
    newAdd(ret, ", c=", vm);
    newAdd(ret, c, vm);
    newAdd(ret, ", d=", vm);
    newAdd(ret, d, vm);
    newAdd(ret, ", tx=", vm);
    newAdd(ret, tx, vm);
    newAdd(ret, ", ty=", vm);
    newAdd(ret, ty, vm);
    newAdd(ret, ")", vm);

    return ret;
    
}

as_value
matrix_transformPoint(const fn_call& fn)
{
    // Doesn't have to be a Matrix
    as_object* ptr = ensure<ValidThis>(fn);

    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("Matrix.translate(%s): needs one argument"), ss.str());
        );
        return as_value();
    }

    const as_value& arg = fn.arg(0);
    
    if ( ! arg.is_object() )
    {
        /// Isn't an object...
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("Matrix.transformPoint(%s): needs an object"), ss.str());
        );
        return as_value();
    }
    
    as_object* obj = toObject(arg, getVM(fn));
    assert(obj);
    if (!obj->instanceOf(getClassConstructor(fn, "flash.geom.Point"))) {
        /// Isn't a point.
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("Matrix.transformPoint(%s): object must be a Point"),
                ss.str());
        );
        return as_value();
    }

    as_value tx, ty;
    ptr->get_member(NSV::PROP_TX, &tx);
    ptr->get_member(NSV::PROP_TY, &ty);
    
    const PointType& point = transformPoint(*obj, *ptr);

    // Construct a Point and set its properties.
    as_value pointClass(findObject(fn.env(), "flash.geom.Point"));

    as_function* pointCtor = pointClass.to_function();

    if (!pointCtor) {
        log_error(_("Failed to construct flash.geom.Point!"));
        return as_value();
    }

    fn_call::Args args;
    args += point(0) + toNumber(tx, getVM(fn)), point(1) + toNumber(ty, getVM(fn));

    as_value ret = constructInstance(*pointCtor, fn.env(), args);

    return as_value(ret);
}

as_value
matrix_translate(const fn_call& fn)
{
    // Doesn't have to be a Matrix
    as_object* ptr = ensure<ValidThis>(fn);
    
    if (fn.nargs < 2)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("Matrix.translate(%s): needs two arguments"), ss.str());
        );
        return as_value();    
    }
    
    if (fn.nargs == 2)
    {

        as_value tx, ty;

        ptr->get_member(NSV::PROP_TX, &tx);
        ptr->get_member(NSV::PROP_TY, &ty);

        double transX = toNumber(fn.arg(0), getVM(fn)) + toNumber(tx, getVM(fn));
        double transY = toNumber(fn.arg(1), getVM(fn)) + toNumber(ty, getVM(fn));
                
        ptr->set_member(NSV::PROP_TX, as_value(transX));
        ptr->set_member(NSV::PROP_TY, as_value(transY));
    }
    return as_value();
}

// A helper function to transform a point using a matrix object,
// after which the translation can be applied if necessary
// (transformPoint) or not if not (deltaTransformPoint). Just
// make sure the objects are what they're supposed to be.
PointType
transformPoint(as_object& pointObject, as_object& matrixObject)
{
    // Get the point co-ordinates.
    as_value x, y;
    
    pointObject.get_member(NSV::PROP_X, &x);
    pointObject.get_member(NSV::PROP_Y, &y);

    // Get the matrix elements to use as a transformation matrix.
    as_value a, b, c, d;

    matrixObject.get_member(NSV::PROP_A, &a);
    matrixObject.get_member(NSV::PROP_B, &b);
    matrixObject.get_member(NSV::PROP_C, &c);
    matrixObject.get_member(NSV::PROP_D, &d);

    VM& vm = getVM(pointObject);

    // Construct the matrix
    boost::numeric::ublas::c_matrix<double, 2, 2> transformMatrix;
    transformMatrix(0, 0) = toNumber(a, vm);
    transformMatrix(0, 1) = toNumber(b, vm);
    transformMatrix(1, 0) = toNumber(c, vm);
    transformMatrix(1, 1) = toNumber(d, vm);

    // Construct the point
    PointType point;
    point(0) = toNumber(x, vm);
    point(1) = toNumber(y, vm);

    // Transform
    point = boost::numeric::ublas::prod(point, transformMatrix);

    return point;

}

// A helper function to create a boost matrix from a Matrix object
void
fillMatrix(MatrixType& matrix, as_object& matrixObject)
{

    const double u = 0.0;
    const double v = 0.0;
    const double w = 1.0;

    as_value a, b, c, d, tx, ty;

    matrixObject.get_member(NSV::PROP_A, &a);
    matrixObject.get_member(NSV::PROP_B, &b);
    matrixObject.get_member(NSV::PROP_C, &c);
    matrixObject.get_member(NSV::PROP_D, &d);
    matrixObject.get_member(NSV::PROP_TX, &tx);
    matrixObject.get_member(NSV::PROP_TY, &ty);

    VM& vm = getVM(matrixObject);

    matrix(0, 0) = toNumber(a, vm);
    matrix(0, 1) = toNumber(c, vm);
    matrix(0, 2) = toNumber(tx, vm);
    matrix(1, 0) = toNumber(b, vm);
    matrix(1, 1) = toNumber(d, vm);
    matrix(1, 2) = toNumber(ty, vm);
    matrix(2, 0) = u;
    matrix(2, 1) = v;
    matrix(2, 2) = w;

}


// Any arguments can be passed, not just valid ones.
// If no arguments are passed, an identity matrix is created.
// TODO: check:
// If at least one argument is passed, any missing arguments are undefined.
// Extra arguments are discarded
as_value
matrix_ctor(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);
    
    if (!fn.nargs) {
        // TODO: use NSV
        const ObjectURI& identity = getURI(getVM(fn), "identity");
        callMethod(obj, identity);
        return as_value();
    }
    
    obj->set_member(NSV::PROP_A, fn.arg(0));
    obj->set_member(NSV::PROP_B, fn.nargs > 1 ? fn.arg(1) : as_value());
    obj->set_member(NSV::PROP_C, fn.nargs > 2 ? fn.arg(2) : as_value());
    obj->set_member(NSV::PROP_D, fn.nargs > 3 ? fn.arg(3) : as_value());
    obj->set_member(NSV::PROP_TX, fn.nargs > 4 ? fn.arg(4) : as_value());
    obj->set_member(NSV::PROP_TY, fn.nargs > 5 ? fn.arg(5) : as_value());

    return as_value(); 
}


as_value
get_flash_geom_matrix_constructor(const fn_call& fn)
{
    log_debug("Loading flash.geom.Matrix class");
    Global_as& gl = getGlobal(fn);
    as_object* proto = createObject(gl);
    attachMatrixInterface(*proto);
    return gl.createClass(&matrix_ctor, proto);
}

} // anonymous namespace
} // end of gnash namespace
