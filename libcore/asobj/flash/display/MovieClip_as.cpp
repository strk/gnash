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

#include "display/MovieClip_as.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value movieclip_gotoAndStop(const fn_call& fn);
    as_value movieclip_nextFrame(const fn_call& fn);
    as_value movieclip_nextScene(const fn_call& fn);
    as_value movieclip_play(const fn_call& fn);
    as_value movieclip_prevFrame(const fn_call& fn);
    as_value movieclip_prevScene(const fn_call& fn);
    as_value movieclip_stop(const fn_call& fn);
    as_value movieclip_ctor(const fn_call& fn);
    void attachMovieClipInterface(as_object& o);
    void attachMovieClipStaticInterface(as_object& o);
    as_object* getMovieClipInterface();

}

// extern (used by Global.cpp)
void movieclip_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&movieclip_ctor, getMovieClipInterface());
        attachMovieClipStaticInterface(*cl);
    }

    // Register _global.MovieClip
    global.init_member("MovieClip", cl.get());
}

namespace {

void
attachMovieClipInterface(as_object& o)
{
    o.init_member("gotoAndStop", new builtin_function(movieclip_gotoAndStop));
    o.init_member("nextFrame", new builtin_function(movieclip_nextFrame));
    o.init_member("nextScene", new builtin_function(movieclip_nextScene));
    o.init_member("play", new builtin_function(movieclip_play));
    o.init_member("prevFrame", new builtin_function(movieclip_prevFrame));
    o.init_member("prevScene", new builtin_function(movieclip_prevScene));
    o.init_member("stop", new builtin_function(movieclip_stop));
}

void
attachMovieClipStaticInterface(as_object& o)
{

}

as_object*
getMovieClipInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachMovieClipInterface(*o);
    }
    return o.get();
}

as_value
movieclip_gotoAndStop(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip_as> ptr =
        ensureType<MovieClip_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
movieclip_nextFrame(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip_as> ptr =
        ensureType<MovieClip_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
movieclip_nextScene(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip_as> ptr =
        ensureType<MovieClip_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
movieclip_play(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip_as> ptr =
        ensureType<MovieClip_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
movieclip_prevFrame(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip_as> ptr =
        ensureType<MovieClip_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
movieclip_prevScene(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip_as> ptr =
        ensureType<MovieClip_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
movieclip_stop(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip_as> ptr =
        ensureType<MovieClip_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
movieclip_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new MovieClip_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

