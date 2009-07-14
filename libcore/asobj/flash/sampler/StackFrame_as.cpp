// StackFrame_as.cpp:  ActionScript "StackFrame" class, for Gnash.
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
    as_object* getStackFrameInterface();

}

class StackFrame_as : public as_object
{

public:

    StackFrame_as()
        :
        as_object(getStackFrameInterface())
    {}
};

// extern (used by Global.cpp)
void stackframe_class_init(as_object& global)
{
    static boost::intrusive_ptr<as_object> cl;

    if (!cl) {
        Global_as* gl = getGlobal(global);
        cl = gl->createClass(&stackframe_ctor, getStackFrameInterface());;
        attachStackFrameStaticInterface(*cl);
    }

    // Register _global.StackFrame
    global.init_member("StackFrame", cl.get());
}

namespace {

void
attachStackFrameInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    o.init_member("file", gl->createFunction(stackframe_file));
    o.init_member("line", gl->createFunction(stackframe_line));
    o.init_member("name", gl->createFunction(stackframe_name));
}

void
attachStackFrameStaticInterface(as_object& o)
{

}

as_object*
getStackFrameInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachStackFrameInterface(*o);
    }
    return o.get();
}

as_value
stackframe_file(const fn_call& fn)
{
    boost::intrusive_ptr<StackFrame_as> ptr =
        ensureType<StackFrame_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stackframe_line(const fn_call& fn)
{
    boost::intrusive_ptr<StackFrame_as> ptr =
        ensureType<StackFrame_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stackframe_name(const fn_call& fn)
{
    boost::intrusive_ptr<StackFrame_as> ptr =
        ensureType<StackFrame_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stackframe_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new StackFrame_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

