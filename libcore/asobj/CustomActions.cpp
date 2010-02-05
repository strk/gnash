// CustomActions.cpp:  ActionScript CustomActions class, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#include "CustomActions.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "Global_as.h"

namespace gnash {

namespace {

    as_value customactions_get(const fn_call& fn);
    as_value customactions_install(const fn_call& fn);
    as_value customactions_list(const fn_call& fn);
    as_value customactions_uninstall(const fn_call& fn);
    as_value customactions_ctor(const fn_call& fn);
    void attachCustomActionsInterface(as_object& o);

}

// extern (used by Global.cpp)
void 
customactions_class_init(as_object& where, const ObjectURI& uri)
{
    registerBuiltinObject(where, attachCustomActionsInterface, uri);
}


namespace {

void
attachCustomActionsInterface(as_object& o)
{
    Global_as& gl = getGlobal(o);
	o.init_member("get", gl.createFunction(customactions_get));
	o.init_member("install", gl.createFunction(customactions_install));
	o.init_member("list", gl.createFunction(customactions_list));
	o.init_member("uninstall", gl.createFunction(customactions_uninstall));
}

as_value
customactions_get(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
customactions_install(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
customactions_list(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
customactions_uninstall(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

}

} // end of gnash namespace

