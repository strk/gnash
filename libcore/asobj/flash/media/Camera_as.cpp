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

#include "flash/media/Camera_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "Object.h" // for getObjectInterface
#include "Array_as.h"
#include <sstream>

#ifdef USE_GST
#include "gst/VideoInputGst.h"
#endif

#ifdef USE_FFMPEG
#include "VideoInput.h"
#endif

namespace gnash {

as_value camera_get(const fn_call& fn);
as_value camera_getCamera(const fn_call& fn);
as_value camera_setmode(const fn_call& fn);
as_value camera_setmotionlevel(const fn_call& fn);
as_value camera_setquality(const fn_call& fn);
as_value camera_setLoopback(const fn_call& fn);
as_value camera_setCursor(const fn_call& fn);
as_value camera_setKeyFrameInterval(const fn_call& fn);

as_value camera_activitylevel(const fn_call& fn);
as_value camera_bandwidth(const fn_call& fn);
as_value camera_currentFPS(const fn_call& fn); //as3
as_value camera_currentFps(const fn_call& fn); //as2
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


void
attachCameraStaticInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    
    const int flags = 0;

    // get() is a function with an Object() as prototype.
    as_object* proto = gl->createObject(getObjectInterface());

    // TODO: avoid the creative abuse of createClass.
	o.init_member("get", gl->createClass(camera_get, proto), flags);

    boost::intrusive_ptr<builtin_function> getset =
        gl->createFunction(camera_names);
    o.init_property("names", *getset, *getset);

}

void
attachCameraAS3StaticInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    o.init_member("getCamera", gl->createFunction(camera_getCamera));
}

static void
attachCameraInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    
    o.init_member("setMode", gl->createFunction(camera_setmode));
    o.init_member("setMotionLevel", gl->createFunction(camera_setmotionlevel));
    o.init_member("setQuality", gl->createFunction(camera_setquality));
    o.init_member("setCursor", gl->createFunction(camera_setCursor));
    o.init_member("setLoopback", gl->createFunction(camera_setLoopback));
    o.init_member("setKeyFrameInterval",
            gl->createFunction(camera_setKeyFrameInterval));

}

// Properties attached to the prototype when Camera.get() is called
void
attachCameraProperties(as_object& o)
{
    Global_as* gl = getGlobal(o);
    boost::intrusive_ptr<builtin_function> getset;

    getset = gl->createFunction(camera_activitylevel);
    o.init_property("activityLevel", *getset, *getset);
    getset = gl->createFunction(camera_bandwidth);
    o.init_property("bandwidth", *getset, *getset);
    getset = gl->createFunction(camera_currentFps);
    o.init_property("currentFps", *getset, *getset);
    getset = gl->createFunction(camera_fps);
    o.init_property("fps", *getset, *getset);
    getset = gl->createFunction(camera_height);
    o.init_property("height", *getset, *getset);
    getset = gl->createFunction(camera_index);
    o.init_property("index", *getset, *getset);
    getset = gl->createFunction(camera_motionLevel);
    o.init_property("motionLevel", *getset, *getset);
    getset = gl->createFunction(camera_motionTimeout);
    o.init_property("motionTimeout", *getset, *getset);
    getset = gl->createFunction(camera_muted);
    o.init_property("muted", *getset, *getset);
    getset = gl->createFunction(camera_name);
    o.init_property("name", *getset, *getset);
    getset = gl->createFunction(camera_quality);
    o.init_property("quality", *getset, *getset);
    getset = gl->createFunction(camera_width);
    o.init_property("width", *getset, *getset);
}

static as_object*
getCameraInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o )
    {
        o = new as_object(getObjectInterface());
        attachCameraInterface(*o);
    }
    return o.get();
}

#ifdef USE_GST
class camera_as_object: public as_object, public media::gst::VideoInputGst {

public:

    camera_as_object()
        :
        as_object(getCameraInterface())
    {}
};
#endif

#ifdef USE_FFMPEG
class camera_as_object: public as_object, public media::VideoInput {
public:

    camera_as_object()
        :
        as_object(getCameraInterface())
    {}
};
#endif

// AS2 static accessor.
as_value
camera_get(const fn_call& fn)
{

    // Properties are attached to the prototype when get() is called.
    as_object* proto = getCameraInterface();

    // This is an AS2-only function, so don't worry about VM version.
    attachCameraProperties(*proto);

    // TODO: this should return the same object when the same device is
    // meant, not a new object each time. It will be necessary to query
    // the MediaHandler for this, and possibly to store the as_objects
    // somewhere.
    boost::intrusive_ptr<as_object> obj = new camera_as_object; 
     

    int numargs = fn.nargs;
    if (numargs > 0) {
        log_debug("%s: the camera is automatically chosen from gnashrc",
                __FUNCTION__);
    }
    return as_value(obj.get()); // will keep alive
}

// AS3 static accessor.
as_value
camera_getCamera(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new camera_as_object;
    
    int numargs = fn.nargs;
    if (numargs > 0) {
        log_debug("%s: the camera is automatically chosen from gnashrc", __FUNCTION__);
    }
    return as_value(obj.get()); // will keep alive
}

as_value
camera_setmode(const fn_call& fn)
{
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>(fn.this_ptr);
    
    int numargs = fn.nargs;
    switch (numargs) {
        case 4:
            ptr->set_width(fn.arg(0).to_int());
            ptr->set_height(fn.arg(1).to_int());
            ptr->set_fps(fn.arg(2).to_int());
            log_unimpl("Camera_as::setmode argument 4 (favorArea)");
#ifdef USE_GST
                ptr->webcamChangeSourceBin(ptr->getGlobalWebcam());
#endif
            break;
        case 3:
            ptr->set_width(fn.arg(0).to_int());
            ptr->set_height(fn.arg(1).to_int());
            ptr->set_fps(fn.arg(2).to_int());
#ifdef USE_GST
                ptr->webcamChangeSourceBin(ptr->getGlobalWebcam());
#endif
            break;
        case 2:
            ptr->set_width(fn.arg(0).to_int());
            ptr->set_height(fn.arg(1).to_int());
            if (ptr->get_fps() != 15) {
                ptr->set_fps(15);
            }
#ifdef USE_GST
                ptr->webcamChangeSourceBin(ptr->getGlobalWebcam());
#endif
            break;
        case 1:
            ptr->set_width(fn.arg(0).to_int()); //set to the specified width argument
            if (ptr->get_height() != 120) {
                ptr->set_height(120);
            }
            if (ptr->get_fps() != 15) {
                ptr->set_fps(15);
            }
#ifdef USE_GST
                ptr->webcamChangeSourceBin(ptr->getGlobalWebcam());
#endif
            break;
        case 0:
            log_debug("%s: no arguments passed, using default values", __FUNCTION__);
            if (ptr->get_width() != 160) {
                ptr->set_width(160);
            }
            if (ptr->get_height() != 120) {
                ptr->set_height(120);
            }
            if (ptr->get_fps() != 15) {
                ptr->set_fps(15);
            }
#ifdef USE_GST
                ptr->webcamChangeSourceBin(ptr->getGlobalWebcam());
#endif
            break;
    }
    
    return as_value();
}

as_value
camera_setmotionlevel(const fn_call& fn)
{
    log_unimpl ("Camera::motionLevel can be set, but it's not implemented");
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>
        (fn.this_ptr);
    
    int numargs = fn.nargs;
    if (numargs > 2) {
        log_error("%s: Too many arguments", __FUNCTION__);
    } else {
        switch (numargs) {
            case 0:
                log_debug("%s: no args passed, using defaults", __FUNCTION__);
                if (ptr->get_motionLevel() != 50) {
                    ptr->set_motionLevel(50);
                }
                if (ptr->get_motionTimeout() != 2000) {
                    ptr->set_motionTimeout(2000);
                }
                break;
            case 1:
            {
                double argument = fn.arg(0).to_number();
                if ((argument >= 0) && (argument <= 100)) { 
                    ptr->set_motionLevel(argument);
                } else {
                    log_error("%s: bad value passed for first argument", __FUNCTION__);
                    ptr->set_motionLevel(100);
                }
                if (ptr->get_motionTimeout() != 2000) {
                    ptr->set_motionTimeout(2000);
                }
                break;
            }
            case 2:
            {
                double argument1 = fn.arg(0).to_number();
                if ((argument1 >= 0) && (argument1 <= 100)) {
                    ptr->set_motionLevel(argument1);
                } else {
                    log_error("%s: bad value passed for first argument", __FUNCTION__);
                    ptr->set_motionLevel(100);
                }
                ptr->set_motionTimeout(fn.arg(1).to_number());
                break;
            }
        }
    }
    return as_value();
}


as_value
camera_setquality(const fn_call& fn)
{
    log_unimpl ("Camera::quality can be set, but it's not implemented");
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>
        (fn.this_ptr);
    
    int numargs = fn.nargs;
    if (numargs > 2) {
        log_error("%s: Too many arguments", __FUNCTION__);
    } else {
        switch (numargs) {
            case 0:
                log_debug("%s: No arguments passed, using defaults", __FUNCTION__);
                if (ptr->get_bandwidth() != 16384) {
                    ptr->set_bandwidth(16384);
                }
                if (ptr->get_quality() != 0) {
                    ptr->set_quality(0);
                }
                break;
            case 1:
                ptr->set_bandwidth(fn.arg(0).to_number());
                if (ptr->get_quality() != 0) {
                    ptr->set_quality(0);
                }
                break;
            case 2:
            {
                double argument2 = fn.arg(1).to_number();
                ptr->set_bandwidth(fn.arg(0).to_number());
                if ((argument2 >= 0) && (argument2 <= 100)) {
                    ptr->set_quality(fn.arg(1).to_number());
                } else {
                    log_error("%s: Second argument not in range 0-100", __FUNCTION__);
                    ptr->set_quality(100);
                }
                break;
            }
        }
    }
    return as_value();
}


as_value
camera_activitylevel(const fn_call& fn)
{
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
        log_unimpl("Camera::activityLevel only has default value");
        return as_value(ptr->get_activityLevel());
    }
    else // setter
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set activity property of Camera"));
        );
    }

    return as_value();
}

as_value
camera_bandwidth(const fn_call& fn)
{
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
        log_unimpl("Camera::bandwidth only has default value");
        return as_value(ptr->get_bandwidth());
    }
    else // setter
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set bandwidth property of Camera"));
        );
    }

    return as_value();
}

//as3 capitalization
as_value
camera_currentFPS(const fn_call& fn)
{
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
        return as_value(ptr->get_currentFPS());
    }
    else // setter
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set currentFPS property of Camera"));
        );
    }

    return as_value();
}

//as3 capitalization
as_value
camera_currentFps(const fn_call& fn)
{
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
        return as_value(ptr->get_currentFPS());
    }
    else // setter
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set currentFPS property of Camera"));
        );
    }

    return as_value();
}

as_value
camera_fps(const fn_call& fn)
{
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
        return as_value(ptr->get_fps());
    }
    else // setter
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set fps property of Camera"));
        );
    }

    return as_value();
}

as_value
camera_height(const fn_call& fn)
{
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
        return as_value(ptr->get_height());
    }
    else // setter
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set height property of Camera, use setMode"));
        );
    }

    return as_value();
}

as_value
camera_index(const fn_call& fn)
{
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
        //livedocs say that this function should return an integer, but in testing
        //the pp appears to, in practice, return the value as a string
        int value = ptr->get_index();
        char val = value + '0';
        
        std::stringstream ss;
        std::string str;
        ss << val;
        ss >> str;
        as_value name(str);
        name.convert_to_string();
        return (name);
    }
    else // setter
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set index property of Camera"));
        );
    }

    return as_value();
}

as_value
camera_motionLevel(const fn_call& fn)
{
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
        log_unimpl("Camera::motionLevel only has default value");
        return as_value(ptr->get_motionLevel());
    }
    else // setter
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set motionLevel property of Camera"));
        );
    }

    return as_value();
}

as_value
camera_motionTimeout(const fn_call& fn)
{
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>
        (fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
        log_unimpl("Camera::motionTimeout");
        return as_value(ptr->get_motionTimeout());
    }
    else // setter
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set motionTimeout property of Camera"));
        );
    }

    return as_value();
}

as_value
camera_muted(const fn_call& fn) {
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
        log_unimpl("Camera::muted");
        return as_value(ptr->get_muted());
    }
    else // setter
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set muted property of Camera"));
        );
    }

    return as_value();
}

as_value
camera_name(const fn_call& fn)
{
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
        return as_value(ptr->get_name());
    }
    else // setter
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set name property of Camera"));
        );
    }

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

    // TODO: this is a static function, not a member function. Because there
    // is no this pointer, it cannot use camera_as_object to get the
    // names. It will have to query the MediaHandler directly (much of the
    // rest of the code should do this too).
    boost::intrusive_ptr<camera_as_object> ptr =
        ensureType<camera_as_object>(fn.this_ptr);
    
    //transfer from vector to an array
    std::vector<std::string> vect;
    vect = ptr->get_names();
    
    const size_t size = vect.size();
    
    boost::intrusive_ptr<Array_as> data = new Array_as;

    for (size_t i = 0; i < size; ++i) {
        data->push(vect[i]);
    }
    
    return as_value(data.get());
} 


as_value
camera_quality(const fn_call& fn)
{
    boost::intrusive_ptr<camera_as_object> ptr =
        ensureType<camera_as_object>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
        log_unimpl("Camera::quality has only default values");
        return as_value(ptr->get_quality());
    }
    else // setter
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set quality property of Camera"));
        );
    }

    return as_value();
}

as_value
camera_new(const fn_call& fn)
{
    as_object* proto = getCameraInterface();
    Global_as* gl = getGlobal(fn);
    return gl->createObject(proto);
}

as_value
camera_setLoopback(const fn_call& fn)
{
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>
        (fn.this_ptr);
    
    int numargs = fn.nargs;
    if (numargs > 1) {
        log_error("%s: Too many arguments", __FUNCTION__);
    } else {
        ptr->set_loopback(fn.arg(0).to_bool());
    }
    
    return as_value();
}

as_value
camera_setCursor(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl("Camera.setCursor"));
    return as_value();
}

as_value
camera_setKeyFrameInterval(const fn_call& fn)
{
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>
        (fn.this_ptr);
    
    int numargs = fn.nargs;
    if (numargs > 1) {
        log_error("%s: Too many arguments", "Camera.setKeyFrameInterval");
    } else {
        ptr->set_loopback(fn.arg(0).to_int());
    }
    
    return as_value();
}

as_value
camera_width(const fn_call& fn)
{
    boost::intrusive_ptr<camera_as_object> ptr =
        ensureType<camera_as_object>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
        return as_value(ptr->get_width());
    }
    else // setter
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set width property of Camera, use setMode"));
        );
    }

    return as_value();
}


// extern (used by Global.cpp)
void
camera_class_init(as_object& where, const ObjectURI& uri)
{

    Global_as* gl = getGlobal(where);
    
    as_object* proto = getCameraInterface();
    
    // This is going to be the global Camera "class"/"function"
    as_object* cl;

    //for versions lower than 8, the ctor call was get(), for 9 and higher
    //the ctor was getCamera()
    if (isAS3(getVM(where))) {
        cl = gl->createClass(&camera_new, proto);
        attachCameraAS3StaticInterface(*cl);
    } else {
        cl = gl->createClass(&camera_new, proto);
        attachCameraStaticInterface(*cl);
    }
    
    // Register _global.Camera
    where.init_member(getName(uri), cl, as_object::DefaultFlags,
            getNamespace(uri));

}


} // end of gnash namespace

