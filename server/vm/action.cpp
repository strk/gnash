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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "action.h"
#include "as_object.h"
#include "log.h"
#include "movie_definition.h"
#include "MovieClipLoader.h"
#include "as_function.h"
#include "timers.h"
#include "textformat.h"
#include "sound_definition.h"
#include "array.h"
#include "types.h"
#include "sprite_instance.h"
#include "movie_instance.h"
//#include "movie_root.h" // to reset root movie from attach_extern_movie
#include "Global.h"
#include "swf.h"
#include "URL.h"
#include "GnashException.h"
#include "as_environment.h"
#include "fn_call.h"
#include "VM.h"
#include "StringPredicates.h"
#include "xml.h"
#include "xmlsocket.h"
#include "namedStrings.h"


#include <typeinfo>
#include <string>
#include <algorithm>

using namespace gnash;
using namespace SWF;

#if defined(_WIN32) || defined(WIN32)
#define snprintf _snprintf
#endif // _WIN32

// NOTES:
//
// Buttons
// on (press)                 onPress
// on (release)               onRelease
// on (releaseOutside)        onReleaseOutside
// on (rollOver)              onRollOver
// on (rollOut)               onRollOut
// on (dragOver)              onDragOver
// on (dragOut)               onDragOut
// on (keyPress"...")         onKeyDown, onKeyUp      <----- IMPORTANT
//
// Sprites
// onClipEvent (load)         onLoad
// onClipEvent (unload)       onUnload                Hm.
// onClipEvent (enterFrame)   onEnterFrame
// onClipEvent (mouseDown)    onMouseDown
// onClipEvent (mouseUp)      onMouseUp
// onClipEvent (mouseMove)    onMouseMove
// onClipEvent (keyDown)      onKeyDown
// onClipEvent (keyUp)        onKeyUp
// onClipEvent (data)         onData

// Text fields have event handlers too!

// Sprite built in methods:
// play()
// stop()
// gotoAndStop()
// gotoAndPlay()
// nextFrame()
// startDrag()
// getURL()
// getBytesLoaded()
// getBytesTotal()

// Built-in functions: (do these actually exist in the VM, or are they just opcodes?)
// Number()
// String()


// TODO builtins
//
// Number.toString() -- takes an optional arg that specifies the base
//
// Boolean() type cast
//
// typeof operator --> "number", "string", "boolean", "object" (also
// for arrays), "null", "movieclip", "function", "undefined"
//
// Number.MAX_VALUE, Number.MIN_VALUE
//
// String.fromCharCode()

namespace gnash {

//
// action stuff
//

// Statics.
bool	s_inited = false;

void register_component(const std::string& name, as_c_function_ptr handler)
{
	as_object* global = VM::get().getGlobal();
	global->set_member(VM::get().getStringTable().find(name), handler);
}

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
			char buf[256];
			snprintf(buf, 256, _("Attempt to call a value which is neither a C nor an ActionScript function (%s)"),
				method.to_debug_string().c_str());
			buf[255] = '\0';
		
			throw ActionException(buf);
		}
	}
	catch (ActionException& ex)
	{
		assert(val.is_undefined());
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("%s", ex.what());
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

// Printf-like vararg interface for calling ActionScript.
// Handy for external binding.
const char*	call_method_parsed(
    as_environment* env,
    as_object* this_ptr,
    const char* method_name,
    const char* method_arg_fmt,
    va_list args)
{
    log_debug(_("FIXME(%d): %s"), __LINE__, __FUNCTION__);

    // Parse va_list args
    int	starting_index = env->get_top_index();
    const char* p = method_arg_fmt;
    for (;; p++)
	{
	    char	c = *p;
	    if (c == 0)
		{
		    // End of args.
		    break;
		}
	    else if (c == '%')
		{
		    p++;
		    c = *p;
		    // Here's an arg.
		    if (c == 'd')
			{
			    // Integer.
			    env->push(va_arg(args, int));
			}
		    else if (c == 'f')
			{
			    // Double
			    env->push(va_arg(args, double));
			}
		    else if (c == 's')
			{
			    // String
			    env->push(va_arg(args, const char *));
			}
		    else if (c == 'l')
			{
			    p++;
			    c = *p;
			    if (c == 's')
				{
				    // Wide string.
				    env->push(va_arg(args, const wchar_t *));
				}
			    else
				{
				    log_error(_("call_method_parsed('%s','%s') -- invalid fmt '%%l%c'"),
					      method_name,
					      method_arg_fmt,
					      c);
				}
			}
		    else
			{
			    // Invalid fmt, warn.
			    log_error(_("call_method_parsed('%s','%s') -- invalid fmt '%%%c'"),
				      method_name,
				      method_arg_fmt,
				      c);
			}
		}
	    else
		{
		    // Ignore whitespace and commas.
		    if (c == ' ' || c == '\t' || c == ',')
			{
			    // OK
			}
		    else
			{
			    // Invalid arg; warn.
			    log_error(_("call_method_parsed('%s','%s') -- invalid char '%c'"),
				      method_name,
				      method_arg_fmt,
				      c);
			}
		}
	}

    as_value	method = env->get_variable(method_name);

    // check method

    // Reverse the order of pushed args
    int	nargs = env->get_top_index() - starting_index;
    for (int i = 0; i < (nargs >> 1); i++)
	{
	    int	i0 = starting_index + 1 + i;
	    int	i1 = starting_index + nargs - i;
	    assert(i0 < i1);

	    std::swap(env->bottom(i0), env->bottom(i1));
	}

    // Do the call.
    as_value	result = call_method(method, env, this_ptr, nargs, env->get_top_index());
    env->drop(nargs);

    // Return pointer to static string for return value.
    static std::string	s_retval;
    s_retval = result.to_string();
    return s_retval.c_str();
}

void
movie_load()
{
    log_action(_("-- start movie"));
}

//
// Built-in objects
//


as_value
event_test(const fn_call& /*fn*/)
{
    log_debug(_("FIXME: %s"), __FUNCTION__);
    return as_value();
}


//
// global init
//



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
		"onRelease_Outside",	 // RELEASE_OUTSIDE
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
		// These are for the MoveClipLoader ActionScript only
		"onLoadStart",		 // LOAD_START
		"onLoadError",		 // LOAD_ERROR
		"onLoadProgress",	 // LOAD_PROGRESS
		"onLoadInit",		 // LOAD_INIT
		// These are for the XMLSocket ActionScript only
		"onSockClose",		 // CLOSE
		"onSockConnect",	 // CONNECT
		"onSockData",		 // Data
		"onSockXML",		 // XML
		// These are for the XML ActionScript only
		"onXMLLoad",		 // XML_LOAD
		"onXMLData",		 // XML_DATA
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
		// These are for the MoveClipLoader ActionScript only
		NSV::PROP_ON_LOAD_START,	// LOAD_START
		NSV::PROP_ON_LOAD_ERROR,	// LOAD_ERROR
		NSV::PROP_ON_LOAD_PROGRESS,	// LOAD_PROGRESS
		NSV::PROP_ON_LOAD_INIT,		// LOAD_INIT
		// These are for the XMLSocket ActionScript only
		NSV::PROP_ON_SOCK_CLOSE,	// CLOSE
		NSV::PROP_ON_SOCK_CONNECT,	// CONNECT
		NSV::PROP_ON_SOCK_DATA,		// Data
		NSV::PROP_ON_SOCK_XML,		// XML
		// These are for the XML ActionScript only
		NSV::PROP_ON_XML_LOAD,		// XML_LOAD
		NSV::PROP_ON_XML_DATA,		// XML_DATA
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

} // end of namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
