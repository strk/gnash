// ByteArray_as.cpp:  ActionScript "ByteArray" class, for Gnash.
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

#include "utils/ByteArray_as.h"
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

class ByteArray_as : public as_object
{

public:

    ByteArray_as()
        :
        as_object(getByteArrayInterface())
    {}
};

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
    o.init_member("readBoolean", gl->createFunction(bytearray_readBoolean));
    o.init_member("readByte", gl->createFunction(bytearray_readByte));
    o.init_member("readBytes", gl->createFunction(bytearray_readBytes));
    o.init_member("readDouble", gl->createFunction(bytearray_readDouble));
    o.init_member("readFloat", gl->createFunction(bytearray_readFloat));
    o.init_member("readInt", gl->createFunction(bytearray_readInt));
    o.init_member("readMultiByte", gl->createFunction(bytearray_readMultiByte));
    o.init_member("readObject", gl->createFunction(bytearray_readObject));
    o.init_member("readShort", gl->createFunction(bytearray_readShort));
    o.init_member("readUnsignedByte", gl->createFunction(bytearray_readUnsignedByte));
    o.init_member("readUnsignedInt", gl->createFunction(bytearray_readUnsignedInt));
    o.init_member("readUnsignedShort", gl->createFunction(bytearray_readUnsignedShort));
    o.init_member("readUTF", gl->createFunction(bytearray_readUTF));
    o.init_member("readUTFBytes", gl->createFunction(bytearray_readUTFBytes));
    o.init_member("toString", gl->createFunction(bytearray_toString));
    o.init_member("uncompress", gl->createFunction(bytearray_uncompress));
    o.init_member("writeBoolean", gl->createFunction(bytearray_writeBoolean));
    o.init_member("writeByte", gl->createFunction(bytearray_writeByte));
    o.init_member("writeBytes", gl->createFunction(bytearray_writeBytes));
    o.init_member("writeDouble", gl->createFunction(bytearray_writeDouble));
    o.init_member("writeFloat", gl->createFunction(bytearray_writeFloat));
    o.init_member("writeInt", gl->createFunction(bytearray_writeInt));
    o.init_member("writeMultiByte", gl->createFunction(bytearray_writeMultiByte));
    o.init_member("writeObject", gl->createFunction(bytearray_writeObject));
    o.init_member("writeShort", gl->createFunction(bytearray_writeShort));
    o.init_member("writeUnsignedInt", gl->createFunction(bytearray_writeUnsignedInt));
    o.init_member("writeUTF", gl->createFunction(bytearray_writeUTF));
    o.init_member("writeUTFBytes", gl->createFunction(bytearray_writeUTFBytes));
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
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_readByte(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_readBytes(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_readDouble(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_readFloat(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_readInt(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_readMultiByte(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_readObject(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_readShort(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_readUnsignedByte(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_readUnsignedInt(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_readUnsignedShort(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_readUTF(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_readUTFBytes(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_toString(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_uncompress(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_writeBoolean(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_writeByte(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_writeBytes(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_writeDouble(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_writeFloat(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_writeInt(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_writeMultiByte(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_writeObject(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_writeShort(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_writeUnsignedInt(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_writeUTF(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_writeUTFBytes(const fn_call& fn)
{
    boost::intrusive_ptr<ByteArray_as> ptr =
        ensureType<ByteArray_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
bytearray_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new ByteArray_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

