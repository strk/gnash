// Selection.cpp:  Selectable graphical things, for Gnash.
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

#include "Selection.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "Object.h" // for getObjectInterface

// For getting and setting focus
#include "VM.h"
#include "movie_root.h"

namespace gnash {

namespace {    
    as_value selection_addlistener(const fn_call& fn);
    as_value selection_getbeginindex(const fn_call& fn);
    as_value selection_getcaretindex(const fn_call& fn);
    as_value selection_getendindex(const fn_call& fn);
    as_value selection_getfocus(const fn_call& fn);
    as_value selection_removelistener(const fn_call& fn);
    as_value selection_setfocus(const fn_call& fn);
    as_value selection_setselection(const fn_call& fn);

    as_object* getSelectionInterface();
    void attachSelectionInterface(as_object& o);
}


// extern (used by Global.cpp)
void
selection_class_init(as_object& global)
{
	// Selection is NOT a class, but a simple object, see Selection.as

	static boost::intrusive_ptr<as_object> obj = 
        new as_object(getObjectInterface());
	attachSelectionInterface(*obj);
	global.init_member("Selection", obj.get());

}

namespace {

void
attachSelectionInterface(as_object& o)
{
	o.init_member("addListener", new builtin_function(selection_addlistener));
	o.init_member("getBeginIndex",
            new builtin_function(selection_getbeginindex));
	o.init_member("getCaretIndex",
            new builtin_function(selection_getcaretindex));
	o.init_member("getEndIndex",
            new builtin_function(selection_getendindex));
	o.init_member("getFocus",
            new builtin_function(selection_getfocus));
	o.init_member("removeListener",
            new builtin_function(selection_removelistener));
	o.init_member("setFocus", new builtin_function(selection_setfocus));
	o.init_member("setSelection", new builtin_function(selection_setselection));
}

as_object*
getSelectionInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object(getObjectInterface());
		attachSelectionInterface(*o);
	}
	return o.get();
}

as_value
selection_addlistener(const fn_call& /*fn*/) {
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}


as_value
selection_getbeginindex(const fn_call& /*fn*/) {
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}


as_value
selection_getcaretindex(const fn_call& /*fn*/) {
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}


as_value
selection_getendindex(const fn_call& /*fn*/) {
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

/// Returns null when there is no focus, otherwise the target of the
/// character.
as_value
selection_getfocus(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);
    
    movie_root& mr = ptr->getVM().getRoot();

    character* ch = mr.getFocus();
    if (!ch) {
        as_value null;
        null.set_null();
        return null;
    }

    return as_value(ch->getTarget());
}


as_value
selection_removelistener(const fn_call& /*fn*/) {
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}


// Documented to return true when setFocus succeeds, but that seems like the
// usual Adobe crap.
//
// TODO: clean this up when it's better tested.
//
// Returns true if the character can normally receive focus (TextField), false
// if it can't (MovieClip, any other object), regardless of whether focus
// was set or not.
//
// A MovieClip must have the focusEnabled property evaluate to true in order
// to receive focus.
//
// At least MovieClip behaves differently for SWF5, where focusEnabled
// is probably irrelevant and setFocus can return true for MovieClips.
// No idea what a TextField has to do to receive focus. 
//
// Button? Should be able to receive focus normally, so perhaps like TextField.
//
// Any number of arguments other than one returns false and does nothing. The
// single argument can be a character or a full target path.
as_value
selection_setfocus(const fn_call& fn)
{

    boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);

    /// Handle invalid arguments: must be one argument, or no action is
    /// taken.
    if (!fn.nargs || fn.nargs > 1) {
        IF_VERBOSE_ASCODING_ERRORS(
           log_aserror("Selection.setFocus: expected 1 argument, got %d",
               fn.nargs);
        );
        return as_value(false);
    }

    bool ret = false;

    movie_root& mr = ptr->getVM().getRoot();

    const as_value& focus = fn.arg(0);

    /// These should remove focus.
    if (focus.is_null() || focus.is_undefined()) {
        mr.setFocus(0);
        return as_value(ret);
    }

    character* ch;

    if (focus.is_string()) {
        const std::string& target = focus.to_string();
        ch = fn.env().find_target(target);
    }
    else if (!focus.is_object()) {
        // No action if it's not a string, undefined, null, or an object.
        return as_value(false);
    }
    else {
        /// Convert to character. If it's an object, but not a valid character,
        /// current focus is removed by passing 0 to movie_root::setFocus.
        ch = dynamic_cast<character*>(focus.to_object().get());
        log_debug("setFocus(%s)", ch->getTarget());
    }

    // Will handle whether to set focus or not.
    mr.setFocus(ch);

    return as_value(ret);
}


as_value
selection_setselection(const fn_call& /*fn*/) {
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

} // anonymous namespace
} // end of gnash namespace
