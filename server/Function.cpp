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
#include "Function.h"

namespace gnash {

Function::Function() {
}

Function::~Function() {
}


void
Function::apply()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Function::call()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void
function_new(const fn_call& fn)
{
    function_as_object *function_obj = new function_as_object;

    function_obj->set_member("apply", &function_apply);
    function_obj->set_member("call", &function_call);

    fn.result->set_as_object_interface(function_obj);
}
void function_apply(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void function_call(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

} // end of gnaash namespace

