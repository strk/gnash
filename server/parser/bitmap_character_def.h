// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
//

#ifndef GNASH_BITMAP_CHARACTER_DEF_H
#define GNASH_BITMAP_CHARACTER_DEF_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gnash.h" // for bitmap_info definition
#include "character_def.h" // for character_def inheritance
#include "action.h"
#include "types.h"
#include "container.h"
#include "utility.h"
#include "smart_ptr.h"
//#include "movie_interface.h"
#include <cstdarg>
#include <cassert>

namespace gnash {

/// Dirty wrapper around image::{rgb,rgba}
struct image_rgb_or_rgba : public ref_counted
{
	int type; // 0: rgb, 1: rgba
	union {
		image::rgb* rgb;
		image::rgba* rgba;
	};
};

/// Definition of a bitmap character
//
/// This includes:
///
///	- SWF::DEFINEBITS
///	- SWF::DEFINEBITSJPEG2
///	- SWF::DEFINEBITSJPEG3
///	- SWF::DEFINELOSSLESS
///	- SWF::DEFINELOSSLESS2
///
/// The definition currently only takes an image::rgb 
/// or image::rgba pointer. We should probably move
/// the methods for actually reading such tags instead.
///
class bitmap_character_def : public character_def
{

public:

 	bitmap_character_def(image::rgb* image)
 	{
 		assert(image != 0);
		_image.type = 0;
		_image.rgb = image;
 	}

 	bitmap_character_def(image::rgba* image)
 	{
 		assert(image != 0);
		_image.type = 1;
		_image.rgba = image;
 	}

	virtual character* create_character_instance(character* parent,
			int id);

	// Use the renderer to create a bitmap_info from the image
	// information. DO NOT CALL THIS FUNCTION FROM THE PARSER LIB !
	gnash::bitmap_info* get_bitmap_info();

#if 0 // this would be the preferred interface to set a cache from the
      // outside. currently unused
	void set_bitmap_info(smart_ptr<gnash::bitmap_info> bi)
	{
		_bitmap_info = bi;
	}
#endif

private:

	smart_ptr<gnash::bitmap_info> _bitmap_info;

	image_rgb_or_rgba _image;
};



}	// end namespace gnash


#endif // GNASH_BITMAP_CHARACTER_DEF_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
