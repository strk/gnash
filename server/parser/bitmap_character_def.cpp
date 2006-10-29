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
#include <memory> // for auto_ptr

using namespace std;

namespace gnash {

bitmap_character_def::bitmap_character_def(std::auto_ptr<image::rgb> image)
	:
 	_bitmap_info ( gnash::render::create_bitmap_info_rgb(image.get()) )
{
}

bitmap_character_def::bitmap_character_def(std::auto_ptr<image::rgba> image)
	:
 	_bitmap_info ( gnash::render::create_bitmap_info_rgba(image.get()) )
{
}


} // namespace gnash
