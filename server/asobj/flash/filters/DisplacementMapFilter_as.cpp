// DisplacementMapFilter_as.cpp:  ActionScript "DisplacementMapFilter" class, for Gnash.
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

#include "DisplacementMapFilter_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "flash/filters/BitmapFilter_as.h" // for AS inheritance

#include <sstream>

namespace gnash {

static as_value DisplacementMapFilter_clone(const fn_call& fn);
as_value DisplacementMapFilter_ctor(const fn_call& fn);

static void
attachDisplacementMapFilterInterface(as_object& o)
{
    o.init_member("clone", new builtin_function(DisplacementMapFilter_clone));
}

static as_object*
getDisplacementMapFilterInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object(bitmapFilter_interface());
		attachDisplacementMapFilterInterface(*o);
	}
	return o.get();
}

class DisplacementMapFilter_as: public as_object
{

public:

	DisplacementMapFilter_as()
		:
		as_object(getDisplacementMapFilterInterface())
	{}

	// override from as_object ?
	//std::string get_text_value() const { return "DisplacementMapFilter"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }
};


static as_value
DisplacementMapFilter_clone(const fn_call& fn)
{
	boost::intrusive_ptr<DisplacementMapFilter_as> ptr = ensureType<DisplacementMapFilter_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
DisplacementMapFilter_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = new DisplacementMapFilter_as;

	if ( fn.nargs )
	{
		std::stringstream ss;
		fn.dump_args(ss);
		LOG_ONCE( log_unimpl("DisplacementMapFilter(%s): %s", ss.str(), _("arguments discarded")) );
	}

	return as_value(obj.get()); // will keep alive
}

// extern (used by Global.cpp)
void DisplacementMapFilter_class_init(as_object& global)
{
	// This is going to be the global DisplacementMapFilter "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&DisplacementMapFilter_ctor, getDisplacementMapFilterInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attachDisplacementMapFilterInterface(*cl);
	}

	// Register _global.DisplacementMapFilter
	global.init_member("DisplacementMapFilter", cl.get());
}

} // end of gnash namespace
