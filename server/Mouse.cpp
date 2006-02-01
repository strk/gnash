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
#include "Mouse.h"

namespace gnash {

Mouse::Mouse() {
}

Mouse::~Mouse() {
}


void
Mouse::addListener()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Mouse::hide()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Mouse::removeListener()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Mouse::show()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void
mouse_new(const fn_call& fn)
{
    mouse_as_object *mouse_obj = new mouse_as_object;

    mouse_obj->set_member("addlistener", &mouse_addlistener);
    mouse_obj->set_member("hide", &mouse_hide);
    mouse_obj->set_member("removelistener", &mouse_removelistener);
    mouse_obj->set_member("show", &mouse_show);

    fn.result->set_as_object_interface(mouse_obj);
}
void mouse_addlistener(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void mouse_hide(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void mouse_removelistener(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void mouse_show(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

} // end of gnaash namespace

