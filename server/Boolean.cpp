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
#include "Boolean.h"

namespace gnash {

Boolean::Boolean() {
}

Boolean::~Boolean() {
}


void
Boolean::toString()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Boolean::valueOf()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void
boolean_new(const fn_call& fn)
{
    boolean_as_object *boolean_obj = new boolean_as_object;

    boolean_obj->set_member("tostring", &boolean_tostring);
    boolean_obj->set_member("valueof", &boolean_valueof);

    fn.result->set_as_object_interface(boolean_obj);
}
void boolean_tostring(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void boolean_valueof(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

} // end of gnaash namespace

