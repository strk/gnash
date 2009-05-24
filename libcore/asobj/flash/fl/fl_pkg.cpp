// fl_pkg.cpp:  ActionScript top level "fl" package.
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

#include "Object.h" // for getObjectInterface
#include "as_object.h"

#include "string_table.h"
#include "VM.h"

#include "MovieClip.h"

#include "accessibility/fl_accessibility_pkg.h"
#include "containers/containers_pkg.h"
#include "controls/controls_pkg.h"
#include "core/core_pkg.h"
#include "managers/managers_pkg.h"
#include "motion/motion_pkg.h"
#include "transitions/transitions_pkg.h"
#include "video/video_pkg.h"

#include "fl_pkg.h"
#include "flclasses.h"

namespace gnash {

static as_value
get_fl_package(const fn_call& /*fn*/)
{
	log_debug("Loading flash.flash package");
	as_object *pkg = new as_object(getObjectInterface());

	// Call the [objectname]_init() function for each class.
	int i = 0;
	do {
	    asclasses[i](*pkg);
	} while (asclasses[++i] != 0);

	return pkg;
}

void
fl_package_init(as_object& where)
{
	string_table& st = where.getVM().getStringTable();
	where.init_destructive_property(st.find("flash"), get_fl_package);
}


} // end of gnash namespace
