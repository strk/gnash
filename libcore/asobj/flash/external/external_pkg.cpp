// desktop_pkg.cpp:  ActionScript "flash.desktop" package, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "Object.h" // for getObjectInterface
#include "as_object.h"

#include "string_table.h"
#include "VM.h"
#include "MovieClip.h"

#include "ExternalInterface_as.h"

#include "external_pkg.h"
#include "externalclasses.h"

namespace gnash {

static as_value
get_flash_external_package(const fn_call& fn)
{
    const bool as3 = isAS3(fn.getVM());
    // This package is identical for AS2 and AS3 (as far as we know)
    log_debug("Loading %s flash.external package", as3 ? "AVM2" : "AVM1");
    
    as_object *pkg = new as_object(getObjectInterface());

    // Call the [objectname]_init() function for each class.
    int i = 0;
    while (as3externalclasses[i]) {
        as3externalclasses[i](*pkg);
        ++i;
    } 

    return pkg;
}

void
flash_external_package_init(as_object& where)
{
    string_table& st = where.getVM().getStringTable();

    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
    where.init_destructive_property(st.find("external"),
            get_flash_external_package, flags);
}


} // end of gnash namespace
