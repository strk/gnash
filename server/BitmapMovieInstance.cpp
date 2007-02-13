// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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


#include "BitmapMovieInstance.h"
#include "BitmapMovieDefinition.h"
#include "render.h" // for ::display

using namespace std;

namespace gnash {

BitmapMovieInstance::BitmapMovieInstance(BitmapMovieDefinition* def,
		character* parent)
	:
	movie_instance(def, parent),
	_bitmap(def->get_bitmap_char_def())
{
	// TODO: 
	//   - Define a rectangle fill shape using the bitmap as a fill
	//   - add the shape to the display list 

}

void
BitmapMovieInstance::display()
{
	// TODO: when things in the constructor are done we can
	//       completely avoid overriding ::display
	//       and things should automatically work
	//       (use sprite_instance::display)
	//
	log_error("FIXME: display of BitmapMovieInstance unimplemented !");

	clear_invalidated(); // clear_invalidated is just needed so display()
	                     // is not called over and over - to reduce verbosity
			     // of the FIXME error above.

	do_display_callback();
}

} // namespace gnash
