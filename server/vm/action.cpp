// action.cpp:  ActionScript execution, for Gnash.
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

#include "action.h"
#include "as_object.h"
#include "log.h"
#include "tu_random.h"
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
#include "movie_root.h" // to reset root movie from attach_extern_movie
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


#include <typeinfo>
#include <string>

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
	global->set_member(name, handler);
}

void
attach_extern_movie(const char* c_url,
		const sprite_instance* target,
		const sprite_instance* root_movie)
{
	URL url(c_url);

	boost::intrusive_ptr<movie_definition> md ( create_library_movie(url) );
	if (md == NULL)
	{
	    log_error(_("can't create movie_definition for %s"), url.str().c_str());
	    return;
	}

	// TODO: stuff in an intrusive pointer ?
	boost::intrusive_ptr<sprite_instance> extern_movie;

	if (target == root_movie)
	{
		extern_movie = create_library_movie_inst(md.get());
		if (extern_movie == NULL)
		{
			log_error(_("can't create extern root sprite for %s"), url.str().c_str());
			return;
		}

		// It would be better if create_library_movie_inst() returned a movie_instance
		// directly !
		gnash::movie_instance* mi = dynamic_cast<movie_instance*>(extern_movie.get());
		VM::get().getRoot().setRootMovie(mi);
		sprite_instance* m = extern_movie->get_root_movie();

		m->on_event(event_id::LOAD);
	}
	else
	{
		extern_movie = md->create_instance();
		if (extern_movie == NULL)
		{
			log_error(_("can't create extern sprite for %s"), url.str().c_str());
			return;
		}

		save_extern_movie(extern_movie.get());

		const character* tar = (const character*)target;
		const char* name = tar->get_name().c_str();
		uint16_t depth = tar->get_depth();
		int ratio = tar->get_ratio();
		uint16_t clip_depth = tar->get_clip_depth();

		character* parent = tar->get_parent();
		boost::intrusive_ptr<sprite_instance> newsprite ( extern_movie->get_root_movie() );

		assert(parent);
		assert(newsprite);

		newsprite->set_parent(parent);

		sprite_instance* parent_sprite = parent->to_movie();
		assert(parent_sprite);

	    parent_sprite->replace_display_object(
		newsprite.get(),
		name,
		depth,
		NULL, // don't use color transform
		NULL, // don't use matrix transform
		ratio,
		clip_depth);
	}
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
    int first_arg_bottom_index)
    // first_arg_bottom_index is the stack index, from the bottom,
    // of the first argument.
    // Subsequent arguments are at *lower* indices.
    // E.g. if first_arg_bottom_index = 7, then arg1 is at env->bottom(7),
    // arg2 is at env->bottom(6), etc.
{
	as_value val;
	fn_call call(this_ptr, env, nargs, first_arg_bottom_index);

	try
	{
		if ( as_function* as_func = method.to_as_function() )
		{
		    // It's an ActionScript function.  Call it.
		    val = (*as_func)(call);
		}
		else
		{
			throw ActionException(_("Attempt to call a value which is neither a C nor an ActionScript function"));
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
    log_msg(_("FIXME(%d): %s"), __LINE__, __FUNCTION__);

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

	    swap(&(env->bottom(i0)), &(env->bottom(i1)));
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
    log_msg(_("FIXME: %s"), __FUNCTION__);
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

// Standard member lookup.
// TODO: move to character.h ?
// TODO: case-insensitive search ?
as_standard_member
get_standard_member(const std::string& name)
{
	typedef std::map< std::string, as_standard_member, StringNoCaseLessThen>  maptype; 

	static bool s_inited = false;
	static std::map< std::string, as_standard_member, StringNoCaseLessThen> membersMap;

	if (!s_inited)
	{
		s_inited = true;

		// worth reserving ?
		//membersMap.resize(int(AS_STANDARD_MEMBER_COUNT));

		membersMap["_x"] = M_X;
		membersMap["_y"] = M_Y;
		membersMap["_xscale"] = M_XSCALE;
		membersMap["_yscale"] = M_YSCALE;
		membersMap["_currentframe"] = M_CURRENTFRAME;
		membersMap["_totalframes"] = M_TOTALFRAMES;
		membersMap["_alpha"] = M_ALPHA;
		membersMap["_visible"] = M_VISIBLE;
		membersMap["_width"] = M_WIDTH;
		membersMap["_height"] = M_HEIGHT;
		membersMap["_rotation"] = M_ROTATION;
		membersMap["_target"] = M_TARGET;
		membersMap["_framesloaded"] = M_FRAMESLOADED;
		membersMap["_name"] = M_NAME;
		membersMap["_droptarget"] = M_DROPTARGET;
		membersMap["_url"] = M_URL;
		membersMap["_highquality"] = M_HIGHQUALITY;
		membersMap["_focusrect"] = M_FOCUSRECT;
		membersMap["_soundbuftime"] = M_SOUNDBUFTIME;
		membersMap["_xmouse"] = M_XMOUSE;
		membersMap["_ymouse"] = M_YMOUSE;
		membersMap["_parent"] = M_PARENT;
		membersMap["text"] = M_TEXT;
		membersMap["textWidth"] = M_TEXTWIDTH;
		membersMap["textColor"] = M_TEXTCOLOR;
		membersMap["onLoad"] = M_ONLOAD;
		membersMap["onRollOver"] = M_ONROLLOVER;
		membersMap["onRollOut"] = M_ONROLLOUT;
	}

	as_standard_member result;
	maptype::const_iterator it = membersMap.find(name);
	if ( it == membersMap.end() )
	{
		result = M_INVALID_MEMBER;
	} else {
		result = it->second;
	}

	return result;
}

} // end of namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
