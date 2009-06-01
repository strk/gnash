// flash_pkg.cpp:  ActionScript top level "flash" package.
// 
//   Copyright (C)  2009 Free Software Foundation, Inc.
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

#include "string_table.h"
#include "VM.h"

#include "Object.h" // for getObjectInterface
#include "flash_pkg.h"
#include "flashclasses.h"

namespace gnash {

class as_value;
class as_object;

static as_value
get_flash_package(const fn_call& fn)
{
    bool as3 = isAS3(fn.getVM());
	log_debug("Loading %s flash package", as3 ? "AVM2" : "AVM1");
    
    as_object *pkg = new as_object(getObjectInterface());

	// Call the [objectname]_init() function for each class.
	int i = 0;

    if (as3) {
        while (as3classes[i]) {
            as3classes[i](*pkg);
            ++i;
        }
    }
    else {
        while (as2classes[i]) {
            as2classes[i](*pkg);
            ++i;
        }
    }

	return pkg;
}

void
flash_package_init(as_object& where)
{
	string_table& st = where.getVM().getStringTable();
	where.init_destructive_property(st.find("flash"), get_flash_package,
		as_prop_flags::dontEnum | as_prop_flags::onlySWF8Up);
}

}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
