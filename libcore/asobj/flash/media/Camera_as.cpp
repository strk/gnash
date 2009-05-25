// Camera_as.cpp:  ActionScript "Camera" class, for Gnash.
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

#include "media/Camera_as.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value camera_getCamera(const fn_call& fn);
    as_value camera_setKeyFrameInterval(const fn_call& fn);
    as_value camera_setLoopback(const fn_call& fn);
    as_value camera_setMode(const fn_call& fn);
    as_value camera_setMotionLevel(const fn_call& fn);
    as_value camera_setQuality(const fn_call& fn);
    as_value camera_activity(const fn_call& fn);
    as_value camera_status(const fn_call& fn);
    as_value camera_ctor(const fn_call& fn);
    void attachCameraInterface(as_object& o);
    void attachCameraStaticInterface(as_object& o);
    as_object* getCameraInterface();

}

// extern (used by Global.cpp)
void camera_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&camera_ctor, getCameraInterface());
        attachCameraStaticInterface(*cl);
    }

    // Register _global.Camera
    global.init_member("Camera", cl.get());
}

namespace {

void
attachCameraInterface(as_object& o)
{
    o.init_member("getCamera", new builtin_function(camera_getCamera));
    o.init_member("setKeyFrameInterval", new builtin_function(camera_setKeyFrameInterval));
    o.init_member("setLoopback", new builtin_function(camera_setLoopback));
    o.init_member("setMode", new builtin_function(camera_setMode));
    o.init_member("setMotionLevel", new builtin_function(camera_setMotionLevel));
    o.init_member("setQuality", new builtin_function(camera_setQuality));
    o.init_member("activity", new builtin_function(camera_activity));
    o.init_member("status", new builtin_function(camera_status));
}

void
attachCameraStaticInterface(as_object& o)
{

}

as_object*
getCameraInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachCameraInterface(*o);
    }
    return o.get();
}

as_value
camera_getCamera(const fn_call& fn)
{
    boost::intrusive_ptr<Camera_as> ptr =
        ensureType<Camera_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
camera_setKeyFrameInterval(const fn_call& fn)
{
    boost::intrusive_ptr<Camera_as> ptr =
        ensureType<Camera_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
camera_setLoopback(const fn_call& fn)
{
    boost::intrusive_ptr<Camera_as> ptr =
        ensureType<Camera_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
camera_setMode(const fn_call& fn)
{
    boost::intrusive_ptr<Camera_as> ptr =
        ensureType<Camera_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
camera_setMotionLevel(const fn_call& fn)
{
    boost::intrusive_ptr<Camera_as> ptr =
        ensureType<Camera_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
camera_setQuality(const fn_call& fn)
{
    boost::intrusive_ptr<Camera_as> ptr =
        ensureType<Camera_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
camera_activity(const fn_call& fn)
{
    boost::intrusive_ptr<Camera_as> ptr =
        ensureType<Camera_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
camera_status(const fn_call& fn)
{
    boost::intrusive_ptr<Camera_as> ptr =
        ensureType<Camera_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
camera_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new Camera_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

