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
#include "fn_call.h"
#include "MovieClip.h"

#include "AntiAliasType_as.h"
#include "CSMSettings_as.h"
#include "FontStyle_as.h"
#include "FontType_as.h"
#include "Font_as.h"
#include "GridFitType_as.h"
#include "StaticText_as.h"
#include "StyleSheet_as.h"
#include "TextColorType_as.h"
#include "TextDisplayMode_as.h"
#include "TextFieldAutoSize_as.h"
#include "TextFieldType_as.h"
#include "TextField_as.h"
#include "TextFormatAlign_as.h"
#include "TextFormat_as.h"
#include "TextLineMetrics_as.h"
#include "TextRenderer_as.h"
#include "TextSnapshot_as.h"

#include "text_pkg.h"
#include "textclasses.h"

namespace gnash {

static as_value
get_flash_text_package(const fn_call& fn)
{
	log_debug("Loading flash.text package");
    
    as_object *pkg = new as_object(getObjectInterface());

	// Call the [objectname]_init() function for each class.
	int i = 0;

    while (textclasses[i]) {
        textclasses[i](*pkg);
        ++i;
    }

	return pkg;
}

void
flash_text_package_init(as_object& where)
{
	string_table& st = where.getVM().getStringTable();

    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
	where.init_destructive_property(st.find("text"),
			get_flash_text_package, flags);
}


} // end of gnash namespace
