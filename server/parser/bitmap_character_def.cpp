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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// This needs to be included first for NetBSD systems or we get a weird
// problem with pthread_t being defined too many times if we use any
// STL containers.
#ifdef HAVE_PTHREADS
#include <pthread.h>
#endif

#include "bitmap_character_def.h"
//#include "bitmap_character_instance.h"
#include "gnash.h" // for bitmap_info
#include "render.h"

#include <vector>
#include <string>
#include <cassert>

using namespace std;

namespace gnash {

gnash::bitmap_info* 
bitmap_character_def::get_bitmap_info()
{
	if ( ! _bitmap_info.get_ptr() )
	{
 		// Create our bitmap info, from our image.
		if ( _image.type == 0 )
		{
 			_bitmap_info = gnash::render::create_bitmap_info_rgb(_image.rgb);
		}
		else
		{
 			_bitmap_info = gnash::render::create_bitmap_info_rgba(_image.rgba);
		}
	}
	assert(_bitmap_info.get_ptr());
	return _bitmap_info.get_ptr();
}

#if 0
character*
bitmap_character_def::create_character_instance(character* parent, int id)
{
	bitmap_character_instance* instance = new bitmap_character_instance(
		this, parent, -1);
	return instance;
}
#endif

} // namespace gnash
