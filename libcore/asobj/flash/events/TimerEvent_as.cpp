// TimerEvent_as.cpp:  ActionScript "TimerEvent" class, for Gnash.
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

#include "events/TimerEvent_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value timerevent_toString(const fn_call& fn);
    as_value timerevent_updateAfterEvent(const fn_call& fn);
    as_value timerevent_TIMER(const fn_call& fn);
    as_value timerevent_TIMER_COMPLETE(const fn_call& fn);
    as_value timerevent_ctor(const fn_call& fn);
    void attachTimerEventInterface(as_object& o);
    void attachTimerEventStaticInterface(as_object& o);
    as_object* getTimerEventInterface();

}

class TimerEvent_as : public as_object
{

public:

    TimerEvent_as()
        :
        as_object(getTimerEventInterface())
    {}
};

// extern (used by Global.cpp)
void timerevent_class_init(as_object& global)
{
    static boost::intrusive_ptr<as_object> cl;

    if (!cl) {
        Global_as* gl = getGlobal(global);
        cl = gl->createClass(&timerevent_ctor, getTimerEventInterface());;
        attachTimerEventStaticInterface(*cl);
    }

    // Register _global.TimerEvent
    global.init_member("TimerEvent", cl.get());
}

namespace {

void
attachTimerEventInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    o.init_member("toString", gl->createFunction(timerevent_toString));
    o.init_member("updateAfterEvent", gl->createFunction(timerevent_updateAfterEvent));
    o.init_member("TIMER", gl->createFunction(timerevent_TIMER));
    o.init_member("TIMER_COMPLETE", gl->createFunction(timerevent_TIMER_COMPLETE));
}

void
attachTimerEventStaticInterface(as_object& o)
{
}

as_object*
getTimerEventInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachTimerEventInterface(*o);
    }
    return o.get();
}

as_value
timerevent_toString(const fn_call& fn)
{
    boost::intrusive_ptr<TimerEvent_as> ptr =
        ensureType<TimerEvent_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
timerevent_updateAfterEvent(const fn_call& fn)
{
    boost::intrusive_ptr<TimerEvent_as> ptr =
        ensureType<TimerEvent_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
timerevent_TIMER(const fn_call& fn)
{
    boost::intrusive_ptr<TimerEvent_as> ptr =
        ensureType<TimerEvent_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
timerevent_TIMER_COMPLETE(const fn_call& fn)
{
    boost::intrusive_ptr<TimerEvent_as> ptr =
        ensureType<TimerEvent_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
timerevent_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new TimerEvent_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

