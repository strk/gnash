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

#include "Selection_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "Object.h" // for getObjectInterface
#include "array.h"
#include "AsBroadcaster.h"

// For getting and setting focus
#include "VM.h"
#include "movie_root.h"

namespace gnash {

namespace {    
    as_value selection_getbeginindex(const fn_call& fn);
    as_value selection_getcaretindex(const fn_call& fn);
    as_value selection_getendindex(const fn_call& fn);
    as_value selection_getfocus(const fn_call& fn);
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

    const int flags = as_prop_flags::dontEnum |
                      as_prop_flags::dontDelete |
                      as_prop_flags::readOnly;

	o.init_member("getBeginIndex",
            new builtin_function(selection_getbeginindex), flags);
	o.init_member("getCaretIndex",
            new builtin_function(selection_getcaretindex), flags);
	o.init_member("getEndIndex",
            new builtin_function(selection_getendindex), flags);
	o.init_member("getFocus",
            new builtin_function(selection_getfocus), flags);
	o.init_member("setFocus", new builtin_function(selection_setfocus), flags);
	o.init_member("setSelection",
            new builtin_function(selection_setselection), flags);

    /// Handles addListener, removeListener, and _listeners.
    AsBroadcaster::initialize(o);
 
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

    boost::intrusive_ptr<character> ch = mr.getFocus();
    if (!ch.get()) {
        as_value null;
        null.set_null();
        return null;
    }

    return as_value(ch->getTarget());
}


// Documented to return true when setFocus succeeds, but that seems like the
// usual Adobe crap.
//
// Returns true if focus is set to 0 (no focus), otherwise false. It is 
// irrelevant whether focus was set. 
//
// A MovieClip must have the focusEnabled property evaluate to true or at 
// least one mouse event handler defined in order to receive focus.
//
// TextFields can only receive focus if selectable (TODO: check this).
// Buttons are documented to be able to receive focus always.
//
// focusEnabled has no effect in SWF5.
//
// Any number of arguments other than one returns false and does nothing. The
// single argument can be a character or a full target path, otherwise it's
// a no-op and returns false.
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

    movie_root& mr = ptr->getVM().getRoot();

    const as_value& focus = fn.arg(0);

    /// These should remove focus.
    if (focus.is_null() || focus.is_undefined()) {
        mr.setFocus(0);
        return as_value(true);
    }

    boost::intrusive_ptr<character> ch;

    if (focus.is_string()) {
        const std::string& target = focus.to_string();
        ch = fn.env().find_target(target);
    }
    else {
        /// Try converting directly to character.
        ch = dynamic_cast<character*>(focus.to_object().get());
    }

    // If the argument does not resolve to a character, do nothing.
    if (!ch) return as_value(false);

    // Will handle whether to set focus or not.
    mr.setFocus(ch);

    return as_value(false);
}


as_value
selection_setselection(const fn_call& /*fn*/) {
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

} // anonymous namespace
} // end of gnash namespace
