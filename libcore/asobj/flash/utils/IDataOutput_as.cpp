// IDataOutput_as.cpp:  ActionScript "IDataOutput" class, for Gnash.
//
//   Copyright (C) 2009, 2010 Free Software Foundation, Inc.
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

#include "utils/IDataOutput_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
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
}

// extern (used by Global.cpp)
void
idataoutput_class_init(as_object& where, const ObjectURI& uri)
{
    registerBuiltinClass(where, idataoutput_ctor, attachIDataOutputInterface,
            attachIDataOutputStaticInterface, uri);
}

namespace {

void
attachIDataOutputInterface(as_object& o)
{
    Global_as& gl = getGlobal(o);
    o.init_member("writeByte", gl.createFunction(idataoutput_writeByte));
    o.init_member("writeBytes", gl.createFunction(idataoutput_writeBytes));
    o.init_member("writeDouble", gl.createFunction(idataoutput_writeDouble));
    o.init_member("writeFloat", gl.createFunction(idataoutput_writeFloat));
    o.init_member("writeInt", gl.createFunction(idataoutput_writeInt));
    o.init_member("writeMultiByte", gl.createFunction(idataoutput_writeMultiByte));
    o.init_member("writeObject", gl.createFunction(idataoutput_writeObject));
    o.init_member("writeShort", gl.createFunction(idataoutput_writeShort));
    o.init_member("writeUnsignedInt", gl.createFunction(idataoutput_writeUnsignedInt));
    o.init_member("writeUTF", gl.createFunction(idataoutput_writeUTF));
    o.init_member("writeUTFBytes", gl.createFunction(idataoutput_writeUTFBytes));
}

void
attachIDataOutputStaticInterface(as_object& /*o*/)
{
}

as_value
idataoutput_writeByte(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
idataoutput_writeBytes(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
idataoutput_writeDouble(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
idataoutput_writeFloat(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
idataoutput_writeInt(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
idataoutput_writeMultiByte(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
idataoutput_writeObject(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
idataoutput_writeShort(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
idataoutput_writeUnsignedInt(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
idataoutput_writeUTF(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
idataoutput_writeUTFBytes(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
idataoutput_ctor(const fn_call& /*fn*/)
{
    return as_value();
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

