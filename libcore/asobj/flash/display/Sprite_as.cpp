// EventDispatcher.cpp:  Implementation of ActionScript Sprite class, for Gnash.
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

#include "Object.h"
#include "smart_ptr.h"
#include "fn_call.h"
#include "as_object.h" // for inheritance
#include "builtin_function.h" // need builtin_function
#include "movie_definition.h" // for Object.registerClass (get_exported_resource)
//#include "character.h" // for Object.registerClass  (get_root_movie)
#include "sprite_instance.h" // for Object.registerClass  (get_movie_definition)
#include "sprite_definition.h" // for Object.registerClass  (get_movie_definition)
#include "VM.h" // for SWF version (attachObjectInterface)
#include "namedStrings.h" // for NSV::PROP_TO_STRING
#include "flash/display/DisplayObjectContainer_as.h"

#include "log.h"

#include <string>
#include <sstream>

namespace gnash {
class sprite_as_object : public as_object
{

public:

	sprite_as_object()
		:
		as_object()
	{
	}

};

static as_value
sprite_as_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = new sprite_as_object();
	
	return as_value(obj.get()); // will keep alive
}

as_object*
getSpriteAsInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object(getDisplayObjectContainerInterface());
	}
	return o.get();
}

// extern
void sprite_as_class_init(as_object& where)
{
    static boost::intrusive_ptr<builtin_function> cl;

	cl=new builtin_function(&sprite_as_ctor, getSpriteAsInterface());

	where.init_member("Sprite", cl.get());
}

std::auto_ptr<as_object>
init_sprite_as_instance()
{
	return std::auto_ptr<as_object>(new sprite_as_object);
}


}