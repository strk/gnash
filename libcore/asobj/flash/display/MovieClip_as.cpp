// EventDispatcher.cpp:  Implementation of ActionScript MovieClip class, for Gnash.
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

#include "smart_ptr.h"
#include "fn_call.h"
#include "as_object.h" // for inheritance
#include "builtin_function.h" // need builtin_function
#include "Sprite_as.h"

#include "log.h"

#include <string>
#include <sstream>

namespace gnash {
class movie_clip_as_object : public as_object
{

public:

	movie_clip_as_object()
		:
		as_object()
	{
	}

};

static as_value
movie_clip_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = new movie_clip_as_object();
	
	return as_value(obj.get()); // will keep alive
}

as_object*
getMovieClipInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object(getSpriteAsInterface());
	}
	return o.get();
}

// extern
void movie_clip_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

	cl=new builtin_function(&movie_clip_ctor, getMovieClipInterface());
	// Register _global.MovieClip
	global.init_member("MovieClip", cl.get());

}

std::auto_ptr<as_object>
init_movie_clip_instance()
{
	return std::auto_ptr<as_object>(new movie_clip_as_object);
}


}
