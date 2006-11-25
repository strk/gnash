// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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
#include "config.h"
#endif

#include "Boolean.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function

namespace gnash {

void boolean_tostring(const fn_call& fn);
void boolean_valueof(const fn_call& fn);
void boolean_ctor(const fn_call& fn);

static void
attachBooleanInterface(as_object& o)
{
	o.set_member("toString", &boolean_tostring);
	o.set_member("valueOf", &boolean_valueof);
}

static as_object*
getBooleanInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object();
		attachBooleanInterface(*o);
	}
	return o.get();
}

class boolean_as_object: public as_object
{

public:

	boolean_as_object()
		:
		as_object(getBooleanInterface())
	{}

	boolean_as_object(bool _val)
		:
		as_object(getBooleanInterface())
	{
		val = _val;
	}
	
	// override from as_object ?
	//const char* get_text_value() const { return "Boolean"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }	
	
	bool val;
	
};

void boolean_tostring(const fn_call& fn) {

	static char* strtrue = "true";
	static char* strfalse = "false";

	boolean_as_object* boolobj = (boolean_as_object*) (as_object*) fn.this_ptr;
	
	if (boolobj->val) 
		fn.result->set_string(strtrue);
	else
		fn.result->set_string(strfalse);
}
void boolean_valueof(const fn_call& fn) {
    boolean_as_object* boolobj = (boolean_as_object*) (as_object*) fn.this_ptr;
    
    fn.result->set_bool(boolobj->val);
}

void
boolean_ctor(const fn_call& fn)
{
	bool val = false;
	if (fn.nargs > 0)
	{
		val = fn.arg(0).to_bool();
	}
	boost::intrusive_ptr<as_object> obj = new boolean_as_object(val);

	fn.result->set_as_object(obj.get()); // will keep alive
}

// extern (used by Global.cpp)
void boolean_class_init(as_object& global)
{
	// This is going to be the global Boolean "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&boolean_ctor, getBooleanInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attachBooleanInterface(*cl);
		     
	}

	// Register _global.Boolean
	global.set_member("Boolean", cl.get());

}


} // end of gnash namespace

