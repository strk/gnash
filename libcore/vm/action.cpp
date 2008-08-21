// action.cpp:  ActionScript execution, for Gnash.
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


#include "action.h"
#include "as_object.h"
#include "log.h"
#include "as_function.h"
#include "types.h"
#include "swf.h"
#include "GnashException.h"
#include "as_environment.h"
#include "fn_call.h"
#include "VM.h"
#include "StringPredicates.h"
#include "namedStrings.h"

#include <typeinfo>
#include <string>
#include <algorithm>
#include <boost/format.hpp>


namespace gnash {

//
// action stuff
//

//
// Function/method dispatch.
//

as_value
call_method(
    const as_value& method,
    as_environment* env,
    as_object* this_ptr, // this is ourself
    int nargs,
    int first_arg_bottom_index,
    as_object* super)
    // first_arg_bottom_index is the stack index, from the bottom,
    // of the first argument.
    // Subsequent arguments are at *lower* indices.
    // E.g. if first_arg_bottom_index = 7, then arg1 is at env->bottom(7),
    // arg2 is at env->bottom(6), etc.
{
	as_value val;
	fn_call call(this_ptr, env, nargs, first_arg_bottom_index, super);

	try
	{
		if ( as_function* as_func = method.to_as_function() )
		{
		    // It's an ActionScript function.  Call it.
		    val = (*as_func)(call);
		}
		else
		{
			boost::format fmt =
			            boost::format(_("Attempt to call a value which is neither a "
			                            "C nor an ActionScript function (%s)")) % method;
			throw ActionTypeError(fmt.str());
		}
	}
	catch (ActionTypeError& e)
	{
		assert(val.is_undefined());
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("%s", e.what());
		);
	}

	return val;
}


as_value	call_method0(
    const as_value& method,
    as_environment* env,
    as_object* this_ptr)
{
    return call_method(method, env, this_ptr, 0, env->get_top_index() + 1);
}


//
// event_id
//

const std::string&
event_id::get_function_name() const
{
	// TODO: use a case-insensitive matching
	static std::string s_function_names[EVENT_COUNT] =
	{
		"INVALID",		 // INVALID
		"onPress",		 // PRESS
		"onRelease",		 // RELEASE
		"onReleaseOutside",	 // RELEASE_OUTSIDE
		"onRollOver",		 // ROLL_OVER
		"onRollOut",		 // ROLL_OUT
		"onDragOver",		 // DRAG_OVER
		"onDragOut",		 // DRAG_OUT
		"onKeyPress",		 // KEY_PRESS
		"onInitialize",		 // INITIALIZE
		"onLoad",		 // LOAD
		"onUnload",		 // UNLOAD
		"onEnterFrame",		 // ENTER_FRAME
		"onMouseDown",		 // MOUSE_DOWN
		"onMouseUp",		 // MOUSE_UP
		"onMouseMove",		 // MOUSE_MOVE
		"onKeyDown",		 // KEY_DOWN
		"onKeyUp",		 // KEY_UP
		"onData",		 // DATA
		"onLoadStart",		 // LOAD_START
		"onLoadError",		 // LOAD_ERROR
		"onLoadProgress",	 // LOAD_PROGRESS
		"onLoadInit",		 // LOAD_INIT
		"onClose",		 // CLOSE
		"onConnect",	 // CONNECT
		"onXML",		 // XML
		"onTimer",	         // setInterval Timer expired
		"onConstruct",
		"onSetFocus",
		"onKillFocus"
	};

	assert(m_id > INVALID && m_id < EVENT_COUNT);
	return s_function_names[m_id];
}

string_table::key
event_id::get_function_key() const
{
	// TODO: use a case-insensitive matching
	static string_table::key function_keys[EVENT_COUNT] =
	{
		0,				// INVALID
		NSV::PROP_ON_PRESS,		// PRESS
		NSV::PROP_ON_RELEASE,		// RELEASE
		NSV::PROP_ON_RELEASE_OUTSIDE,	// RELEASE_OUTSIDE
		NSV::PROP_ON_ROLL_OVER,		// ROLL_OVER
		NSV::PROP_ON_ROLL_OUT,		// ROLL_OUT
		NSV::PROP_ON_DRAG_OVER,		// DRAG_OVER
		NSV::PROP_ON_DRAG_OUT,		// DRAG_OUT
		NSV::PROP_ON_KEY_PRESS,		// KEY_PRESS
		NSV::PROP_ON_INITIALIZE,	// INITIALIZE
		NSV::PROP_ON_LOAD,		// LOAD
		NSV::PROP_ON_UNLOAD,		// UNLOAD
		NSV::PROP_ON_ENTER_FRAME,	// ENTER_FRAME
		NSV::PROP_ON_MOUSE_DOWN,	// MOUSE_DOWN
		NSV::PROP_ON_MOUSE_UP,		// MOUSE_UP
		NSV::PROP_ON_MOUSE_MOVE,	//  MOUSE_MOVE
		NSV::PROP_ON_KEY_DOWN,		// KEY_DOWN
		NSV::PROP_ON_KEY_UP,		// KEY_UP
		NSV::PROP_ON_DATA,		// DATA
		NSV::PROP_ON_LOAD_START,	// LOAD_START
		NSV::PROP_ON_LOAD_ERROR,	// LOAD_ERROR
		NSV::PROP_ON_LOAD_PROGRESS,	// LOAD_PROGRESS
		NSV::PROP_ON_LOAD_INIT,		// LOAD_INIT
		NSV::PROP_ON_CLOSE,	// CLOSE
		NSV::PROP_ON_CONNECT,	// CONNECT
		NSV::PROP_ON_XML,		// XML
		NSV::PROP_ON_TIMER,		// setInterval Timer expired
		NSV::PROP_ON_CONSTRUCT,		// onConstruct
		NSV::PROP_ON_SET_FOCUS, 	// onSetFocus
		NSV::PROP_ON_KILL_FOCUS 	// onKillFocus
	};

	assert(m_id > INVALID && m_id < EVENT_COUNT);
	return function_keys[m_id];
}

bool
event_id::is_mouse_event() const
{
	switch (m_id)
	{
		case event_id::PRESS:
		case event_id::RELEASE:
		case event_id::RELEASE_OUTSIDE:
		case event_id::MOUSE_UP:
		case event_id::MOUSE_DOWN:
		case event_id::ROLL_OVER:
		case event_id::ROLL_OUT:
		case event_id::DRAG_OVER:
		case event_id::DRAG_OUT:
			return true;
		default:
			return false;
	}
}

bool
event_id::is_key_event() const
{
	switch (m_id)
	{
		case event_id::KEY_DOWN:
		case event_id::KEY_PRESS :
		case event_id::KEY_UP:
			return true;
		default:
			return false;
	}
}

bool
event_id::is_button_event() const
{
	switch (m_id)
	{
		case event_id::PRESS:
		case event_id::RELEASE :
		case event_id::RELEASE_OUTSIDE:
		case event_id::ROLL_OVER:
		case event_id::ROLL_OUT:
		case event_id::DRAG_OVER:
		case event_id::DRAG_OUT:
		case event_id::KEY_PRESS:
			return true;
		default:
			return false;
	}
}

std::ostream& operator<< (std::ostream& o, const event_id& ev)
{
    return (o << ev.get_function_name());
}

} // end of namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
