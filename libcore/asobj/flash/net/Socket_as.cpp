// Socket_as.cpp:  ActionScript "Socket" class, for Gnash.
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

#include "net/Socket_as.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value socket_connect(const fn_call& fn);
    as_value socket_flush(const fn_call& fn);
    as_value socket_readBoolean(const fn_call& fn);
    as_value socket_readByte(const fn_call& fn);
    as_value socket_readBytes(const fn_call& fn);
    as_value socket_readDouble(const fn_call& fn);
    as_value socket_readFloat(const fn_call& fn);
    as_value socket_readInt(const fn_call& fn);
    as_value socket_readMultiByte(const fn_call& fn);
    as_value socket_readObject(const fn_call& fn);
    as_value socket_readShort(const fn_call& fn);
    as_value socket_readUnsignedByte(const fn_call& fn);
    as_value socket_readUnsignedInt(const fn_call& fn);
    as_value socket_readUnsignedShort(const fn_call& fn);
    as_value socket_readUTF(const fn_call& fn);
    as_value socket_readUTFBytes(const fn_call& fn);
    as_value socket_writeBoolean(const fn_call& fn);
    as_value socket_writeByte(const fn_call& fn);
    as_value socket_writeBytes(const fn_call& fn);
    as_value socket_writeDouble(const fn_call& fn);
    as_value socket_writeFloat(const fn_call& fn);
    as_value socket_writeInt(const fn_call& fn);
    as_value socket_writeMultiByte(const fn_call& fn);
    as_value socket_writeObject(const fn_call& fn);
    as_value socket_writeShort(const fn_call& fn);
    as_value socket_writeUnsignedInt(const fn_call& fn);
    as_value socket_writeUTF(const fn_call& fn);
    as_value socket_writeUTFBytes(const fn_call& fn);
    as_value socket_close(const fn_call& fn);
    as_value socket_ioError(const fn_call& fn);
    as_value socket_securityError(const fn_call& fn);
    as_value socket_socketData(const fn_call& fn);
    as_value socket_ctor(const fn_call& fn);
    void attachSocketInterface(as_object& o);
    void attachSocketStaticInterface(as_object& o);
    as_object* getSocketInterface();

}

class Socket_as : public as_object
{

public:

    Socket_as()
        :
        as_object(getSocketInterface())
    {}
};

// extern (used by Global.cpp)
void socket_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&socket_ctor, getSocketInterface());
        attachSocketStaticInterface(*cl);
    }

    // Register _global.Socket
    global.init_member("Socket", cl.get());
}

namespace {

void
attachSocketInterface(as_object& o)
{
    o.init_member("connect", new builtin_function(socket_connect));
    o.init_member("flush", new builtin_function(socket_flush));
    o.init_member("readBoolean", new builtin_function(socket_readBoolean));
    o.init_member("readByte", new builtin_function(socket_readByte));
    o.init_member("readBytes", new builtin_function(socket_readBytes));
    o.init_member("readDouble", new builtin_function(socket_readDouble));
    o.init_member("readFloat", new builtin_function(socket_readFloat));
    o.init_member("readInt", new builtin_function(socket_readInt));
    o.init_member("readMultiByte", new builtin_function(socket_readMultiByte));
    o.init_member("readObject", new builtin_function(socket_readObject));
    o.init_member("readShort", new builtin_function(socket_readShort));
    o.init_member("readUnsignedByte", new builtin_function(socket_readUnsignedByte));
    o.init_member("readUnsignedInt", new builtin_function(socket_readUnsignedInt));
    o.init_member("readUnsignedShort", new builtin_function(socket_readUnsignedShort));
    o.init_member("readUTF", new builtin_function(socket_readUTF));
    o.init_member("readUTFBytes", new builtin_function(socket_readUTFBytes));
    o.init_member("writeBoolean", new builtin_function(socket_writeBoolean));
    o.init_member("writeByte", new builtin_function(socket_writeByte));
    o.init_member("writeBytes", new builtin_function(socket_writeBytes));
    o.init_member("writeDouble", new builtin_function(socket_writeDouble));
    o.init_member("writeFloat", new builtin_function(socket_writeFloat));
    o.init_member("writeInt", new builtin_function(socket_writeInt));
    o.init_member("writeMultiByte", new builtin_function(socket_writeMultiByte));
    o.init_member("writeObject", new builtin_function(socket_writeObject));
    o.init_member("writeShort", new builtin_function(socket_writeShort));
    o.init_member("writeUnsignedInt", new builtin_function(socket_writeUnsignedInt));
    o.init_member("writeUTF", new builtin_function(socket_writeUTF));
    o.init_member("writeUTFBytes", new builtin_function(socket_writeUTFBytes));
    o.init_member("close", new builtin_function(socket_close));
    o.init_member("connect", new builtin_function(socket_connect));
    o.init_member("ioError", new builtin_function(socket_ioError));
    o.init_member("securityError", new builtin_function(socket_securityError));
    o.init_member("socketData", new builtin_function(socket_socketData));
}

void
attachSocketStaticInterface(as_object& o)
{

}

as_object*
getSocketInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachSocketInterface(*o);
    }
    return o.get();
}

as_value
socket_connect(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_flush(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_readBoolean(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_readByte(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_readBytes(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_readDouble(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_readFloat(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_readInt(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_readMultiByte(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_readObject(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_readShort(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_readUnsignedByte(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_readUnsignedInt(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_readUnsignedShort(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_readUTF(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_readUTFBytes(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_writeBoolean(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_writeByte(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_writeBytes(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_writeDouble(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_writeFloat(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_writeInt(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_writeMultiByte(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_writeObject(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_writeShort(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_writeUnsignedInt(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_writeUTF(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_writeUTFBytes(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_close(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_ioError(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_securityError(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_socketData(const fn_call& fn)
{
    boost::intrusive_ptr<Socket_as> ptr =
        ensureType<Socket_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
socket_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new Socket_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

