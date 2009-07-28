// EventDispatcher.cpp:  Implementation of ActionScript TextFieldAutoSize class, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#include "smart_ptr.h"
#include "fn_call.h"
#include "Global_as.h"
#include "as_object.h" // for inheritance
#include "builtin_function.h" // need builtin_function
#include "Object.h"
#include "log.h"

/// TextFieldAutoSize is an AVM2-only class

namespace gnash {

// Forward declarations   
namespace {
    void attachTextFieldAutoSizeInterface(as_object& o);
}

// extern
void
textfieldautosize_class_init(as_object& where, const ObjectURI& uri)
{
    static boost::intrusive_ptr<as_object> obj =
    Global_as* gl = getGlobal(global);
    as_object* proto = getObjectInterface();
        gl->createObject(proto);

    attachTextFieldAutoSizeInterface(*obj);
	where.init_member(getName(uri), obj.get(), as_object::DefaultFlags,
            getNamespace(uri));
}

namespace {

void
attachTextFieldAutoSizeInterface(as_object& o)
{
	o.init_member("CENTER", as_value("center"));
	o.init_member("LEFT", as_value("left"));
	o.init_member("RIGHT", as_value("right"));
	o.init_member("NONE", as_value("none"));
}

} // anonymous namespace
} // gnash namespace
