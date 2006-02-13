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
#include "SharedObject.h"

namespace gnash {

SharedObject::SharedObject() {
}

SharedObject::~SharedObject() {
}


void
SharedObject::clear()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
SharedObject::flush()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
SharedObject::getLocal()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
SharedObject::getSize()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void
sharedobject_new(const fn_call& fn)
{
    sharedobject_as_object *sharedobject_obj = new sharedobject_as_object;

    sharedobject_obj->set_member("clear", &sharedobject_clear);
    sharedobject_obj->set_member("flush", &sharedobject_flush);
    sharedobject_obj->set_member("getlocal", &sharedobject_getlocal);
    sharedobject_obj->set_member("getsize", &sharedobject_getsize);

    fn.result->set_as_object(sharedobject_obj);
}
void sharedobject_clear(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void sharedobject_flush(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void sharedobject_getlocal(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void sharedobject_getsize(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

} // end of gnaash namespace

