// Video.cpp:  Draw individual video frames, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "Video_as.h"

#include "Video.h"
#include "fn_call.h"
#include "as_value.h"
#include "Global_as.h"
#include "as_object.h"
#include "NativeFunction.h"
#include "NetStream_as.h"
#include "namedStrings.h"

namespace gnash {

namespace {    
    void attachPrototypeProperties(as_object& o);
    void attachVideoInterface(as_object& o);
    as_value video_attach(const fn_call& fn);
    as_value video_clear(const fn_call& fn);
    as_value video_deblocking(const fn_call& fn);
    as_value video_smoothing(const fn_call& fn);
    as_value video_width(const fn_call& fn);
    as_value video_height(const fn_call& fn);
}

as_object*
createVideoObject(Global_as& gl)
{
    as_object* obj = getObjectWithPrototype(gl, NSV::CLASS_VIDEO);
    as_object* proto = obj->get_prototype();
    if (proto) {
        attachPrototypeProperties(*proto);
    }
    return obj;
}

// extern (used by Global.cpp)
void
video_class_init(as_object& global, const ObjectURI& uri)
{
    Global_as& gl = getGlobal(global);
    as_object* proto = createObject(gl);
    as_object* cl = gl.createClass(emptyFunction, proto);
    attachVideoInterface(*proto);
    
    // Register _global.Video
    global.init_member(uri, cl, as_object::DefaultFlags);
}

void
registerVideoNative(as_object& global)
{
    VM& vm = getVM(global);
    // Constructor. Since it's native, this is probably used internally
    // and isn't the class constructor, which is an empty function.
    vm.registerNative(emptyFunction, 667, 0);
    vm.registerNative(video_attach, 667, 1);
    vm.registerNative(video_clear, 667, 2);
}

namespace {

void
attachVideoInterface(as_object& o)
{
    VM& vm = getVM(o);
    o.init_member("attachVideo", vm.getNative(667, 1));
    o.init_member("clear", vm.getNative(667, 2));
}

void
attachPrototypeProperties(as_object& proto)
{
    const int protect = PropFlags::dontDelete;
    
    proto.init_property("deblocking", &video_deblocking, &video_deblocking,
            protect);
    proto.init_property("smoothing", &video_smoothing, &video_smoothing,
            protect);
    
    const int flags = PropFlags::dontDelete |
        PropFlags::readOnly;

    proto.init_property("height", &video_height, &video_height, flags);
    proto.init_property("width", &video_width, &video_width, flags);
}

as_value
video_attach(const fn_call& fn)
{
    Video* video = ensure<IsDisplayObject<Video> >(fn);
    
    if (fn.nargs < 1) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("attachVideo needs 1 arg"));
            );
        return as_value();
    }

    as_object* obj = toObject(fn.arg(0), getVM(fn));
    NetStream_as* ns;

    if (isNativeType(obj, ns)) {
        video->setStream(ns);
    } else {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("attachVideo(%s) first arg is not a NetStream instance"),
			fn.arg(0));
            );
    }
    return as_value();
}

as_value
video_deblocking(const fn_call& fn)
{
    Video* video = ensure<IsDisplayObject<Video> >(fn);
    UNUSED(video);

    log_unimpl("Video.deblocking");
    return as_value();
}

as_value
video_smoothing(const fn_call& fn)
{
    Video* video = ensure<IsDisplayObject<Video> >(fn);

    if (!fn.nargs) {
        return as_value(video->smoothing());
    }

    const bool smooth = toBool(fn.arg(0), getVM(fn));
    video->setSmoothing(smooth);

    return as_value();
}

as_value
video_width(const fn_call& fn)
{
    Video* video = ensure<IsDisplayObject<Video> >(fn);
    return as_value(video->width());
}

as_value
video_height(const fn_call& fn)
{
    Video* video = ensure<IsDisplayObject<Video> >(fn);
    return as_value(video->height());
}

as_value
video_clear(const fn_call& fn)
{
    Video* video = ensure<IsDisplayObject<Video> >(fn);
    video->clear();
    return as_value();
}

} // anonymous namespace
} // namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
