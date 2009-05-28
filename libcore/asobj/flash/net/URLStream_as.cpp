// URLStream_as.cpp:  ActionScript "URLStream" class, for Gnash.
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

#include "net/URLStream_as.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value urlstream_load(const fn_call& fn);
    as_value urlstream_readBoolean(const fn_call& fn);
    as_value urlstream_readByte(const fn_call& fn);
    as_value urlstream_readBytes(const fn_call& fn);
    as_value urlstream_readDouble(const fn_call& fn);
    as_value urlstream_readFloat(const fn_call& fn);
    as_value urlstream_readInt(const fn_call& fn);
    as_value urlstream_readMultiByte(const fn_call& fn);
    as_value urlstream_readObject(const fn_call& fn);
    as_value urlstream_readShort(const fn_call& fn);
    as_value urlstream_readUnsignedByte(const fn_call& fn);
    as_value urlstream_readUnsignedInt(const fn_call& fn);
    as_value urlstream_readUnsignedShort(const fn_call& fn);
    as_value urlstream_readUTF(const fn_call& fn);
    as_value urlstream_readUTFBytes(const fn_call& fn);
    as_value urlstream_complete(const fn_call& fn);
    as_value urlstream_httpStatus(const fn_call& fn);
    as_value urlstream_ioError(const fn_call& fn);
    as_value urlstream_open(const fn_call& fn);
    as_value urlstream_progress(const fn_call& fn);
    as_value urlstream_securityError(const fn_call& fn);
    as_value urlstream_ctor(const fn_call& fn);
    void attachURLStreamInterface(as_object& o);
    void attachURLStreamStaticInterface(as_object& o);
    as_object* getURLStreamInterface();

}

class URLStream_as : public as_object
{

public:

    URLStream_as()
        :
        as_object(getURLStreamInterface())
    {}
};

// extern (used by Global.cpp)
void urlstream_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&urlstream_ctor, getURLStreamInterface());
        attachURLStreamStaticInterface(*cl);
    }

    // Register _global.URLStream
    global.init_member("URLStream", cl.get());
}

namespace {

void
attachURLStreamInterface(as_object& o)
{
    o.init_member("load", new builtin_function(urlstream_load));
    o.init_member("readBoolean", new builtin_function(urlstream_readBoolean));
    o.init_member("readByte", new builtin_function(urlstream_readByte));
    o.init_member("readBytes", new builtin_function(urlstream_readBytes));
    o.init_member("readDouble", new builtin_function(urlstream_readDouble));
    o.init_member("readFloat", new builtin_function(urlstream_readFloat));
    o.init_member("readInt", new builtin_function(urlstream_readInt));
    o.init_member("readMultiByte", new builtin_function(urlstream_readMultiByte));
    o.init_member("readObject", new builtin_function(urlstream_readObject));
    o.init_member("readShort", new builtin_function(urlstream_readShort));
    o.init_member("readUnsignedByte", new builtin_function(urlstream_readUnsignedByte));
    o.init_member("readUnsignedInt", new builtin_function(urlstream_readUnsignedInt));
    o.init_member("readUnsignedShort", new builtin_function(urlstream_readUnsignedShort));
    o.init_member("readUTF", new builtin_function(urlstream_readUTF));
    o.init_member("readUTFBytes", new builtin_function(urlstream_readUTFBytes));
    o.init_member("complete", new builtin_function(urlstream_complete));
    o.init_member("httpStatus", new builtin_function(urlstream_httpStatus));
    o.init_member("ioError", new builtin_function(urlstream_ioError));
    o.init_member("open", new builtin_function(urlstream_open));
    o.init_member("progress", new builtin_function(urlstream_progress));
    o.init_member("securityError", new builtin_function(urlstream_securityError));
}

void
attachURLStreamStaticInterface(as_object& o)
{

}

as_object*
getURLStreamInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachURLStreamInterface(*o);
    }
    return o.get();
}

as_value
urlstream_load(const fn_call& fn)
{
    boost::intrusive_ptr<URLStream_as> ptr =
        ensureType<URLStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlstream_readBoolean(const fn_call& fn)
{
    boost::intrusive_ptr<URLStream_as> ptr =
        ensureType<URLStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlstream_readByte(const fn_call& fn)
{
    boost::intrusive_ptr<URLStream_as> ptr =
        ensureType<URLStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlstream_readBytes(const fn_call& fn)
{
    boost::intrusive_ptr<URLStream_as> ptr =
        ensureType<URLStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlstream_readDouble(const fn_call& fn)
{
    boost::intrusive_ptr<URLStream_as> ptr =
        ensureType<URLStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlstream_readFloat(const fn_call& fn)
{
    boost::intrusive_ptr<URLStream_as> ptr =
        ensureType<URLStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlstream_readInt(const fn_call& fn)
{
    boost::intrusive_ptr<URLStream_as> ptr =
        ensureType<URLStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlstream_readMultiByte(const fn_call& fn)
{
    boost::intrusive_ptr<URLStream_as> ptr =
        ensureType<URLStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlstream_readObject(const fn_call& fn)
{
    boost::intrusive_ptr<URLStream_as> ptr =
        ensureType<URLStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlstream_readShort(const fn_call& fn)
{
    boost::intrusive_ptr<URLStream_as> ptr =
        ensureType<URLStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlstream_readUnsignedByte(const fn_call& fn)
{
    boost::intrusive_ptr<URLStream_as> ptr =
        ensureType<URLStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlstream_readUnsignedInt(const fn_call& fn)
{
    boost::intrusive_ptr<URLStream_as> ptr =
        ensureType<URLStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlstream_readUnsignedShort(const fn_call& fn)
{
    boost::intrusive_ptr<URLStream_as> ptr =
        ensureType<URLStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlstream_readUTF(const fn_call& fn)
{
    boost::intrusive_ptr<URLStream_as> ptr =
        ensureType<URLStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlstream_readUTFBytes(const fn_call& fn)
{
    boost::intrusive_ptr<URLStream_as> ptr =
        ensureType<URLStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlstream_complete(const fn_call& fn)
{
    boost::intrusive_ptr<URLStream_as> ptr =
        ensureType<URLStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlstream_httpStatus(const fn_call& fn)
{
    boost::intrusive_ptr<URLStream_as> ptr =
        ensureType<URLStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlstream_ioError(const fn_call& fn)
{
    boost::intrusive_ptr<URLStream_as> ptr =
        ensureType<URLStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlstream_open(const fn_call& fn)
{
    boost::intrusive_ptr<URLStream_as> ptr =
        ensureType<URLStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlstream_progress(const fn_call& fn)
{
    boost::intrusive_ptr<URLStream_as> ptr =
        ensureType<URLStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlstream_securityError(const fn_call& fn)
{
    boost::intrusive_ptr<URLStream_as> ptr =
        ensureType<URLStream_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlstream_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new URLStream_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

