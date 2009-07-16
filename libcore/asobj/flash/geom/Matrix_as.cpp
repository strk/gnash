// Matrix_as.cpp:  ActionScript "Matrix" class, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "Object.h" // for AS inheritance
#include "VM.h" // for addStatics
#include "Point_as.h" // Matrix needs to operate on Points.

#include <cmath>
#include <boost/numeric/ublas/matrix.hpp> // boost matrix
#include <boost/numeric/ublas/io.hpp>
#include <sstream>

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

typedef boost::numeric::ublas::c_matrix<double, 3, 3> MatrixType;
typedef boost::numeric::ublas::c_vector<double, 2> PointType;

// Forward declarations
static as_value Matrix_clone(const fn_call& fn);
static as_value Matrix_concat(const fn_call& fn);
static as_value Matrix_createBox(const fn_call& fn);
static as_value Matrix_createGradientBox(const fn_call& fn);
static as_value Matrix_deltaTransformPoint(const fn_call& fn);
static as_value Matrix_identity(const fn_call& fn);
static as_value Matrix_invert(const fn_call& fn);
static as_value Matrix_rotate(const fn_call& fn);
static as_value Matrix_scale(const fn_call& fn);
static as_value Matrix_toString(const fn_call& fn);
static as_value Matrix_transformPoint(const fn_call& fn);
static as_value Matrix_translate(const fn_call& fn);
static void fillMatrix(MatrixType& matrix, as_object& matrixObject);
static PointType transformPoint(as_object* const pointObject, as_object* const matrixObject);

as_value Matrix_ctor(const fn_call& fn);

static void
attachMatrixInterface(as_object& o)
{
    int fl = 0;

    Global_as* gl = getGlobal(o);
    o.init_member("clone", gl->createFunction(Matrix_clone), fl);
    o.init_member("concat", gl->createFunction(Matrix_concat), fl);
    o.init_member("createBox", gl->createFunction(Matrix_createBox), fl);
    o.init_member("createGradientBox",
            gl->createFunction(Matrix_createGradientBox), fl);
    o.init_member("deltaTransformPoint",
            gl->createFunction(Matrix_deltaTransformPoint), fl);
    o.init_member("identity", gl->createFunction(Matrix_identity), fl);
    o.init_member("invert", gl->createFunction(Matrix_invert), fl);
    o.init_member("rotate", gl->createFunction(Matrix_rotate), fl);
    o.init_member("scale", gl->createFunction(Matrix_scale), fl);
    o.init_member("toString", gl->createFunction(Matrix_toString), fl);
    o.init_member("transformPoint",
            gl->createFunction(Matrix_transformPoint), fl);
    o.init_member("translate", gl->createFunction(Matrix_translate), fl);
}


static as_object*
getMatrixInterface()
{
    static boost::intrusive_ptr<as_object> o;

    if ( ! o )
    {
        // Inherits from Object.
        o = new as_object(getObjectInterface());
        VM::get().addStatic(o.get());

        attachMatrixInterface(*o);

    }

    return o.get();
}

class Matrix_as: public as_object
{

public:

    Matrix_as()
        :
        as_object(getMatrixInterface())
    {
    }

};


/// Return an exact copy of the matrix.
static as_value
Matrix_clone(const fn_call& fn)
{
    // It doesn't matter whether it is a matrix or not; a new Matrix
    // is created using any Matrix properties the object may have.
    boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);

    as_value a, b, c, d, tx, ty;
    ptr->get_member(NSV::PROP_A, &a);
    ptr->get_member(NSV::PROP_B, &b);
    ptr->get_member(NSV::PROP_C, &c);
    ptr->get_member(NSV::PROP_D, &d);
    ptr->get_member(NSV::PROP_TX, &tx);
    ptr->get_member(NSV::PROP_TY, &ty);

    boost::intrusive_ptr<as_object> ret = new Matrix_as;
    ret->set_member(NSV::PROP_A, a);
    ret->set_member(NSV::PROP_B, b);
    ret->set_member(NSV::PROP_C, c);
    ret->set_member(NSV::PROP_D, d);
    ret->set_member(NSV::PROP_TX, tx);
    ret->set_member(NSV::PROP_TY, ty);
    
    return as_value(ret.get());
}

// A full, normal concatenation, so use full 3x3 matrices.
static as_value
Matrix_concat(const fn_call& fn)
{
    // Doesn't have to be a Matrix.
    boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);

    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("Matrix.concat(%s): needs one argument", ss.str());
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
            log_aserror("Matrix.concat(%s): needs a Matrix object", ss.str());
        );
        return as_value();
    }

    // The object to concatenate doesn't have to be a matrix.    
    as_object* obj = arg.to_object(*getGlobal(fn)).get();
    assert(obj);

    MatrixType concatMatrix;
    fillMatrix(concatMatrix, *obj);

    // Current ('this') Matrix
    MatrixType currentMatrix;
    fillMatrix(currentMatrix, *ptr);

#ifdef GNASH_DEBUG_GEOM_MATRIX
    log_debug("(Matrix.concat) This matrix (pre-transform): %s", currentMatrix);
    log_debug("(Matrix.concat) Transform matrix: %s", concatMatrix);
#endif
    
    currentMatrix = boost::numeric::ublas::prod(concatMatrix, currentMatrix);
 
#ifdef GNASH_DEBUG_GEOM_MATRIX
    log_debug("(Matrix.concat) This matrix (post-transform): %s", currentMatrix);
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
static as_value
Matrix_createBox(const fn_call& fn)
{
    // Doesn't have to be a Matrix
    boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);

    if (fn.nargs < 2)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("Matrix.createBox(%s): needs at least two arguments", ss.str());
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
            rotation = fn.arg(2).to_number();
        case 2:
            // There must be a minimum of 2 arguments.
            scaleY = fn.arg(1).to_number();
            scaleX = fn.arg(0).to_number();
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
static as_value
Matrix_createGradientBox(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);

    if (fn.nargs < 2)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("Matrix.createGradientBox(%s): needs at least two arguments", ss.str());
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
            rotation = fn.arg(2).to_number();
        case 2:
            // There must be a minimum of 2 arguments.
            widthY = fn.arg(1).to_number();
            widthX = fn.arg(0).to_number();
            break;
    }
    
    // A bit of a magic number: the maximum positive co-ordinate of
    // a gradient square.
    const double gradientSquareMax = 16384.0;
    
    const double a = std::cos(rotation) * widthX * 10 / gradientSquareMax;
    const double b = std::sin(rotation) * widthY * 10 / gradientSquareMax;
    const double c = -std::sin(rotation) * widthX * 10 / gradientSquareMax;
    const double d = std::cos(rotation) * widthY * 10 / gradientSquareMax;
    
    ptr->set_member(NSV::PROP_A, as_value(a));
    ptr->set_member(NSV::PROP_B, as_value(b));
    ptr->set_member(NSV::PROP_C, as_value(c));
    ptr->set_member(NSV::PROP_D, as_value(d));
    
    // The translation is offset by half the size of the corresponding
    // dimension. Or rather, half the dimension is added to the translation,
    // whether it's a number or not.
    tx.newAdd(widthX / 2.0);
    ty.newAdd(widthY / 2.0);

    ptr->set_member(NSV::PROP_TX, tx);
    ptr->set_member(NSV::PROP_TY, ty);
    
    return as_value();

    return as_value();
}


/// Transform the point using a Matrix. The translation
/// elements (tx, ty) do not have any effect.
///
/// Returns a new Point, leaving the object passed untouched.
static as_value
Matrix_deltaTransformPoint(const fn_call& fn)
{
    // Doesn't have to be a Matrix
    boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);

    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("Matrix.deltaTransformPoint(%s): needs one argument",
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
            log_aserror("Matrix.deltaTransformPoint(%s): needs an object",
                ss.str());
        );
        return as_value();
    }

    // It doesn't have to be a point. If it has x and y
    // properties, they will be used.    
    as_object* obj = arg.to_object(*getGlobal(fn)).get();
    assert(obj);

    const PointType& point = transformPoint(obj, ptr.get());

    // Construct a Point and set its properties.
    boost::intrusive_ptr<as_object> ret = init_Point_instance();
    ret->set_member(NSV::PROP_X, point(0));
    ret->set_member(NSV::PROP_Y, point(1));

    return as_value(ret.get());
}


/// Reset the matrix to a null-transformation
///
/// | 1   0   0 |
/// | 0   1   0 |
///(| 0   0   1 |)
/// Returns void.
static as_value
Matrix_identity(const fn_call& fn)
{
    // Doesn't have to be a Matrix
    boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);

    ptr->set_member(NSV::PROP_A, 1.0);
    ptr->set_member(NSV::PROP_B, 0.0);
    ptr->set_member(NSV::PROP_C, 0.0);
    ptr->set_member(NSV::PROP_D, 1.0);
    ptr->set_member(NSV::PROP_TX, 0.0);
    ptr->set_member(NSV::PROP_TY, 0.0);

    return as_value();
}


static inline double
getMinorDeterminant(const MatrixType& m)
{
    return m(0, 0) * m(1, 1) - m(0, 1) * m(1, 0);
}

static as_value
Matrix_invert(const fn_call& fn)
{

    // Doesn't have to be a Matrix
    boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);

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


static as_value
Matrix_rotate(const fn_call& fn)
{
    // Apparently has to be a Matrix.
    boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);

    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("Matrix.rotate(%s): needs one argument", ss.str());
        );
        return as_value();
    }

    // Make rotation matrix
    boost::numeric::ublas::c_matrix<double, 2, 2> transformMatrix;
    
    const double rot = fn.arg(0).to_number();
    
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
            
    currentMatrix(0, 0) = a.to_number();
    currentMatrix(0, 1) = b.to_number();
    currentMatrix(1, 0) = c.to_number();
    currentMatrix(1, 1) = d.to_number();

#ifdef GNASH_DEBUG_GEOM_MATRIX
    log_debug("(Matrix.rotate) This matrix (pre-transform): %s", currentMatrix);
#endif

    // Apply rotation to current matrix.
    currentMatrix = boost::numeric::ublas::prod(currentMatrix, transformMatrix);

#ifdef GNASH_DEBUG_GEOM_MATRIX
    log_debug("(Matrix.rotate) Transformation matrix: %s", transformMatrix);
    log_debug("(Matrix.rotate) This matrix (post-transform): %s", currentMatrix);
#endif

    ptr->set_member(NSV::PROP_A, as_value(currentMatrix(0, 0)));
    ptr->set_member(NSV::PROP_B, as_value(currentMatrix(0, 1)));
    ptr->set_member(NSV::PROP_C, as_value(currentMatrix(1, 0)));
    ptr->set_member(NSV::PROP_D, as_value(currentMatrix(1, 1)));

    // Do rotation separately.
    PointType translation;
    translation(0) = tx.to_number();
    translation(1) = ty.to_number();
    
    translation = boost::numeric::ublas::prod(translation, transformMatrix);

    ptr->set_member(NSV::PROP_TX, translation(0));
    ptr->set_member(NSV::PROP_TY, translation(1));  

    return as_value();
}

static as_value
Matrix_scale(const fn_call& fn)
{
    // Apparently does have to be a Matrix.
    boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);

    if (fn.nargs < 2)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("Matrix.translate(%s): needs two arguments", ss.str());
        );
        return as_value();
    }

    // Make scale matrix
    boost::numeric::ublas::c_matrix<double, 2, 2> transformMatrix;
    
    const double scaleX = fn.arg(0).to_number();
    const double scaleY = fn.arg(1).to_number();
    
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
            
    currentMatrix(0, 0) = a.to_number();
    currentMatrix(0, 1) = b.to_number();
    currentMatrix(1, 0) = c.to_number();
    currentMatrix(1, 1) = d.to_number();
    
#ifdef GNASH_DEBUG_GEOM_MATRIX
    log_debug("(Matrix.scale) This matrix (pre-transform): %s", currentMatrix);
#endif
    
    // Apply scale to current matrix.
    currentMatrix = boost::numeric::ublas::prod(currentMatrix, transformMatrix);

#ifdef GNASH_DEBUG_GEOM_MATRIX
    log_debug("(Matrix.scale) Transformation matrix: %s", transformMatrix);
    log_debug("(Matrix.scale) This matrix (post-transform): %s", currentMatrix);
#endif

    ptr->set_member(NSV::PROP_A, currentMatrix(0, 0));
    ptr->set_member(NSV::PROP_B, currentMatrix(0, 1));
    ptr->set_member(NSV::PROP_C, currentMatrix(1, 0));
    ptr->set_member(NSV::PROP_D, currentMatrix(1, 1));

    // This is just a simple multiplication, so do it separately.
    ptr->set_member(NSV::PROP_TX, as_value(tx.to_number() * scaleX));
    ptr->set_member(NSV::PROP_TY, as_value(ty.to_number() * scaleY));  

    return as_value();
}

static as_value
Matrix_toString(const fn_call& fn)
{
    // Doesn't have to be a Matrix
    boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);

    as_value a, b, c, d, tx, ty;

    ptr->get_member(NSV::PROP_A, &a);
    ptr->get_member(NSV::PROP_B, &b);
    ptr->get_member(NSV::PROP_C, &c);
    ptr->get_member(NSV::PROP_D, &d);
    ptr->get_member(NSV::PROP_TX, &tx);
    ptr->get_member(NSV::PROP_TY, &ty);
    
    std::ostringstream ss;
    
    const int version = getSWFVersion(fn);

    ss << "(a=" << a.to_string_versioned(version) << ", "
          "b="<< b.to_string_versioned(version) << ", "
          "c="<< c.to_string_versioned(version) << ", "
          "d="<< d.to_string_versioned(version) << ", "
          "tx="<< tx.to_string_versioned(version) << ", "
          "ty="<< ty.to_string_versioned(version) << ")";
    
    return as_value(ss.str());
}

static as_value
Matrix_transformPoint(const fn_call& fn)
{
    // Doesn't have to be a Matrix
    boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);

    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("Matrix.translate(%s): needs one argument", ss.str());
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
            log_aserror("Matrix.transformPoint(%s): needs an object", ss.str());
        );
        return as_value();
    }
    
    as_object* obj = arg.to_object(*getGlobal(fn)).get();
    assert(obj);
    if (!obj->instanceOf(getFlashGeomPointConstructor(fn))) {
        /// Isn't a point.
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("Matrix.transformPoint(%s): object must be a Point",
                ss.str());
        );
        return as_value();
    }

    as_value tx, ty;
    ptr->get_member(NSV::PROP_TX, &tx);
    ptr->get_member(NSV::PROP_TY, &ty);
    
    const PointType& point = transformPoint(obj, ptr.get());

    boost::intrusive_ptr<as_object> ret = init_Point_instance();
    ret->set_member(NSV::PROP_X, point(0) + tx.to_number());
    ret->set_member(NSV::PROP_Y, point(1) + ty.to_number());

    return as_value(ret.get());
}

static as_value
Matrix_translate(const fn_call& fn)
{
    // Doesn't have to be a Matrix
    boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);
    
    if (fn.nargs < 2)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("Matrix.translate(%s): needs two arguments", ss.str());
        );
        return as_value();    
    }
    
    if (fn.nargs == 2)
    {

        as_value tx, ty;

        ptr->get_member(NSV::PROP_TX, &tx);
        ptr->get_member(NSV::PROP_TY, &ty);

        double transX = fn.arg(0).to_number() + tx.to_number();
        double transY = fn.arg(1).to_number() + ty.to_number();
                
        ptr->set_member(NSV::PROP_TX, as_value(transX));
        ptr->set_member(NSV::PROP_TY, as_value(transY));
    }
    return as_value();
}

// A helper function to transform a point using a matrix object,
// after which the translation can be applied if necessary
// (transformPoint) or not if not (deltaTransformPoint). Just
// make sure the objects are what they're supposed to be.
static PointType
transformPoint(as_object* const pointObject, as_object* const matrixObject)
{
    // Get the point co-ordinates.
    as_value x, y;
    
    pointObject->get_member(NSV::PROP_X, &x);
    pointObject->get_member(NSV::PROP_Y, &y);

    // Get the matrix elements to use as a transformation matrix.
    as_value a, b, c, d;

    matrixObject->get_member(NSV::PROP_A, &a);
    matrixObject->get_member(NSV::PROP_B, &b);
    matrixObject->get_member(NSV::PROP_C, &c);
    matrixObject->get_member(NSV::PROP_D, &d);

    // Construct the matrix
    boost::numeric::ublas::c_matrix<double, 2, 2> transformMatrix;
    transformMatrix(0, 0) = a.to_number();
    transformMatrix(0, 1) = b.to_number();
    transformMatrix(1, 0) = c.to_number();
    transformMatrix(1, 1) = d.to_number();

    // Construct the point
    PointType point;
    point(0) = x.to_number();
    point(1) = y.to_number();

#ifdef GNASH_DEBUG_GEOM_MATRIX
    log_debug("(Matrix.{delta}TransformPoint) This matrix: %s", transformMatrix);
    log_debug("(Matrix.{delta}TransformPoint) Point vector (pre-transform): %s", point);
#endif

    // Transform
    point = boost::numeric::ublas::prod(point, transformMatrix);

#ifdef GNASH_DEBUG_GEOM_MATRIX
    log_debug("(Matrix.{delta}TransformPoint) Point vector (post-transform): %s", point);
#endif

    return point;

}

// A helper function to create a boost matrix from a Matrix object
static void fillMatrix(MatrixType& matrix,
                         as_object& matrixObject)
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

    matrix(0, 0) = a.to_number();
    matrix(0, 1) = c.to_number();
    matrix(0, 2) = tx.to_number();
    matrix(1, 0) = b.to_number();
    matrix(1, 1) = d.to_number();
    matrix(1, 2) = ty.to_number();
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
Matrix_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new Matrix_as;
    
    as_value a, b, c, d, tx, ty;

    if (fn.nargs == 0)
    {
        a.set_double(1);
        b.set_double(0);
        c.set_double(0);
        d.set_double(1);
        tx.set_double(0);
        ty.set_double(0);
    }
    else
    {
        switch (fn.nargs)
        {
            default:
                IF_VERBOSE_ASCODING_ERRORS(
                    std::ostringstream ss;
                    fn.dump_args(ss);
                    log_aserror("Matrix(%s): discarding extra arguments", ss.str());
                );
            case 6:
                ty = fn.arg(5);
            case 5:
                tx = fn.arg(4);
            case 4:
                d = fn.arg(3);
            case 3:
                c = fn.arg(2);
            case 2:
                b = fn.arg(1);
            case 1:
                a = fn.arg(0);
                break;
        }
    
    }

    obj->set_member(NSV::PROP_TY, ty);
    obj->set_member(NSV::PROP_TX, tx);
    obj->set_member(NSV::PROP_D, d);
    obj->set_member(NSV::PROP_C, c);
    obj->set_member(NSV::PROP_B, b);
    obj->set_member(NSV::PROP_A, a);

    return as_value(obj.get()); // will keep alive
}


static as_value
get_flash_geom_matrix_constructor(const fn_call& fn)
{
    log_debug("Loading flash.geom.Matrix class");
    Global_as* gl = getGlobal(fn);
    return gl->createClass(&Matrix_ctor, getMatrixInterface());
}

// extern 
void matrix_class_init(as_object& where, const ObjectURI& uri)
{
    // This is going to be the Matrix "class"/"function"
    // in the 'where' package
    string_table& st = getStringTable(where);
    
    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
    where.init_destructive_property(st.find("Matrix"),
            get_flash_geom_matrix_constructor, flags);
}

} // end of gnash namespace
