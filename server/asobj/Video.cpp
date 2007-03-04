// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

#include "Video.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function

namespace gnash {

void video_attachvideo(const fn_call& fn);
void video_clear(const fn_call& fn);
void video_ctor(const fn_call& fn);

static void
attachVideoInterface(as_object& o)
{
	o.init_member("attachVideo", new builtin_function(video_attachvideo));
	o.init_member("clear", new builtin_function(video_clear));
}

static as_object*
getVideoInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object();
		attachVideoInterface(*o);
	}
	return o.get();
}

class video_as_object: public as_object
{

public:

	video_as_object()
		:
		as_object(getVideoInterface())
	{}

	// override from as_object ?
	//const char* get_text_value() const { return "Video"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }
};

void video_attachvideo(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}
void video_clear(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}

void
video_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = new video_as_object;
	
	fn.result->set_as_object(obj.get()); // will keep alive
}

// extern (used by Global.cpp)
void video_class_init(as_object& global)
{
	// This is going to be the global Video "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&video_ctor, getVideoInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attachVideoInterface(*cl);
		     
	}

	// Register _global.Video
	global.init_member("Video", cl.get());

}


} // end of gnash namespace

