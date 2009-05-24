// ByteArray_as3.cpp:  ActionScript "ByteArray" class, for Gnash.
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

#include "utils/ByteArray_as3.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value bytearray_readBoolean(const fn_call& fn);
    as_value bytearray_readByte(const fn_call& fn);
    as_value bytearray_readBytes(const fn_call& fn);
    as_value bytearray_readDouble(const fn_call& fn);
    as_value bytearray_readFloat(const fn_call& fn);
    as_value bytearray_readInt(const fn_call& fn);
    as_value bytearray_readMultiByte(const fn_call& fn);
    as_value bytearray_readObject(const fn_call& fn);
    as_value bytearray_readShort(const fn_call& fn);
    as_value bytearray_readUnsignedByte(const fn_call& fn);
    as_value bytearray_readUnsignedInt(const fn_call& fn);
    as_value bytearray_readUnsignedShort(const fn_call& fn);
    as_value bytearray_readUTF(const fn_call& fn);
    as_value bytearray_readUTFBytes(const fn_call& fn);
    as_value bytearray_toString(const fn_call& fn);
    as_value bytearray_uncompress(const fn_call& fn);
    as_value bytearray_writeBoolean(const fn_call& fn);
    as_value bytearray_writeByte(const fn_call& fn);
    as_value bytearray_writeBytes(const fn_call& fn);
    as_value bytearray_writeDouble(const fn_call& fn);
    as_value bytearray_writeFloat(const fn_call& fn);
    as_value bytearray_writeInt(const fn_call& fn);
    as_value bytearray_writeMultiByte(const fn_call& fn);
    as_value bytearray_writeObject(const fn_call& fn);
    as_value bytearray_writeShort(const fn_call& fn);
    as_value bytearray_writeUnsignedInt(const fn_call& fn);
    as_value bytearray_writeUTF(const fn_call& fn);
    as_value bytearray_writeUTFBytes(const fn_call& fn);
    as_value bytearray_ctor(const fn_call& fn);
    void attachByteArrayInterface(as_object& o);
    void attachByteArrayStaticInterface(as_object& o);
    as_object* getByteArrayInterface();

}

// extern (used by Global.cpp)
void bytearray_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&bytearray_ctor, getByteArrayInterface());
        attachByteArrayStaticInterface(*cl);
    }

    // Register _global.ByteArray
    global.init_member("ByteArray", cl.get());
}

namespace {

void
attachByteArrayInterface(as_object& o)
{
    o.init_member("readBoolean", new builtin_function(bytearray_readBoolean));
    o.init_member("readByte", new builtin_function(bytearray_readByte));
    o.init_member("readBytes", new builtin_function(bytearray_readBytes));
    o.init_member("readDouble", new builtin_function(bytearray_readDouble));
    o.init_member("readFloat", new builtin_function(bytearray_readFloat));
    o.init_member("readInt", new builtin_function(bytearray_readInt));
    o.init_member("readMultiByte", new builtin_function(bytearray_readMultiByte));
    o.init_member("readObject", new builtin_function(bytearray_readObject));
    o.init_member("readShort", new builtin_function(bytearray_readShort));
    o.init_member("readUnsignedByte", new builtin_function(bytearray_readUnsignedByte));
    o.init_member("readUnsignedInt", new builtin_function(bytearray_readUnsignedInt));
    o.init_member("readUnsignedShort", new builtin_function(bytearray_readUnsignedShort));
    o.init_member("readUTF", new builtin_function(bytearray_readUTF));
    o.init_member("readUTFBytes", new builtin_function(bytearray_readUTFBytes));
    o.init_member("toString", new builtin_function(bytearray_toString));
    o.init_member("uncompress", new builtin_function(bytearray_uncompress));
    o.init_member("writeBoolean", new builtin_function(bytearray_writeBoolean));
    o.init_member("writeByte", new builtin_function(bytearray_writeByte));
    o.init_member("writeBytes", new builtin_function(bytearray_writeBytes));
    o.init_member("writeDouble", new builtin_function(bytearray_writeDouble));
    o.init_member("writeFloat", new builtin_function(bytearray_writeFloat));
    o.init_member("writeInt", new builtin_function(bytearray_writeInt));
    o.init_member("writeMultiByte", new builtin_function(bytearray_writeMultiByte));
    o.init_member("writeObject", new builtin_function(bytearray_writeObject));
    o.init_member("writeShort", new builtin_function(bytearray_writeShort));
    o.init_member("writeUnsignedInt", new builtin_function(bytearray_writeUnsignedInt));
    o.init_member("writeUTF", new builtin_function(bytearray_writeUTF));
    o.init_member("writeUTFBytes", new builtin_function(bytearray_writeUTFBytes));
}

void
attachByteArrayStaticInterface(as_object& o)
{

}

as_object*
getByteArrayInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachByteArrayInterface(*o);
    }
    return o.get();
}

as_value
bytearray_readBoolean(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_readByte(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_readBytes(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_readDouble(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_readFloat(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_readInt(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_readMultiByte(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_readObject(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_readShort(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_readUnsignedByte(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_readUnsignedInt(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_readUnsignedShort(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_readUTF(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_readUTFBytes(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_toString(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_uncompress(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_writeBoolean(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_writeByte(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_writeBytes(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_writeDouble(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_writeFloat(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_writeInt(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_writeMultiByte(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_writeObject(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_writeShort(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_writeUnsignedInt(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_writeUTF(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_writeUTFBytes(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as3> ptr =
        ensureType<ByteArray_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new ByteArray_as3;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

