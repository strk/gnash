// Sound_as3.cpp:  ActionScript "Sound" class, for Gnash.
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

#include "media/Sound_as3.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value sound_load(const fn_call& fn);
    as_value sound_play(const fn_call& fn);
    as_value sound_complete(const fn_call& fn);
    as_value sound_id3(const fn_call& fn);
    as_value sound_ioError(const fn_call& fn);
    as_value sound_open(const fn_call& fn);
    as_value sound_progress(const fn_call& fn);
    as_value sound_ctor(const fn_call& fn);
    void attachSoundInterface(as_object& o);
    void attachSoundStaticInterface(as_object& o);
    as_object* getSoundInterface();

}

// extern (used by Global.cpp)
void sound_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&sound_ctor, getSoundInterface());
        attachSoundStaticInterface(*cl);
    }

    // Register _global.Sound
    global.init_member("Sound", cl.get());
}

namespace {

void
attachSoundInterface(as_object& o)
{
    o.init_member("load", new builtin_function(sound_load));
    o.init_member("play", new builtin_function(sound_play));
    o.init_member("complete", new builtin_function(sound_complete));
    o.init_member("id3", new builtin_function(sound_id3));
    o.init_member("ioError", new builtin_function(sound_ioError));
    o.init_member("open", new builtin_function(sound_open));
    o.init_member("progress", new builtin_function(sound_progress));
}

void
attachSoundStaticInterface(as_object& o)
{

}

as_object*
getSoundInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachSoundInterface(*o);
    }
    return o.get();
}

as_value
sound_load(const fn_call& fn)
{
    boost::intrusive_ptr<Sound_as3> ptr =
        ensureType<Sound_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
sound_play(const fn_call& fn)
{
    boost::intrusive_ptr<Sound_as3> ptr =
        ensureType<Sound_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
sound_complete(const fn_call& fn)
{
    boost::intrusive_ptr<Sound_as3> ptr =
        ensureType<Sound_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
sound_id3(const fn_call& fn)
{
    boost::intrusive_ptr<Sound_as3> ptr =
        ensureType<Sound_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
sound_ioError(const fn_call& fn)
{
    boost::intrusive_ptr<Sound_as3> ptr =
        ensureType<Sound_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
sound_open(const fn_call& fn)
{
    boost::intrusive_ptr<Sound_as3> ptr =
        ensureType<Sound_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
sound_progress(const fn_call& fn)
{
    boost::intrusive_ptr<Sound_as3> ptr =
        ensureType<Sound_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
sound_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new Sound_as3;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

