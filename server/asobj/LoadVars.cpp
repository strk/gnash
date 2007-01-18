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

#include "LoadVars.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function

namespace gnash {

void loadvars_addrequestheader(const fn_call& fn);
void loadvars_decode(const fn_call& fn);
void loadvars_getbytesloaded(const fn_call& fn);
void loadvars_getbytestotal(const fn_call& fn);
void loadvars_load(const fn_call& fn);
void loadvars_send(const fn_call& fn);
void loadvars_sendandload(const fn_call& fn);
void loadvars_tostring(const fn_call& fn);
void loadvars_ctor(const fn_call& fn);

static void
attachLoadVarsInterface(as_object& o)
{
	o.init_member("addRequestHeader", &loadvars_addrequestheader);
	o.init_member("decode", &loadvars_decode);
	o.init_member("getBytesLoaded", &loadvars_getbytesloaded);
	o.init_member("getBytesTotal", &loadvars_getbytestotal);
	o.init_member("load", &loadvars_load);
	o.init_member("send", &loadvars_send);
	o.init_member("sendAndLoad", &loadvars_sendandload);
	o.init_member("toString", &loadvars_tostring);
}

static as_object*
getLoadVarsInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object();
		attachLoadVarsInterface(*o);
	}
	return o.get();
}

class loadvars_as_object: public as_object
{

public:

	loadvars_as_object()
		:
		as_object(getLoadVarsInterface())
	{}

	// override from as_object ?
	//const char* get_text_value() const { return "LoadVars"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }
};

void loadvars_addrequestheader(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}
void loadvars_decode(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}
void loadvars_getbytesloaded(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}
void loadvars_getbytestotal(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}
void loadvars_load(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}
void loadvars_send(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}
void loadvars_sendandload(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}
void loadvars_tostring(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}

void
loadvars_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = new loadvars_as_object;
	
	fn.result->set_as_object(obj.get()); // will keep alive
}

// extern (used by Global.cpp)
void loadvars_class_init(as_object& global)
{
	// This is going to be the global LoadVars "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&loadvars_ctor, getLoadVarsInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attachLoadVarsInterface(*cl);
		     
	}

	// Register _global.LoadVars
	global.init_member("LoadVars", cl.get());

}


} // end of gnash namespace

