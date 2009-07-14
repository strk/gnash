// display_pkg.cpp:  ActionScript "flash.display" package, for Gnash.
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
#include "display/AVM1Movie_as.h"
#include "display/ActionScriptVersion_as.h"
#include "display/BitmapDataChannel_as.h"
#include "display/BitmapData_as.h"
#include "display/Bitmap_as.h"
#include "display/BlendMode_as.h"
#include "display/CapsStyle_as.h"
#include "display/DisplayObjectContainer_as.h"
#include "display/DisplayObject_as.h"
#include "display/FrameLabel_as.h"
#include "display/GradientType_as.h"
#include "display/Graphics_as.h"
#include "display/IBitmapDrawable_as.h"
#include "display/InteractiveObject_as.h"
#include "display/InterpolationMethod_as.h"
#include "display/JointStyle_as.h"
#include "display/LineScaleMode_as.h"
#include "display/LoaderInfo_as.h"
#include "display/Loader_as.h"
#include "display/MorphShape_as.h"
#include "display/MovieClip_as.h"
#include "display/PixelSnapping_as.h"
#include "display/SWFVersion_as.h"
#include "display/Scene_as.h"
#include "display/Shape_as.h"
#include "display/SimpleButton_as.h"
#include "display/SpreadMethod_as.h"
#include "display/Sprite_as.h"
#include "display/StageAlign_as.h"
#include "display/StageDisplayState_as.h"
#include "display/StageQuality_as.h"
#include "display/StageScaleMode_as.h"
#include "display/Stage_as.h"

#include "display_pkg.h"
#include "displayclasses.h"

namespace gnash {

static as_value
get_flash_display_package(const fn_call& fn)
{
	log_debug("Loading flash.display package");

    as_object *pkg = new as_object(getObjectInterface());

	// Call the [objectname]_init() function for each class.
	int i = 0;

    while (displayclasses[i]) {
        displayclasses[i](*pkg);
        ++i;
    }

	return pkg;
}

void
flash_display_package_init(as_object& where)
{
	string_table& st = getStringTable(where);

    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
	where.init_destructive_property(st.find("display"),
			get_flash_display_package, flags);
}


} // end of gnash namespace
