// NetStream_as.cpp:  ActionScript "NetStream" class, for Gnash.
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

#include "net/NetStream_as.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value netstream_attachCamera(const fn_call& fn);
    as_value netstream_close(const fn_call& fn);
    as_value netstream_pause(const fn_call& fn);
    as_value netstream_play(const fn_call& fn);
    as_value netstream_publish(const fn_call& fn);
    as_value netstream_receiveAudio(const fn_call& fn);
    as_value netstream_receiveVideo(const fn_call& fn);
    as_value netstream_receiveVideoFPS(const fn_call& fn);
    as_value netstream_resume(const fn_call& fn);
    as_value netstream_seek(const fn_call& fn);
    as_value netstream_send(const fn_call& fn);
    as_value netstream_togglePause(const fn_call& fn);
    as_value netstream_asyncError(const fn_call& fn);
    as_value netstream_ioError(const fn_call& fn);
    as_value netstream_netStatus(const fn_call& fn);
    as_value netstream_onCuePoint(const fn_call& fn);
    as_value netstream_onImageData(const fn_call& fn);
    as_value netstream_onMetaData(const fn_call& fn);
    as_value netstream_onPlayStatus(const fn_call& fn);
    as_value netstream_onTextData(const fn_call& fn);
    as_value netstream_ctor(const fn_call& fn);
    void attachNetStreamInterface(as_object& o);
    void attachNetStreamStaticInterface(as_object& o);
    as_object* getNetStreamInterface();

}

class NetStream_as : public as_object
{

public:

    NetStream_as()
        :
        as_object(getNetStreamInterface())
    {}
};

// extern (used by Global.cpp)
void netstream_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&netstream_ctor, getNetStreamInterface());
        attachNetStreamStaticInterface(*cl);
    }

    // Register _global.NetStream
    global.init_member("NetStream", cl.get());
}

namespace {

void
attachNetStreamInterface(as_object& o)
{
    o.init_member("attachCamera", new builtin_function(netstream_attachCamera));
    o.init_member("close", new builtin_function(netstream_close));
    o.init_member("pause", new builtin_function(netstream_pause));
    o.init_member("play", new builtin_function(netstream_play));
    o.init_member("publish", new builtin_function(netstream_publish));
    o.init_member("receiveAudio", new builtin_function(netstream_receiveAudio));
    o.init_member("receiveVideo", new builtin_function(netstream_receiveVideo));
    o.init_member("receiveVideoFPS", new builtin_function(netstream_receiveVideoFPS));
    o.init_member("resume", new builtin_function(netstream_resume));
    o.init_member("seek", new builtin_function(netstream_seek));
    o.init_member("send", new builtin_function(netstream_send));
    o.init_member("togglePause", new builtin_function(netstream_togglePause));
    o.init_member("asyncError", new builtin_function(netstream_asyncError));
    o.init_member("ioError", new builtin_function(netstream_ioError));
    o.init_member("netStatus", new builtin_function(netstream_netStatus));
    o.init_member("onCuePoint", new builtin_function(netstream_onCuePoint));
    o.init_member("onImageData", new builtin_function(netstream_onImageData));
    o.init_member("onMetaData", new builtin_function(netstream_onMetaData));
    o.init_member("onPlayStatus", new builtin_function(netstream_onPlayStatus));
    o.init_member("onTextData", new builtin_function(netstream_onTextData));
}

void
attachNetStreamStaticInterface(as_object& o)
{

}

as_object*
getNetStreamInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachNetStreamInterface(*o);
    }
    return o.get();
}

as_value
netstream_attachCamera(const fn_call& fn)
{
    boost::intrusive_ptr<NetStream_as> ptr =
        ensureType<NetStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netstream_close(const fn_call& fn)
{
    boost::intrusive_ptr<NetStream_as> ptr =
        ensureType<NetStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netstream_pause(const fn_call& fn)
{
    boost::intrusive_ptr<NetStream_as> ptr =
        ensureType<NetStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netstream_play(const fn_call& fn)
{
    boost::intrusive_ptr<NetStream_as> ptr =
        ensureType<NetStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netstream_publish(const fn_call& fn)
{
    boost::intrusive_ptr<NetStream_as> ptr =
        ensureType<NetStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netstream_receiveAudio(const fn_call& fn)
{
    boost::intrusive_ptr<NetStream_as> ptr =
        ensureType<NetStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netstream_receiveVideo(const fn_call& fn)
{
    boost::intrusive_ptr<NetStream_as> ptr =
        ensureType<NetStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netstream_receiveVideoFPS(const fn_call& fn)
{
    boost::intrusive_ptr<NetStream_as> ptr =
        ensureType<NetStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netstream_resume(const fn_call& fn)
{
    boost::intrusive_ptr<NetStream_as> ptr =
        ensureType<NetStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netstream_seek(const fn_call& fn)
{
    boost::intrusive_ptr<NetStream_as> ptr =
        ensureType<NetStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netstream_send(const fn_call& fn)
{
    boost::intrusive_ptr<NetStream_as> ptr =
        ensureType<NetStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netstream_togglePause(const fn_call& fn)
{
    boost::intrusive_ptr<NetStream_as> ptr =
        ensureType<NetStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netstream_asyncError(const fn_call& fn)
{
    boost::intrusive_ptr<NetStream_as> ptr =
        ensureType<NetStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netstream_ioError(const fn_call& fn)
{
    boost::intrusive_ptr<NetStream_as> ptr =
        ensureType<NetStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netstream_netStatus(const fn_call& fn)
{
    boost::intrusive_ptr<NetStream_as> ptr =
        ensureType<NetStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netstream_onCuePoint(const fn_call& fn)
{
    boost::intrusive_ptr<NetStream_as> ptr =
        ensureType<NetStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netstream_onImageData(const fn_call& fn)
{
    boost::intrusive_ptr<NetStream_as> ptr =
        ensureType<NetStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netstream_onMetaData(const fn_call& fn)
{
    boost::intrusive_ptr<NetStream_as> ptr =
        ensureType<NetStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netstream_onPlayStatus(const fn_call& fn)
{
    boost::intrusive_ptr<NetStream_as> ptr =
        ensureType<NetStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netstream_onTextData(const fn_call& fn)
{
    boost::intrusive_ptr<NetStream_as> ptr =
        ensureType<NetStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
netstream_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new NetStream_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

