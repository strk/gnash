// FileReference_as3.cpp:  ActionScript "FileReference" class, for Gnash.
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

#include "net/FileReference_as3.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value filereference_cancel(const fn_call& fn);
    as_value filereference_download(const fn_call& fn);
    as_value filereference_upload(const fn_call& fn);
    as_value filereference_complete(const fn_call& fn);
    as_value filereference_httpStatus(const fn_call& fn);
    as_value filereference_ioError(const fn_call& fn);
    as_value filereference_open(const fn_call& fn);
    as_value filereference_progress(const fn_call& fn);
    as_value filereference_securityError(const fn_call& fn);
    as_value filereference_select(const fn_call& fn);
    as_value filereference_uploadCompleteData(const fn_call& fn);
    as_value filereference_ctor(const fn_call& fn);
    void attachFileReferenceInterface(as_object& o);
    void attachFileReferenceStaticInterface(as_object& o);
    as_object* getFileReferenceInterface();

}

// extern (used by Global.cpp)
void filereference_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&filereference_ctor, getFileReferenceInterface());
        attachFileReferenceStaticInterface(*cl);
    }

    // Register _global.FileReference
    global.init_member("FileReference", cl.get());
}

namespace {

void
attachFileReferenceInterface(as_object& o)
{
    o.init_member("cancel", new builtin_function(filereference_cancel));
    o.init_member("download", new builtin_function(filereference_download));
    o.init_member("upload", new builtin_function(filereference_upload));
    o.init_member("cancel", new builtin_function(filereference_cancel));
    o.init_member("complete", new builtin_function(filereference_complete));
    o.init_member("httpStatus", new builtin_function(filereference_httpStatus));
    o.init_member("ioError", new builtin_function(filereference_ioError));
    o.init_member("open", new builtin_function(filereference_open));
    o.init_member("progress", new builtin_function(filereference_progress));
    o.init_member("securityError", new builtin_function(filereference_securityError));
    o.init_member("select", new builtin_function(filereference_select));
    o.init_member("uploadCompleteData", new builtin_function(filereference_uploadCompleteData));
}

void
attachFileReferenceStaticInterface(as_object& o)
{

}

as_object*
getFileReferenceInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachFileReferenceInterface(*o);
    }
    return o.get();
}

as_value
filereference_cancel(const fn_call& fn)
{
    boost::intrusive_ptr<FileReference_as3> ptr =
        ensureType<FileReference_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
filereference_download(const fn_call& fn)
{
    boost::intrusive_ptr<FileReference_as3> ptr =
        ensureType<FileReference_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
filereference_upload(const fn_call& fn)
{
    boost::intrusive_ptr<FileReference_as3> ptr =
        ensureType<FileReference_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
filereference_complete(const fn_call& fn)
{
    boost::intrusive_ptr<FileReference_as3> ptr =
        ensureType<FileReference_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
filereference_httpStatus(const fn_call& fn)
{
    boost::intrusive_ptr<FileReference_as3> ptr =
        ensureType<FileReference_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
filereference_ioError(const fn_call& fn)
{
    boost::intrusive_ptr<FileReference_as3> ptr =
        ensureType<FileReference_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
filereference_open(const fn_call& fn)
{
    boost::intrusive_ptr<FileReference_as3> ptr =
        ensureType<FileReference_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
filereference_progress(const fn_call& fn)
{
    boost::intrusive_ptr<FileReference_as3> ptr =
        ensureType<FileReference_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
filereference_securityError(const fn_call& fn)
{
    boost::intrusive_ptr<FileReference_as3> ptr =
        ensureType<FileReference_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
filereference_select(const fn_call& fn)
{
    boost::intrusive_ptr<FileReference_as3> ptr =
        ensureType<FileReference_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
filereference_uploadCompleteData(const fn_call& fn)
{
    boost::intrusive_ptr<FileReference_as3> ptr =
        ensureType<FileReference_as3>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
filereference_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new FileReference_as3;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

