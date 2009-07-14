// DataEvent_as.cpp:  ActionScript "DataEvent" class, for Gnash.
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

#include "events/DataEvent_as.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value dataevent_toString(const fn_call& fn);
    as_value dataevent_DATA(const fn_call& fn);
    as_value dataevent_UPLOAD_COMPLETE_DATA(const fn_call& fn);
    as_value dataevent_ctor(const fn_call& fn);
    void attachDataEventInterface(as_object& o);
    void attachDataEventStaticInterface(as_object& o);
    as_object* getDataEventInterface();

}

class DataEvent_as : public as_object
{

public:

    DataEvent_as()
        :
        as_object(getDataEventInterface())
    {}
};

// extern (used by Global.cpp)
void dataevent_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&dataevent_ctor, getDataEventInterface());
        attachDataEventStaticInterface(*cl);
    }

    // Register _global.DataEvent
    global.init_member("DataEvent", cl.get());
}

namespace {

void
attachDataEventInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    o.init_member("toString", gl->createFunction(dataevent_toString));
    o.init_member("DATA", gl->createFunction(dataevent_DATA));
    o.init_member("UPLOAD_COMPLETE_DATA", gl->createFunction(dataevent_UPLOAD_COMPLETE_DATA));
}

void
attachDataEventStaticInterface(as_object& o)
{

}

as_object*
getDataEventInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachDataEventInterface(*o);
    }
    return o.get();
}

as_value
dataevent_toString(const fn_call& fn)
{
    boost::intrusive_ptr<DataEvent_as> ptr =
        ensureType<DataEvent_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
dataevent_DATA(const fn_call& fn)
{
    boost::intrusive_ptr<DataEvent_as> ptr =
        ensureType<DataEvent_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
dataevent_UPLOAD_COMPLETE_DATA(const fn_call& fn)
{
    boost::intrusive_ptr<DataEvent_as> ptr =
        ensureType<DataEvent_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
dataevent_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new DataEvent_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

