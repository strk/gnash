// Matrix_as.cpp:  ActionScript "Matrix" class, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "Matrix_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "Object.h" // for AS inheritance
#include "VM.h" // for addStatics
#include "Point_as.h" // Matrix needs to operate on Points.

#include <cmath>
#include <limits>
#include <boost/numeric/ublas/matrix.hpp> // boost matrix
#include <boost/numeric/ublas/io.hpp>
#include <sstream>
#include <memory> // std::auto_ptr

/// According to senocular, Flash docs get this wrong (b and c swapped).
/// However, most of the transformations only apply to a
/// subset of the elements, so it's unnecessary to use a full 3x3 matrix for
/// every operation.
///
// A transformation matrix for affine transformations:
//    a  c  tx
//    b  d  ty
//    0  0  1
// The matrix can only operate in 2d space.

namespace gnash {

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
static as_value Matrix_a_getset(const fn_call& fn);
static as_value Matrix_b_getset(const fn_call& fn);
static as_value Matrix_c_getset(const fn_call& fn);
static as_value Matrix_d_getset(const fn_call& fn);
static as_value Matrix_tx_getset(const fn_call& fn);
static as_value Matrix_ty_getset(const fn_call& fn);


as_value Matrix_ctor(const fn_call& fn);

static void
attachMatrixInterface(as_object& o)
{
    int fl = 0;

    o.init_member("clone", new builtin_function(Matrix_clone), fl);
    o.init_member("concat", new builtin_function(Matrix_concat), fl);
    o.init_member("createBox", new builtin_function(Matrix_createBox), fl);
    o.init_member("createGradientBox", new builtin_function(Matrix_createGradientBox), fl);
    o.init_member("deltaTransformPoint", new builtin_function(Matrix_deltaTransformPoint), fl);
    o.init_member("identity", new builtin_function(Matrix_identity), fl);
    o.init_member("invert", new builtin_function(Matrix_invert), fl);
    o.init_member("rotate", new builtin_function(Matrix_rotate), fl);
    o.init_member("scale", new builtin_function(Matrix_scale), fl);
    o.init_member("toString", new builtin_function(Matrix_toString), fl);
    o.init_member("transformPoint", new builtin_function(Matrix_transformPoint), fl);
    o.init_member("translate", new builtin_function(Matrix_translate), fl);
}

static void
attachMatrixStaticProperties(as_object& /*o*/)
{
   
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

    // override from as_object ?
    //std::string get_text_value() const { return "Matrix"; }

    // override from as_object ?
    //double get_numeric_value() const { return 0; }

};

as_function* getFlashGeomMatrixConstructor()
{
    static builtin_function* cl = NULL;
    if ( ! cl )
    {
        cl=new builtin_function(&Matrix_ctor, getMatrixInterface());
        VM::get().addStatic(cl);
        attachMatrixStaticProperties(*cl);
    }
    return cl;
}

/// Return an exact copy of the matrix.
static as_value
Matrix_clone(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);

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

static as_value
Matrix_concat(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

static as_value
Matrix_createBox(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

static as_value
Matrix_createGradientBox(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}


/// Transform the point using a Matrix. The translation
/// elements (tx, ty) do not have any effect.
///
/// Returns a new Point, leaving the object passed untouched.
static as_value
Matrix_deltaTransformPoint(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);

    if (fn.nargs != 1)
    {
        //log error
        return as_value();
    }

    const as_value& arg = fn.arg(0);
    
    if ( ! arg.is_object() )
    {
        /// Isn't an object...
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("Matrix.deltaTransformPoint(%s): needs an object", ss.str());
        );
        return as_value();
    }
    
    as_object* obj = arg.to_object().get();
    assert(obj);
    if ( ! obj->instanceOf(getFlashGeomPointConstructor()) )
    {
        /// Isn't a point.
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("Matrix.deltaTransformPoint(%s): object must be a Point", ss.str());
        );
        return as_value();
    }

    // Get the point co-ordinates.
    as_value x, y;
    
    obj->get_member(NSV::PROP_X, &x);
    obj->get_member(NSV::PROP_Y, &y);

    // Get the matrix elements to use as a transformation matrix.
    as_value a, b, c, d;

    ptr->get_member(NSV::PROP_A, &a);
    ptr->get_member(NSV::PROP_B, &b);
    ptr->get_member(NSV::PROP_C, &c);
    ptr->get_member(NSV::PROP_D, &d);

    // Construct the matrix
    boost::numeric::ublas::c_matrix<double, 2, 2> transformMatrix;
    transformMatrix(0, 0) = a.to_number();
    transformMatrix(0, 1) = b.to_number();
    transformMatrix(1, 0) = c.to_number();
    transformMatrix(1, 1) = d.to_number();

    // Construct the point
    boost::numeric::ublas::vector<double> point(2);
    point(0) = x.to_number();
    point(1) = y.to_number();

    // Transform
    point = boost::numeric::ublas::prod(point, transformMatrix);

    // Get an auto_ptr to a Point and keep alive.
    boost::intrusive_ptr<as_object> ret = init_Point_instance().release();
    ret->set_member(NSV::PROP_X, point(0));
    ret->set_member(NSV::PROP_Y, point(1));

    return as_value(ret.get());
}


/// Reset the matrix to a null-transformation
///
/// | 1   0   0 |
/// | 0   1   0 |
///
/// Returns void.
static as_value
Matrix_identity(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);

    ptr->set_member(NSV::PROP_A, as_value(1.0));
    ptr->set_member(NSV::PROP_B, as_value(0.0));
    ptr->set_member(NSV::PROP_C, as_value(0.0));
    ptr->set_member(NSV::PROP_D, as_value(1.0));
    ptr->set_member(NSV::PROP_TX, as_value(0.0));
    ptr->set_member(NSV::PROP_TY, as_value(0.0));

    return as_value();
}

static as_value
Matrix_invert(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

static as_value
Matrix_rotate(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);

    if (fn.nargs == 1)
    {
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
        
        // Apply rotation to current matrix.
        currentMatrix = boost::numeric::ublas::prod(currentMatrix, transformMatrix);

        log_debug("Modified matrix: %s", currentMatrix);
        log_debug("Transformation matrix%s", transformMatrix);
 
        ptr->set_member(NSV::PROP_A, as_value(currentMatrix(0, 0)));
        ptr->set_member(NSV::PROP_B, as_value(currentMatrix(0, 1)));
        ptr->set_member(NSV::PROP_C, as_value(currentMatrix(1, 0)));
        ptr->set_member(NSV::PROP_D, as_value(currentMatrix(1, 1)));

        // Do rotation separately.
        boost::numeric::ublas::vector<double> translation(2);
        translation(0) = tx.to_number();
        translation(1) = ty.to_number();
        
        translation = boost::numeric::ublas::prod(translation, transformMatrix);

        ptr->set_member(NSV::PROP_TX, as_value(translation(0)));
        ptr->set_member(NSV::PROP_TY, as_value(translation(1)));  
    }      

    return as_value();
}

static as_value
Matrix_scale(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);
    if (fn.nargs == 2)
    {
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
        
        log_debug("%s - %s", currentMatrix, transformMatrix);
        
        // Apply scale to current matrix.
        currentMatrix = boost::numeric::ublas::prod(currentMatrix, transformMatrix);

        ptr->set_member(NSV::PROP_A, as_value(currentMatrix(0, 0)));
        ptr->set_member(NSV::PROP_B, as_value(currentMatrix(0, 1)));
        ptr->set_member(NSV::PROP_C, as_value(currentMatrix(1, 0)));
        ptr->set_member(NSV::PROP_D, as_value(currentMatrix(1, 1)));

        // This is just a simple multiplication, so do it separately.
        ptr->set_member(NSV::PROP_TX, as_value(tx.to_number() * scaleX));
        ptr->set_member(NSV::PROP_TY, as_value(ty.to_number() * scaleY));  
    }      

    return as_value();
}

static as_value
Matrix_toString(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);

    as_value a, b, c, d, tx, ty;

    ptr->get_member(NSV::PROP_A, &a);
    ptr->get_member(NSV::PROP_B, &b);
    ptr->get_member(NSV::PROP_C, &c);
    ptr->get_member(NSV::PROP_D, &d);
    ptr->get_member(NSV::PROP_TX, &tx);
    ptr->get_member(NSV::PROP_TY, &ty);
    
    std::ostringstream ss;
    
    ss << "(a=" << a.to_string() << ", "
          "b="<< b.to_string() << ", "
          "c="<< c.to_string() << ", "
          "d="<< d.to_string() << ", "
          "tx="<< tx.to_string() << ", "
          "ty="<< ty.to_string() << ")";
    
    return as_value(ss.str());
}

static as_value
Matrix_transformPoint(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

static as_value
Matrix_translate(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);
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

static as_value
Matrix_a_getset(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);

    if ( ! fn.nargs ) // getter
    {
        as_value a;
        ptr->get_member(NSV::PROP_A, &a);
        return a;
    }
    else // setter
    {
        as_value a = fn.arg(0);
        ptr->set_member(NSV::PROP_A, a);
    }

    return as_value();
}

static as_value
Matrix_b_getset(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);

    if ( ! fn.nargs ) // getter
    {
        as_value b;
        ptr->get_member(NSV::PROP_B, &b);
        return b;
    }
    else // setter
    {
        as_value b = fn.arg(0);
        ptr->set_member(NSV::PROP_B, b);
    }

    return as_value();
}

static as_value
Matrix_c_getset(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);

    if ( ! fn.nargs ) // getter
    {
        as_value c;
        ptr->get_member(NSV::PROP_C, &c);
        return c;
    }
    else // setter
    {
        as_value c = fn.arg(0);
        ptr->set_member(NSV::PROP_C, c);
    }

    return as_value();
}

static as_value
Matrix_d_getset(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);

    if ( ! fn.nargs ) // getter
    {
        as_value d;
        ptr->get_member(NSV::PROP_D, &d);
        return d;
    }
    else // setter
    {
        as_value d = fn.arg(0);
        ptr->set_member(NSV::PROP_D, d);
    }

    return as_value();
}

static as_value
Matrix_tx_getset(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);

    if ( ! fn.nargs ) // getter
    {
        as_value tx;
        ptr->get_member(NSV::PROP_TX, &tx);
        return tx;
    }
    else // setter
    {
        as_value tx = fn.arg(0);
        ptr->set_member(NSV::PROP_TX, tx);
    }

    return as_value();
}

static as_value
Matrix_ty_getset(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);

    if ( ! fn.nargs ) // getter
    {
        as_value ty;
        ptr->get_member(NSV::PROP_TY, &ty);
        return ty;
    }
    else // setter
    {
        as_value ty = fn.arg(0);
        ptr->set_member(NSV::PROP_TY, ty);
    }

    return as_value();
}



as_value
Matrix_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new Matrix_as;
    
    as_value a, b, c, d, tx, ty;

    if ( ! fn.nargs )
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
                // Log as coding error
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
get_flash_geom_matrix_constructor(const fn_call& /*fn*/)
{
    log_debug("Loading flash.geom.Matrix class");

    return getFlashGeomMatrixConstructor();
}

// extern 
void Matrix_class_init(as_object& where)
{
    // This is going to be the Matrix "class"/"function"
    // in the 'where' package
    string_table& st = where.getVM().getStringTable();
    where.init_destructive_property(st.find("Matrix"), get_flash_geom_matrix_constructor);
}

} // end of gnash namespace
