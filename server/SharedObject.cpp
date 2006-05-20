// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "SharedObject.h"
#include "fn_call.h"

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

