// PrintJob_as.cpp:  ActionScript "PrintJob" class, for Gnash.
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

#include "printing/PrintJob_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value printjob_send(const fn_call& fn);
    as_value printjob_start(const fn_call& fn);
    as_value printjob_ctor(const fn_call& fn);
    void attachPrintJobInterface(as_object& o);
    void attachPrintJobStaticInterface(as_object& o);
    as_object* getPrintJobInterface();

}

class PrintJob_as : public as_object
{

public:

    PrintJob_as()
        :
        as_object(getPrintJobInterface())
    {}
};

// extern (used by Global.cpp)
void printjob_class_init(as_object& where, const ObjectURI& uri)
{
    static boost::intrusive_ptr<as_object> cl;

    if (!cl) {
        Global_as* gl = getGlobal(where);
        as_object* proto = getPrintJobInterface();
        cl = gl->createClass(&printjob_ctor, proto);
        attachPrintJobStaticInterface(*cl);
    }

    // Register _global.PrintJob
    where.init_member(getName(uri), cl.get(), as_object::DefaultFlags,
            getNamespace(uri));
}

namespace {

void
attachPrintJobInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    o.init_member("send", gl->createFunction(printjob_send));
    o.init_member("start", gl->createFunction(printjob_start));
}

void
attachPrintJobStaticInterface(as_object& /*o*/)
{
}

as_object*
getPrintJobInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachPrintJobInterface(*o);
    }
    return o.get();
}

as_value
printjob_send(const fn_call& fn)
{
    boost::intrusive_ptr<PrintJob_as> ptr =
        ensureType<PrintJob_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
printjob_start(const fn_call& fn)
{
    boost::intrusive_ptr<PrintJob_as> ptr =
        ensureType<PrintJob_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
printjob_ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> obj = new PrintJob_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

