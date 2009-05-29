// media_pkg.cpp:  ActionScript "flash.media" package, for Gnash.
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

#include "Camera_as.h"
#include "ID3Info_as.h"
#include "Microphone_as.h"
#include "SoundChannel_as.h"
#include "SoundLoaderContext_as.h"
#include "SoundMixer_as.h"
#include "SoundTransform_as.h"
#include "Sound_as.h"
#include "Video_as.h"

#include "media_pkg.h"
#include "mediaclasses.h"

namespace gnash {

static as_value
get_flash_media_package(const fn_call& fn)
{
    // This package is AS3 only!
    assert(isAS3(fn.getVM()));

	log_debug("Loading AVM2 flash.media package");
	as_object *pkg = new as_object(getObjectInterface());

	// Call the [objectname]_init() function for each class.
	int i = 0;
	while (as3classes[i]) {
	    as3classes[i](*pkg);
        ++i;
    }

	return pkg;
}

void
flash_media_package_init(as_object& where)
{
	string_table& st = where.getVM().getStringTable();
	where.init_destructive_property(st.find("media"), get_flash_media_package);
}


} // end of gnash namespace
