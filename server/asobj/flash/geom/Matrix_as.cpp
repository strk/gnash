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

#include <sstream>

namespace gnash {

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
    o.init_member("clone", new builtin_function(Matrix_clone));
    o.init_member("concat", new builtin_function(Matrix_concat));
    o.init_member("createBox", new builtin_function(Matrix_createBox));
    o.init_member("createGradientBox", new builtin_function(Matrix_createGradientBox));
    o.init_member("deltaTransformPoint", new builtin_function(Matrix_deltaTransformPoint));
    o.init_member("identity", new builtin_function(Matrix_identity));
    o.init_member("invert", new builtin_function(Matrix_invert));
    o.init_member("rotate", new builtin_function(Matrix_rotate));
    o.init_member("scale", new builtin_function(Matrix_scale));
    o.init_member("toString", new builtin_function(Matrix_toString));
    o.init_member("transformPoint", new builtin_function(Matrix_transformPoint));
    o.init_member("translate", new builtin_function(Matrix_translate));
    o.init_property("a", Matrix_a_getset, Matrix_a_getset);
    o.init_property("b", Matrix_b_getset, Matrix_b_getset);
    o.init_property("c", Matrix_c_getset, Matrix_c_getset);
    o.init_property("d", Matrix_d_getset, Matrix_d_getset);
    o.init_property("tx", Matrix_tx_getset, Matrix_tx_getset);
    o.init_property("ty", Matrix_ty_getset, Matrix_ty_getset);
}

static void
attachMatrixStaticProperties(as_object& o)
{
   
}

static as_object*
getMatrixInterface()
{
	boost::intrusive_ptr<as_object> o;
	// TODO: check if this class should inherit from Object
	//       or from a different class
	o = new as_object(getObjectInterface());
	attachMatrixInterface(*o);
	return o.get();
}

class Matrix_as: public as_object
{

public:

	Matrix_as()
		:
		as_object(getMatrixInterface())
	{}

	// override from as_object ?
	//std::string get_text_value() const { return "Matrix"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }
};


static as_value
Matrix_clone(const fn_call& fn)
{
	boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
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

static as_value
Matrix_deltaTransformPoint(const fn_call& fn)
{
	boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Matrix_identity(const fn_call& fn)
{
	boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
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
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Matrix_scale(const fn_call& fn)
{
	boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Matrix_toString(const fn_call& fn)
{
	boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
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
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Matrix_a_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Matrix_b_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Matrix_c_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Matrix_d_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Matrix_tx_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Matrix_ty_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Matrix_as> ptr = ensureType<Matrix_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}



as_value
Matrix_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = new Matrix_as;

	if ( fn.nargs )
	{
		std::stringstream ss;
		fn.dump_args(ss);
		LOG_ONCE( log_unimpl("Matrix(%s): %s", ss.str(), _("arguments discarded")) );
	}

	return as_value(obj.get()); // will keep alive
}

// extern 
void Matrix_class_init(as_object& where)
{
	// This is going to be the Matrix "class"/"function"
	// in the 'where' package
	boost::intrusive_ptr<builtin_function> cl;
	cl=new builtin_function(&Matrix_ctor, getMatrixInterface());
	attachMatrixStaticProperties(*cl);

	// Register _global.Matrix
	where.init_member("Matrix", cl.get());
}

} // end of gnash namespace
