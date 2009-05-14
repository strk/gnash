// flash_pkg.cpp:  ActionScript "flash" package, for Gnash.
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

#include "log.h"
#include "VM.h" // for getPlayerVersion() 
#include "Object.h" // for getObjectInterface
#include "as_object.h"

#include "flash/display_pkg.h"
#include "flash/external_pkg.h"
#include "flash/filters_pkg.h"
#include "flash/geom_pkg.h"
#include "flash/net_pkg.h"
#include "flash/text_pkg.h"

namespace gnash {

static as_value
get_flash_package(const fn_call& /*fn*/)
{
	log_debug("Loading flash package");

	as_object* pkg = new as_object(getObjectInterface());

	// sub-packages:
	// TODO: use a destructive getter-setter for these 
	flash_display_package_init(*pkg);
	flash_external_package_init(*pkg);
	flash_filters_package_init(*pkg);
	flash_geom_package_init(*pkg);
	flash_net_package_init(*pkg);
	flash_text_package_init(*pkg);
	return pkg;
}

void
flash_package_init(as_object& global)
{
	string_table& st = global.getVM().getStringTable();
	global.init_destructive_property(st.find("flash"), get_flash_package,
		as_prop_flags::dontEnum|as_prop_flags::onlySWF8Up);
}

} // end of gnash namespace
