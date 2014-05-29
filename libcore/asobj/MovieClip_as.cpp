// MovieClip_as.cpp:  ActionScript "MovieClip" class, for Gnash.
//
//   Copyright (C) 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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

#include "MovieClip_as.h"

#include <boost/lexical_cast.hpp>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

#include "MovieClip.h"
#include "Movie.h"
#include "display/BitmapData_as.h"
#include "NetStream_as.h"
#include "movie_root.h"
#include "GnashNumeric.h"
#include "as_value.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "NativeFunction.h" 
#include "Bitmap.h"
#include "Array_as.h"
#include "FillStyle.h"
#include "namedStrings.h"
#include "Renderer.h"
#include "RunResources.h"
#include "ASConversions.h"

namespace gnash {

// Forward declarations
namespace {

    void attachMovieClipAS2Interface(as_object& o);

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
    as_value movieclip_lockroot(const fn_call& fn);
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
    as_value movieclip_beginMeshFill(const fn_call& fn);
    as_value movieclip_getRect(const fn_call& fn);
    as_value movieclip_meth(const fn_call& fn);
    as_value movieclip_getSWFVersion(const fn_call& fn);
    as_value movieclip_loadVariables(const fn_call& fn);

}

// extern (used by Global.cpp)
// proto = getDisplayObjectContainerInterface();
// proto = getDisplayObjectContainerInterface();
void
movieclip_class_init(as_object& where, const ObjectURI& uri)
{
    Global_as& gl = getGlobal(where);
    as_object* proto = createObject(gl);

    as_object* cl = gl.createClass(emptyFunction, proto);
    attachMovieClipAS2Interface(*proto);

    where.init_member(uri, cl, as_object::DefaultFlags);
}

void
registerMovieClipNative(as_object& where)
{
    VM& vm = getVM(where);

    vm.registerNative(movieclip_attachMovie, 900, 0); 
    vm.registerNative(movieclip_swapDepths, 900, 1); 
    vm.registerNative(movieclip_localToGlobal, 900, 2);
    vm.registerNative(movieclip_globalToLocal, 900, 3);
    vm.registerNative(movieclip_hitTest, 900, 4);
    vm.registerNative(movieclip_getBounds, 900, 5);
    vm.registerNative(movieclip_getBytesTotal, 900, 6);
    vm.registerNative(movieclip_getBytesLoaded, 900, 7);
    vm.registerNative(movieclip_attachAudio, 900, 8);
    vm.registerNative(movieclip_attachVideo, 900, 9);
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
    vm.registerNative(movieclip_getNextHighestDepth, 900, 22);
    vm.registerNative(movieclip_getInstanceAtDepth, 900, 23);
    vm.registerNative(movieclip_getSWFVersion, 900, 24);
    vm.registerNative(movieclip_attachBitmap, 900, 25);
    vm.registerNative(movieclip_getRect, 900, 26);
    
    vm.registerNative(movieclip_tabIndex, 900, 200);
    
    vm.registerNative(movieclip_lockroot, 900, 300);
    
    vm.registerNative(movieclip_cacheAsBitmap, 900, 401);
    vm.registerNative(movieclip_opaqueBackground, 900, 402);
    vm.registerNative(movieclip_scrollRect, 900, 403);

    vm.registerNative(movieclip_filters, 900, 417);
    vm.registerNative(movieclip_transform, 900, 418);
    vm.registerNative(DisplayObject::blendMode, 900, 500);
    vm.registerNative(movieclip_forceSmoothing, 900, 502);

    vm.registerNative(movieclip_createEmptyMovieClip, 901, 0);
    vm.registerNative(movieclip_beginFill, 901, 1);
    vm.registerNative(movieclip_beginGradientFill, 901, 2);
    vm.registerNative(movieclip_moveTo, 901, 3);
    vm.registerNative(movieclip_lineTo, 901, 4);
    vm.registerNative(movieclip_curveTo, 901, 5);
    vm.registerNative(movieclip_lineStyle, 901, 6);
    vm.registerNative(movieclip_endFill, 901, 7);
    vm.registerNative(movieclip_clear, 901, 8);
    vm.registerNative(movieclip_lineGradientStyle, 901, 9);
    vm.registerNative(movieclip_beginMeshFill, 901, 10);
    vm.registerNative(movieclip_beginBitmapFill, 901, 11);
    vm.registerNative(movieclip_scale9Grid, 901, 12);

}


namespace {

// =======================
// AS2 interface
// =======================

/// Properties (and/or methods) *inherited* by MovieClip instances
void
attachMovieClipAS2Interface(as_object& o)
{
    Global_as& gl = getGlobal(o);
    VM& vm = getVM(o);
    
    const int swf6Flags = as_object::DefaultFlags | PropFlags::onlySWF6Up;
    const int swf7Flags = as_object::DefaultFlags | PropFlags::onlySWF7Up;
    const int swf8Flags = as_object::DefaultFlags | PropFlags::onlySWF8Up;

    o.init_member("attachMovie", vm.getNative(900, 0)); 
    o.init_member("swapDepths", vm.getNative(900, 1));
    o.init_member("localToGlobal", vm.getNative(900, 2));
    o.init_member("globalToLocal", vm.getNative(900, 3));
    o.init_member("hitTest", vm.getNative(900, 4));
    o.init_member("getBounds", vm.getNative(900, 5));
    o.init_member("getBytesTotal", vm.getNative(900, 6));
    o.init_member("getBytesLoaded", vm.getNative(900, 7));
    o.init_member("attachAudio", vm.getNative(900, 8), swf6Flags);
    o.init_member("attachVideo", vm.getNative(900, 9), swf6Flags);
    o.init_member("getDepth", vm.getNative(900, 10), swf6Flags);
    o.init_member("setMask", vm.getNative(900, 11), swf6Flags);
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
    o.init_member("getNextHighestDepth", vm.getNative(900, 22), swf7Flags);
    o.init_member("getInstanceAtDepth", vm.getNative(900, 23), swf7Flags);
    o.init_member("getSWFVersion", vm.getNative(900, 24));
    o.init_member("attachBitmap", vm.getNative(900, 25), swf8Flags); 
    o.init_member("getRect", vm.getNative(900, 26), swf8Flags);

    o.init_member("loadMovie", gl.createFunction(movieclip_loadMovie));
    o.init_member("loadVariables", gl.createFunction(movieclip_loadVariables));
    o.init_member("unloadMovie", gl.createFunction( movieclip_unloadMovie));
    o.init_member("getURL", gl.createFunction(movieclip_getURL));
    o.init_member("meth", gl.createFunction(movieclip_meth));

    o.init_member("enabled", true);
    o.init_member("useHandCursor", true);

    o.init_member("createEmptyMovieClip", vm.getNative(901, 0), swf6Flags);
    o.init_member("beginFill", vm.getNative(901, 1), swf6Flags);
    o.init_member("beginGradientFill", vm.getNative(901, 2), swf6Flags);
    o.init_member("moveTo", vm.getNative(901, 3), swf6Flags);
    o.init_member("lineTo", vm.getNative(901, 4), swf6Flags);
    o.init_member("curveTo", vm.getNative(901, 5), swf6Flags);
    o.init_member("lineStyle", vm.getNative(901, 6), swf6Flags);
    o.init_member("endFill", vm.getNative(901, 7), swf6Flags);
    o.init_member("clear", vm.getNative(901, 8), swf6Flags);
    o.init_member("lineGradientStyle", vm.getNative(901, 9), swf8Flags);
    o.init_member("beginMeshFill", vm.getNative(901, 10), swf8Flags);
    o.init_member("beginBitmapFill", vm.getNative(901, 11), swf8Flags);

    // Accessors

    NativeFunction* getset;

    getset = vm.getNative(900, 200);
    o.init_property("tabIndex", *getset, *getset);
    getset = vm.getNative(900, 300);
    o.init_property("_lockroot", *getset, *getset);
    getset = vm.getNative(900, 401);
    o.init_property("cacheAsBitmap", *getset, *getset, swf8Flags);
    getset = vm.getNative(900, 402);
    o.init_property("opaqueBackground", *getset, *getset, swf8Flags);
    getset = vm.getNative(900, 403);
    o.init_property("scrollRect", *getset, *getset, swf8Flags);
    getset = vm.getNative(900, 417);
    o.init_property("filters", *getset, *getset, swf8Flags);
    getset = vm.getNative(900, 418);
    o.init_property("transform", *getset, *getset, swf8Flags);
    getset = vm.getNative(900, 500);
    o.init_property("blendMode", *getset, *getset, swf8Flags);
    getset = vm.getNative(900, 502);
    o.init_property("forceSmoothing", *getset, *getset, swf8Flags);
    getset = vm.getNative(901, 12);
    o.init_property("scale9Grid", *getset, *getset, swf8Flags);

    // External functions.
    o.init_member("createTextField", vm.getNative(104, 200));
    o.init_member("getTextSnapshot", 
            gl.createFunction(movieclip_getTextSnapshot), swf6Flags);

}

//createEmptyMovieClip(name:String, depth:Number) : MovieClip
as_value
movieclip_createEmptyMovieClip(const fn_call& fn)
{
    MovieClip* ptr = ensure<IsDisplayObject<MovieClip> >(fn);

    if (fn.nargs != 2) {
        if (fn.nargs < 2) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("createEmptyMovieClip needs "
                    "2 args, but %d given,"
                    " returning undefined"),
                    fn.nargs);
            );
            return as_value();
        }
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("createEmptyMovieClip takes "
                "2 args, but %d given, discarding"
                " the excess"),
                fn.nargs);
        )
    }

    Movie* m = ptr->get_root();
    as_object* o = getObjectWithPrototype(getGlobal(fn), NSV::CLASS_MOVIE_CLIP);
    MovieClip* mc = new MovieClip(o, nullptr, m, ptr);

    VM& vm = getVM(fn);
    mc->set_name(getURI(vm, fn.arg(0).to_string()));
    mc->setDynamic();

    // Unlike other MovieClip methods, the depth argument of an empty movie clip
    // can be any number. All numbers are converted to an int32_t, and are valid
    // depths even when outside the usual bounds.
    ptr->addDisplayListObject(mc, toInt(fn.arg(1), getVM(fn)));
    return as_value(o);
}


as_value
movieclip_play(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);

    movieclip->setPlayState(MovieClip::PLAYSTATE_PLAY);
    return as_value();
}

as_value
movieclip_stop(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);

    movieclip->setPlayState(MovieClip::PLAYSTATE_STOP);

    return as_value();
}


//removeMovieClip() : Void
as_value
movieclip_removeMovieClip(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);
    movieclip->removeMovieClip();
    return as_value();
}


as_value
movieclip_cacheAsBitmap(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);
    UNUSED(movieclip);
    LOG_ONCE( log_unimpl(_("MovieClip.cacheAsBitmap()")) );
    return as_value();
}


as_value
movieclip_filters(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);
    
    UNUSED(movieclip);

    LOG_ONCE(log_unimpl(_("MovieClip.filters()")));

    if (!fn.nargs) {
        // Getter
        Global_as& gl = getGlobal(fn);
        as_object* array = gl.createArray();
        return as_value(array);
    }

    // Setter
    return as_value();
}


as_value
movieclip_forceSmoothing(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);
    UNUSED(movieclip);
    LOG_ONCE(log_unimpl(_("MovieClip.forceSmoothing()")));
    return as_value();
}


as_value
movieclip_opaqueBackground(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);
    UNUSED(movieclip);
    LOG_ONCE(log_unimpl(_("MovieClip.opaqueBackground()")));
    return as_value();
}

    
as_value
movieclip_scale9Grid(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);
    UNUSED(movieclip);
    LOG_ONCE(log_unimpl(_("MovieClip.scale9Grid()")));
    return as_value();
}


as_value
movieclip_scrollRect(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);
    UNUSED(movieclip);
    LOG_ONCE(log_unimpl(_("MovieClip.scrollRect()")));
    return as_value();
}


as_value
movieclip_tabIndex(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);
    UNUSED(movieclip);
    LOG_ONCE(log_unimpl(_("MovieClip.tabIndex()")));
    return as_value();
}


// attachMovie(idName:String, newName:String,
//                         depth:Number [, initObject:Object]) : MovieClip
as_value
movieclip_attachMovie(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);

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

    SWF::DefinitionTag* exported_movie = 
        movieclip->get_root()->exportedCharacter(id_name);

    if (!exported_movie)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("attachMovie: exported resource '%s' "
            "is not a DisplayObject definition. Returning undefined"), id_name);
        );
        return as_value();
    }

    const std::string& newname = fn.arg(1).to_string();

    // Movies should be attachable from -16384 to 2130690045, according to
    // kirupa (http://www.kirupa.com/developer/actionscript/depths2.htm)
    // Tests in misc-ming.all/DepthLimitsTest.c show that 2130690044 is the
    // maximum valid depth.
    const double depth = toNumber(fn.arg(2), getVM(fn));
    
    // This also checks for overflow, as both numbers are expressible as
    // std::int32_t.
    if (depth < DisplayObject::lowerAccessibleBound ||
            depth > DisplayObject::upperAccessibleBound)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("MovieClip.attachMovie: invalid depth %d "
                    "passed; not attaching"), depth);
        );
        return as_value();
    }
    
    std::int32_t depthValue = static_cast<std::int32_t>(depth);

    Global_as& gl = getGlobal(fn);
    DisplayObject* newch = exported_movie->createDisplayObject(gl, movieclip);

    VM& vm = getVM(fn);
    newch->set_name(getURI(vm, newname));
    newch->setDynamic();

    as_object* initObj(nullptr);

    if (fn.nargs > 3 ) {
        initObj = toObject(fn.arg(3), getVM(fn));
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
    movieclip->attachCharacter(*newch, depthValue, initObj);

    return as_value(getObject(newch));
}


// attachAudio(id:Object) : Void
as_value
movieclip_attachAudio(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);

    if (!fn.nargs) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("MovieClip.attachAudio(): %s"),
			  _("missing arguments"));
        );
        return as_value();
    }

    NetStream_as* ns;
    if (!isNativeType(toObject(fn.arg(0), getVM(fn)), ns)) { 
        std::stringstream ss; fn.dump_args(ss);
        // TODO: find out what to do here
        log_error(_("MovieClip.attachAudio(%s): first arg doesn't cast to a "
		    "NetStream"), ss.str());
        return as_value();
    }

    ns->setAudioController(movieclip);

    return as_value();
}


// MovieClip.attachVideo
as_value
movieclip_attachVideo(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);
    UNUSED(movieclip);

    LOG_ONCE(log_unimpl(_("MovieClip.attachVideo()")));
    return as_value();
}


as_value
movieclip_getDepth(const fn_call& fn)
{
    // Unlike TextField.getDepth this works for any DisplayObject
    DisplayObject* d = ensure<IsDisplayObject<> >(fn);
    return as_value(d->get_depth());
}

//swapDepths(target:Object|target:Number)
//
// Returns void.
as_value
movieclip_swapDepths(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);

    const int this_depth = movieclip->get_depth();

    if (fn.nargs < 1) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("%s.swapDepths() needs one arg"), movieclip->getTarget());
        );
        return as_value();
    }

    // Lower bound of source depth below which swapDepth has no effect
    // (below Timeline/static zone)
    if (this_depth < DisplayObject::lowerAccessibleBound) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss;
            fn.dump_args(ss);
            log_aserror(_("%s.swapDepths(%s): won't swap a clip below "
                "depth %d (%d)"), movieclip->getTarget(), ss.str(),
                DisplayObject::lowerAccessibleBound,
                this_depth);
        )
        return as_value();
    }

    MovieClip* this_parent = dynamic_cast<MovieClip*>(movieclip->parent());

    //CharPtr target = NULL;
    int target_depth = 0;

    // movieclip.swapDepth(movieclip)
    if (MovieClip* target_movieclip = fn.arg(0).toMovieClip()) {

        if (movieclip == target_movieclip) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("%s.swapDepths(%s): invalid call, "
                    "swapping to self?"), movieclip->getTarget(),
                    target_movieclip->getTarget());
            )
            return as_value();
        }

        MovieClip* target_parent =
            dynamic_cast<MovieClip*>(movieclip->parent());

        if (this_parent != target_parent) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("%s.swapDepths(%s): invalid call, the two "
                    "DisplayObjects don't have the same parent"),
                    movieclip->getTarget(), target_movieclip->getTarget());
            )
            return as_value();
        }

        target_depth = target_movieclip->get_depth();

        // Check we're not swapping the our own depth so
        // to avoid unecessary bounds invalidation and immunizing
        // the instance from subsequent PlaceObject tags attempting
        // to transform it.
        if (movieclip->get_depth() == target_depth)
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
    else {
        
        const double td = toNumber(fn.arg(0), getVM(fn));
        if (isNaN(td)) {
            IF_VERBOSE_ASCODING_ERRORS(
                std::stringstream ss; fn.dump_args(ss);
                log_aserror(_("%s.swapDepths(%s): first argument invalid "
                    "(neither a movieclip nor a number)"),
                    movieclip->getTarget(), ss.str());
            )
            return as_value();
        }
        if (td > DisplayObject::upperAccessibleBound) {
            IF_VERBOSE_ASCODING_ERRORS(
                std::stringstream ss; fn.dump_args(ss);
                log_aserror(_("%s.swapDepths(%s): requested depth is above "
                    "the accessible range."),
                    movieclip->getTarget(), ss.str());
            )
            return as_value();
        }

        target_depth = static_cast<int>(td);

        // Check we're not swapping the our own depth so
        // to avoid unecessary bounds invalidation and immunizing
        // the instance from subsequent PlaceObjec tags attempting
        // to transform it.
        if (movieclip->get_depth() == target_depth) {
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

    if (this_parent) {
        this_parent->swapDepths(movieclip, target_depth);
    }
    else {
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
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);
    
    if (fn.nargs < 2)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.duplicateMovieClip() needs 2 or 3 args"));
                );
        return as_value();
    }

    const std::string& newname = fn.arg(0).to_string();

    // Depth as in attachMovie
    const double depth = toNumber(fn.arg(1), getVM(fn));
    
    // This also checks for overflow, as both numbers are expressible as
    // std::int32_t.
    if (depth < DisplayObject::lowerAccessibleBound ||
            depth > DisplayObject::upperAccessibleBound)
    {
        IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("MovieClip.duplicateMovieClip: "
                        "invalid depth %d passed; not duplicating"), depth);
        );    
        return as_value();
    }
    
    std::int32_t depthValue = static_cast<std::int32_t>(depth);

    MovieClip* ch;

    // Copy members from initObject
    if (fn.nargs == 3)
    {
        as_object* initObject = toObject(fn.arg(2), getVM(fn));
        ch = movieclip->duplicateMovieClip(newname, depthValue, initObject);
    }
    else
    {
        ch = movieclip->duplicateMovieClip(newname, depthValue);
    }

    return as_value(getObject(ch));
}

as_value
movieclip_gotoAndPlay(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);

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

as_value
movieclip_gotoAndStop(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);

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

as_value
movieclip_nextFrame(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);

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
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);

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
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);

    return as_value(movieclip->get_bytes_loaded());
}

as_value
movieclip_getBytesTotal(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);

    return as_value(movieclip->get_bytes_total());
}

// MovieClip.loadMovie(url:String [,variables:String]).
//
// Returns 1 for "get", 2 for "post", and otherwise 0. Case-insensitive.
// This *always* calls MovieClip.meth.
as_value
movieclip_loadMovie(const fn_call& fn)
{
    DisplayObject* dobj = ensure<IsDisplayObject<> >(fn);

    as_value val;
    if (fn.nargs > 1) {
        val = callMethod(getObject(dobj), NSV::PROP_METH, fn.arg(1));
    }
    else val = callMethod(getObject(dobj), NSV::PROP_METH);

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
    std::string target = dobj->getTarget();

    // TODO: if GET/POST should send variables of *this* movie,
    // no matter if the target will be replaced by another movie !!
    const MovieClip::VariablesMethod method =
        static_cast<MovieClip::VariablesMethod>(toInt(val, getVM(fn)));

    std::string data;

    // This is just an optimization if we aren't going
    // to send the data anyway. It might be wrong, though.
    if (method != MovieClip::METHOD_NONE) {
        data = getURLEncodedVars(*getObject(dobj));
    }
 
    mr.loadMovie(urlstr, target, data, method);

    return as_value();
}

// my_mc.loadVariables(url:String [, variables:String]) : Void
as_value
movieclip_loadVariables(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);

    as_object* obj = getObject(movieclip);
    assert(obj);

    // This always calls MovieClip.meth, even when there are no
    // arguments.
    as_value val;
    if (fn.nargs > 1)
    {
        val = callMethod(obj, NSV::PROP_METH, fn.arg(1));
    }
    else val = callMethod(obj, NSV::PROP_METH);

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
        static_cast<MovieClip::VariablesMethod>(toInt(val, getVM(fn)));

    movieclip->loadVariables(urlstr, method);
    log_debug("MovieClip.loadVariables(%s) - TESTING ", urlstr);

    return as_value();
}

// my_mc.unloadMovie() : Void
as_value
movieclip_unloadMovie(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);

    movieclip->unloadMovie();

    return as_value();
}

as_value
movieclip_hitTest(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);

    switch (fn.nargs)
    {
        case 1: // target
        {
            const as_value& tgt_val = fn.arg(0);
            DisplayObject* target = findTarget(fn.env(), tgt_val.to_string());
            if ( ! target )
            {
                IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("Can't find hitTest target %s"),
                    tgt_val);
                );
                return as_value();
            }

            SWFRect thisbounds = movieclip->getBounds();
            const SWFMatrix thismat = getWorldMatrix(*movieclip);
            thismat.transform(thisbounds);

            SWFRect tgtbounds = target->getBounds();
            SWFMatrix tgtmat = getWorldMatrix(*target);
            tgtmat.transform(tgtbounds);

            return thisbounds.getRange().intersects(tgtbounds.getRange());

            break;
        }

        case 2: // x, y
        {
            std::int32_t x = pixelsToTwips(toNumber(fn.arg(0), getVM(fn)));
            std::int32_t y = pixelsToTwips(toNumber(fn.arg(1), getVM(fn)));

            return movieclip->pointInBounds(x, y);
        }

        case 3: // x, y, shapeFlag
        {
             const std::int32_t x = pixelsToTwips(toNumber(fn.arg(0),
                         getVM(fn)));
             const std::int32_t y = pixelsToTwips(toNumber(fn.arg(1),
                         getVM(fn)));
             const bool shapeFlag = toBool(fn.arg(2), getVM(fn));

             if (!shapeFlag) return movieclip->pointInBounds(x, y);
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

//getNextHighestDepth() : Number
as_value
movieclip_getNextHighestDepth(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);

    int nextdepth = movieclip->getNextHighestDepth();
    return as_value(static_cast<double>(nextdepth));
}

//getInstanceAtDepth(depth:Number) : MovieClip
as_value
movieclip_getInstanceAtDepth(const fn_call& fn)
{
    MovieClip* mc = ensure<IsDisplayObject<MovieClip> >(fn);

    if (fn.nargs < 1 || fn.arg(0).is_undefined()) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.getInstanceAtDepth(): missing or "
		      "undefined depth argument"));
        );
        return as_value();
    }

    const int depth = toInt(fn.arg(0), getVM(fn));

    DisplayObject* ch = mc->getDisplayObjectAtDepth(depth);
 
    // we want 'undefined', not 'null'
    if (!ch) return as_value();

    return as_value(getObject(ch));
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
    as_object* movieclip = ensure<ValidThis>(fn);

    std::string urlstr;
    std::string target;

    as_value val;
    if (fn.nargs > 2)
    {
        val = callMethod(movieclip, NSV::PROP_METH, fn.arg(2));
    }
    else val = callMethod(movieclip, NSV::PROP_METH);

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
            /* Fall through */
        case 3:
            // This argument has already been handled.
        case 2:
             target = fn.arg(1).to_string();
        case 1:
             urlstr = fn.arg(0).to_string();
             break;
    }


    MovieClip::VariablesMethod method =
        static_cast<MovieClip::VariablesMethod>(toInt(val, getVM(fn)));

    std::string vars;

    if (method != MovieClip::METHOD_NONE) {
        // Get encoded vars.
        vars = getURLEncodedVars(*movieclip);
    }

    movie_root& m = getRoot(fn);
    
    m.getURL(urlstr, target, vars, method);

    return as_value();
}

// getSWFVersion() : Number
as_value
movieclip_getSWFVersion(const fn_call& fn)
{
    DisplayObject* o = get<DisplayObject>(fn.this_ptr);
    if (!o) return as_value(-1);
    return as_value(o->getDefinitionVersion());
}

// MovieClip.meth(<string>) : Number
//
// Parses case-insensitive "get" and "post" into 1 and 2, 0 anything else
// 
as_value
movieclip_meth(const fn_call& fn)
{

    if (!fn.nargs) return as_value(MovieClip::METHOD_NONE); 

    as_object* o = toObject(fn.arg(0), getVM(fn));
    if (!o) {
        return as_value(MovieClip::METHOD_NONE);
    }

    as_value lc = callMethod(o, NSV::PROP_TO_LOWER_CASE);

    std::string s = lc.to_string();

    if (s == "get") return as_value(MovieClip::METHOD_GET);
    if (s == "post") return as_value(MovieClip::METHOD_POST);
    return as_value(MovieClip::METHOD_NONE);
}


// getTextSnapshot() : TextSnapshot
as_value
movieclip_getTextSnapshot(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);

    // If not found, construction fails.
    as_value textSnapshot(findObject(fn.env(), "TextSnapshot"));

    as_function* tsCtor = textSnapshot.to_function();

    if (!tsCtor) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("MovieClip.getTextSnapshot: failed to construct "
			  "TextSnapshot (object probably overridden)"));
        );
        return as_value();
    }

    // Construct a flash.geom.Transform object with "this" as argument.
    fn_call::Args args;
    args += getObject(movieclip);

    as_object* ts = constructInstance(*tsCtor, fn.env(), args);

    return as_value(ts);
}


// getBounds(targetCoordinateSpace:Object) : Object
as_value
movieclip_getBounds(const fn_call& fn)
{
    DisplayObject* movieclip = ensure<IsDisplayObject<> >(fn);

    SWFRect bounds = movieclip->getBounds();

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

        const SWFMatrix tgtwmat = getWorldMatrix(*target).invert();
        const SWFMatrix srcwmat = getWorldMatrix(*movieclip);

        srcwmat.transform(bounds);
        tgtwmat.transform(bounds);
    }

    double xMin, yMin, xMax, yMax;

    if (!bounds.is_null()) {
        // Round to the twip
        xMin = twipsToPixels(bounds.get_x_min());
        yMin = twipsToPixels(bounds.get_y_min());
        xMax = twipsToPixels(bounds.get_x_max());
        yMax = twipsToPixels(bounds.get_y_max());
    }
    else {
        const double magicMin = 6710886.35;
        xMin = yMin = xMax = yMax = magicMin;
    }

    // This is a bare object.
    as_object* bounds_obj = new as_object(getGlobal(fn));
    bounds_obj->init_member("xMin", xMin);
    bounds_obj->init_member("yMin", yMin);
    bounds_obj->init_member("xMax", xMax);
    bounds_obj->init_member("yMax", yMax);

    return as_value(bounds_obj);
}

as_value
movieclip_globalToLocal(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);

    as_value ret;

    if ( fn.nargs < 1 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.globalToLocal() takes one arg"));
        );
        return ret;
    }

    as_object* obj = toObject(fn.arg(0), getVM(fn));
    if (!obj) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.globalToLocal(%s): "
                "first argument doesn't cast to an object"),
            fn.arg(0));
        );
        return ret;
    }

    as_value tmp;
    std::int32_t    x = 0;
    std::int32_t    y = 0;

    if ( ! obj->get_member(NSV::PROP_X, &tmp) )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.globalToLocal(%s): "
                "object parameter doesn't have an 'x' member"),
            fn.arg(0));
        );
        return ret;
    }
    x = pixelsToTwips(toNumber(tmp, getVM(fn)));

    if ( ! obj->get_member(NSV::PROP_Y, &tmp) )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.globalToLocal(%s): "
                "object parameter doesn't have an 'y' member"),
            fn.arg(0));
        );
        return ret;
    }
    y = pixelsToTwips(toNumber(tmp, getVM(fn)));

    point    pt(x, y);
    const SWFMatrix world_mat = getWorldMatrix(*movieclip).invert();
    world_mat.transform(pt);

    obj->set_member(NSV::PROP_X, twipsToPixels(pt.x));
    obj->set_member(NSV::PROP_Y, twipsToPixels(pt.y));

    return ret;
}

as_value
movieclip_localToGlobal(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);

    as_value ret;

    if ( fn.nargs < 1 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.localToGlobal() takes one arg"));
        );
        return ret;
    }

    as_object* obj = toObject(fn.arg(0), getVM(fn));
    if (!obj) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.localToGlobal(%s): "
                "first argument doesn't cast to an object"),
            fn.arg(0));
        );
        return ret;
    }

    as_value tmp;
    std::int32_t x = 0;
    std::int32_t y = 0;

    if (!obj->get_member(NSV::PROP_X, &tmp)) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.localToGlobal(%s): "
                "object parameter doesn't have an 'x' member"),
            fn.arg(0));
        );
        return ret;
    }
    x = pixelsToTwips(toNumber(tmp, getVM(fn)));

    if ( ! obj->get_member(NSV::PROP_Y, &tmp) )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.localToGlobal(%s): "
                "object parameter doesn't have an 'y' member"),
            fn.arg(0));
        );
        return ret;
    }
    y = pixelsToTwips(toNumber(tmp, getVM(fn)));

    point pt(x, y);
    const SWFMatrix world_mat = getWorldMatrix(*movieclip);
    world_mat.transform(pt);

    obj->set_member(NSV::PROP_X, twipsToPixels(pt.x));
    obj->set_member(NSV::PROP_Y, twipsToPixels(pt.y));
    return ret;

}

as_value
movieclip_setMask(const fn_call& fn)
{
    // swfdec/test/image/mask-textfield-6.swf shows that setMask should also
    // work against TextFields, we have no tests for other DisplayObject
    // types so we generalize it for any DisplayObject.
    DisplayObject* maskee = ensure<IsDisplayObject<> >(fn);

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
        maskee->setMask(nullptr);
    }
    else
    {

        as_object* obj = toObject(arg, getVM(fn));
        DisplayObject* mask = get<DisplayObject>(obj);
        if (!mask)
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
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);
    movieclip->graphics().endFill();
    return as_value();
}

as_value
movieclip_lineTo(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);

    if (fn.nargs < 2) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("MovieClip.lineTo() needs at least two arguments"));
        );
        return as_value();
    }

    double x = toNumber(fn.arg(0), getVM(fn));
    double y = toNumber(fn.arg(1), getVM(fn));
        
    if (!isFinite(x)) x = 0;
    if (!isFinite(y)) y = 0;

#ifdef DEBUG_DRAWING_API
    log_debug("%s.lineTo(%g,%g);", movieclip->getTarget(), x, y);
#endif
    movieclip->graphics().lineTo(pixelsToTwips(x), pixelsToTwips(y),
            movieclip->getDefinitionVersion());
    return as_value();
}

as_value
movieclip_moveTo(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);

    if (fn.nargs < 2) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("MovieClip.moveTo() takes two args"));
        );
        return as_value();
    }

    double x = toNumber(fn.arg(0), getVM(fn));
    double y = toNumber(fn.arg(1), getVM(fn));
     
    if (!isFinite(x)) x = 0;
    if (!isFinite(y)) y = 0;

    movieclip->graphics().moveTo(pixelsToTwips(x), pixelsToTwips(y));
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
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);

    if (!fn.nargs) {
        movieclip->graphics().resetLineStyle();
        return as_value();
    }

    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;
    std::uint16_t thickness = 0;
    bool scaleThicknessVertically = true;
    bool scaleThicknessHorizontally = true;
    bool pixelHinting = false;
    bool noClose = false;
    CapStyle capStyle = CAP_ROUND;
    JoinStyle joinStyle = JOIN_ROUND;
    float miterLimitFactor = 1.0f;

    int arguments = fn.nargs;

    const int swfVersion = getSWFVersion(fn);
    if (swfVersion < 8 && fn.nargs > 3) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("MovieClip.lineStyle(%s): args after the "
                          "first three will be discarded"), ss.str());
            );
        arguments = 3;
    }

    switch (arguments) {
        default:
        case 8:
            miterLimitFactor = clamp<int>(toInt(fn.arg(7), getVM(fn)), 1, 255);
        case 7:
        {
            std::string joinStyleStr = fn.arg(6).to_string();
            if (joinStyleStr == "miter") joinStyle = JOIN_MITER;
            else if (joinStyleStr == "round") joinStyle = JOIN_ROUND;
            else if (joinStyleStr == "bevel") joinStyle = JOIN_BEVEL;
            else {
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
            else {
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
            if (noScaleString == "none") {
                scaleThicknessVertically = false;
                scaleThicknessHorizontally = false;
            }
            else if (noScaleString == "vertical") {
                scaleThicknessVertically = false;
            }
            else if (noScaleString == "horizontal") {
                scaleThicknessHorizontally = false;
            }
            else if (noScaleString != "normal") {
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
            pixelHinting = toBool(fn.arg(3), getVM(fn));
        case 3:
        {
            const float alphaval = clamp<float>(toNumber(fn.arg(2), getVM(fn)),
                                     0, 100);
            a = std::uint8_t(255 * (alphaval / 100));
        }
        case 2:
        {
            // See pollock.swf for eventual regressions.
            // It sets color to a random number from
            // 0 to 160000000 (about 10 times more then the max).
            std::uint32_t rgbval = toInt(fn.arg(1), getVM(fn));
            r = std::uint8_t((rgbval & 0xFF0000) >> 16);
            g = std::uint8_t((rgbval & 0x00FF00) >> 8);
            b = std::uint8_t((rgbval & 0x0000FF) );
        }
        case 1:
            thickness = std::uint16_t(pixelsToTwips(clamp<float>(
                            toNumber(fn.arg(0), getVM(fn)), 0, 255)));
            break;
    }

    rgba color(r, g, b, a);

    movieclip->graphics().lineStyle(thickness, color,
        scaleThicknessVertically, scaleThicknessHorizontally,
        pixelHinting, noClose, capStyle, capStyle, joinStyle, miterLimitFactor);

    return as_value();
}

as_value
movieclip_curveTo(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);

    if (fn.nargs < 4) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("MovieClip.curveTo() takes four args"));
        );
        return as_value();
    }

    double cx = toNumber(fn.arg(0), getVM(fn));
    double cy = toNumber(fn.arg(1), getVM(fn));
    double ax = toNumber(fn.arg(2), getVM(fn));
    double ay = toNumber(fn.arg(3), getVM(fn));

    if (!isFinite(cx)) cx = 0;
    if (!isFinite(cy)) cy = 0;
    if (!isFinite(ax)) ax = 0;
    if (!isFinite(ay)) ay = 0;

#ifdef DEBUG_DRAWING_API
    log_debug("%s.curveTo(%g,%g,%g,%g);", movieclip->getTarget(),
            cx, cy, ax, ay);
#endif
    movieclip->graphics().curveTo(pixelsToTwips(cx), pixelsToTwips(cy),
            pixelsToTwips(ax), pixelsToTwips(ay),
            movieclip->getDefinitionVersion());

    return as_value();
}

as_value
movieclip_clear(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);
    movieclip->graphics().clear();
    return as_value();
}

as_value
movieclip_beginFill(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);

    if (fn.nargs < 1) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("beginFill() with no args is a no-op"));
        );
        return as_value();
    }

    // 2^24 is the max here
    const std::uint32_t rgbval =
        clamp<float>(toNumber(fn.arg(0), getVM(fn)), 0, 16777216);

    const std::uint8_t r = (rgbval & 0xFF0000) >> 16;
    const std::uint8_t g = (rgbval & 0x00FF00) >> 8;
    const std::uint8_t b = rgbval & 0x0000FF;
    std::uint8_t a = 255;

    if (fn.nargs > 1) {
        a = 255 * clamp<int>(toInt(fn.arg(1), getVM(fn)), 0, 100) / 100;
    }

    rgba color(r, g, b, a);

    const FillStyle f = SolidFill(color);
    movieclip->graphics().beginFill(f);

    return as_value();
}


/// Create a dynamic gradient fill.
//
/// fillType, colors, alphas, ratios, matrix, [spreadMethod,
/// [interpolationMethod, [focalPointRatio]]]
//
/// Colors, alphas and ratios must be (possibly fake) arrays, and must have
/// the same size. This applies even when there are more gradients than
/// allowed.
as_value
movieclip_beginGradientFill(const fn_call& fn)
{
    MovieClip* movieclip = ensure<IsDisplayObject<MovieClip> >(fn);

    // The arguments up to and including matrix must be present.
    if (fn.nargs < 5) {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("%s.beginGradientFill(%s): invalid call: 5 arguments "
                "needed"), movieclip->getTarget(), ss.str());
        );
        return as_value();
    }

    const size_t maxargs = getSWFVersion(fn) >= 8 ? 8 : 5;

    if (fn.nargs > maxargs) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss; fn.dump_args(ss);
            log_aserror(_("%s.beginGradientFill(%s): extra arguments "
                    "invalidate call!"));
            );
        return as_value();
    }

    GradientFill::Type t;

    std::string typeStr = fn.arg(0).to_string();

    // An unexpected fill type results in no fill in all versions.
    if (typeStr == "radial") {
        t = GradientFill::RADIAL;
    }
    else if (typeStr == "linear") {
        t = GradientFill::LINEAR;
    }
    else {
        IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss; fn.dump_args(ss);
            log_aserror(_("%s.beginGradientFill(%s): first arg must be "
                "'radial', 'focal', or 'linear'"),
                movieclip->getTarget(), ss.str());
            );
        return as_value();
    }

    typedef as_object* ObjPtr;
    ObjPtr colors = toObject(fn.arg(1), getVM(fn));
    ObjPtr alphas = toObject(fn.arg(2), getVM(fn));
    ObjPtr ratios = toObject(fn.arg(3), getVM(fn));
    ObjPtr matrix = toObject(fn.arg(4), getVM(fn));

    if (!colors || !alphas || !ratios || !matrix) {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("%s.beginGradientFill(%s): one or more of the"
            " args from 2nd to 5th don't cast to objects"),
            movieclip->getTarget(), ss.str());
        );
        return as_value();
    }

    size_t stops = arrayLength(*colors);

    // Check that the arrays are all the same size.
    if (stops != arrayLength(*alphas) || stops != arrayLength(*ratios)) {

        IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss; fn.dump_args(ss);
            log_aserror(_("%s.beginGradientFill(%s): colors, alphas and "
                "ratios args don't have same length"),
                movieclip->getTarget(), ss.str());
        );
        return as_value();
    }
    
    // Then limit gradients. It's documented to be a maximum of 15, though
    // this isn't tested. The arrays must be the same size regardless of
    // this limit.
    if (stops > 15) {
        std::stringstream ss; fn.dump_args(ss);
        log_debug("%s.beginGradientFill(%s): too many array elements"
            " for colors and ratios (%d), will trim to 8", 
            movieclip->getTarget(), ss.str(), stops); 
        stops = 15;
    }

    SWFMatrix mat = toSWFMatrix(*matrix);

    // ----------------------------
    // Create the gradients vector
    // ----------------------------

    VM& vm = getVM(fn);

    std::vector<GradientRecord> gradients;
    gradients.reserve(stops);
    for (size_t i = 0; i < stops; ++i) {

        const ObjectURI& key = getURI(vm, boost::lexical_cast<std::string>(i));

        as_value colVal = getMember(*colors, key);
        std::uint32_t col = colVal.is_number() ? toInt(colVal, getVM(fn)) : 0;

        /// Alpha is the range 0..100.
        as_value alpVal = getMember(*alphas, key);
        const double a = alpVal.is_number() ?
            clamp<double>(toNumber(alpVal, getVM(fn)), 0, 100) : 0;
        const std::uint8_t alp = 0xff * (a / 100);

        // Ratio is the range 0..255, but a ratio may never be smaller than
        // the previous value. The pp adjusts it to be greater than the
        // last value if it is smaller. But we must be careful not to exceed
        // 0xff.

        // From looking it looks like the minimum adjustment is 2. Even 
        // steps of 1 appear to be adjusted.
        const int step = 2;
        const as_value& ratVal = getMember(*ratios, key);
        const std::uint32_t minRatio =
            gradients.empty() ? 0 :
            std::min<std::uint32_t>(gradients[i - 1].ratio + step, 0xff);

        std::uint8_t rat = ratVal.is_number() ?
            clamp<std::uint32_t>(toInt(ratVal, getVM(fn)), minRatio, 0xff)
            : minRatio;

        // The renderer may expect successively larger ratios; failure to
        // do this can lead to memory errors.
        if (!gradients.empty()) {
            assert((rat != 0xff && rat > gradients[i - 1].ratio) ||
                    (rat >= gradients[i - 1].ratio));
        }

        rgba color;
        color.parseRGB(col);
        color.m_a = alp;

        gradients.emplace_back(rat, color);
    }

    // Make sure we don't try to construct a GradientFill with only 1 stop!
    if (stops < 2) {
        const FillStyle f = SolidFill(gradients[0].color);
        movieclip->graphics().beginFill(f);
        return as_value();
    }

    GradientFill fd(t, mat.invert(), gradients);

    // Set spread mode if present. Defaults to "pad", which is GradientFill's
    // default.
    if (fn.nargs > 5) {
        const std::string& spread = fn.arg(5).to_string();
        if (spread == "reflect") fd.spreadMode = GradientFill::REFLECT;
        else if (spread == "repeat") fd.spreadMode = GradientFill::REPEAT;
        else assert(fd.spreadMode == GradientFill::PAD);
    }

    if (fn.nargs > 6) {
        const std::string& inter = fn.arg(6).to_string();
        if (inter == "rgb") fd.interpolation = GradientFill::RGB;
        else if (inter == "linearRGB") fd.interpolation = GradientFill::LINEAR_RGB;
        else assert(fd.interpolation == GradientFill::RGB);
    }

    /// Add a focus if present.
    if (fn.nargs > 7) {
        fd.setFocalPoint(toNumber(fn.arg(7), getVM(fn)));
    }

    movieclip->graphics().beginFill(fd);

    return as_value();
}

// startDrag([lockCenter:Boolean], [left:Number], [top:Number],
//    [right:Number], [bottom:Number]) : Void`
//
//    This can also be applied to other DisplayObjects such as TextFields.
as_value
movieclip_startDrag(const fn_call& fn)
{
    DisplayObject* o = ensure<IsDisplayObject<> >(fn);

    // mark this DisplayObject is transformed.
    o->transformedByScript();

    const bool lock = fn.nargs ? toBool(fn.arg(0), getVM(fn)) : false;

    DragState st(o, lock);

    if (fn.nargs > 4) {
        double x0 = toNumber(fn.arg(1), getVM(fn));
        double y0 = toNumber(fn.arg(2), getVM(fn));
        double x1 = toNumber(fn.arg(3), getVM(fn));
        double y1 = toNumber(fn.arg(4), getVM(fn));

        // check for infinite values
        bool gotinf = false;
        if (!isFinite(x0)) { x0=0; gotinf=true; }
        if (!isFinite(y0)) { y0=0; gotinf=true; }
        if (!isFinite(x1)) { x1=0; gotinf=true; }
        if (!isFinite(y1)) { y1=0; gotinf=true; }

        // check for swapped values
        bool swapped = false;
        if (y1 < y0) {
            std::swap(y1, y0);
            swapped = true;
        }

        if (x1 < x0) {
            std::swap(x1, x0);
            swapped = true;
        }

        IF_VERBOSE_ASCODING_ERRORS(
            if (gotinf || swapped) {
                std::stringstream ss; fn.dump_args(ss);
                if (swapped) { 
                    log_aserror(_("min/max bbox values in "
                        "MovieClip.startDrag(%s) swapped, fixing"),
                        ss.str());
                }
                if (gotinf) {
                    log_aserror(_("non-finite bbox values in "
                        "MovieClip.startDrag(%s), took as zero"),
                        ss.str());
                }
            }
        );

        SWFRect bounds(pixelsToTwips(x0), pixelsToTwips(y0),
                pixelsToTwips(x1), pixelsToTwips(y1));
        st.setBounds(bounds);
    }

    getRoot(fn).setDragState(st);

    return as_value();
}

// stopDrag() : Void
as_value
movieclip_stopDrag(const fn_call& fn)
{
    getRoot(fn).stop_drag();
    return as_value();
}


as_value
movieclip_beginBitmapFill(const fn_call& fn)
{
    MovieClip* ptr = ensure<IsDisplayObject<MovieClip> >(fn);
    if (fn.nargs < 1) {
        return as_value();
    }

    as_object* obj = toObject(fn.arg(0), getVM(fn));
    BitmapData_as* bd;

    if (!isNativeType(obj, bd) || bd->disposed()) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_debug("MovieClip.attachBitmap: first argument should be a "
			"valid BitmapData", fn.arg(1));
        );
        return as_value();
    }
    
    SWFMatrix mat;

    if (fn.nargs > 1) {
        as_object* matrix = toObject(fn.arg(1), getVM(fn));
        if (matrix) {
            mat = toSWFMatrix(*matrix);
        }
    }

    BitmapFill::Type t = BitmapFill::TILED;
    if (fn.nargs > 2) {
        const bool repeat = toBool(fn.arg(2), getVM(fn));
        if (!repeat) t = BitmapFill::CLIPPED;
    }

    BitmapFill::SmoothingPolicy p = BitmapFill::SMOOTHING_OFF;
    if (fn.nargs > 3 && toBool(fn.arg(3), getVM(fn))) {
        p = BitmapFill::SMOOTHING_ON;
    }

    // This is needed to get the bitmap to the right size and have it in the
    // correct place. Maybe it would be better handled somewhere else, as it's
    // not exactly intuitive.
    mat.invert();
    mat.concatenate_scale(1 / 20., 1 / 20.);
    mat.set_x_translation(mat.tx() / 20);
    mat.set_y_translation(mat.ty() / 20);

    ptr->graphics().beginFill(BitmapFill(t, bd->bitmapInfo(), mat, p));
    bd->attach(ptr);

    return as_value();
}


as_value
movieclip_getRect(const fn_call& fn)
{
    MovieClip* ptr = ensure<IsDisplayObject<MovieClip> >(fn);
    UNUSED(ptr);
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}


as_value
movieclip_lineGradientStyle(const fn_call& fn)
{
    MovieClip* ptr = ensure<IsDisplayObject<MovieClip> >(fn);
    UNUSED(ptr);
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}


as_value
movieclip_attachBitmap(const fn_call& fn)
{
    MovieClip* ptr = ensure<IsDisplayObject<MovieClip> >(fn);

    if (fn.nargs < 2) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_debug("MovieClip.attachBitmap: expected 2 args, got %d",
                fn.nargs);
        );
        return as_value();
    }

    as_object* obj = toObject(fn.arg(0), getVM(fn));
    BitmapData_as* bd;

    if (!isNativeType(obj, bd) || bd->disposed()) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_debug("MovieClip.attachBitmap: first argument should be a "
		      "valid BitmapData", fn.arg(1));
        );
        return as_value();
    }

    int depth = toInt(fn.arg(1), getVM(fn));

    DisplayObject* bm = new Bitmap(getRoot(fn), nullptr, bd, ptr);
    ptr->attachCharacter(*bm, depth, nullptr);

    return as_value();
}

as_value
movieclip_transform(const fn_call& fn)
{
    MovieClip* ptr = ensure<IsDisplayObject<MovieClip> >(fn);

    // If not found, construction fails.
    as_value transform(findObject(fn.env(), "flash.geom.Transform"));

    as_function* transCtor = transform.to_function();

    if (!transCtor) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Failed to construct flash.geom.Transform!"));
        );
        return as_value();
    }

    // Construct a flash.geom.Transform object with "this" as argument.
    fn_call::Args args;
    args += getObject(ptr);

    as_object* newTrans = constructInstance(*transCtor, fn.env(), args);

    return as_value(newTrans);
}

as_value
movieclip_beginMeshFill(const fn_call& /*fn*/)
{

    LOG_ONCE(log_unimpl(_("MovieClip.beginMeshFill")));
    return as_value();
}


as_value
movieclip_lockroot(const fn_call& fn)
{
    MovieClip* ptr = ensure<IsDisplayObject<MovieClip> >(fn);

    if (!fn.nargs) {
        return as_value(ptr->getLockRoot());
    }
    
    ptr->setLockRoot(toBool(fn.arg(0), getVM(fn)));
    return as_value();
}
    
} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

