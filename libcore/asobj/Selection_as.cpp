// Selection.cpp:  Selectable graphical things, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "namedStrings.h"
#include "Selection_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "NativeFunction.h" 
#include "AsBroadcaster.h"
#include "TextField.h"

// For getting and setting focus
#include "VM.h"
#include "movie_root.h"

namespace gnash {

namespace {    
    as_value selection_getBeginIndex(const fn_call& fn);
    as_value selection_getCaretIndex(const fn_call& fn);
    as_value selection_getEndIndex(const fn_call& fn);
    as_value selection_getFocus(const fn_call& fn);
    as_value selection_setFocus(const fn_call& fn);
    as_value selection_setSelection(const fn_call& fn);

    void attachSelectionInterface(as_object& o);
}


// extern (used by Global.cpp)
void
selection_class_init(as_object& where, const ObjectURI& uri)
{
	// Selection is NOT a class, but a simple object, see Selection.as
    as_object* o = registerBuiltinObject(where, attachSelectionInterface,
            uri);

    /// Handles addListener, removeListener, and _listeners.
    AsBroadcaster::initialize(*o);

    // All properties are protected using ASSetPropFlags.
    Global_as& gl = getGlobal(where);
    as_object* null = 0;
    callMethod(&gl, NSV::PROP_AS_SET_PROP_FLAGS, o, null, 7);
}

void
registerSelectionNative(as_object& global)
{
    VM& vm = getVM(global);

    vm.registerNative(selection_getBeginIndex, 600, 0);
    vm.registerNative(selection_getEndIndex, 600, 1);
    vm.registerNative(selection_getCaretIndex, 600, 2);
    vm.registerNative(selection_getFocus, 600, 3);
    vm.registerNative(selection_setFocus, 600, 4);
    vm.registerNative(selection_setSelection, 600, 5);
}

namespace {

void
attachSelectionInterface(as_object& o)
{
    VM& vm = getVM(o);

    const int flags = PropFlags::dontEnum |
                      PropFlags::dontDelete |
                      PropFlags::readOnly;

	o.init_member("getBeginIndex", vm.getNative(600, 0), flags);
	o.init_member("getEndIndex", vm.getNative(600, 1), flags);
	o.init_member("getCaretIndex", vm.getNative(600, 2), flags); 
	o.init_member("getFocus", vm.getNative(600, 3), flags);
	o.init_member("setFocus", vm.getNative(600, 4), flags);
	o.init_member("setSelection", vm.getNative(600, 5), flags);
 
}

as_value
selection_getBeginIndex(const fn_call& fn)
{
    movie_root& mr = getRoot(fn);
    DisplayObject* focus = mr.getFocus();

    TextField* tf = dynamic_cast<TextField*>(focus);

    if (!tf) return as_value(-1);

    return as_value(tf->getSelection().first);

}

/// Return -1 if focus is not a TextField, otherwise the 0-based index of the
/// selection.
//
/// An alternative implementation would have a getCaretIndex in the DisplayObject
/// base class, with a default implementation returning -1. We would still
/// have to check for no-focus events here, though.
as_value
selection_getCaretIndex(const fn_call& fn)
{
    movie_root& mr = getRoot(fn);
    DisplayObject* focus = mr.getFocus();

    TextField* tf = dynamic_cast<TextField*>(focus);

    if (!tf) return as_value(-1);

    return as_value(tf->getCaretIndex());
}


as_value
selection_getEndIndex(const fn_call& fn)
{
    movie_root& mr = getRoot(fn);
    DisplayObject* focus = mr.getFocus();

    TextField* tf = dynamic_cast<TextField*>(focus);

    if (!tf) return as_value(-1);

    return as_value(tf->getSelection().second);
}

/// Returns null when there is no focus, otherwise the target of the
/// DisplayObject.
as_value
selection_getFocus(const fn_call& fn)
{
    movie_root& mr = getRoot(fn);

    DisplayObject* ch = mr.getFocus();
    if (!ch) {
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
// single argument can be a DisplayObject or a full target path, otherwise it's
// a no-op and returns false.
as_value
selection_setFocus(const fn_call& fn)
{

    /// Handle invalid arguments: must be one argument, or no action is
    /// taken.
    if (!fn.nargs || fn.nargs > 1) {
        IF_VERBOSE_ASCODING_ERRORS(
           log_aserror("Selection.setFocus: expected 1 argument, got %d",
               fn.nargs);
        );
        return as_value(false);
    }

    movie_root& mr = getRoot(fn);

    const as_value& focus = fn.arg(0);

    /// These should remove focus.
    if (focus.is_null() || focus.is_undefined()) {
        mr.setFocus(0);
        return as_value(true);
    }

    DisplayObject* ch;

    if (focus.is_string()) {
        const std::string& target = focus.to_string();
        ch = findTarget(fn.env(), target);
    }
    else {
        /// Try converting directly to DisplayObject.
        as_object* obj = toObject(focus, getVM(fn));
        ch = get<DisplayObject>(obj);
    }

    // If the argument does not resolve to a DisplayObject, do nothing.
    if (!ch) return as_value(false);

    // HACK FIXME ! This is an hack to succeed an swfdec testcase
    if (getSWFVersion(fn) >= 6) {
        // Will handle whether to set focus or not.
        mr.setFocus(ch);
    }

    return as_value(false);
}


as_value
selection_setSelection(const fn_call& fn)
{

    movie_root& mr = getRoot(fn);
    DisplayObject* focus = mr.getFocus();

    TextField* tf = dynamic_cast<TextField*>(focus);

    if (!tf) return as_value();

    if (fn.nargs != 2) {
        // Only two arguments are acceptable.
        return as_value();
    }

    int start = toInt(fn.arg(0), getVM(fn));
    int end = toInt(fn.arg(1), getVM(fn));

    tf->setSelection(start, end);

    return as_value();
}

} // anonymous namespace
} // end of gnash namespace
