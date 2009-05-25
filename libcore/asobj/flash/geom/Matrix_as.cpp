// Matrix_as.cpp:  ActionScript "Matrix" class, for Gnash.
//
//   Copyright (C) 2009 Free Software Foundation, Inc.
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

#include "geom/Matrix_as.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
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
    as_value matrix_ctor(const fn_call& fn);
    void attachMatrixInterface(as_object& o);
    void attachMatrixStaticInterface(as_object& o);
    as_object* getMatrixInterface();

}

// extern (used by Global.cpp)
void matrix_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&matrix_ctor, getMatrixInterface());
        attachMatrixStaticInterface(*cl);
    }

    // Register _global.Matrix
    global.init_member("Matrix", cl.get());
}

namespace {

void
attachMatrixInterface(as_object& o)
{
    o.init_member("concat", new builtin_function(matrix_concat));
    o.init_member("createBox", new builtin_function(matrix_createBox));
    o.init_member("createGradientBox", new builtin_function(matrix_createGradientBox));
    o.init_member("deltaTransformPoint", new builtin_function(matrix_deltaTransformPoint));
    o.init_member("identity", new builtin_function(matrix_identity));
    o.init_member("invert", new builtin_function(matrix_invert));
    o.init_member("rotate", new builtin_function(matrix_rotate));
    o.init_member("scale", new builtin_function(matrix_scale));
    o.init_member("toString", new builtin_function(matrix_toString));
    o.init_member("transformPoint", new builtin_function(matrix_transformPoint));
    o.init_member("translate", new builtin_function(matrix_translate));
}

void
attachMatrixStaticInterface(as_object& o)
{

}

as_object*
getMatrixInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachMatrixInterface(*o);
    }
    return o.get();
}

as_value
matrix_concat(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr =
        ensureType<Matrix_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
matrix_createBox(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr =
        ensureType<Matrix_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
matrix_createGradientBox(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr =
        ensureType<Matrix_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
matrix_deltaTransformPoint(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr =
        ensureType<Matrix_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
matrix_identity(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr =
        ensureType<Matrix_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
matrix_invert(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr =
        ensureType<Matrix_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
matrix_rotate(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr =
        ensureType<Matrix_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
matrix_scale(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr =
        ensureType<Matrix_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
matrix_toString(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr =
        ensureType<Matrix_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
matrix_transformPoint(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr =
        ensureType<Matrix_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
matrix_translate(const fn_call& fn)
{
    boost::intrusive_ptr<Matrix_as> ptr =
        ensureType<Matrix_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
matrix_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new Matrix_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

