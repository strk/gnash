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

#include "Stage.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function

namespace gnash {

void stage_addlistener(const fn_call& fn);
void stage_removelistener(const fn_call& fn);
void stage_ctor(const fn_call& fn);

static void
attachStageInterface(as_object& o)
{
	o.set_member("addListener", &stage_addlistener);
	o.set_member("removeListener", &stage_removelistener);
}

static as_object*
getStageInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object();
		attachStageInterface(*o);
	}
	return o.get();
}

class stage_as_object: public as_object
{

public:

	stage_as_object()
		:
		as_object(getStageInterface())
	{}

	// override from as_object ?
	//const char* get_text_value() const { return "Stage"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }
};

void stage_addlistener(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}
void stage_removelistener(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}

void
stage_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = new stage_as_object;
	
	fn.result->set_as_object(obj.get()); // will keep alive
}

// extern (used by Global.cpp)
void stage_class_init(as_object& global)
{

	static boost::intrusive_ptr<as_object> obj = new as_object();
	attachStageInterface(*obj);
	global.set_member("Stage", obj.get());

#if 0 // Stage is NOT a class, but a simple object, see Stage.as

	// This is going to be the global Stage "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&stage_ctor, getStageInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attachStageInterface(*cl);
		     
	}

	// Register _global.Stage
	global.set_member("Stage", cl.get());
#endif

}


} // end of gnash namespace

