// StackFrame_as.cpp:  ActionScript "StackFrame" class, for Gnash.
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


#include "sampler/StackFrame_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value stackframe_file(const fn_call& fn);
    as_value stackframe_line(const fn_call& fn);
    as_value stackframe_name(const fn_call& fn);
    as_value stackframe_ctor(const fn_call& fn);
    void attachStackFrameInterface(as_object& o);
    void attachStackFrameStaticInterface(as_object& o);
}

// extern (used by Global.cpp)
void
stackframe_class_init(as_object& where, const ObjectURI& uri)
{
    registerBuiltinClass(where, stackframe_ctor, attachStackFrameInterface, 
        attachStackFrameStaticInterface, uri);
}

namespace {

void
attachStackFrameInterface(as_object& o)
{
    Global_as& gl = getGlobal(o);
    o.init_member("file", gl.createFunction(stackframe_file));
    o.init_member("line", gl.createFunction(stackframe_line));
    o.init_member("name", gl.createFunction(stackframe_name));
}

void
attachStackFrameStaticInterface(as_object& /*o*/)
{

}

as_value
stackframe_file(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stackframe_line(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stackframe_name(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stackframe_ctor(const fn_call& /*fn*/)
{
    return as_value();
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

