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
#include "gst/VideoInputGst.h"


namespace gnash {

as_value camera_get(const fn_call& fn);
as_value camera_getCamera(const fn_call& fn);
as_value camera_setmode(const fn_call& fn);
as_value camera_setmotionlevel(const fn_call& fn);
as_value camera_setquality(const fn_call& fn);

static void
attachCameraInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    o.init_member("get", gl->createFunction(camera_get));
    o.init_member("getCamera", gl->createFunction(camera_getCamera));
    o.init_member("setmode", gl->createFunction(camera_setmode));
    o.init_member("setmotionlevel", gl->createFunction(camera_setmotionlevel));
    o.init_member("setquality", gl->createFunction(camera_setquality));
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

// AS2 ctor
as_value camera_get(const fn_call& fn) {
    boost::intrusive_ptr<as_object> obj = new camera_as_object;
    
    int numargs = fn.nargs;
    if (numargs > 0) {
        log_debug("%s: the camera is automatically chosen from gnashrc", __FUNCTION__);
    } else {
        return as_value(obj.get()); // will keep alive
    }
}

// AS3 ctor
as_value camera_getCamera(const fn_call& fn) {
    boost::intrusive_ptr<as_object> obj = new camera_as_object;
    
    int numargs = fn.nargs;
    if (numargs > 0) {
        log_debug("%s: the camera is automatically chosen from gnashrc", __FUNCTION__);
    } else {
        return as_value(obj.get()); // will keep alive
    }
}

as_value camera_setmode(const fn_call& fn) {
    //this will need to go through the arguments and set the proper values
    //...which will also mean changing up some things in the VideoInputGst
    //implementation (e.g. be able to set the fps values, etc.)
    boost::intrusive_ptr<camera_as_object> ptr = ensureType<camera_as_object>(fn.this_ptr);
    
    int numargs = fn.nargs;
    if (numargs == 0) {
        log_debug("%s: no arguments passed, using default values", __FUNCTION__);
        if (ptr->getXResolution() != 160) {
            ptr->setXResolution(160);
        }
        if (ptr->getYResolution() != 120) {
            ptr->setYResolution(160);
        }
        if (ptr->getFps() != 15) {
            ptr->setFps(15);
        }
    }
    
    return as_value();
}
as_value camera_setmotionlevel(const fn_call& /*fn*/) {
    log_unimpl (__FUNCTION__);
    return as_value();
}
as_value camera_setquality(const fn_call& /*fn*/) {
    log_unimpl (__FUNCTION__);

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
        if (vm.getSWFVersion() <= 8) {
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

