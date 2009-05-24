// IDataOutput_as3.cpp:  ActionScript "IDataOutput" class, for Gnash.
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

#include "utils/IDataOutput_as3.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value idataoutput_writeByte(const fn_call& fn);
    as_value idataoutput_writeBytes(const fn_call& fn);
    as_value idataoutput_writeDouble(const fn_call& fn);
    as_value idataoutput_writeFloat(const fn_call& fn);
    as_value idataoutput_writeInt(const fn_call& fn);
    as_value idataoutput_writeMultiByte(const fn_call& fn);
    as_value idataoutput_writeObject(const fn_call& fn);
    as_value idataoutput_writeShort(const fn_call& fn);
    as_value idataoutput_writeUnsignedInt(const fn_call& fn);
    as_value idataoutput_writeUTF(const fn_call& fn);
    as_value idataoutput_writeUTFBytes(const fn_call& fn);
    as_value idataoutput_ctor(const fn_call& fn);
    void attachIDataOutputInterface(as_object& o);
    void attachIDataOutputStaticInterface(as_object& o);
    as_object* getIDataOutputInterface();

}

// extern (used by Global.cpp)
void idataoutput_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&idataoutput_ctor, getIDataOutputInterface());
        attachIDataOutputStaticInterface(*cl);
    }

    // Register _global.IDataOutput
    global.init_member("IDataOutput", cl.get());
}

namespace {

void
attachIDataOutputInterface(as_object& o)
{
    o.init_member("writeByte", new builtin_function(idataoutput_writeByte));
    o.init_member("writeBytes", new builtin_function(idataoutput_writeBytes));
    o.init_member("writeDouble", new builtin_function(idataoutput_writeDouble));
    o.init_member("writeFloat", new builtin_function(idataoutput_writeFloat));
    o.init_member("writeInt", new builtin_function(idataoutput_writeInt));
    o.init_member("writeMultiByte", new builtin_function(idataoutput_writeMultiByte));
    o.init_member("writeObject", new builtin_function(idataoutput_writeObject));
    o.init_member("writeShort", new builtin_function(idataoutput_writeShort));
    o.init_member("writeUnsignedInt", new builtin_function(idataoutput_writeUnsignedInt));
    o.init_member("writeUTF", new builtin_function(idataoutput_writeUTF));
    o.init_member("writeUTFBytes", new builtin_function(idataoutput_writeUTFBytes));
}

void
attachIDataOutputStaticInterface(as_object& o)
{

}

as_object*
getIDataOutputInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachIDataOutputInterface(*o);
    }
    return o.get();
}

as_value
idataoutput_writeByte(const fn_call& fn)
{
    boost::intrusive_ptr<IDataOutput_as3> ptr =
        ensureType<IDataOutput_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
idataoutput_writeBytes(const fn_call& fn)
{
    boost::intrusive_ptr<IDataOutput_as3> ptr =
        ensureType<IDataOutput_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
idataoutput_writeDouble(const fn_call& fn)
{
    boost::intrusive_ptr<IDataOutput_as3> ptr =
        ensureType<IDataOutput_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
idataoutput_writeFloat(const fn_call& fn)
{
    boost::intrusive_ptr<IDataOutput_as3> ptr =
        ensureType<IDataOutput_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
idataoutput_writeInt(const fn_call& fn)
{
    boost::intrusive_ptr<IDataOutput_as3> ptr =
        ensureType<IDataOutput_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
idataoutput_writeMultiByte(const fn_call& fn)
{
    boost::intrusive_ptr<IDataOutput_as3> ptr =
        ensureType<IDataOutput_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
idataoutput_writeObject(const fn_call& fn)
{
    boost::intrusive_ptr<IDataOutput_as3> ptr =
        ensureType<IDataOutput_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
idataoutput_writeShort(const fn_call& fn)
{
    boost::intrusive_ptr<IDataOutput_as3> ptr =
        ensureType<IDataOutput_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
idataoutput_writeUnsignedInt(const fn_call& fn)
{
    boost::intrusive_ptr<IDataOutput_as3> ptr =
        ensureType<IDataOutput_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
idataoutput_writeUTF(const fn_call& fn)
{
    boost::intrusive_ptr<IDataOutput_as3> ptr =
        ensureType<IDataOutput_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
idataoutput_writeUTFBytes(const fn_call& fn)
{
    boost::intrusive_ptr<IDataOutput_as3> ptr =
        ensureType<IDataOutput_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
idataoutput_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new IDataOutput_as3;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

