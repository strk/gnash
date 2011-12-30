// desktop_pkg.cpp:  ActionScript "flash.desktop" package, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "as_object.h"
#include "VM.h"
#include "VM.h"
#include "fn_call.h"
#include "ExternalInterface_as.h"
#include "namedStrings.h"
#include "external_pkg.h"
#include "Global_as.h"

namespace gnash {

namespace {

as_value
get_flash_external_package(const fn_call& fn)
{

    log_debug(_("Loading flash.external package"));

    Global_as& gl = getGlobal(fn);

    as_object* pkg = createObject(gl);
    
    VM& vm = getVM(fn);

    externalinterface_class_init(*pkg, getURI(vm, "ExternalInterface"));

    return pkg;
}

}

void
flash_external_package_init(as_object& where, const ObjectURI& uri)
{
    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
    where.init_destructive_property(uri, get_flash_external_package, flags);
}


} // end of gnash namespace
