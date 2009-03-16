// Accessibility_as.cpp:  ActionScript "Accessibility" class, for Gnash.
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

#include "Accessibility_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "Object.h" // for AS inheritance
#include "VM.h" // for addStatics

#include <sstream>

namespace gnash {

static as_value Accessibility_isActive(const fn_call& fn);
static as_value Accessibility_updateProperties(const fn_call& fn);
static as_value Accessibility_sendEvent(const fn_call& fn);

as_value Accessibility_ctor(const fn_call& fn);

static void
attachAccessibilityInterface(as_object& o)
{
    const int flags = as_prop_flags::dontDelete
                | as_prop_flags::readOnly;

    o.init_member("isActive", new builtin_function(Accessibility_isActive), flags);
    o.init_member("updateProperties", new builtin_function(Accessibility_updateProperties), flags);
    o.init_member("sendEvent", new builtin_function(Accessibility_sendEvent), flags);
}


static as_value
Accessibility_isActive(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Accessibility_updateProperties(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
Accessibility_sendEvent(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

// extern 
void Accessibility_class_init(as_object& where)
{
	// This is going to be the Accessibility "class"/"function"
	// in the 'where' package
	boost::intrusive_ptr<as_object> obj = new as_object(getObjectInterface());
	attachAccessibilityInterface(*obj);

	// Register _global.Accessibility
	where.init_member("Accessibility", obj.get());
}

} // end of gnash namespace
