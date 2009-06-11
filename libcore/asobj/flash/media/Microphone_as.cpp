// Microphone_as.cpp:  ActionScript "Microphone" class, for Gnash.
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

#include "flash/media/Microphone_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "Object.h" // for getObjectInterface

namespace gnash {

as_value microphone_get(const fn_call& fn);
as_value microphone_setgain(const fn_call& fn);
as_value microphone_setrate(const fn_call& fn);
as_value microphone_setsilencelevel(const fn_call& fn);
as_value microphone_setuseechosuppression(const fn_call& fn);
as_value microphone_ctor(const fn_call& fn);

static void
attachMicrophoneInterface(as_object& o)
{
	o.init_member("get", new builtin_function(microphone_get));
	o.init_member("setGain", new builtin_function(microphone_setgain));
	o.init_member("setRate", new builtin_function(microphone_setrate));
	o.init_member("setSilenceLevel", new builtin_function(microphone_setsilencelevel));
	o.init_member("setUseEchoSuppression", new builtin_function(microphone_setuseechosuppression));
}

static as_object*
getMicrophoneInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object(getObjectInterface());
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
	//std::string get_text_value() const { return "Microphone"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }
};

as_value microphone_get(const fn_call& /*fn*/) {
    log_unimpl (__FUNCTION__);
    return as_value();
}
as_value microphone_setgain(const fn_call& /*fn*/) {
    log_unimpl (__FUNCTION__);
    return as_value();
}
as_value microphone_setrate(const fn_call& /*fn*/) {
    log_unimpl (__FUNCTION__);
    return as_value();
}
as_value microphone_setsilencelevel(const fn_call& /*fn*/) {
    log_unimpl (__FUNCTION__);
    return as_value();
}
as_value microphone_setuseechosuppression(const fn_call& /*fn*/) {
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
microphone_ctor(const fn_call& /* fn */)
{
	boost::intrusive_ptr<as_object> obj = new microphone_as_object;
	
	return as_value(obj.get()); // will keep alive
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
