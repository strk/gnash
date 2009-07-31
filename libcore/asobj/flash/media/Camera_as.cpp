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
//as_value camera_names(const fn_call& fn);
as_value camera_quality(const fn_call& fn);
as_value camera_width(const fn_call& fn);

static void
attachCameraInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    boost::intrusive_ptr<builtin_function> getset;
    
    o.init_member("get", gl->createFunction(camera_get));
    o.init_member("getCamera", gl->createFunction(camera_getCamera));
    o.init_member("setmode", gl->createFunction(camera_setmode));
    o.init_member("setmotionlevel", gl->createFunction(camera_setmotionlevel));
    o.init_member("setquality", gl->createFunction(camera_setquality));


    getset = gl->createFunction(camera_activitylevel);
    o.init_property("activityLevel", *getset, *getset);
    getset = gl->createFunction(camera_bandwidth);
    o.init_property("bandwidth", *getset, *getset);
    getset = gl->createFunction(camera_currentFPS);
    o.init_property("currentFPS", *getset, *getset);
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
    //getset = gl->createFunction(camera_names);  //need to figure out how to
    //o.init_property("names", *getset, *getset); //implement this
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
class camera_as_object: public as_object, public media::gst::VideoInputGst
{

public:

    camera_as_object()
        :
        as_object(getCameraInterface())
    {}

    // override from as_object ?
    //const char* get_text_value() const { return "Camera"; }

    // override from as_object ?
    //double get_numeric_value() const { return 0; }
};
#endif

#ifdef USE_FFMPEG
class camera_as_object: public as_object, public media::VideoInput
{

public:

    camera_as_object()
        :
        as_object(getCameraInterface())
    {}

    // override from as_object ?
    //const char* get_text_value() const { return "Camera"; }

    // override from as_object ?
    //double get_numeric_value() const { return 0; }
};
#endif

// AS2 ctor
as_value
camera_get(const fn_call& fn) {
    boost::intrusive_ptr<as_object> obj = new camera_as_object;  

    int numargs = fn.nargs;
    if (numargs > 0) {
        log_debug("%s: the camera is automatically chosen from gnashrc", __FUNCTION__);
    }
    return as_value(obj.get()); // will keep alive
}

// AS3 ctor
as_value
camera_getCamera(const fn_call& fn) {
    boost::intrusive_ptr<as_object> obj = new camera_as_object;
    
    int numargs = fn.nargs;
    if (numargs > 0) {
        log_debug("%s: the camera is automatically chosen from gnashrc", __FUNCTION__);
    }
    return as_value(obj.get()); // will keep alive
}

as_value
camera_setmode(const fn_call& fn) {
    
    //this will need to go through the arguments and set the proper values
    //...which will also mean changing up some things in the VideoInputGst
    //implementation (e.g. be able to set the fps values, etc.)
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
camera_setmotionlevel(const fn_call& /*fn*/) {
    log_unimpl (__FUNCTION__);
    return as_value();
}
as_value
camera_setquality(const fn_call& /*fn*/) {
    log_unimpl (__FUNCTION__);

    return as_value();
}
as_value
camera_activitylevel(const fn_call& fn) {
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
        log_unimpl("Camera::activityLevel only has default value");
#ifdef USE_GST
        return as_value(ptr->get_activityLevel());
#endif
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
camera_bandwidth(const fn_call& fn) {
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
        log_unimpl("Camera::bandwidth only has default value");
#ifdef USE_GST
        return as_value(ptr->get_bandwidth());
#endif
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
camera_currentFPS(const fn_call& fn) {
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
#ifdef USE_GST
        return as_value(ptr->get_currentFPS());
#endif
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
camera_currentFps(const fn_call& fn) {
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
#ifdef USE_GST
        return as_value(ptr->get_currentFPS());
#endif
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
camera_fps(const fn_call& fn) {
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
#ifdef USE_GST
        return as_value(ptr->get_fps());
#endif
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
camera_height(const fn_call& fn) {
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
#ifdef USE_GST
        return as_value(ptr->get_height());
#endif
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
camera_index(const fn_call& fn) {
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
#ifdef USE_GST
        return as_value(ptr->get_index());
#endif
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
camera_motionLevel(const fn_call& fn) {
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
        log_unimpl("Camera::motionLevel only has default value");
#ifdef USE_GST
        return as_value(ptr->get_motionLevel());
#endif
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
camera_motionTimeout(const fn_call& fn) {
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
        log_unimpl("Camera::motionTimeout");
#ifdef USE_GST
        return as_value(ptr->get_motionTimeout());
#endif
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
#ifdef USE_GST
        return as_value(ptr->get_muted());
#endif
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
camera_name(const fn_call& fn) {
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
#ifdef USE_GST
        return as_value(ptr->get_name());
#endif
    }
    else // setter
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set name property of Camera"));
        );
    }

    return as_value();
}

//can gnash return a static array as an as_value(array)?
/*
as_value
camera_names(const fn_call& fn) {
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>(fn.this_ptr);
    
    //transfer from vector to an array
    std::vector<std::string> vect;
    vect = ptr->get_names();
    
    int size = vect.size();
    std::string data[size];
    int i;
    for (i = 0; i < size; ++i) {
        data[i] = vect[i];
    }
    
    if ( fn.nargs == 0 ) // getter
    {
        log_trace("holld.....");
        return as_value(data);
    }
    else // setter
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Attempt to set names property of Camera"));
        );
    }

    return as_value();
} 
*/

as_value
camera_quality(const fn_call& fn) {
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
        log_unimpl("Camera::quality has only default values");
#ifdef USE_GST
        return as_value(ptr->get_quality());
#endif
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
camera_width(const fn_call& fn) {
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>(fn.this_ptr);

    if ( fn.nargs == 0 ) // getter
    {
#ifdef USE_GST
        return as_value(ptr->get_width());
#endif
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
void camera_class_init(as_object& where, const ObjectURI& uri)
{
    // This is going to be the global Camera "class"/"function"
    static boost::intrusive_ptr<as_object> cl;

    if ( cl == NULL )
    {
        VM& vm = getVM(where);
        Global_as* gl = getGlobal(where);
        
        //for versions lower than 8, the ctor call was get(), for 9 and higher
        //the ctor was getCamera()
        if (isAS3(getVM(where))) {
            cl = gl->createClass(&camera_get, getCameraInterface());
        } else {
            cl = gl->createClass(&camera_getCamera, getCameraInterface());
        }
        
        attachCameraInterface(*cl);
    }

    // Register _global.Camera
    where.init_member(getName(uri), cl.get(), as_object::DefaultFlags,
            getNamespace(uri));

}


} // end of gnash namespace

