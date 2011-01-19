// Stage_as.cpp:  All the world is one, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#include "Stage_as.h"
#include "movie_root.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "NativeFunction.h" // for ActionException
#include "VM.h"
#include "AsBroadcaster.h" // for initializing self as a broadcaster
#include "namedStrings.h"
#include "StringPredicates.h"

#include <string>

namespace gnash {

namespace {
    as_value stage_scalemode(const fn_call& fn);
    as_value stage_align(const fn_call& fn);
    as_value stage_showMenu(const fn_call& fn);
    as_value stage_width(const fn_call& fn);
    as_value stage_height(const fn_call& fn);
    as_value stage_displaystate(const fn_call& fn);
    const char* getScaleModeString(movie_root::ScaleMode sm);
    const char* getDisplayStateString(movie_root::DisplayState ds);
    void attachStageInterface(as_object& o);
}

// extern (used by Global.cpp)
void
stage_class_init(as_object& where, const ObjectURI& uri)
{
    as_object* obj = registerBuiltinObject(where, attachStageInterface, uri);
    AsBroadcaster::initialize(*obj);
}

void
registerStageNative(as_object& o)
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

namespace {

void
attachStageInterface(as_object& o)
{
    o.init_property("scaleMode", &stage_scalemode, &stage_scalemode);
    o.init_property("align", &stage_align, &stage_align);
    o.init_property("width", &stage_width, &stage_width);
    o.init_property("height", &stage_height, &stage_height);
    o.init_property("showMenu", &stage_showMenu, &stage_showMenu);
    o.init_property("displayState", &stage_displaystate, &stage_displaystate);

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

    movie_root& m = getRoot(fn);
    
    if (!fn.nargs) {
        return as_value(getScaleModeString(m.getStageScaleMode()));
    }
    
    // Defaults to showAll if the string is invalid.
    movie_root::ScaleMode mode = movie_root::SCALEMODE_SHOWALL;

    const int version = getSWFVersion(fn);
    const std::string& str = fn.arg(0).to_string(version);
    
    StringNoCaseEqual noCaseCompare;
    
    if (noCaseCompare(str, "noScale")) mode = movie_root::SCALEMODE_NOSCALE;
    else if (noCaseCompare(str, "exactFit")) {
        mode = movie_root::SCALEMODE_EXACTFIT;
    }
    else if (noCaseCompare(str, "noBorder")) {
        mode = movie_root::SCALEMODE_NOBORDER;
    }

    m.setStageScaleMode(mode);
    return as_value();
}

as_value
stage_width(const fn_call& fn)
{
    // setter
    if ( fn.nargs > 0 ) {
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

    // setter
    if ( fn.nargs > 0 ) {
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
    movie_root& m = getRoot(fn);
    
    if (!fn.nargs) {
        return as_value (m.getStageAlignMode());
    }
    
    const int version = getSWFVersion(fn);
    const std::string& str = fn.arg(0).to_string(version);
    
    const short am = stringToStageAlign(str);
    
    m.setStageAlignment(am);
    
    return as_value();
}

as_value
stage_showMenu(const fn_call& fn)
{

    movie_root& m = getRoot(fn);

    if (!fn.nargs) {
        return as_value(m.getShowMenuState());
    }
    
    LOG_ONCE(log_unimpl("Stage.showMenu implemented by setting gnashrc "
                "option and for gtk only"));

    const bool state = toBool(fn.arg(0), getVM(fn));
    
    m.setShowMenuState(state);
    return as_value();
}

as_value
stage_displaystate(const fn_call& fn)
{

    movie_root& m = getRoot(fn);

    if (!fn.nargs) {
        return getDisplayStateString(m.getStageDisplayState());
    }
    
    StringNoCaseEqual noCaseCompare;

    const int version = getSWFVersion(fn);
    const std::string& str = fn.arg(0).to_string(version);

    if (noCaseCompare(str, "normal")) {
        m.setStageDisplayState(movie_root::DISPLAYSTATE_NORMAL);
    }
    else if (noCaseCompare(str, "fullScreen")) {
        m.setStageDisplayState(movie_root::DISPLAYSTATE_FULLSCREEN);
    }

    // If invalid, do nothing.
    return as_value();
}

} // anonymous namespace 

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
