// MovieClip_as.cpp:  ActionScript "MovieClip" class, for Gnash.
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

#include "MovieClip.h"
#include "Movie.h"
#include "display/MovieClip_as.h"
#include "display/DisplayObjectContainer_as.h"
#include "display/BitmapData_as.h"
#include "NetStream_as.h"
#include "movie_root.h"
#include "GnashNumeric.h"
#include "as_value.h"
#include "Object.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function

#include <boost/lexical_cast.hpp>

namespace gnash {

// Forward declarations
namespace {

    void attachMovieClipAS2Interface(as_object& o);

    as_value movieclip_as2_ctor(const fn_call& fn);
    as_value movieclip_transform(const fn_call& fn);
    as_value movieclip_scale9Grid(const fn_call& fn);
    as_value movieclip_attachVideo(const fn_call& fn);
    as_value movieclip_attachAudio(const fn_call& fn);
    as_value movieclip_attachMovie(const fn_call& fn);
    as_value movieclip_unloadMovie(const fn_call& fn);
    as_value movieclip_loadMovie(const fn_call& fn);
    as_value movieclip_getURL(const fn_call& fn);
    as_value movieclip_attachBitmap(const fn_call& fn);
    as_value movieclip_beginBitmapFill(const fn_call& fn);
    as_value movieclip_createEmptyMovieClip(const fn_call& fn);
    as_value movieclip_removeMovieClip(const fn_call& fn);
    as_value movieclip_createTextField(const fn_call& fn);
    as_value movieclip_curveTo(const fn_call& fn);
    as_value movieclip_beginFill(const fn_call& fn);
    as_value movieclip_prevFrame(const fn_call& fn);
    as_value movieclip_nextFrame(const fn_call& fn);
    as_value movieclip_endFill(const fn_call& fn);
    as_value movieclip_clear(const fn_call& fn);
    as_value movieclip_lineStyle(const fn_call& fn);
    as_value movieclip_lineTo(const fn_call& fn);
    as_value movieclip_moveTo(const fn_call& fn);
    as_value movieclip_beginGradientFill(const fn_call& fn);
    as_value movieclip_stopDrag(const fn_call& fn);
    as_value movieclip_startDrag(const fn_call& fn);
    as_value movieclip_gotoAndStop(const fn_call& fn);
    as_value movieclip_duplicateMovieClip(const fn_call& fn);
    as_value movieclip_gotoAndPlay(const fn_call& fn);
    as_value movieclip_stop(const fn_call& fn);
    as_value movieclip_play(const fn_call& fn);
    as_value movieclip_setMask(const fn_call& fn);
    as_value movieclip_getDepth(const fn_call& fn);
    as_value movieclip_getBytesTotal(const fn_call& fn);
    as_value movieclip_getBytesLoaded(const fn_call& fn);
    as_value movieclip_getBounds(const fn_call& fn);
    as_value movieclip_hitTest(const fn_call& fn);
    as_value movieclip_globalToLocal(const fn_call& fn);
    as_value movieclip_localToGlobal(const fn_call& fn);
    as_value movieclip_swapDepths(const fn_call& fn);
    as_value movieclip_scrollRect(const fn_call& fn);
    as_value movieclip_getInstanceAtDepth(const fn_call& fn);
    as_value movieclip_getNextHighestDepth(const fn_call& fn);
    as_value movieclip_getTextSnapshot(const fn_call& fn);
    as_value movieclip_tabIndex(const fn_call& fn);
    as_value movieclip_opaqueBackground(const fn_call& fn);
    as_value movieclip_filters(const fn_call& fn);
    as_value movieclip_forceSmoothing(const fn_call& fn);
    as_value movieclip_cacheAsBitmap(const fn_call& fn);
    as_value movieclip_lineGradientStyle(const fn_call& fn);
    as_value movieclip_getRect(const fn_call& fn);
    as_value movieclip_meth(const fn_call& fn);
    as_value movieclip_getSWFVersion(const fn_call& fn);
    as_value movieclip_loadVariables(const fn_call& fn);
    as_value movieclip_currentFrame(const fn_call& fn);
    as_value movieclip_totalFrames(const fn_call& fn);
    as_value movieclip_framesLoaded(const fn_call& fn);
    as_value movieclip_dropTarget(const fn_call& fn);
    as_value movieclip_url(const fn_call& fn);
    as_value movieclip_focusRect(const fn_call& fn);
    as_value movieclip_soundbuftime(const fn_call& fn);

    // =============================================
    // AS3 methods
    // =============================================

    void attachMovieClipAS3Interface(as_object& o);
    as_value movieclip_as3_ctor(const fn_call& fn);
    as_value movieclip_nextScene(const fn_call& fn);
    as_value movieclip_prevScene(const fn_call& fn);
    as_value movieclip_addFrameScript(const fn_call& fn);

}

// extern (used by Global.cpp)
void
movieclip_class_init(as_object& where, const ObjectURI& uri)
{
    if (isAS3(getVM(where))) {

        static boost::intrusive_ptr<as_object> cl =
            new as_object(getMovieClipAS3Interface());
        
        // TODO: fix AVM2Global::createClass to work for AVM2.
        Global_as* gl = getGlobal(where);
        cl->init_member(NSV::PROP_CONSTRUCTOR,
                gl->createFunction(movieclip_as3_ctor));

        log_debug("AVM2 MovieClip, proto %s", cl);

        where.init_member("MovieClip", cl);
        return;
    }

    static boost::intrusive_ptr<as_object> cl;

    if (!cl) {
        Global_as* gl = getGlobal(where);
        cl = gl->createClass(&movieclip_as2_ctor, getMovieClipAS2Interface());
        getVM(where).addStatic(cl.get());
    }

    where.init_member("MovieClip", cl.get());
}

as_object*
getMovieClipAS3Interface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = getDisplayObjectContainerInterface();
        attachMovieClipAS3Interface(*o);
    }
    return o.get();
}

void
registerMovieClipNative(as_object& global)
{
    VM& vm = getVM(global);

    // Natives are always here    (at least in swf5 I guess)
    vm.registerNative(movieclip_attachMovie, 900, 0); 
    // TODO: generalize to DisplayObject::swapDepths_method ?
    vm.registerNative(movieclip_swapDepths, 900, 1); 
    vm.registerNative(movieclip_localToGlobal, 900, 2);
    vm.registerNative(movieclip_globalToLocal, 900, 3);
    vm.registerNative(movieclip_hitTest, 900, 4);
    vm.registerNative(movieclip_getBounds, 900, 5);
    vm.registerNative(movieclip_getBytesTotal, 900, 6);
    vm.registerNative(movieclip_getBytesLoaded, 900, 7);
    vm.registerNative(movieclip_attachAudio, 900, 8);
    vm.registerNative(movieclip_attachVideo, 900, 9);
    // TODO: generalize to DisplayObject::getDepth_method ?
    vm.registerNative(movieclip_getDepth, 900, 10);
    vm.registerNative(movieclip_setMask, 900, 11); 
    vm.registerNative(movieclip_play, 900, 12); 
    vm.registerNative(movieclip_stop, 900, 13);
    vm.registerNative(movieclip_nextFrame, 900, 14);
    vm.registerNative(movieclip_prevFrame, 900, 15);
    vm.registerNative(movieclip_gotoAndPlay, 900, 16);
    vm.registerNative(movieclip_gotoAndStop, 900, 17);
    vm.registerNative(movieclip_duplicateMovieClip, 900, 18);
    vm.registerNative(movieclip_removeMovieClip, 900, 19);
    vm.registerNative(movieclip_startDrag, 900, 20);
    vm.registerNative(movieclip_stopDrag, 900, 21);
    vm.registerNative(movieclip_createEmptyMovieClip, 901, 0);
    vm.registerNative(movieclip_beginFill, 901, 1);
    vm.registerNative(movieclip_beginGradientFill, 901, 2);
    vm.registerNative(movieclip_moveTo, 901, 3);
    vm.registerNative(movieclip_lineTo, 901, 4);
    vm.registerNative(movieclip_curveTo, 901, 5);
    vm.registerNative(movieclip_lineStyle, 901, 6);
    vm.registerNative(movieclip_endFill, 901, 7);
    vm.registerNative(movieclip_clear, 901, 8);

    vm.registerNative(movieclip_createTextField, 104, 200);

}

/// Properties (and/or methods) attached to every *instance* of a MovieClip 
void
attachMovieClipAS2Properties(DisplayObject& o)
{

    // This is a normal property, can be overridden, deleted and enumerated
    // See swfdec/test/trace/movieclip-version-#.swf for why we only
    // initialize this if we don't have a parent
    if (!o.get_parent()) o.init_member("$version",
            getVM(o).getPlayerVersion(), 0); 

    as_c_function_ptr gettersetter;

    gettersetter = DisplayObject::x_getset;
    o.init_property(NSV::PROP_uX, gettersetter, gettersetter);

    gettersetter = DisplayObject::y_getset;
    o.init_property(NSV::PROP_uY, gettersetter, gettersetter);

    gettersetter = DisplayObject::xscale_getset;
    o.init_property(NSV::PROP_uXSCALE, gettersetter, gettersetter);

    gettersetter = DisplayObject::yscale_getset;
    o.init_property(NSV::PROP_uYSCALE, gettersetter, gettersetter);

    gettersetter = DisplayObject::xmouse_get;
    o.init_readonly_property(NSV::PROP_uXMOUSE, gettersetter);

    gettersetter = DisplayObject::ymouse_get;
    o.init_readonly_property(NSV::PROP_uYMOUSE, gettersetter);

    gettersetter = DisplayObject::alpha_getset;
    o.init_property(NSV::PROP_uALPHA, gettersetter, gettersetter);

    gettersetter = DisplayObject::visible_getset;
    o.init_property(NSV::PROP_uVISIBLE, gettersetter, gettersetter);

    gettersetter = DisplayObject::width_getset;
    o.init_property(NSV::PROP_uWIDTH, gettersetter, gettersetter);

    gettersetter = DisplayObject::height_getset;
    o.init_property(NSV::PROP_uHEIGHT, gettersetter, gettersetter);

    gettersetter = DisplayObject::rotation_getset;
    o.init_property(NSV::PROP_uROTATION, gettersetter, gettersetter);

    gettersetter = DisplayObject::parent_getset;
    o.init_property(NSV::PROP_uPARENT, gettersetter, gettersetter);

    gettersetter = movieclip_currentFrame;
    o.init_property(NSV::PROP_uCURRENTFRAME, gettersetter, gettersetter);

    gettersetter = movieclip_totalFrames;
    o.init_property(NSV::PROP_uTOTALFRAMES, gettersetter, gettersetter);

    gettersetter = movieclip_framesLoaded;
    o.init_property(NSV::PROP_uFRAMESLOADED, gettersetter, gettersetter);

    gettersetter = DisplayObject::target_getset;
    o.init_property(NSV::PROP_uTARGET, gettersetter, gettersetter);

    gettersetter = DisplayObject::name_getset;
    o.init_property(NSV::PROP_uNAME, gettersetter, gettersetter);

    gettersetter = movieclip_dropTarget;
    o.init_property(NSV::PROP_uDROPTARGET, gettersetter, gettersetter);

    gettersetter = movieclip_url;
    o.init_property(NSV::PROP_uURL, gettersetter, gettersetter);

    gettersetter = DisplayObject::quality;
    o.init_property(NSV::PROP_uQUALITY, gettersetter, gettersetter);
    
    gettersetter = DisplayObject::highquality;
    o.init_property(NSV::PROP_uHIGHQUALITY, gettersetter, gettersetter);

    gettersetter = movieclip_focusRect;
    o.init_property(NSV::PROP_uFOCUSRECT, gettersetter, gettersetter);

    gettersetter = movieclip_soundbuftime;
    o.init_property(NSV::PROP_uSOUNDBUFTIME, gettersetter, gettersetter);

}

as_object*
getMovieClipAS2Interface()
{
    static boost::intrusive_ptr<as_object> proto;
    if ( proto == NULL )
    {
        proto = new as_object(getObjectInterface());
        VM& vm = VM::get();
        vm.addStatic(proto.get());
        attachMovieClipAS2Interface(*proto);
    }
    return proto.get();
}


namespace {

// =======================
// AS2 interface
// =======================

/// Properties (and/or methods) *inherited* by MovieClip instances
void
attachMovieClipAS2Interface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    VM& vm = getVM(o);

    o.init_member("attachMovie", vm.getNative(900, 0)); 
    o.init_member("swapDepths", vm.getNative(900, 1));
    o.init_member("localToGlobal", vm.getNative(900, 2));
    o.init_member("globalToLocal", vm.getNative(900, 3));
    o.init_member("hitTest", vm.getNative(900, 4));
    o.init_member("getBounds", vm.getNative(900, 5));
    o.init_member("getBytesTotal", vm.getNative(900, 6));
    o.init_member("getBytesLoaded", vm.getNative(900, 7));
    o.init_member("play", vm.getNative(900, 12));
    o.init_member("stop", vm.getNative(900, 13));
    o.init_member("nextFrame", vm.getNative(900, 14));
    o.init_member("prevFrame", vm.getNative(900, 15));
    o.init_member("gotoAndPlay", vm.getNative(900, 16));
    o.init_member("gotoAndStop", vm.getNative(900, 17));
    o.init_member("duplicateMovieClip", vm.getNative(900, 18));
    o.init_member("removeMovieClip", vm.getNative(900, 19));
    o.init_member("startDrag", vm.getNative(900, 20));
    o.init_member("stopDrag", vm.getNative(900, 21));
    o.init_member("loadMovie", gl->createFunction(movieclip_loadMovie));
    o.init_member("loadVariables", gl->createFunction(
                movieclip_loadVariables));
    o.init_member("unloadMovie", gl->createFunction(
                movieclip_unloadMovie));
    o.init_member("getURL", gl->createFunction(movieclip_getURL));
    o.init_member("getSWFVersion", gl->createFunction(
                movieclip_getSWFVersion));
    o.init_member("meth", gl->createFunction(movieclip_meth));
    o.init_member("enabled", true);
    o.init_member("useHandCursor", true);
    o.init_property("_lockroot", &MovieClip::lockroot_getset,
          &MovieClip::lockroot_getset);
    o.init_member("beginBitmapFill", gl->createFunction(
                movieclip_beginBitmapFill));
    o.init_member("getRect", gl->createFunction(
                movieclip_getRect));
    o.init_member("lineGradientStyle", gl->createFunction(
                movieclip_lineGradientStyle));
    o.init_member("attachBitmap", gl->createFunction(
                movieclip_attachBitmap));
    o.init_property("blendMode", &DisplayObject::blendMode,
            &DisplayObject::blendMode);
    o.init_property("cacheAsBitmap", &movieclip_cacheAsBitmap, 
            &movieclip_cacheAsBitmap);
    o.init_property("filters", &movieclip_filters, &movieclip_filters);
    o.init_property("forceSmoothing", &movieclip_forceSmoothing,
            &movieclip_forceSmoothing);
    o.init_property("opaqueBackground", &movieclip_opaqueBackground,
            &movieclip_opaqueBackground);
    o.init_property("scale9Grid", &movieclip_scale9Grid,
            movieclip_scale9Grid);
    o.init_property("scrollRect", &movieclip_scrollRect,
        &movieclip_scrollRect);
    o.init_property("tabIndex", &movieclip_tabIndex, &movieclip_tabIndex);
    o.init_property("transform", &movieclip_transform, 
            &movieclip_transform);

    const int swf6Flags = as_prop_flags::dontDelete |
                as_prop_flags::dontEnum |
                as_prop_flags::onlySWF6Up;

    o.init_member("attachAudio", vm.getNative(900, 8), swf6Flags);
    o.init_member("attachVideo", vm.getNative(900, 9), swf6Flags);
    o.init_member("getDepth", vm.getNative(900, 10), swf6Flags);
    o.init_member("setMask", vm.getNative(900, 11), swf6Flags);
    o.init_member("createEmptyMovieClip", vm.getNative(901, 0), swf6Flags);
    o.init_member("beginFill", vm.getNative(901, 1), swf6Flags);
    o.init_member("beginGradientFill", vm.getNative(901, 2), swf6Flags);
    o.init_member("moveTo", vm.getNative(901, 3), swf6Flags);
    o.init_member("lineTo", vm.getNative(901, 4), swf6Flags);
    o.init_member("curveTo", vm.getNative(901, 5), swf6Flags);
    o.init_member("lineStyle", vm.getNative(901, 6), swf6Flags);
    o.init_member("endFill", vm.getNative(901, 7), swf6Flags);
    o.init_member("clear", vm.getNative(901, 8), swf6Flags);
    o.init_member("createTextField", vm.getNative(104, 200), swf6Flags);
    o.init_member("getTextSnapshot", 
            gl->createFunction(movieclip_getTextSnapshot), swf6Flags);

    const int swf7Flags = as_prop_flags::dontDelete |
                as_prop_flags::dontEnum |
                as_prop_flags::onlySWF7Up;

    o.init_member("getNextHighestDepth", gl->createFunction(
                movieclip_getNextHighestDepth), swf7Flags);
    o.init_member("getInstanceAtDepth", gl->createFunction(
                movieclip_getInstanceAtDepth), swf7Flags);

}


as_value
movieclip_play(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
        ensureType<MovieClip>(fn.this_ptr);

    movieclip->setPlayState(MovieClip::PLAYSTATE_PLAY);
    return as_value();
}

as_value
movieclip_stop(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
        ensureType<MovieClip>(fn.this_ptr);

    movieclip->setPlayState(MovieClip::PLAYSTATE_STOP);

    return as_value();
}


//removeMovieClip() : Void
as_value
movieclip_removeMovieClip(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);
    movieclip->removeMovieClip();
    return as_value();
}


as_value
movieclip_cacheAsBitmap(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(movieclip);
    LOG_ONCE( log_unimpl(_("MovieClip.cacheAsBitmap()")) );
    return as_value();
}


as_value
movieclip_filters(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(movieclip);
    LOG_ONCE(log_unimpl(_("MovieClip.filters()")));
    return as_value();
}


as_value
movieclip_forceSmoothing(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(movieclip);
    LOG_ONCE(log_unimpl(_("MovieClip.forceSmoothing()")));
    return as_value();
}


as_value
movieclip_opaqueBackground(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(movieclip);
    LOG_ONCE(log_unimpl(_("MovieClip.opaqueBackground()")));
    return as_value();
}

    
as_value
movieclip_scale9Grid(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(movieclip);
    LOG_ONCE(log_unimpl(_("MovieClip.scale9Grid()")));
    return as_value();
}


as_value
movieclip_scrollRect(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(movieclip);
    LOG_ONCE(log_unimpl(_("MovieClip.scrollRect()")));
    return as_value();
}


as_value
movieclip_tabIndex(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(movieclip);
    LOG_ONCE(log_unimpl(_("MovieClip.tabIndex()")));
    return as_value();
}


// attachMovie(idName:String, newName:String,
//                         depth:Number [, initObject:Object]) : MovieClip
as_value
movieclip_attachMovie(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    if (fn.nargs < 3 || fn.nargs > 4)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("attachMovie called with wrong number of arguments"
                " expected 3 to 4, got (%d) - returning undefined"), fn.nargs);
        );
        return as_value();
    }

    // Get exported resource 
    const std::string& id_name = fn.arg(0).to_string();

    boost::intrusive_ptr<ExportableResource> exported =
        movieclip->get_root()->definition()->get_exported_resource(id_name);

    if (!exported)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("attachMovie: '%s': no such exported resource - "
            "returning undefined"), id_name);
        );
        return as_value(); 
    }
    
    SWF::DefinitionTag* exported_movie =
        dynamic_cast<SWF::DefinitionTag*>(exported.get());

    if (!exported_movie)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("attachMovie: exported resource '%s' "
            "is not a DisplayObject definition (%s) -- "
            "returning undefined"), id_name,
            typeid(*(exported.get())).name());
        );
        return as_value();
    }

    const std::string& newname = fn.arg(1).to_string();

    // Movies should be attachable from -16384 to 2130690045, according to
    // kirupa (http://www.kirupa.com/developer/actionscript/depths2.htm)
    // Tests in misc-ming.all/DepthLimitsTest.c show that 2130690044 is the
    // maximum valid depth.
    const double depth = fn.arg(2).to_number();
    
    // This also checks for overflow, as both numbers are expressible as
    // boost::int32_t.
    if (depth < DisplayObject::lowerAccessibleBound ||
            depth > DisplayObject::upperAccessibleBound)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("MovieClip.attachMovie: invalid depth %d "
                    "passed; not attaching"), depth);
        );
        return as_value();
    }
    
    boost::int32_t depthValue = static_cast<boost::int32_t>(depth);

    boost::intrusive_ptr<DisplayObject> newch =
        exported_movie->createDisplayObject(movieclip.get(), 0);

#ifndef GNASH_USE_GC
    assert(newch->get_ref_count() > 0);
#endif // ndef GNASH_USE_GC

    newch->set_name(newname);
    newch->setDynamic();

    boost::intrusive_ptr<as_object> initObj;

    if (fn.nargs > 3 ) {
        initObj = fn.arg(3).to_object(*getGlobal(fn));
        if (!initObj) {
            // This is actually a valid thing to do,
            // the documented behaviour is to just NOT
            // initialize the properties in this
            // case.
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("Fourth argument of attachMovie doesn't cast to "
                    "an object (%s), we'll act as if it wasn't given"),
                    fn.arg(3));
            );
        }
    }

    // placeDisplayObject() will set depth on newch
    if (!movieclip->attachCharacter(*newch, depthValue, initObj.get()))
    {
        log_error(_("Could not attach DisplayObject at depth %d"), depthValue);
        return as_value();
    }

    return as_value(newch.get());
}


// attachAudio(id:Object) : Void
as_value
movieclip_attachAudio(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
        ensureType<MovieClip>(fn.this_ptr);

    if (!fn.nargs)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror("MovieClip.attachAudio(): %s", _("missing arguments"));
        );
        return as_value();
    }

    as_object* obj = fn.arg(0).to_object(*getGlobal(fn)).get();
    if ( ! obj )
    { 
        std::stringstream ss; fn.dump_args(ss);
        // TODO: find out what to do here
        log_error("MovieClip.attachAudio(%s): first arg doesn't cast to "
                "an object", ss.str());
        return as_value();
    }

    NetStream_as* ns = dynamic_cast<NetStream_as*>(obj);
    if ( ! ns )
    { 
        std::stringstream ss; fn.dump_args(ss);
        // TODO: find out what to do here
        log_error("MovieClip.attachAudio(%s): first arg doesn't cast to a "
                "NetStream", ss.str());
        return as_value();
    }

    ns->setAudioController(movieclip.get());

    LOG_ONCE( log_unimpl("MovieClip.attachAudio() - TESTING") );
    return as_value();
}


// MovieClip.attachVideo
as_value
movieclip_attachVideo(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(movieclip);

    LOG_ONCE( log_unimpl("MovieClip.attachVideo()") );
    return as_value();
}


//createEmptyMovieClip(name:String, depth:Number) : MovieClip
as_value
movieclip_createEmptyMovieClip(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    if (fn.nargs != 2)
    {
        if (fn.nargs < 2)
        {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("createEmptyMovieClip needs "
                    "2 args, but %d given,"
                    " returning undefined"),
                    fn.nargs);
            );
            return as_value();
        }
        else
        {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("createEmptyMovieClip takes "
                    "2 args, but %d given, discarding"
                    " the excess"),
                    fn.nargs);
            )
        }
    }

    // Unlike other MovieClip methods, the depth argument of an empty movie clip
    // can be any number. All numbers are converted to an int32_t, and are valid
    // depths even when outside the usual bounds.
    DisplayObject* ch = movieclip->add_empty_movieclip(fn.arg(0).to_string(),
            fn.arg(1).to_int());
    return as_value(ch);
}

as_value
movieclip_getDepth(const fn_call& fn)
{
    // TODO: make this a DisplayObject::getDepth_method function...
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    const int n = movieclip->get_depth();

    return as_value(n);
}

//swapDepths(target:Object|target:Number)
//
// Returns void.
as_value
movieclip_swapDepths(const fn_call& fn)
{

    boost::intrusive_ptr<MovieClip> movieclip =
        ensureType<MovieClip>(fn.this_ptr);

    const int this_depth = movieclip->get_depth();

    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("%s.swapDepths() needs one arg"), movieclip->getTarget());
        );
        return as_value();
    }

    // Lower bound of source depth below which swapDepth has no effect
    // (below Timeline/static zone)
    if ( this_depth < DisplayObject::lowerAccessibleBound )
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss;
            fn.dump_args(ss);
            log_aserror(_("%s.swapDepths(%s): won't swap a clip below "
                    "depth %d (%d)"),
            movieclip->getTarget(), ss.str(), DisplayObject::lowerAccessibleBound,
                this_depth);
        );
        return as_value();
    }

    typedef boost::intrusive_ptr<DisplayObject> CharPtr;
    typedef boost::intrusive_ptr<MovieClip> SpritePtr;

    SpritePtr this_parent = dynamic_cast<MovieClip*>(
            movieclip->get_parent());

    //CharPtr target = NULL;
    int target_depth = 0;

    // movieclip.swapDepth(movieclip)
    if ( SpritePtr target_movieclip = fn.arg(0).to_sprite() )
    {
        if ( movieclip == target_movieclip )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("%s.swapDepths(%s): invalid call, swapping to self?"),
                movieclip->getTarget(), target_movieclip->getTarget());
            );
            return as_value();
        }

        SpritePtr target_parent =
            dynamic_cast<MovieClip*>(movieclip->get_parent());
        if ( this_parent != target_parent )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("%s.swapDepths(%s): invalid call, the two "
                    "DisplayObjects don't have the same parent"),
                movieclip->getTarget(), target_movieclip->getTarget());
            );
            return as_value();
        }

        target_depth = target_movieclip->get_depth();

        // Check we're not swapping the our own depth so
        // to avoid unecessary bounds invalidation and immunizing
        // the instance from subsequent PlaceObject tags attempting
        // to transform it.
        if ( movieclip->get_depth() == target_depth )
        {
            IF_VERBOSE_ASCODING_ERRORS(
                std::stringstream ss; fn.dump_args(ss);
                log_aserror(_("%s.swapDepths(%s): ignored, source and "
                    "target DisplayObjects have the same depth %d"),
                    movieclip->getTarget(), ss.str(), target_depth);
            );
            return as_value();
        }
    }

    // movieclip.swapDepth(depth)
    else
    {
        double td = fn.arg(0).to_number();
        if ( isNaN(td) )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss; fn.dump_args(ss);
            log_aserror(_("%s.swapDepths(%s): first argument invalid "
                "(neither a movieclip nor a number)"),
                movieclip->getTarget(), ss.str());
            );
            return as_value();
        }

        target_depth = int(td);

        // Check we're not swapping the our own depth so
        // to avoid unecessary bounds invalidation and immunizing
        // the instance from subsequent PlaceObjec tags attempting
        // to transform it.
        if ( movieclip->get_depth() == target_depth )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss; fn.dump_args(ss);
            log_aserror(_("%s.swapDepths(%s): ignored, DisplayObject already "
                    "at depth %d"),
                movieclip->getTarget(), ss.str(), target_depth);
            );
            return as_value();
        }


        // TODO : check other kind of validities ?


    }

    if ( this_parent )
    {
        this_parent->swapDepths(movieclip.get(), target_depth);
    }
    else
    {
        movie_root& root = getRoot(fn);
        root.swapLevels(movieclip, target_depth);
        return as_value();
    }

    return as_value();

}

// TODO: wrap the functionality in a MovieClip method
//             and invoke it from here, this should only be a wrapper
//
//duplicateMovieClip(name:String, depth:Number, [initObject:Object]) : MovieClip
as_value
movieclip_duplicateMovieClip(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);
    
    if (fn.nargs < 2)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.duplicateMovieClip() needs 2 or 3 args"));
                );
        return as_value();
    }

    const std::string& newname = fn.arg(0).to_string();

    // Depth as in attachMovie
    const double depth = fn.arg(1).to_number();
    
    // This also checks for overflow, as both numbers are expressible as
    // boost::int32_t.
    if (depth < DisplayObject::lowerAccessibleBound ||
            depth > DisplayObject::upperAccessibleBound)
    {
        IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("MovieClip.duplicateMovieClip: "
                        "invalid depth %d passed; not duplicating"), depth);
        );    
        return as_value();
    }
    
    boost::int32_t depthValue = static_cast<boost::int32_t>(depth);

    boost::intrusive_ptr<MovieClip> ch;

    // Copy members from initObject
    if (fn.nargs == 3)
    {
        boost::intrusive_ptr<as_object> initObject = fn.arg(2).to_object(*getGlobal(fn));
        ch = movieclip->duplicateMovieClip(newname, depthValue,
                initObject.get());
    }
    else
    {
        ch = movieclip->duplicateMovieClip(newname, depthValue);
    }

    return as_value(ch.get());
}

as_value
movieclip_gotoAndPlay(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("movieclip_goto_and_play needs one arg"));
        );
        return as_value();
    }

    size_t frame_number;
    if ( ! movieclip->get_frame_number(fn.arg(0), frame_number) )
    {
        // No dice.
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("movieclip_goto_and_play('%s') -- invalid frame"),
                    fn.arg(0));
        );
        return as_value();
    }

    // Convert to 0-based
    movieclip->goto_frame(frame_number);
    movieclip->setPlayState(MovieClip::PLAYSTATE_PLAY);
    return as_value();
}

as_value movieclip_gotoAndStop(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("movieclip_goto_and_stop needs one arg"));
        );
        return as_value();
    }

    size_t frame_number;
    if ( ! movieclip->get_frame_number(fn.arg(0), frame_number) )
    {
        // No dice.
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("movieclip_goto_and_stop('%s') -- invalid frame"),
                    fn.arg(0));
        );
        return as_value();
    }

    // Convert to 0-based
    movieclip->goto_frame(frame_number);
    movieclip->setPlayState(MovieClip::PLAYSTATE_STOP);
    return as_value();
}

as_value movieclip_nextFrame(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    const size_t frame_count = movieclip->get_frame_count();
    const size_t current_frame = movieclip->get_current_frame();
    if (current_frame < frame_count)
    {
        movieclip->goto_frame(current_frame + 1);
    }
    movieclip->setPlayState(MovieClip::PLAYSTATE_STOP);
    return as_value();
}

as_value
movieclip_prevFrame(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    const size_t current_frame = movieclip->get_current_frame();
    if (current_frame > 0)
    {
        movieclip->goto_frame(current_frame - 1);
    }
    movieclip->setPlayState(MovieClip::PLAYSTATE_STOP);
    return as_value();
}

as_value
movieclip_getBytesLoaded(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    return as_value(movieclip->get_bytes_loaded());
}

as_value
movieclip_getBytesTotal(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    // @@ horrible uh ?
    return as_value(movieclip->get_bytes_total());
}

// MovieClip.loadMovie(url:String [,variables:String]).
//
// Returns 1 for "get", 2 for "post", and otherwise 0. Case-insensitive.
// This *always* calls MovieClip.meth.
as_value
movieclip_loadMovie(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    as_value val;
    if (fn.nargs > 1) {
        val = movieclip->callMethod(NSV::PROP_METH, fn.arg(1));
    }
    else val = movieclip->callMethod(NSV::PROP_METH);

    if (fn.nargs < 1) // url
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.loadMovie() "
            "expected 1 or 2 args, got %d - returning undefined"),
            fn.nargs);
        );
        return as_value();
    }

    const std::string& urlstr = fn.arg(0).to_string();
    if (urlstr.empty())
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("First argument of MovieClip.loadMovie(%s) "
            "evaluates to an empty string - "
            "returning undefined"),
            ss.str());
        );
        return as_value();
    }

    movie_root& mr = getRoot(fn);
    std::string target = movieclip->getTarget();

    // TODO: if GET/POST should send variables of *this* movie,
    // no matter if the target will be replaced by another movie !!
    const MovieClip::VariablesMethod method =
        static_cast<MovieClip::VariablesMethod>(val.to_int());

    std::string data;

    // This is just an optimization if we aren't going
    // to send the data anyway. It might be wrong, though.
    if (method != MovieClip::METHOD_NONE)
    {
        movieclip->getURLEncodedVars(data);
    }
 
    mr.loadMovie(urlstr, target, data, method);

    return as_value();
}

// my_mc.loadVariables(url:String [, variables:String]) : Void
as_value
movieclip_loadVariables(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    // This always calls MovieClip.meth, even when there are no
    // arguments.
    as_value val;
    if (fn.nargs > 1)
    {
        val = movieclip->callMethod(NSV::PROP_METH, fn.arg(1));
    }
    else val = movieclip->callMethod(NSV::PROP_METH);

    if (fn.nargs < 1) // url
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.loadVariables() "
            "expected 1 or 2 args, got %d - returning undefined"),
            fn.nargs);
        );
        return as_value();
    }

    const std::string& urlstr = fn.arg(0).to_string();
    if (urlstr.empty())
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("First argument passed to MovieClip.loadVariables(%s) "
            "evaluates to an empty string - "
            "returning undefined"),
            ss.str());
        );
        return as_value();
    }

    const MovieClip::VariablesMethod method =
        static_cast<MovieClip::VariablesMethod>(val.to_int());

    movieclip->loadVariables(urlstr, method);
    log_debug("MovieClip.loadVariables(%s) - TESTING ", urlstr);

    return as_value();
}

// my_mc.unloadMovie() : Void
as_value
movieclip_unloadMovie(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    movieclip->unloadMovie();

    return as_value();
}

as_value
movieclip_hitTest(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    switch (fn.nargs)
    {
        case 1: // target
        {
            const as_value& tgt_val = fn.arg(0);
            DisplayObject* target = fn.env().find_target(tgt_val.to_string());
            if ( ! target )
            {
                IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("Can't find hitTest target %s"),
                    tgt_val);
                );
                return as_value();
            }

            rect thisbounds = movieclip->getBounds();
            SWFMatrix thismat = movieclip->getWorldMatrix();
            thismat.transform(thisbounds);

            rect tgtbounds = target->getBounds();
            SWFMatrix tgtmat = target->getWorldMatrix();
            tgtmat.transform(tgtbounds);

            return thisbounds.getRange().intersects(tgtbounds.getRange());

            break;
        }

        case 2: // x, y
        {
            boost::int32_t x = pixelsToTwips(fn.arg(0).to_number());
            boost::int32_t y = pixelsToTwips(fn.arg(1).to_number());

            return movieclip->pointInBounds(x, y);
        }

        case 3: // x, y, shapeFlag
        {
             boost::int32_t x = pixelsToTwips(fn.arg(0).to_number());
             boost::int32_t y = pixelsToTwips(fn.arg(1).to_number());
             bool shapeFlag = fn.arg(2).to_bool();

             if ( ! shapeFlag ) return movieclip->pointInBounds(x, y);
             else return movieclip->pointInHitableShape(x, y);
        }

        default:
        {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("hitTest() called with %u args"),
                    fn.nargs);
            );
            break;
        }
    }

    return as_value();

}

as_value
movieclip_createTextField(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    if (fn.nargs < 6) // name, depth, x, y, width, height
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("createTextField called with %d args, "
            "expected 6 - returning undefined"), fn.nargs);
        );
        return as_value();
    }

    std::string txt_name = fn.arg(0).to_string();

    int txt_depth = fn.arg(1).to_int();

    int txt_x = fn.arg(2).to_int();

    int txt_y = fn.arg(3).to_int();

    int txt_width = fn.arg(4).to_int();
    if ( txt_width < 0 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("createTextField: negative width (%d)"
            " - reverting sign"), txt_width);
        );
        txt_width = -txt_width;
    }

    int txt_height = fn.arg(5).to_int();
    if ( txt_height < 0 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("createTextField: negative height (%d)"
            " - reverting sign"), txt_height);
        );
        txt_height = -txt_height;
    }

    boost::intrusive_ptr<DisplayObject> txt = movieclip->add_textfield(txt_name,
            txt_depth, txt_x, txt_y, txt_width, txt_height);

    // createTextField returns void, it seems
    if (getSWFVersion(fn) > 7) return as_value(txt.get());
    else return as_value(); 
}

//getNextHighestDepth() : Number
as_value
movieclip_getNextHighestDepth(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    int nextdepth = movieclip->getNextHighestDepth();
    return as_value(static_cast<double>(nextdepth));
}

//getInstanceAtDepth(depth:Number) : MovieClip
as_value
movieclip_getInstanceAtDepth(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> mc = ensureType<MovieClip>(fn.this_ptr);

    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror("MovieClip.getInstanceAtDepth(): missing depth argument");
        );
        return as_value();
    }

    int depth = fn.arg(0).to_int();
    boost::intrusive_ptr<DisplayObject> ch = mc->getDisplayObjectAtDepth(depth);
 
    // we want 'undefined', not 'null'
    if (!ch) return as_value();
    return as_value(ch.get());
}

/// MovieClip.getURL(url:String[, window:String[, method:String]])
//
/// Tested manually to function as a method of any as_object. Hard to
/// test automatically as it doesn't return anything and only has external
/// side-effects.
/// Returns void.
as_value
movieclip_getURL(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> movieclip =
        ensureType<as_object>(fn.this_ptr);

    std::string urlstr;
    std::string target;

    as_value val;
    if (fn.nargs > 2)
    {
        val = movieclip->callMethod(NSV::PROP_METH, fn.arg(2));
    }
    else val = movieclip->callMethod(NSV::PROP_METH);

    switch (fn.nargs)
    {
        case 0:
        {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("No arguments passed to MovieClip.getURL()"));
            );
            return as_value();
        }
        default:
        {
            IF_VERBOSE_ASCODING_ERRORS(
                std::ostringstream os;
                fn.dump_args(os);
                log_aserror(_("MovieClip.getURL(%s): extra arguments "
                    "dropped"), os.str());
            );
        }
        case 3:
            // This argument has already been handled.
        case 2:
             target = fn.arg(1).to_string();
        case 1:
             urlstr = fn.arg(0).to_string();
             break;
    }


    MovieClip::VariablesMethod method =
        static_cast<MovieClip::VariablesMethod>(val.to_int());

    std::string vars;

    if (method != MovieClip::METHOD_NONE) {
        // Get encoded vars.
        movieclip->getURLEncodedVars(vars);
    }

    movie_root& m = getRoot(fn);
    
    m.getURL(urlstr, target, vars, method);

    return as_value();
}

// getSWFVersion() : Number
as_value
movieclip_getSWFVersion(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    return as_value(movieclip->getMovieVersion());
}

// MovieClip.meth(<string>) : Number
//
// Parses case-insensitive "get" and "post" into 1 and 2, 0 anything else
// 
as_value
movieclip_meth(const fn_call& fn)
{

    if (!fn.nargs) return as_value(MovieClip::METHOD_NONE); 

    const as_value& v = fn.arg(0);
    boost::intrusive_ptr<as_object> o = v.to_object(*getGlobal(fn));
    if ( ! o )
    {
        log_debug(_("meth(%s): first argument doesn't cast to object"), v);
        return as_value(MovieClip::METHOD_NONE);
    }

    as_value lc = o->callMethod(NSV::PROP_TO_LOWER_CASE);

    std::string s = lc.to_string();

    if (s == "get") return as_value(MovieClip::METHOD_GET);
    if (s == "post") return as_value(MovieClip::METHOD_POST);
    return as_value(MovieClip::METHOD_NONE);
}


// getTextSnapshot() : TextSnapshot
as_value
movieclip_getTextSnapshot(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> obj = ensureType<MovieClip>(fn.this_ptr);

    // If not found, construction fails.
    as_value textSnapshot(fn.env().find_object("TextSnapshot"));

    boost::intrusive_ptr<as_function> tsCtor = textSnapshot.to_as_function();

    if (!tsCtor) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("MovieClip.getTextSnapshot: failed to construct "
                "TextSnapshot (object probably overridden)");
        );
        return as_value();
    }

    // Construct a flash.geom.Transform object with "this" as argument.
    std::auto_ptr<std::vector<as_value> > args(new std::vector<as_value>);
    args->push_back(obj.get());

    boost::intrusive_ptr<as_object> ts =
        tsCtor->constructInstance(fn.env(), args);

    return as_value(ts.get());
}


// getBounds(targetCoordinateSpace:Object) : Object
as_value
movieclip_getBounds(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObject> movieclip =
        ensureType<DisplayObject>(fn.this_ptr);

    rect bounds = movieclip->getBounds();

    if ( fn.nargs > 0 )
    {
        DisplayObject* target = fn.arg(0).toDisplayObject();
        if ( ! target )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("MovieClip.getBounds(%s): invalid call, first "
                    "arg must be a DisplayObject"),
                fn.arg(0));
            );
            return as_value();
        }

        SWFMatrix tgtwmat = target->getWorldMatrix();
        SWFMatrix srcwmat = movieclip->getWorldMatrix();

        srcwmat.transform(bounds);
        tgtwmat.invert().transform(bounds);
    }

    // Magic numbers here... dunno why
    double xMin = 6710886.35;
    double yMin = 6710886.35;
    double xMax = 6710886.35;
    double yMax = 6710886.35;

    if ( !bounds.is_null() )
    {
        // Round to the twip
        xMin = twipsToPixels(bounds.get_x_min());
        yMin = twipsToPixels(bounds.get_y_min());
        xMax = twipsToPixels(bounds.get_x_max());
        yMax = twipsToPixels(bounds.get_y_max());
    }

    boost::intrusive_ptr<as_object> bounds_obj(new as_object());
    bounds_obj->init_member("xMin", as_value(xMin));
    bounds_obj->init_member("yMin", as_value(yMin));
    bounds_obj->init_member("xMax", as_value(xMax));
    bounds_obj->init_member("yMax", as_value(yMax));

    return as_value(bounds_obj.get());
}

as_value
movieclip_globalToLocal(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
            ensureType<MovieClip>(fn.this_ptr);

    as_value ret;

    if ( fn.nargs < 1 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.globalToLocal() takes one arg"));
        );
        return ret;
    }

    boost::intrusive_ptr<as_object> obj = fn.arg(0).to_object(*getGlobal(fn));
    if ( ! obj )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.globalToLocal(%s): "
                "first argument doesn't cast to an object"),
            fn.arg(0));
        );
        return ret;
    }

    as_value tmp;
    boost::int32_t    x = 0;
    boost::int32_t    y = 0;

    if ( ! obj->get_member(NSV::PROP_X, &tmp) )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.globalToLocal(%s): "
                "object parameter doesn't have an 'x' member"),
            fn.arg(0));
        );
        return ret;
    }
    x = pixelsToTwips(tmp.to_number());

    if ( ! obj->get_member(NSV::PROP_Y, &tmp) )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.globalToLocal(%s): "
                "object parameter doesn't have an 'y' member"),
            fn.arg(0));
        );
        return ret;
    }
    y = pixelsToTwips(tmp.to_number());

    point    pt(x, y);
    SWFMatrix world_mat = movieclip->getWorldMatrix();
    world_mat.invert().transform(pt);

    obj->set_member(NSV::PROP_X, twipsToPixels(pt.x));
    obj->set_member(NSV::PROP_Y, twipsToPixels(pt.y));

    return ret;
}

as_value
movieclip_localToGlobal(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
            ensureType<MovieClip>(fn.this_ptr);

    as_value ret;

    if ( fn.nargs < 1 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.localToGlobal() takes one arg"));
        );
        return ret;
    }

    boost::intrusive_ptr<as_object> obj = fn.arg(0).to_object(*getGlobal(fn));
    if ( ! obj )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.localToGlobal(%s): "
                "first argument doesn't cast to an object"),
            fn.arg(0));
        );
        return ret;
    }

    as_value tmp;
    boost::int32_t    x = 0;
    boost::int32_t    y = 0;

    if ( ! obj->get_member(NSV::PROP_X, &tmp) )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.localToGlobal(%s): "
                "object parameter doesn't have an 'x' member"),
            fn.arg(0));
        );
        return ret;
    }
    x = pixelsToTwips(tmp.to_number());

    if ( ! obj->get_member(NSV::PROP_Y, &tmp) )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.localToGlobal(%s): "
                "object parameter doesn't have an 'y' member"),
            fn.arg(0));
        );
        return ret;
    }
    y = pixelsToTwips(tmp.to_number());

    point    pt(x, y);
    SWFMatrix world_mat = movieclip->getWorldMatrix();
    world_mat.transform(pt);

    obj->set_member(NSV::PROP_X, twipsToPixels(pt.x));
    obj->set_member(NSV::PROP_Y, twipsToPixels(pt.y));
    return ret;

}

as_value
movieclip_setMask(const fn_call& fn)
{
    // swfdec/test/image/mask-textfield-6.swf shows that setMask should also
    // work against TextFields, we have no tests for other DisplayObject types so
    // we generalize it for any DisplayObject.
    boost::intrusive_ptr<DisplayObject> maskee = 
        ensureType<DisplayObject>(fn.this_ptr);

    if ( ! fn.nargs )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("%s.setMask() : needs an argument"), maskee->getTarget());
        );
        return as_value();
    }

    const as_value& arg = fn.arg(0);
    if ( arg.is_null() || arg.is_undefined() )
    {
        // disable mask
        maskee->setMask(NULL);
    }
    else
    {

        boost::intrusive_ptr<as_object> obj ( arg.to_object(*getGlobal(fn)) );
        DisplayObject* mask = dynamic_cast<DisplayObject*>(obj.get());
        if ( ! mask )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("%s.setMask(%s) : first argument is not a DisplayObject"),
                maskee->getTarget(), arg);
            );
            return as_value();
        }

        // ch is possibly NULL, which is intended
        maskee->setMask(mask);
    }

    //log_debug("MovieClip.setMask() TESTING");

    return as_value(true);
}

as_value
movieclip_endFill(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
            ensureType<MovieClip>(fn.this_ptr);

    IF_VERBOSE_ASCODING_ERRORS(
    if ( fn.nargs )
    {
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("MovieClip.endFill(%s): args will be discarded"),
            ss.str());
    }
    );
#ifdef DEBUG_DRAWING_API
    log_debug("%s.endFill();", movieclip->getTarget());
#endif
    movieclip->endFill();
    return as_value();
}

as_value
movieclip_lineTo(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
            ensureType<MovieClip>(fn.this_ptr);

    if ( fn.nargs < 2 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("MovieClip.lineTo() needs at least two arguments"));
        );
        return as_value();
    }

    IF_VERBOSE_ASCODING_ERRORS(
    if ( fn.nargs > 2 )
    {
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("MovieClip.lineTo(%s): args after the first two "
                        "will be discarded"), ss.str());
    }
    );

    double x = fn.arg(0).to_number();
    double y = fn.arg(1).to_number();
        
    if (!isFinite(x) )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("%s.lineTo(%s) : non-finite first argument (%s), "
            "converted to zero"), movieclip->getTarget(),
            ss.str(), fn.arg(0));
        );
        x = 0;
    }
     
    if (!isFinite(y) )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("%s.lineTo(%s) : non-finite second argument (%s), "
            "converted to zero"), movieclip->getTarget(),
            ss.str(), fn.arg(1));
        );
        y = 0;
    }

#ifdef DEBUG_DRAWING_API
    log_debug("%s.lineTo(%g,%g);", movieclip->getTarget(), x, y);
#endif
    movieclip->lineTo(pixelsToTwips(x), pixelsToTwips(y));
    return as_value();
}

as_value
movieclip_moveTo(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    if ( fn.nargs < 2 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("MovieClip.moveTo() takes two args"));
        );
        return as_value();
    }

    IF_VERBOSE_ASCODING_ERRORS(
    if ( fn.nargs > 2 )
    {
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("MovieClip.moveTo(%s): args after the first two will "
                        "be discarded"), ss.str());
    }
    );

    double x = fn.arg(0).to_number();
    double y = fn.arg(1).to_number();
     
    if (!isFinite(x) )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("%s.moveTo(%s) : non-finite first argument (%s), "
            "converted to zero"), movieclip->getTarget(),
            ss.str(), fn.arg(0));
        );
        x = 0;
    }
     
    if (!isFinite(y) )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("%s.moveTo(%s) : non-finite second argument (%s), "
            "converted to zero"), movieclip->getTarget(),
            ss.str(), fn.arg(1));
        );
        y = 0;
    }

#ifdef DEBUG_DRAWING_API
    log_debug(_("%s.moveTo(%g,%g);"), movieclip->getTarget(), x, y);
#endif
    movieclip->moveTo(pixelsToTwips(x), pixelsToTwips(y));
    return as_value();
}

// SWF6,7: lineStyle(thickness:Number, rgb:Number, alpha:Number) : Void
//
//    SWF8+: lineStyle(thickness:Number, rgb:Number, alpha:Number,
//                                     pixelHinting:Boolean, noScale:String,
//                                     capsStyle:String, jointStyle:String,
//                                     miterLimit:Number) : Void
as_value
movieclip_lineStyle(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
            ensureType<MovieClip>(fn.this_ptr);

    if ( ! fn.nargs )
    {
        movieclip->resetLineStyle();
        return as_value();
    }

    boost::uint8_t r = 0;
    boost::uint8_t g = 0;
    boost::uint8_t b = 0;
    boost::uint8_t a = 255;
    boost::uint16_t thickness = 0;
    bool scaleThicknessVertically = true;
    bool scaleThicknessHorizontally = true;
    bool pixelHinting = false;
    bool noClose = false;
    cap_style_e capStyle = CAP_ROUND;
    join_style_e joinStyle = JOIN_ROUND;
    float miterLimitFactor = 1.0f;

    int arguments = fn.nargs;

    const int swfVersion = getSWFVersion(fn);
    if (swfVersion < 8 && fn.nargs > 3)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("MovieClip.lineStyle(%s): args after the "
                          "first three will be discarded"), ss.str());
            );
        arguments = 3;
    }

    switch (arguments)
    {
        default:
            IF_VERBOSE_ASCODING_ERRORS(
                std::ostringstream ss;
                fn.dump_args(ss);
                log_aserror(_("MovieClip.lineStyle(%s): args after the "
                              "first eight will be discarded"), ss.str());
                );
        case 8:
            miterLimitFactor = clamp<int>(fn.arg(7).to_int(), 1, 255);
        case 7:
        {
            std::string joinStyleStr = fn.arg(6).to_string();
            if (joinStyleStr == "miter") joinStyle = JOIN_MITER;
            else if (joinStyleStr == "round") joinStyle = JOIN_ROUND;
            else if (joinStyleStr == "bevel") joinStyle = JOIN_BEVEL;
            else
            {
                IF_VERBOSE_ASCODING_ERRORS(
                    std::ostringstream ss;
                    fn.dump_args(ss);
                    log_aserror(_("MovieClip.lineStyle(%s): invalid joinStyle"
                                "value '%s' (valid values: %s|%s|%s)"),
                        ss.str(), joinStyleStr, "miter", "round", "bevel");
                );
            }
        }
        case 6:
        {
            const std::string capStyleStr = fn.arg(5).to_string();
            if (capStyleStr == "none") capStyle = CAP_NONE;
            else if (capStyleStr == "round") capStyle = CAP_ROUND;
            else if (capStyleStr == "square") capStyle = CAP_SQUARE;
            else
            {
                IF_VERBOSE_ASCODING_ERRORS(
                    std::ostringstream ss;
                    fn.dump_args(ss);
                    log_aserror(_("MovieClip.lineStyle(%s): invalid capStyle "
                               "value '%s' (valid values: none|round|square)"),
                               ss.str(), capStyleStr);
                );
            }
        }
        case 5:
        {
            // Both values to be set here are true, so just set the
            // appropriate values to false.
            const std::string noScaleString = fn.arg(4).to_string();
            if (noScaleString == "none")
            {
                scaleThicknessVertically = false;
                scaleThicknessHorizontally = false;
            }
            else if (noScaleString == "vertical")
            {
                scaleThicknessVertically = false;
            }
            else if (noScaleString == "horizontal")
            {
                scaleThicknessHorizontally = false;
            }
            else if (noScaleString != "normal")
            {
                IF_VERBOSE_ASCODING_ERRORS(
                    std::ostringstream ss;
                    fn.dump_args(ss);
                    log_aserror(_("MovieClip.lineStyle(%s): invalid "
                                    "noScale value '%s' (valid values: "
                                    "%s|%s|%s|%s)"),
                                    ss.str(), noScaleString, "none",
                                    "vertical", "horizontal", "normal");
                );
            }
        }
        case 4:
            pixelHinting = fn.arg(3).to_bool();
        case 3:
        {
            const float alphaval = clamp<float>(fn.arg(2).to_number(),
                                     0, 100);
            a = boost::uint8_t(255 * (alphaval / 100));
        }
        case 2:
        {
            // See pollock.swf for eventual regressions.
            // It sets color to a random number from
            // 0 to 160000000 (about 10 times more then the max).
            boost::uint32_t rgbval = fn.arg(1).to_int();
            r = boost::uint8_t((rgbval & 0xFF0000) >> 16);
            g = boost::uint8_t((rgbval & 0x00FF00) >> 8);
            b = boost::uint8_t((rgbval & 0x0000FF) );
        }
        case 1:
            thickness = boost::uint16_t(pixelsToTwips(clamp<float>(
                            fn.arg(0).to_number(), 0, 255)));
            break;
    }

    rgba color(r, g, b, a);

#ifdef DEBUG_DRAWING_API
    log_debug("%s.lineStyle(%d,%d,%d,%d);", movieclip->getTarget(), thickness, r, g, b);
#endif
    movieclip->lineStyle(thickness, color,
    scaleThicknessVertically, scaleThicknessHorizontally,
    pixelHinting, noClose, capStyle, capStyle, joinStyle, miterLimitFactor);

    return as_value();
}

as_value
movieclip_curveTo(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
            ensureType<MovieClip>(fn.this_ptr);

    if ( fn.nargs < 4 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("MovieClip.curveTo() takes four args"));
        );
        return as_value();
    }

    IF_VERBOSE_ASCODING_ERRORS(
    if ( fn.nargs > 4 )
    {
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("MovieClip.curveTo(%s): args after the first four "
                "will be discarded"), ss.str());
    }
    );

    double cx = fn.arg(0).to_number();
    double cy = fn.arg(1).to_number();
    double ax = fn.arg(2).to_number();
    double ay = fn.arg(3).to_number();

    if (!isFinite(cx))
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("%s.curveTo(%s) : non-finite first argument (%s), "
            "converted to zero"), movieclip->getTarget(),
            ss.str(), fn.arg(0));
        );
        cx = 0;
    }
     
    if (!isFinite(cy))
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("%s.curveTo(%s) : non-finite second argument (%s), "
            "converted to zero"), movieclip->getTarget(),
            ss.str(), fn.arg(1));
        );
        cy = 0;
    }

    if (!isFinite(ax))
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("%s.curveTo(%s) : non-finite third argument (%s), "
            "converted to zero"), movieclip->getTarget(),
            ss.str(), fn.arg(0));
        );
        ax = 0;
    }
     
    if (!isFinite(ay))
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("%s.curveTo(%s) : non-finite fourth argument (%s), "
            "converted to zero"), movieclip->getTarget(),
            ss.str(), fn.arg(1));
        );
        ay = 0;
    }

#ifdef DEBUG_DRAWING_API
    log_debug(_("%s.curveTo(%g,%g,%g,%g);"), movieclip->getTarget(),
            cx, cy, ax, ay);
#endif
    movieclip->curveTo(pixelsToTwips(cx), pixelsToTwips(cy),
            pixelsToTwips(ax), pixelsToTwips(ay));

    return as_value();
}

as_value
movieclip_clear(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    IF_VERBOSE_ASCODING_ERRORS(
    if ( fn.nargs )
    {
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("MovieClip.clear(%s): args will be discarded"),
            ss.str());
    }
    );

#ifdef DEBUG_DRAWING_API
    log_debug(_("%s.clear();"), movieclip->getTarget());
#endif
    movieclip->clear();

    return as_value();
}

as_value
movieclip_beginFill(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    if ( fn.nargs < 1 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror("beginFill() with no args is a no-op");
        );
        return as_value();
    }

    boost::uint8_t r = 0;
    boost::uint8_t g = 0;
    boost::uint8_t b = 0;
    boost::uint8_t a = 255;


    // 2^24 is the max here
    boost::uint32_t rgbval = boost::uint32_t(
            clamp<float>(fn.arg(0).to_number(), 0, 16777216));
    r = boost::uint8_t( (rgbval&0xFF0000) >> 16);
    g = boost::uint8_t( (rgbval&0x00FF00) >> 8);
    b = boost::uint8_t( (rgbval&0x0000FF) );

    if ( fn.nargs > 1 )
    {
        a = 255 * clamp<int>(fn.arg(1).to_int(), 0, 100) / 100;
        IF_VERBOSE_ASCODING_ERRORS(
        if ( fn.nargs > 2 )
        {
            std::stringstream ss; fn.dump_args(ss);
            log_aserror(_("MovieClip.beginFill(%s): args after the "
                    "first will be discarded"), ss.str());
        }
        );
    }

    rgba color(r, g, b, a);

#ifdef DEBUG_DRAWING_API
    log_debug(_("%s.beginFill(%d,%d,%d);"), movieclip->getTarget(), r, g, b);
#endif
    movieclip->beginFill(color);

    return as_value();
}

as_value
movieclip_beginGradientFill(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    if ( fn.nargs < 5 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("%s.beginGradientFill(%s): invalid call: 5 arguments "
                "needed"),
            movieclip->getTarget(), ss.str());
        );
        return as_value();
    }

    IF_VERBOSE_ASCODING_ERRORS(
    if ( fn.nargs > 5 )
    {
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("MovieClip.beginGradientFill(%s): args after "
                        "the first five will be discarded"), ss.str());
    }
    );

    bool radial = false;
    std::string typeStr = fn.arg(0).to_string();
    // Case-sensitive comparison needed for this ...
    if ( typeStr == "radial" ) radial = true;
    else if ( typeStr == "linear" ) radial = false;
    else
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("%s.beginGradientFill(%s): first arg must be "
            "'radial' or 'linear'"),
            movieclip->getTarget(), ss.str());
        );
        return as_value();
    }

    typedef boost::intrusive_ptr<as_object> ObjPtr;

    ObjPtr colors = fn.arg(1).to_object(*getGlobal(fn));
    ObjPtr alphas = fn.arg(2).to_object(*getGlobal(fn));
    ObjPtr ratios = fn.arg(3).to_object(*getGlobal(fn));
    ObjPtr matrixArg = fn.arg(4).to_object(*getGlobal(fn));

    if ( ! colors || ! alphas || ! ratios || ! matrixArg )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("%s.beginGradientFill(%s): one or more of the "
            " args from 2nd to 5th don't cast to objects"),
            movieclip->getTarget(), ss.str());
        );
        return as_value();
    }

    // ----------------------------
    // Parse SWFMatrix
    // ----------------------------
    
    //
    // TODO: fix the SWFMatrix build-up, it is NOT correct for
    //             rotation.
    //             For the "boxed" SWFMatrixType and radial fills this
    //             is not a problem as this code just discards the
    //             rotation (which doesn't make sense), but for
    //             the explicit SWFMatrix type (a..i) it is a problem.
    //             The whole code can likely be simplified by 
    //             always transforming the gnash gradients to the
    //             expected gradients and subsequently applying
    //             user-specified SWFMatrix; for 'boxed' SWFMatrixType
    //             this simplification would increas cost, but
    //             it's too early to apply optimizations to the
    //             code (correctness first!!).
    //

    SWFMatrix mat;
    SWFMatrix input_matrix;

    if ( matrixArg->getMember(NSV::PROP_MATRIX_TYPE).to_string() == "box" )
    {
        
        boost::int32_t valX = pixelsToTwips(
                matrixArg->getMember(NSV::PROP_X).to_number()); 
        boost::int32_t valY = pixelsToTwips(
                matrixArg->getMember(NSV::PROP_Y).to_number()); 
        boost::int32_t valW = pixelsToTwips(
                matrixArg->getMember(NSV::PROP_W).to_number()); 
        boost::int32_t valH = pixelsToTwips(
                matrixArg->getMember(NSV::PROP_H).to_number()); 
        float valR = matrixArg->getMember(NSV::PROP_R).to_number(); 

        if ( radial )
        {
            // Radial gradient is 64x64 twips.
            input_matrix.set_scale(64.0/valW, 64.0/valH);

            // For radial gradients, dunno why translation must be negative...
            input_matrix.concatenate_translation( -valX, -valY );

            // NOTE: rotation is intentionally discarded as it would
            //             have no effect (theoretically origin of the radial
            //             fill is at 0,0 making any rotation meaningless).

        }
        else
        {
            // Linear gradient is 256x1 twips.
            //
            // No idea why we should use the 256 value for Y scale, but 
            // empirically seems to give closer results. Note that it only
            // influences rotation, which is still not correct...
            // TODO: fix it !
            input_matrix.set_scale_rotation(256.0/valW, 256.0/valH, -valR);

            // For linear gradients, dunno why translation must be negative...
            input_matrix.concatenate_translation( -valX, -valY );
        }

        mat.concatenate(input_matrix);
    }
    else
    {
        float valA = matrixArg->getMember(NSV::PROP_A).to_number() ; // xx
        float valB = matrixArg->getMember(NSV::PROP_B).to_number() ; // yx
        float valD = matrixArg->getMember(NSV::PROP_D).to_number() ; // xy
        float valE = matrixArg->getMember(NSV::PROP_E).to_number() ; // yy
        boost::int32_t valG = pixelsToTwips(
                matrixArg->getMember(NSV::PROP_G).to_number()); // x0
        boost::int32_t valH = pixelsToTwips(
                matrixArg->getMember(NSV::PROP_H).to_number()); // y0

        input_matrix.sx    = valA * 65536; // sx
        input_matrix.shx = valB * 65536; // shy
        input_matrix.shy = valD * 65536; // shx
        input_matrix.sy    = valE * 65536; // sy
        input_matrix.tx = valG; // x0
        input_matrix.ty = valH; // y0

        // This is the SWFMatrix that would transform the gnash
        // gradient to the expected flash gradient.
        // Transformation is different for linear and radial
        // gradient for Gnash (in flash they should be the same)
        SWFMatrix gnashToFlash;

        if ( radial )
        {

            // Gnash radial gradients are 64x64 with center at 32,32
            // Should be 20x20 with center at 0,0
            const double g2fs = 20.0/64.0; // gnash to flash scale
            gnashToFlash.set_scale(g2fs, g2fs);
            gnashToFlash.concatenate_translation(-32, -32);

        }
        else
        {
            // First define a SWFMatrix that would transform
            // the gnash gradient to the expected flash gradient:
            // this means translating our gradient to put the
            // center of gradient at 0,0 and then scale it to
            // have a size of 20x20 instead of 256x1 as it is
            //
            // Gnash linear gradients are 256x1 with center at 128,0
            // Should be 20x20 with center at 0,0
            gnashToFlash.set_scale(20.0/256.0, 20.0/1);
            gnashToFlash.concatenate_translation(-128, 0);

        }

        // Apply gnash to flash SWFMatrix before user-defined one
        input_matrix.concatenate(gnashToFlash);

        // Finally, and don't know why, take
        // the inverse of the resulting SWFMatrix as
        // the one which would be used.
        mat = input_matrix;
        mat.invert();
    }

    // ----------------------------
    // Create the gradients vector
    // ----------------------------

    size_t ngradients = colors->getMember(NSV::PROP_LENGTH).to_int();
    // Check length compatibility of all args
    if ( ngradients != (size_t)alphas->getMember(NSV::PROP_LENGTH).to_int() ||
        ngradients != (size_t)ratios->getMember(NSV::PROP_LENGTH).to_int() )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("%s.beginGradientFill(%s): colors, alphas and "
            "ratios args don't have same length"),
            movieclip->getTarget(), ss.str());
        );
        return as_value();
    }

    // TODO: limit ngradients to a max ?
    if ( ngradients > 8 )
    {
        std::stringstream ss; fn.dump_args(ss);
        log_debug(_("%s.beginGradientFill(%s) : too many array elements"
            " for colors and ratios (%d), will trim to 8"), 
            movieclip->getTarget(), ss.str(), ngradients); 
        ngradients = 8;
    }

    string_table& st = getStringTable(fn);

    std::vector<gradient_record> gradients;
    gradients.reserve(ngradients);
    for (size_t i=0; i<ngradients; ++i)
    {

        string_table::key key = st.find(boost::lexical_cast<std::string>(i));

        as_value colVal = colors->getMember(key);
        boost::uint32_t col = colVal.is_number() ? colVal.to_int() : 0;

        as_value alpVal = alphas->getMember(key);
        boost::uint8_t alp = alpVal.is_number() ? 
            clamp<int>(alpVal.to_int(), 0, 255) : 0;

        as_value ratVal = ratios->getMember(key);
        boost::uint8_t rat = ratVal.is_number() ? 
            clamp<int>(ratVal.to_int(), 0, 255) : 0;

        rgba color;
        color.parseRGB(col);
        color.m_a = alp;

        gradients.push_back(gradient_record(rat, color));
    }

    if ( radial )
    {
        movieclip->beginRadialGradientFill(gradients, mat);
    }
    else
    {
        movieclip->beginLinearGradientFill(gradients, mat);
    }

    LOG_ONCE( log_debug("MovieClip.beginGradientFill() TESTING") );
    return as_value();
}

// startDrag([lockCenter:Boolean], [left:Number], [top:Number],
//    [right:Number], [bottom:Number]) : Void`
as_value
movieclip_startDrag(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    drag_state st;
    st.setCharacter( movieclip.get() );

    // mark this DisplayObject is transformed.
    movieclip->transformedByScript();

    if ( fn.nargs )
    {
        st.setLockCentered( fn.arg(0).to_bool() );

        if ( fn.nargs >= 5)
        {
            double x0 = fn.arg(1).to_number();
            double y0 = fn.arg(2).to_number();
            double x1 = fn.arg(3).to_number();
            double y1 = fn.arg(4).to_number();

            // check for infinite values
            bool gotinf = false;
            if (!isFinite(x0) ) { x0=0; gotinf=true; }
            if (!isFinite(y0) ) { y0=0; gotinf=true; }
            if (!isFinite(x1) ) { x1=0; gotinf=true; }
            if (!isFinite(y1) ) { y1=0; gotinf=true; }

            // check for swapped values
            bool swapped = false;
            if ( y1 < y0 )
            {
                std::swap(y1, y0);
                swapped = true;
            }

            if ( x1 < x0 )
            {
                std::swap(x1, x0);
                swapped = true;
            }

            IF_VERBOSE_ASCODING_ERRORS(
                if ( gotinf || swapped ) {
                    std::stringstream ss; fn.dump_args(ss);
                    if ( swapped ) { 
                        log_aserror(_("min/max bbox values in "
                            "MovieClip.startDrag(%s) swapped, fixing"),
                            ss.str());
                    }
                    if ( gotinf ) {
                        log_aserror(_("non-finite bbox values in "
                            "MovieClip.startDrag(%s), took as zero"),
                            ss.str());
                    }
                }
            );

            rect bounds(pixelsToTwips(x0), pixelsToTwips(y0),
                    pixelsToTwips(x1), pixelsToTwips(y1));
            st.setBounds(bounds);
        }
    }

    getRoot(fn).set_drag_state(st);

    return as_value();
}

// stopDrag() : Void
as_value
movieclip_stopDrag(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    getRoot(fn).stop_drag();

    return as_value();
}


as_value
movieclip_beginBitmapFill(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = 
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}


as_value
movieclip_getRect(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = 
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}


as_value
movieclip_lineGradientStyle(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = 
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}


as_value
movieclip_attachBitmap(const fn_call& fn)
{

    GNASH_REPORT_FUNCTION;

    boost::intrusive_ptr<MovieClip> ptr = ensureType<MovieClip>(fn.this_ptr);

    if (fn.nargs < 2) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_debug("MovieClip.attachBitmap: expected 2 args, got %d",
                fn.nargs);
        );
        return as_value();
    }

    as_object* obj = fn.arg(0).to_object(*getGlobal(fn)).get();
    boost::intrusive_ptr<BitmapData_as> bd = dynamic_cast<BitmapData_as*>(obj);

    if (!bd) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_debug("MovieClip.attachBitmap: first argument should be a "
                "BitmapData", fn.arg(1));
        );
        return as_value();
    }

    int depth = fn.arg(1).to_int();

    ptr->attachBitmap(bd, depth);

    return as_value();
}


as_value
movieclip_as2_ctor(const fn_call& fn)
{

    assert(!isAS3(fn));

    boost::intrusive_ptr<as_object> clip = 
        new as_object(getMovieClipAS2Interface());

    return as_value(clip.get());
}


as_value
movieclip_currentFrame(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = ensureType<MovieClip>(fn.this_ptr);

    return as_value(std::min(ptr->get_loaded_frames(),
                ptr->get_current_frame() + 1));
}

as_value
movieclip_totalFrames(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = 
        ensureType<MovieClip>(fn.this_ptr);

    return as_value(ptr->get_frame_count());
}

as_value
movieclip_framesLoaded(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = 
        ensureType<MovieClip>(fn.this_ptr);

    return as_value(ptr->get_loaded_frames());
}

as_value
movieclip_dropTarget(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = 
        ensureType<MovieClip>(fn.this_ptr);

    return ptr->getDropTarget();
}

as_value
movieclip_url(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = ensureType<MovieClip>(fn.this_ptr);

    return as_value(ptr->get_root()->url());
}

// TODO: move this to DisplayObject class, _focusrect seems a generic property
as_value
movieclip_focusRect(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = ensureType<MovieClip>(fn.this_ptr);
    UNUSED(ptr);

    if ( fn.nargs == 0 ) // getter
    {
        // Is a yellow rectangle visible around a focused movie clip (?)
        // We don't support focuserct settings
        return as_value(false);
    }
    else // setter
    {
        LOG_ONCE( log_unimpl("MovieClip._focusrect setting") );
    }
    return as_value();
}

as_value
movieclip_soundbuftime(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = 
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(ptr);

    if ( fn.nargs == 0 ) // getter
    {
        // Number of seconds before sound starts to stream.
        return as_value(0.0);
    }
    else // setter
    {
        LOG_ONCE( log_unimpl("MovieClip._soundbuftime setting") );
    }
    return as_value();
}

as_value
movieclip_transform(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = ensureType<MovieClip>(fn.this_ptr);

    // If not found, construction fails.
    as_value transform(fn.env().find_object("flash.geom.Transform"));

    boost::intrusive_ptr<as_function> transCtor = transform.to_as_function();

    if (!transCtor) {
        log_error("Failed to construct flash.geom.Transform!");
        return as_value();
    }

    // Construct a flash.geom.Transform object with "this" as argument.
    std::auto_ptr<std::vector<as_value> > args(new std::vector<as_value>);
    args->push_back(ptr.get());

    boost::intrusive_ptr<as_object> newTrans =
        transCtor->constructInstance(fn.env(), args);

    return as_value(newTrans.get());
}

// =======================
// AS3 interface
// =======================

as_value
movieclip_as3_ctor(const fn_call& fn)
{
    assert(isAS3(fn));

    // TODO: currently it's necessary to have a top-level movie to initialize
    // a MovieClip.
    Movie* m = getRoot(fn).topLevelMovie();

    return new MovieClip(0, m, 0, -1);
}


void
attachMovieClipAS3Interface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    o.init_member("gotoAndStop", gl->createFunction(movieclip_gotoAndStop));
    o.init_member("nextFrame", gl->createFunction(movieclip_nextFrame));
    o.init_member("nextScene", gl->createFunction(movieclip_nextScene));
    o.init_member("play", gl->createFunction(movieclip_play));
    o.init_member("prevFrame", gl->createFunction(movieclip_prevFrame));
    o.init_member("prevScene", gl->createFunction(movieclip_prevScene));
    o.init_member("stop", gl->createFunction(movieclip_stop));
    o.init_member("addFrameScript", gl->createFunction(
                movieclip_addFrameScript));
}

as_value
movieclip_addFrameScript(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = ensureType<MovieClip>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
movieclip_nextScene(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = ensureType<MovieClip>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
movieclip_prevScene(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = ensureType<MovieClip>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

