// Camera_as.cpp:  ActionScript "Camera" class, for Gnash.
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

#include "flash/media/Camera_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "Object.h" // for getObjectInterface

namespace gnash {

as_value camera_get(const fn_call& fn);
as_value camera_setmode(const fn_call& fn);
as_value camera_setmotionlevel(const fn_call& fn);
as_value camera_setquality(const fn_call& fn);
as_value camera_ctor(const fn_call& fn);

static void
attachCameraInterface(as_object& o)
{
	o.init_member("get", new builtin_function(camera_get));
	o.init_member("setmode", new builtin_function(camera_setmode));
	o.init_member("setmotionlevel", new builtin_function(camera_setmotionlevel));
	o.init_member("setquality", new builtin_function(camera_setquality));
}

static as_object*
getCameraInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object(getObjectInterface());
		attachCameraInterface(*o);
	}
	return o.get();
}

class camera_as_object: public as_object
{

public:

	camera_as_object()
		:
		as_object(getCameraInterface())
	{}

	// override from as_object ?
	//const char* get_text_value() const { return "Camera"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }
};

as_value camera_get(const fn_call& /*fn*/) {
    log_unimpl (__FUNCTION__);
    return as_value();
}
as_value camera_setmode(const fn_call& /*fn*/) {
    log_unimpl (__FUNCTION__);
    return as_value();
}
as_value camera_setmotionlevel(const fn_call& /*fn*/) {
    log_unimpl (__FUNCTION__);
    return as_value();
}
as_value camera_setquality(const fn_call& /*fn*/) {
    log_unimpl (__FUNCTION__);

    return as_value();
}

as_value
camera_ctor(const fn_call& /* fn */)
{
	boost::intrusive_ptr<as_object> obj = new camera_as_object;
	
	return as_value(obj.get()); // will keep alive
}

// extern (used by Global.cpp)
void camera_class_init(as_object& global)
{
	// This is going to be the global Camera "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&camera_ctor, getCameraInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attachCameraInterface(*cl);
		     
	}

	// Register _global.Camera
	global.init_member("Camera", cl.get());

}


} // end of gnash namespace

