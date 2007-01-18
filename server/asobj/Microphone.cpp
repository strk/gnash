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

#include "Microphone.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function

namespace gnash {

void microphone_get(const fn_call& fn);
void microphone_setgain(const fn_call& fn);
void microphone_setrate(const fn_call& fn);
void microphone_setsilencelevel(const fn_call& fn);
void microphone_setuseechosuppression(const fn_call& fn);
void microphone_ctor(const fn_call& fn);

static void
attachMicrophoneInterface(as_object& o)
{
	o.init_member("get", &microphone_get);
	o.init_member("setgain", &microphone_setgain);
	o.init_member("setrate", &microphone_setrate);
	o.init_member("setsilencelevel", &microphone_setsilencelevel);
	o.init_member("setuseechosuppression", &microphone_setuseechosuppression);
}

static as_object*
getMicrophoneInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object();
		attachMicrophoneInterface(*o);
	}
	return o.get();
}

class microphone_as_object: public as_object
{

public:

	microphone_as_object()
		:
		as_object(getMicrophoneInterface())
	{}

	// override from as_object ?
	//const char* get_text_value() const { return "Microphone"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }
};

void microphone_get(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}
void microphone_setgain(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}
void microphone_setrate(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}
void microphone_setsilencelevel(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}
void microphone_setuseechosuppression(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}

void
microphone_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = new microphone_as_object;
	
	fn.result->set_as_object(obj.get()); // will keep alive
}

// extern (used by Global.cpp)
void microphone_class_init(as_object& global)
{
	// This is going to be the global Microphone "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&microphone_ctor, getMicrophoneInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attachMicrophoneInterface(*cl);
		     
	}

	// Register _global.Microphone
	global.init_member("Microphone", cl.get());

}


} // end of gnash namespace

