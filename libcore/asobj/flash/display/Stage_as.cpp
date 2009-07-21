// Stage_as.cpp:  All the world is one, for Gnash.
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

#include "display/Stage_as.h"
#include "movie_root.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "VM.h"
#include "Object.h" // for getObjectInterface()
#include "AsBroadcaster.h" // for initializing self as a broadcaster
#include "namedStrings.h"
#include "StringPredicates.h"

#include <string>

namespace gnash {

static as_value stage_scalemode(const fn_call& fn);
static as_value stage_align(const fn_call& fn);
static as_value stage_showMenu(const fn_call& fn);
static as_value stage_width(const fn_call& fn);
static as_value stage_height(const fn_call& fn);
static as_value stage_displaystate(const fn_call& fn);
static const char* getScaleModeString(movie_root::ScaleMode sm);
static const char* getDisplayStateString(movie_root::DisplayState ds);

void registerStageNative(as_object& o)
{
	VM& vm = getVM(o);
	
	vm.registerNative(stage_scalemode, 666, 1);
    vm.registerNative(stage_scalemode, 666, 2);
    vm.registerNative(stage_align, 666, 3);
	vm.registerNative(stage_align, 666, 4);
	vm.registerNative(stage_width, 666, 5);
    vm.registerNative(stage_width, 666, 6);
	vm.registerNative(stage_height, 666, 7);
    vm.registerNative(stage_height, 666, 8);
	vm.registerNative(stage_showMenu, 666, 9);
    vm.registerNative(stage_showMenu, 666, 10);
}

static void
attachStageInterface(as_object& o)
{
    const int version = getSWFVersion(o);

    if ( version < 5 ) return;

    o.init_property("scaleMode", &stage_scalemode, &stage_scalemode);
    o.init_property("align", &stage_align, &stage_align);
    o.init_property("width", &stage_width, &stage_width);
    o.init_property("height", &stage_height, &stage_height);
    o.init_property("showMenu", &stage_showMenu, &stage_showMenu);
    o.init_property("displayState", &stage_displaystate, &stage_displaystate);

}


Stage_as::Stage_as()
	:
	as_object(getObjectInterface())
{
	attachStageInterface(*this);

	const int swfversion = getSWFVersion(*this);
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


const char*
getDisplayStateString(movie_root::DisplayState ds)
{
	static const char* displayStateName[] = {
		"normal",
		"fullScreen" };

	return displayStateName[ds];
}


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


as_value
stage_scalemode(const fn_call& fn)
{

    boost::intrusive_ptr<as_object> obj=ensureType<as_object>(fn.this_ptr);

    movie_root& m = getRoot(fn);

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

as_value
stage_width(const fn_call& fn)
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
    movie_root& m = getRoot(fn);
    return as_value(m.getStageWidth());
}

as_value
stage_height(const fn_call& fn)
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
    movie_root& m = getRoot(fn);
    return as_value(m.getStageHeight());
}


as_value
stage_align(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj=ensureType<as_object>(fn.this_ptr); 
 
    movie_root& m = getRoot(fn);
    
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

as_value
stage_showMenu(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);

    movie_root& m = getRoot(fn);

	if ( fn.nargs == 0 ) // getter
	{
		return as_value(m.getShowMenuState());
	}
	else // setter
	{
		LOG_ONCE(log_unimpl("Stage.showMenu implemented by setting gnashrc option and for gtk only"));

		bool state = fn.arg(0).to_bool();
		
		m.setShowMenuState( state );
		return as_value();
	}
}

as_value
stage_displaystate(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj=ensureType<as_object>(fn.this_ptr);

    movie_root& m = getRoot(fn);

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
void stage_class_init(as_object& global, const ObjectURI& uri)
{
	static boost::intrusive_ptr<as_object> obj = new Stage_as();
	global.init_member(getName(uri), obj.get(), as_object::DefaultFlags,
            getNamespace(uri));
}

} // end of gnash namespace
