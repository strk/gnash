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
#include "log.h"
#include "container.h"
#include "utility.h"
#include "smart_ptr.h"
//#include "movie_interface.h"
#include <stdarg.h>

#include <cassert>

namespace gnash {

/// What's this ? An interface ?
struct bitmap_character_def : public character_def
{
    virtual gnash::bitmap_info*	get_bitmap_info() = 0;
};

#if 1
/// Bitmap character
struct bitmap_character : public bitmap_character_def
{
    bitmap_character(bitmap_info* bi)
	:
	m_bitmap_info(bi)
	{
	}

// 		bitmap_character(image::rgb* image)
// 		{
// 			assert(image != 0);

// 			// Create our bitmap info, from our image.
// 			m_bitmap_info = gnash::render::create_bitmap_info_rgb(image);
// 		}

// 		bitmap_character(image::rgba* image)
// 		{
// 			assert(image != 0);

// 			// Create our bitmap info, from our image.
// 			m_bitmap_info = gnash::render::create_bitmap_info_rgba(image);
// 		}

    gnash::bitmap_info*	get_bitmap_info()
	{
	    return m_bitmap_info.get_ptr();
	}

private:
    smart_ptr<gnash::bitmap_info>	m_bitmap_info;
};

#endif


}	// end namespace gnash


#endif // GNASH_BITMAP_CHARACTER_DEF_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
