// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "Camera.h"

namespace gnash {

Camera::Camera() {
}

Camera::~Camera() {
}


void
Camera::get()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Camera::setMode()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Camera::setMotionLevel()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Camera::setQuality()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void
camera_new(const fn_call& fn)
{
    camera_as_object *camera_obj = new camera_as_object;

    camera_obj->set_member("get", &camera_get);
    camera_obj->set_member("setmode", &camera_setmode);
    camera_obj->set_member("setmotionlevel", &camera_setmotionlevel);
    camera_obj->set_member("setquality", &camera_setquality);

    fn.result->set_as_object_interface(camera_obj);
}
void camera_get(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void camera_setmode(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void camera_setmotionlevel(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void camera_setquality(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

} // end of gnaash namespace

