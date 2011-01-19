// MovieClipLoader.cpp:  Implementation of ActionScript MovieClipLoader class.
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

#include "MovieClipLoader.h"

#include <string>

#include "smart_ptr.h" 
#include "as_value.h"
#include "as_object.h" // for inheritance
#include "movie_root.h"
#include "fn_call.h"
#include "Global_as.h"
#include "as_function.h"
#include "MovieClip.h"
#include "DisplayObject.h" // for loadClip (get_parent)
#include "log.h"
#include "VM.h" // for the string table.
#include "builtin_function.h"
#include "AsBroadcaster.h" // for initializing self as a broadcaster
#include "namedStrings.h"
#include "ExecutableCode.h"
#include "NativeFunction.h"

//#define GNASH_DEBUG 1

namespace gnash {

// Forward declarations
namespace {
    as_value moviecliploader_loadClip(const fn_call& fn);
    as_value moviecliploader_unloadClip(const fn_call& fn);
    as_value moviecliploader_getProgress(const fn_call& fn);
    as_value moviecliploader_new(const fn_call& fn);
    void attachMovieClipLoaderInterface(as_object& o);
}

void
registerMovieClipLoaderNative(as_object& global)
{
    VM& vm = getVM(global);
    vm.registerNative(moviecliploader_loadClip, 112, 100);
    vm.registerNative(moviecliploader_getProgress, 112, 101);
    vm.registerNative(moviecliploader_unloadClip, 112, 102);
}

/// Extern.
void
moviecliploader_class_init(as_object& where, const ObjectURI& uri)
{
	// This is going to be the where Number "class"/"function"
    Global_as& gl = getGlobal(where);

    as_object* proto = createObject(gl);;

    as_object* cl = gl.createClass(&moviecliploader_new, proto);
    attachMovieClipLoaderInterface(*proto);
  
	AsBroadcaster::initialize(*proto);

    as_object* null = 0;
    callMethod(&gl, NSV::PROP_AS_SET_PROP_FLAGS, proto, null, 1027);

	where.init_member(uri, cl, as_object::DefaultFlags); 
}


namespace {

void
attachMovieClipLoaderInterface(as_object& o)
{
    const int flags = PropFlags::onlySWF7Up;

    VM& vm = getVM(o);

  	o.init_member("loadClip", vm.getNative(112, 100), flags);
	o.init_member("getProgress", vm.getNative(112, 101), flags);
	o.init_member("unloadClip", vm.getNative(112, 102), flags);
}

as_value
moviecliploader_loadClip(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);
  
	if (fn.nargs < 2) {
		IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss; fn.dump_args(ss);
            log_aserror(_("MovieClipLoader.loadClip(%s): missing arguments"),
                ss.str());
        );
		return as_value(false);
	}

    if (!fn.arg(0).is_string()) {
		IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss; fn.dump_args(ss);
            log_aserror(_("MovieClipLoader.loadClip(%s): first argument must"
                    "be a string"), ss.str());
        );
        return as_value(false);
    }

	const std::string& str_url = fn.arg(0).to_string(); 

	as_value tgt_arg = fn.arg(1);
	const std::string& tgt_str = tgt_arg.to_string();

    movie_root& mr = getRoot(*ptr);

    // TODO: check if this logic can be generic to movie_root::loadMovie
	DisplayObject* target = findTarget(fn.env(), tgt_str);
    unsigned int junk;
	if (!target && ! isLevelTarget(getSWFVersion(fn), tgt_str, junk) ) {
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Could not find target %s (evaluated from %s)"),
			tgt_str, tgt_arg);
		);
		return as_value(false);
	}

    // TODO: return code is based on target validity if I'm not wrong
    mr.loadMovie(str_url, tgt_str, "", MovieClip::METHOD_NONE, ptr);
    return as_value(true);
}

as_value
moviecliploader_unloadClip(const fn_call& fn)
{
    if (!fn.nargs) {
		IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss; fn.dump_args(ss);
            log_aserror(_("MovieClipLoader.unloadClip(%s): expected at least"
                    "one argument"), ss.str());
        );
        return as_value();
    }

    const std::string filespec = fn.arg(0).to_string();
    log_unimpl(_("MovieClipLoader.unloadClip: %s"), __PRETTY_FUNCTION__, filespec);
    return as_value();
}

as_value
moviecliploader_new(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);
    Global_as& gl = getGlobal(fn);

    as_object* array = gl.createArray();
    callMethod(array, NSV::PROP_PUSH, ptr);
    ptr->set_member(NSV::PROP_uLISTENERS, array);
    ptr->set_member_flags(NSV::PROP_uLISTENERS, as_object::DefaultFlags);
    return as_value();
}

// Invoked every time the loading content is written to disk during
// the loading process.
as_value
moviecliploader_getProgress(const fn_call& fn)
{
	if (!fn.nargs) {
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("MovieClipLoader.getProgress(): missing argument"));
		);
		return as_value();
	}

	as_object* target = toObject(fn.arg(0), getVM(fn));
  
	if (!target) {
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("MovieClipLoader.getProgress(%s): first argument is "
                "not an object"), fn.arg(0));
		);
		return as_value();
	}

	MovieClip* sp = get<MovieClip>(target);
	if (!sp) {
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("MovieClipLoader.getProgress(%s): first argument is "
                "not an sprite"), fn.arg(0));
		);
		return as_value();
	}

	as_object* mcl_obj = new as_object(getGlobal(fn));

	size_t bytesLoaded = sp->get_bytes_loaded();
	size_t bytesTotal = sp->get_bytes_total();

	VM& vm = getVM(fn);

	// We want these to be enumerable
	mcl_obj->set_member(getURI(vm, "bytesLoaded"), bytesLoaded);
	mcl_obj->set_member(getURI(vm, "bytesTotal"),  bytesTotal);
  
	return as_value(mcl_obj); 
}

} // anonymous namespace
} // end of gnash namespace
