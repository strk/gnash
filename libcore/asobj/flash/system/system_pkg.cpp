// system_pkg.cpp:  ActionScript "flash.system" package, for Gnash.
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

#include "ApplicationDomain_as.h"
#include "Capabilities_as.h"
#include "IMEConversionMode_as.h"
#include "IME_as.h"
#include "LoaderContext_as.h"
#include "SecurityDomain_as.h"
#include "SecurityPanel_as.h"
#include "Security_as.h"
#include "System_as.h"

#include "system_pkg.h"
#include "systemclasses.h"

namespace gnash {

static as_value
get_flash_system_package(const fn_call& /*fn*/)
{
	log_debug("Loading flash.system package");
	as_object *pkg = new as_object(getObjectInterface());
        
	// Call the [objectname]_init() function for each class.
	int i = 0;
	while (as3classes[++i] != 0) {
	    as3classes[i](*pkg);
    }
        
	return pkg;
}

void
flash_system_package_init(as_object& where)
{
	log_debug("Initializaing flash.system package");
	string_table& st = where.getVM().getStringTable();
	where.init_destructive_property(st.find("system"), get_flash_system_package);
}


} // end of gnash namespace
