// text_pkg.cpp:  ActionScript "flash.text" package, for Gnash.
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

#include "flash/text/TextRenderer_as.h"
#include "flash/text/TextFieldAutoSize_as.h"
#include "TextField.h"

namespace gnash {

as_value
get_flash_text_package(const fn_call& /*fn*/)
{
	log_debug("Loading flash.text package");

	as_object* pkg = new as_object(getObjectInterface());

	TextRenderer_class_init(*pkg);
	TextFieldAutoSize_class_init(*pkg);
	textfield_class_init(*pkg);

	return pkg;
}

void
flash_text_package_init(as_object& where)
{
	string_table& st = where.getVM().getStringTable();
	where.init_destructive_property(st.find("text"), get_flash_text_package);
}


} // end of gnash namespace
