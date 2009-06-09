// Stage_as.cpp:  ActionScript "Stage" class, for Gnash.
//
//   Copyright (C) 2009 Free Software Foundation, Inc.
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

#include "display/Stage_as.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

// ADDED
#include "movie_root.h"
#include "as_object.h" // for inheritance
#include "VM.h"
#include "Object.h" // for getObjectInterface()
#include "AsBroadcaster.h" // for initializing self as a broadcaster
#include "namedStrings.h"
#include "StringPredicates.h"

#include <string>
// END ADDED SECTION

namespace gnash {

static as_value stage_scalemode_getset(const fn_call& fn);
static as_value stage_align_getset(const fn_call& fn);
static as_value stage_showMenu_getset(const fn_call& fn);
static as_value stage_width_getset(const fn_call& fn);
static as_value stage_height_getset(const fn_call& fn);
static as_value stage_displaystate_getset(const fn_call& fn);
static const char* getScaleModeString(movie_root::ScaleMode sm);
static const char* getDisplayStateString(movie_root::DisplayState ds);
	
void registerStageNative(as_object& o)
{
	VM& vm = o.getVM();
	
	vm.registerNative(stage_scalemode_getset, 666, 1);
    vm.registerNative(stage_scalemode_getset, 666, 2);
    vm.registerNative(stage_align_getset, 666, 3);
	vm.registerNative(stage_align_getset, 666, 4);
	vm.registerNative(stage_width_getset, 666, 5);
    vm.registerNative(stage_width_getset, 666, 6);
	vm.registerNative(stage_height_getset, 666, 7);
    vm.registerNative(stage_height_getset, 666, 8);
	vm.registerNative(stage_showMenu_getset, 666, 9);
    vm.registerNative(stage_showMenu_getset, 666, 10);
}


// Forward declarations
namespace {
    as_value stage_addChildAt(const fn_call& fn);
    as_value stage_addEventListener(const fn_call& fn);
    as_value stage_dispatchEvent(const fn_call& fn);
    as_value stage_hasEventListener(const fn_call& fn);
    as_value stage_invalidate(const fn_call& fn);
    as_value stage_isFocusInaccessible(const fn_call& fn);
    as_value stage_removeChildAt(const fn_call& fn);
    as_value stage_setChildIndex(const fn_call& fn);
    as_value stage_swapChildrenAt(const fn_call& fn);
    as_value stage_willTrigger(const fn_call& fn);
    as_value stage_fullScreen(const fn_call& fn);
    as_value stage_mouseLeave(const fn_call& fn);
    as_value stage_resize(const fn_call& fn);
    as_value stage_ctor(const fn_call& fn);
    void attachStageInterface(as_object& o);
    void attachStageStaticInterface(as_object& o);
    as_object* getStageInterface();
    
// ADDED

}

Stage_as::Stage_as()
	:
	as_object(getObjectInterface())
{
	attachStageInterface(*this);

	const int swfversion = _vm.getSWFVersion();
	if ( swfversion > 5 )
	{
		AsBroadcaster::initialize(*this);
	}
}

void
Stage_as::notifyFullScreen(bool fs)
{
    // Should we notify resize here, or does movie_root do it anyway
    // when the gui changes size?
	log_debug("notifying Stage listeners about fullscreen state");
	callMethod(NSV::PROP_BROADCAST_MESSAGE, "onFullScreen", fs);
}


void
Stage_as::notifyResize()
{
	log_debug("notifying Stage listeners about a resize");
	callMethod(NSV::PROP_BROADCAST_MESSAGE, "onResize");
}

// ADDED
const char*
getDisplayStateString(movie_root::DisplayState ds)
{
	static const char* displayStateName[] = {
		"normal",
		"fullScreen" };

	return displayStateName[ds];
}

// ADDED
const char*
getScaleModeString(movie_root::ScaleMode sm)
{
	static const char* modeName[] = {
		"showAll",
		"noScale",
		"exactFit",
		"noBorder" };

	return modeName[sm];
}

// ADDED
as_value
stage_scalemode_getset(const fn_call& fn)
{

    boost::intrusive_ptr<as_object> obj=ensureType<as_object>(fn.this_ptr);

    movie_root& m = obj->getVM().getRoot();

	if ( fn.nargs == 0 ) // getter
	{
		return as_value(getScaleModeString(m.getStageScaleMode()));
	}
	else // setter
	{
	    // Defaults to showAll if the string is invalid.
		movie_root::ScaleMode mode = movie_root::showAll;

		const std::string& str = fn.arg(0).to_string();
		
		StringNoCaseEqual noCaseCompare;
		
		if ( noCaseCompare(str, "noScale") ) mode = movie_root::noScale;
		else if ( noCaseCompare(str, "exactFit") ) mode = movie_root::exactFit;
		else if ( noCaseCompare(str, "noBorder") ) mode = movie_root::noBorder;

        if ( m.getStageScaleMode() == mode ) return as_value(); // nothing to do

	    m.setStageScaleMode(mode);
		return as_value();
	}
}


// ADDED
as_value
stage_width_getset(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj=ensureType<as_object>(fn.this_ptr);

	if ( fn.nargs > 0 ) // setter
	{
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror(_("Stage.width is a read-only property!"));
		);
		return as_value();
	}

    // getter
    movie_root& m = obj->getVM().getRoot();
    return as_value(m.getStageWidth());
}

// ADDED
as_value
stage_height_getset(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj=ensureType<as_object>(fn.this_ptr);

	if ( fn.nargs > 0 ) // setter
	{
		IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Stage.height is a read-only property!"));
		);
		return as_value();
	}

    // getter
    movie_root& m = obj->getVM().getRoot();
    return as_value(m.getStageHeight());
}

// ADDED
as_value
stage_align_getset(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj=ensureType<as_object>(fn.this_ptr); 
 
    movie_root& m = obj->getVM().getRoot();
    
	if ( fn.nargs == 0 ) // getter
	{
	    return as_value (m.getStageAlignMode());
	}
	else // setter
	{
		const std::string& str = fn.arg(0).to_string();
        short am = 0;

        // Easy enough to do bitwise - std::bitset is not
        // really necessary!
        if (str.find_first_of("lL") != std::string::npos)
        {
            am |= 1 << movie_root::STAGE_ALIGN_L;
        } 

        if (str.find_first_of("tT") != std::string::npos)
        {
            am |= 1 << movie_root::STAGE_ALIGN_T;
        } 

        if (str.find_first_of("rR") != std::string::npos)
        {
            am |= 1 << movie_root::STAGE_ALIGN_R;
        } 
    
        if (str.find_first_of("bB") != std::string::npos)
        {
            am |= 1 << movie_root::STAGE_ALIGN_B;
        }

        m.setStageAlignment(am);

		return as_value();
	}
}

// ADDED
as_value
stage_showMenu_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Stage_as> stage = ensureType<Stage_as>(fn.this_ptr);

	if ( fn.nargs == 0 ) // getter
	{
		LOG_ONCE(log_unimpl("Stage.showMenu getter"));
		return as_value();
	}
	else // setter
	{
		LOG_ONCE(log_unimpl("Stage.showMenu setter"));
		return as_value();
	}
}

// ADDED
as_value
stage_displaystate_getset(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj=ensureType<as_object>(fn.this_ptr);

    movie_root& m = obj->getVM().getRoot();

	if (!fn.nargs) {
		return getDisplayStateString(m.getStageDisplayState());
	}

    StringNoCaseEqual noCaseCompare;

    const std::string& str = fn.arg(0).to_string();
    if (noCaseCompare(str, "normal")) {
        m.setStageDisplayState(movie_root::DISPLAYSTATE_NORMAL);
    }
    else if (noCaseCompare(str, "fullScreen")) {
        m.setStageDisplayState(movie_root::DISPLAYSTATE_FULLSCREEN);
    }

    // If invalid, do nothing.
    return as_value();
}

// extern (used by Global.cpp)
void stage_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&stage_ctor, getStageInterface());
        attachStageStaticInterface(*cl);
    }

    // Register _global.Stage
    global.init_member("Stage", cl.get());
}

namespace {

void
attachStageInterface(as_object& o)
{
	const int version = o.getVM().getSWFVersion();

    if ( version < 5 ) return;

    o.init_property("scaleMode", &stage_scalemode_getset, &stage_scalemode_getset);
    o.init_property("align", &stage_align_getset, &stage_align_getset);
    o.init_property("width", &stage_width_getset, &stage_width_getset);
    o.init_property("height", &stage_height_getset, &stage_height_getset);
    o.init_property("showMenu", &stage_showMenu_getset, &stage_showMenu_getset);
    o.init_property("displayState", &stage_displaystate_getset, &stage_displaystate_getset);


    o.init_member("addChildAt", new builtin_function(stage_addChildAt));
    o.init_member("addEventListener", new builtin_function(stage_addEventListener));
    o.init_member("dispatchEvent", new builtin_function(stage_dispatchEvent));
    o.init_member("hasEventListener", new builtin_function(stage_hasEventListener));
    o.init_member("invalidate", new builtin_function(stage_invalidate));
    o.init_member("isFocusInaccessible", new builtin_function(stage_isFocusInaccessible));
    o.init_member("removeChildAt", new builtin_function(stage_removeChildAt));
    o.init_member("setChildIndex", new builtin_function(stage_setChildIndex));
    o.init_member("swapChildrenAt", new builtin_function(stage_swapChildrenAt));
    o.init_member("willTrigger", new builtin_function(stage_willTrigger));
    o.init_member("fullScreen", new builtin_function(stage_fullScreen));
    o.init_member("mouseLeave", new builtin_function(stage_mouseLeave));
    o.init_member("resize", new builtin_function(stage_resize));
}

void
attachStageStaticInterface(as_object& o)
{

}

as_object*
getStageInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachStageInterface(*o);
    }
    return o.get();
}

as_value
stage_addChildAt(const fn_call& fn)
{
    boost::intrusive_ptr<Stage_as> ptr =
        ensureType<Stage_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stage_addEventListener(const fn_call& fn)
{
    boost::intrusive_ptr<Stage_as> ptr =
        ensureType<Stage_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stage_dispatchEvent(const fn_call& fn)
{
    boost::intrusive_ptr<Stage_as> ptr =
        ensureType<Stage_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stage_hasEventListener(const fn_call& fn)
{
    boost::intrusive_ptr<Stage_as> ptr =
        ensureType<Stage_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stage_invalidate(const fn_call& fn)
{
    boost::intrusive_ptr<Stage_as> ptr =
        ensureType<Stage_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stage_isFocusInaccessible(const fn_call& fn)
{
    boost::intrusive_ptr<Stage_as> ptr =
        ensureType<Stage_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stage_removeChildAt(const fn_call& fn)
{
    boost::intrusive_ptr<Stage_as> ptr =
        ensureType<Stage_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stage_setChildIndex(const fn_call& fn)
{
    boost::intrusive_ptr<Stage_as> ptr =
        ensureType<Stage_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stage_swapChildrenAt(const fn_call& fn)
{
    boost::intrusive_ptr<Stage_as> ptr =
        ensureType<Stage_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stage_willTrigger(const fn_call& fn)
{
    boost::intrusive_ptr<Stage_as> ptr =
        ensureType<Stage_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stage_fullScreen(const fn_call& fn)
{
    boost::intrusive_ptr<Stage_as> ptr =
        ensureType<Stage_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stage_mouseLeave(const fn_call& fn)
{
    boost::intrusive_ptr<Stage_as> ptr =
        ensureType<Stage_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stage_resize(const fn_call& fn)
{
    boost::intrusive_ptr<Stage_as> ptr =
        ensureType<Stage_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stage_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new Stage_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

