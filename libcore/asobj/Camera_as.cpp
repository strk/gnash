// Camera_as.cpp:  ActionScript "Camera" class, for Gnash.
//
//   Copyright (C) 2009, 2010, 2011 Free Software Foundation, Inc.
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

#include "Camera_as.h"

#include <sstream>
#include <boost/scoped_ptr.hpp>
#include <memory>

#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "NativeFunction.h" 
#include "MediaHandler.h"
#include "VideoInput.h"
#include "RunResources.h"
#include "Object.h"
#include "namedStrings.h"


namespace gnash {

namespace {
    as_value camera_get(const fn_call& fn);
    as_value camera_setmode(const fn_call& fn);
    as_value camera_setmotionlevel(const fn_call& fn);
    as_value camera_setquality(const fn_call& fn);
    as_value camera_setLoopback(const fn_call& fn);
    as_value camera_setCursor(const fn_call& fn);
    as_value camera_setKeyFrameInterval(const fn_call& fn);
    as_value camera_activitylevel(const fn_call& fn);
    as_value camera_bandwidth(const fn_call& fn);
    as_value camera_currentFps(const fn_call& fn);
    as_value camera_fps(const fn_call& fn);
    as_value camera_height(const fn_call& fn);
    as_value camera_index(const fn_call& fn);
    as_value camera_motionLevel(const fn_call& fn);
    as_value camera_motionTimeout(const fn_call& fn);
    as_value camera_muted(const fn_call& fn);
    as_value camera_name(const fn_call& fn);
    as_value camera_names(const fn_call& fn);
    as_value camera_quality(const fn_call& fn);
    as_value camera_width(const fn_call& fn);

    void attachCameraStaticInterface(as_object& o);
    void attachCameraInterface(as_object& o);
}

// extern (used by Global.cpp)
void
camera_class_init(as_object& where, const ObjectURI& uri)
{
    registerBuiltinClass(where, emptyFunction, attachCameraInterface,
                         attachCameraStaticInterface, uri);
}

void
registerCameraNative(as_object& global)
{
    VM& vm = getVM(global);
    vm.registerNative(camera_names, 2102, 201);
    vm.registerNative(camera_setmode, 2102, 0);
    vm.registerNative(camera_setquality, 2102, 1);
    vm.registerNative(camera_setKeyFrameInterval, 2102, 2);
    vm.registerNative(camera_setmotionlevel, 2102, 3);
    vm.registerNative(camera_setLoopback, 2102, 4);
    vm.registerNative(camera_setCursor, 2102, 5);
}

namespace {

void
attachCameraStaticInterface(as_object& o)
{
    Global_as& gl = getGlobal(o);
    
    const int flags = 0;

	o.init_member("get", gl.createFunction(camera_get), flags);

    VM& vm = getVM(o);
    NativeFunction* getset = vm.getNative(2102, 201);
    o.init_property("names", *getset, *getset);

}

void
attachCameraInterface(as_object& o)
{
    
    const int flags = as_object::DefaultFlags | PropFlags::onlySWF6Up;

    VM& vm = getVM(o);
    o.init_member("setMode", vm.getNative(2102, 0), flags);
    o.init_member("setQuality", vm.getNative(2102, 1), flags);
    o.init_member("setKeyFrameInterval", vm.getNative(2102, 2), flags);
    o.init_member("setMotionLevel", vm.getNative(2102, 3), flags);
    o.init_member("setLoopback", vm.getNative(2102, 4), flags);
    o.init_member("setCursor", vm.getNative(2102, 5), flags);

}

// Properties attached to the prototype when Camera.get() is called
void
attachCameraProperties(as_object& o)
{
    Global_as& gl = getGlobal(o);

    as_function* getset = gl.createFunction(camera_activitylevel);
    o.init_property("activityLevel", *getset, *getset);
    getset = gl.createFunction(camera_bandwidth);
    o.init_property("bandwidth", *getset, *getset);
    getset = gl.createFunction(camera_currentFps);
    o.init_property("currentFps", *getset, *getset);
    getset = gl.createFunction(camera_fps);
    o.init_property("fps", *getset, *getset);
    getset = gl.createFunction(camera_height);
    o.init_property("height", *getset, *getset);
    getset = gl.createFunction(camera_index);
    o.init_property("index", *getset, *getset);
    getset = gl.createFunction(camera_motionLevel);
    o.init_property("motionLevel", *getset, *getset);
    getset = gl.createFunction(camera_motionTimeout);
    o.init_property("motionTimeout", *getset, *getset);
    getset = gl.createFunction(camera_muted);
    o.init_property("muted", *getset, *getset);
    getset = gl.createFunction(camera_name);
    o.init_property("name", *getset, *getset);
    getset = gl.createFunction(camera_quality);
    o.init_property("quality", *getset, *getset);
    getset = gl.createFunction(camera_width);
    o.init_property("width", *getset, *getset);
}

class Camera_as: public Relay
{
public:

    Camera_as(std::auto_ptr<media::VideoInput> input)
        :
        _input(input.release()),
        _loopback(false)
    {
        assert(_input.get());
    }

    bool muted() const {
        return _input->muted();
    }

    size_t width() const {
        return _input->width();
    }

    size_t height() const {
        return _input->height();
    }

    double fps() const {
        return _input->fps();
    }

    double currentFPS() const {
        return _input->currentFPS();
    }

    double activityLevel() const {
        return _input->activityLevel();
    }

    double bandwidth() const {
        return _input->bandwidth();
    }

    size_t index() const {
        return _input->index();
    }

    void setMode(size_t width, size_t height, double fps, bool favorArea) {
        _input->requestMode(width, height, fps, favorArea);
    }

    void setMotionLevel(size_t level, double timeout) {
        _input->setMotionLevel(level);
        _input->setMotionTimeout(timeout);
    }

    double motionLevel() const {
        return _input->motionLevel();
    }

    double motionTimeout() const {
        return _input->motionTimeout();
    }

    const std::string& name() const {
        return _input->name();
    }

    size_t quality() const {
        return _input->quality();
    }

    void setQuality(double bandwidth, size_t quality) {
        _input->setBandwidth(bandwidth);
        _input->setQuality(quality);
    }

    void setLoopback(bool b) {
        _loopback = b;
    }

private:

    boost::scoped_ptr<media::VideoInput> _input;

    // TODO: see whether this should be handled in the VideoInput class
    bool _loopback;
};

// AS2 static accessor.
as_value
camera_get(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);

    // Properties are attached to the prototype (not __proto__) when get() is
    // called. 
    as_object* proto = toObject(
        getMember(*ptr, NSV::PROP_PROTOTYPE), getVM(fn));

    attachCameraProperties(*proto);

    // TODO: this should return the same object when the same device is
    // meant, not a new object each time. It will be necessary to query
    // the MediaHandler for this, and possibly to store the as_objects
    // somewhere.
    const RunResources& r = getRunResources(getGlobal(fn));
    media::MediaHandler* handler = r.mediaHandler();

    if (!handler) {
        log_error(_("No MediaHandler exists! Cannot create a Camera object"));
        return as_value();
    }
    std::auto_ptr<media::VideoInput> input(handler->getVideoInput(0));

    if (!input.get()) {
        // TODO: what should happen if the index is not available?
        return as_value();
    }

    const size_t nargs = fn.nargs;
    if (nargs > 0) {
        log_debug("%s: the camera is automatically chosen from gnashrc",
                "Camera.get()");
    }

    // Normally the VM would furnish us with a newly instantiated object, if
    // a constructor were used. But we're in a factory, so we have to build
    // one for ourselves.
    as_object* cam_obj = createObject(getGlobal(fn));
    cam_obj->set_prototype(proto);
    attachCameraInterface(*cam_obj);
    attachCameraProperties(*cam_obj);

    cam_obj->setRelay(new Camera_as(input));

    return as_value(cam_obj); 
}

as_value
camera_setmode(const fn_call& fn)
{
    Camera_as* ptr = ensure<ThisIsNative<Camera_as> >(fn);

    const size_t nargs = fn.nargs;

    const double width = nargs ? toNumber(fn.arg(0), getVM(fn)) : 160;
    const double height = nargs > 1 ? toNumber(fn.arg(1), getVM(fn)) : 120;
    const double fps = nargs >  2? toNumber(fn.arg(2), getVM(fn)) : 15;
    const bool favorArea = nargs > 3 ? toBool(fn.arg(3), getVM(fn)) : true;

    // TODO: handle overflow
    const size_t reqWidth = std::max<double>(width, 0);
    const size_t reqHeight = std::max<double>(height, 0);

    ptr->setMode(reqWidth, reqHeight, fps, favorArea);

    return as_value();
}

as_value
camera_setmotionlevel(const fn_call& fn)
{
    log_unimpl ("Camera::motionLevel can be set, but it's not implemented");
    Camera_as* ptr = ensure<ThisIsNative<Camera_as> >(fn);
    
    const size_t nargs = fn.nargs;

    const double ml = nargs > 0 ? toNumber(fn.arg(0), getVM(fn)) : 50;
    const double mt = nargs > 1 ? toNumber(fn.arg(1), getVM(fn)) : 2000;

    const size_t motionLevel = (ml >= 0 && ml <= 100) ? ml : 100;

    ptr->setMotionLevel(motionLevel, mt);

    return as_value();
}


as_value
camera_setquality(const fn_call& fn)
{
    log_unimpl ("Camera::quality can be set, but it's not implemented");
    Camera_as* ptr = ensure<ThisIsNative<Camera_as> >(fn);

    const size_t nargs = fn.nargs;

    const double b = nargs > 0 ? toNumber(fn.arg(0), getVM(fn)) : 16384;
    const double q = nargs > 1 ? toNumber(fn.arg(1), getVM(fn)) : 0;

    size_t quality = (q < 0 || q > 100) ? 100 : q;

    ptr->setQuality(b, quality);

    return as_value();
}


as_value
camera_activitylevel(const fn_call& fn)
{
    Camera_as* ptr = ensure<ThisIsNative<Camera_as> >(fn);

    if (!fn.nargs) {
        log_unimpl("Camera::activityLevel only has default value");
        return as_value(ptr->activityLevel());
    }

    IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set activity property of Camera"));
    );

    return as_value();
}

as_value
camera_bandwidth(const fn_call& fn)
{
    Camera_as* ptr = ensure<ThisIsNative<Camera_as> >(fn);

    if (!fn.nargs) {
        log_unimpl("Camera::bandwidth only has default value");
        return as_value(ptr->bandwidth());
    }

    IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set bandwidth property of Camera"));
    );

    return as_value();
}

as_value
camera_currentFps(const fn_call& fn)
{
    Camera_as* ptr = ensure<ThisIsNative<Camera_as> >(fn);

    if (!fn.nargs) {
        return as_value(ptr->currentFPS());
    }

    IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set currentFPS property of Camera"));
    );

    return as_value();
}

as_value
camera_fps(const fn_call& fn)
{
    Camera_as* ptr = ensure<ThisIsNative<Camera_as> >(fn);

    if (!fn.nargs) {
        return as_value(ptr->fps());
    }

    IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set fps property of Camera"));
    );

    return as_value();
}

as_value
camera_height(const fn_call& fn)
{
    Camera_as* ptr = ensure<ThisIsNative<Camera_as> >(fn);

    if (!fn.nargs) {
        return as_value(ptr->height());
    }

    IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set height property of Camera, use setMode"));
    );

    return as_value();
}

as_value
camera_index(const fn_call& fn)
{
    Camera_as* ptr = ensure<ThisIsNative<Camera_as> >(fn);

    if (!fn.nargs) 
    {
        // livedocs say that this function should return an integer,
        // but in testing the pp returns the value as a string
        int value = ptr->index();
        
        std::ostringstream ss;
        ss << value;
        return as_value(ss.str());
    }

    IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set index property of Camera"));
    );

    return as_value();
}

as_value
camera_motionLevel(const fn_call& fn)
{
    Camera_as* ptr = ensure<ThisIsNative<Camera_as> >(fn);

    if (!fn.nargs) {
        log_unimpl("Camera::motionLevel only has default value");
        return as_value(ptr->motionLevel());
    }

    IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set motionLevel property of Camera"));
    );

    return as_value();
}

as_value
camera_motionTimeout(const fn_call& fn)
{
    Camera_as* ptr = ensure<ThisIsNative<Camera_as> >(fn);

    if (!fn.nargs) {
        log_unimpl("Camera::motionTimeout");
        return as_value(ptr->motionTimeout());
    }

    IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set motionTimeout property of Camera"));
    );

    return as_value();
}

as_value
camera_muted(const fn_call& fn)
{
    Camera_as* ptr = ensure<ThisIsNative<Camera_as> >(fn);

    if (!fn.nargs) {
        log_unimpl("Camera.muted");
        return as_value(ptr->muted());
    }

    IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set muted property of Camera"));
    );

    return as_value();
}

as_value
camera_name(const fn_call& fn)
{
    Camera_as* ptr = ensure<ThisIsNative<Camera_as> >(fn);

    if (!fn.nargs) {
        return as_value(ptr->name());
    }

    IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set name property of Camera"));
    );

    return as_value();
}

as_value
camera_names(const fn_call& fn)
{
    if (fn.nargs) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Attempt to set names property of Camera"));
        );
        return as_value();
    }

    std::vector<std::string> names;
    media::MediaHandler* m = getRunResources(getGlobal(fn)).mediaHandler();
    if (!m) return as_value();

    m->cameraNames(names);
    
    const size_t size = names.size();
    
    Global_as& gl = getGlobal(fn);
    as_object* data = gl.createArray();

    for (size_t i = 0; i < size; ++i) {
        callMethod(data, NSV::PROP_PUSH, names[i]);
    }
    
    return as_value(data);
} 


as_value
camera_quality(const fn_call& fn)
{
    Camera_as* ptr = ensure<ThisIsNative<Camera_as> >(fn);

    if (!fn.nargs) {
        log_unimpl("Camera::quality has only default values");
        return as_value(ptr->quality());
    }

    IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set quality property of Camera"));
    );

    return as_value();
}

as_value
camera_setLoopback(const fn_call& fn)
{
    Camera_as* ptr = ensure<ThisIsNative<Camera_as> >(fn);
    
    if (!fn.nargs) {
        // TODO: log AS error.
        return as_value();
    }

    if (fn.nargs > 1) {
        log_aserror("%s: Too many arguments", "Camera.setLoopback");
    }

    ptr->setLoopback(toBool(fn.arg(0), getVM(fn)));
    
    return as_value();
}

as_value
camera_setCursor(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl("Camera.setCursor"));
    return as_value();
}

as_value
camera_setKeyFrameInterval(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl("Camera.setKeyFrameInterval"));
    return as_value();
}

as_value
camera_width(const fn_call& fn)
{
    Camera_as* ptr = ensure<ThisIsNative<Camera_as> >(fn);

    if (!fn.nargs) {
        return as_value(ptr->width());
    }

    IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set width property of Camera, use setMode"));
    );

    return as_value();
}

} // anonymous namespace

} // end of gnash namespace

