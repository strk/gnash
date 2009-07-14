// DisplacementMapFilter_as.cpp:  ActionScript "DisplacementMapFilter" class, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "Object.h" // for AS inheritance
#include "VM.h" // for addStatics

#include <sstream>

namespace gnash {

static as_value DisplacementMapFilter_clone(const fn_call& fn);
static as_value DisplacementMapFilter_alpha_getset(const fn_call& fn);
static as_value DisplacementMapFilter_color_getset(const fn_call& fn);
static as_value DisplacementMapFilter_componentX_getset(const fn_call& fn);
static as_value DisplacementMapFilter_componentY_getset(const fn_call& fn);
static as_value DisplacementMapFilter_mapBitmap_getset(const fn_call& fn);
static as_value DisplacementMapFilter_mapPoint_getset(const fn_call& fn);
static as_value DisplacementMapFilter_mode_getset(const fn_call& fn);
static as_value DisplacementMapFilter_scaleX_getset(const fn_call& fn);
static as_value DisplacementMapFilter_scaleY_getset(const fn_call& fn);


as_value DisplacementMapFilter_ctor(const fn_call& fn);

static void
attachDisplacementMapFilterInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    o.init_member("clone", gl->createFunction(DisplacementMapFilter_clone));
    o.init_property("alpha", DisplacementMapFilter_alpha_getset, DisplacementMapFilter_alpha_getset);
    o.init_property("color", DisplacementMapFilter_color_getset, DisplacementMapFilter_color_getset);
    o.init_property("componentX", DisplacementMapFilter_componentX_getset, DisplacementMapFilter_componentX_getset);
    o.init_property("componentY", DisplacementMapFilter_componentY_getset, DisplacementMapFilter_componentY_getset);
    o.init_property("mapBitmap", DisplacementMapFilter_mapBitmap_getset, DisplacementMapFilter_mapBitmap_getset);
    o.init_property("mapPoint", DisplacementMapFilter_mapPoint_getset, DisplacementMapFilter_mapPoint_getset);
    o.init_property("mode", DisplacementMapFilter_mode_getset, DisplacementMapFilter_mode_getset);
    o.init_property("scaleX", DisplacementMapFilter_scaleX_getset, DisplacementMapFilter_scaleX_getset);
    o.init_property("scaleY", DisplacementMapFilter_scaleY_getset, DisplacementMapFilter_scaleY_getset);
}

static void
attachDisplacementMapFilterStaticProperties(as_object& /*o*/)
{
}

static as_object*
getDisplacementMapFilterInterface()
{
	static boost::intrusive_ptr<as_object> o;

	if ( ! o )
	{
		// TODO: check if this class should inherit from Object
		//       or from a different class
		o = new as_object(getObjectInterface());
		VM::get().addStatic(o.get());

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

static as_value
DisplacementMapFilter_alpha_getset(const fn_call& fn)
{
	boost::intrusive_ptr<DisplacementMapFilter_as> ptr = ensureType<DisplacementMapFilter_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
DisplacementMapFilter_color_getset(const fn_call& fn)
{
	boost::intrusive_ptr<DisplacementMapFilter_as> ptr = ensureType<DisplacementMapFilter_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
DisplacementMapFilter_componentX_getset(const fn_call& fn)
{
	boost::intrusive_ptr<DisplacementMapFilter_as> ptr = ensureType<DisplacementMapFilter_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
DisplacementMapFilter_componentY_getset(const fn_call& fn)
{
	boost::intrusive_ptr<DisplacementMapFilter_as> ptr = ensureType<DisplacementMapFilter_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
DisplacementMapFilter_mapBitmap_getset(const fn_call& fn)
{
	boost::intrusive_ptr<DisplacementMapFilter_as> ptr = ensureType<DisplacementMapFilter_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
DisplacementMapFilter_mapPoint_getset(const fn_call& fn)
{
	boost::intrusive_ptr<DisplacementMapFilter_as> ptr = ensureType<DisplacementMapFilter_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
DisplacementMapFilter_mode_getset(const fn_call& fn)
{
	boost::intrusive_ptr<DisplacementMapFilter_as> ptr = ensureType<DisplacementMapFilter_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
DisplacementMapFilter_scaleX_getset(const fn_call& fn)
{
	boost::intrusive_ptr<DisplacementMapFilter_as> ptr = ensureType<DisplacementMapFilter_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
DisplacementMapFilter_scaleY_getset(const fn_call& fn)
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

// extern 
void displacementmapfilter_class_init(as_object& where)
{
	// This is going to be the DisplacementMapFilter "class"/"function"
	// in the 'where' package
	boost::intrusive_ptr<as_object> cl;
        Global_as* gl = getGlobal(where);
        cl = gl->createClass(&DisplacementMapFilter_ctor, getDisplacementMapFilterInterface());;
	attachDisplacementMapFilterStaticProperties(*cl);

	// Register _global.DisplacementMapFilter
	where.init_member("DisplacementMapFilter", cl.get());
}

} // end of gnash namespace
