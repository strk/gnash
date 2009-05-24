// Microphone_as3.cpp:  ActionScript "Microphone" class, for Gnash.
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

#include "media/Microphone_as3.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value microphone_setLoopBack(const fn_call& fn);
    as_value microphone_setSilenceLevel(const fn_call& fn);
    as_value microphone_setUseEchoSuppression(const fn_call& fn);
    as_value microphone_activity(const fn_call& fn);
    as_value microphone_status(const fn_call& fn);
    as_value microphone_ctor(const fn_call& fn);
    void attachMicrophoneInterface(as_object& o);
    void attachMicrophoneStaticInterface(as_object& o);
    as_object* getMicrophoneInterface();

}

// extern (used by Global.cpp)
void microphone_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&microphone_ctor, getMicrophoneInterface());
        attachMicrophoneStaticInterface(*cl);
    }

    // Register _global.Microphone
    global.init_member("Microphone", cl.get());
}

namespace {

void
attachMicrophoneInterface(as_object& o)
{
    o.init_member("setLoopBack", new builtin_function(microphone_setLoopBack));
    o.init_member("setSilenceLevel", new builtin_function(microphone_setSilenceLevel));
    o.init_member("setUseEchoSuppression", new builtin_function(microphone_setUseEchoSuppression));
    o.init_member("activity", new builtin_function(microphone_activity));
    o.init_member("status", new builtin_function(microphone_status));
}

void
attachMicrophoneStaticInterface(as_object& o)
{

}

as_object*
getMicrophoneInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachMicrophoneInterface(*o);
    }
    return o.get();
}

as_value
microphone_setLoopBack(const fn_call& fn)
{
    boost::intrusive_ptr<Microphone_as3> ptr =
        ensureType<Microphone_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
microphone_setSilenceLevel(const fn_call& fn)
{
    boost::intrusive_ptr<Microphone_as3> ptr =
        ensureType<Microphone_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
microphone_setUseEchoSuppression(const fn_call& fn)
{
    boost::intrusive_ptr<Microphone_as3> ptr =
        ensureType<Microphone_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
microphone_activity(const fn_call& fn)
{
    boost::intrusive_ptr<Microphone_as3> ptr =
        ensureType<Microphone_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
microphone_status(const fn_call& fn)
{
    boost::intrusive_ptr<Microphone_as3> ptr =
        ensureType<Microphone_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
microphone_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new Microphone_as3;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

