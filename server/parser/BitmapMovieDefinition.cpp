// BitmapMovieDefinition.cpp:  Bitmap movie definition, for Gnash.
// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "BitmapMovieInstance.h"
#include "BitmapMovieDefinition.h"
#include "fill_style.h"
#include "shape.h" // for class path and class edge
#include "render.h" // for ::display

using namespace std;

namespace gnash {


shape_character_def*
BitmapMovieDefinition::getShapeDef()
{
	if ( _shapedef ) return _shapedef.get();

	_bitmap = new bitmap_character_def(_image);

	// Create the shape definition

	_shapedef = new DynamicShape();

	// Set its boundaries

	_shapedef->set_bound(_framesize);

	// Add the bitmap fill style (fill style 0)

	fill_style bmFill(_bitmap.get());
	size_t fillLeft = _shapedef->add_fill_style(bmFill);

	// Define a rectangle filled with the bitmap style

	// We use one twip for each pixel in the image
	// The character will be scaled * 20
	// when placed in BitmapMovieInstance's DisplayList
	float w = _framesize.width()/20;
	float h = _framesize.height()/20;

	log_msg(_("Creating a shape_definition wrapping a %g x %g bitmap"), w, h);

	path bmPath(w, h, fillLeft, 0, 0);
	bmPath.drawLineTo(w, 0);
	bmPath.drawLineTo(0, 0);
	bmPath.drawLineTo(0, h);
	bmPath.drawLineTo(w, h);

	// Add the path 

	_shapedef->add_path(bmPath);

	return _shapedef.get();
}

BitmapMovieDefinition::BitmapMovieDefinition(
		std::auto_ptr<image::rgb> image,
		const std::string& url)
	:
	_version(6),
	// image::rgb size is in pixels
	_framesize(0, 0, image->width()*20, image->height()*20),
	_framecount(1),
	_playlist(_framecount),
	_framerate(12),
	_url(url),
	_image(image)
{
	// Do not create shape_character_def now (why?)
}

#ifdef GNASH_USE_GC
void
BitmapMovieDefinition::markReachableResources() const
{
	if ( _shapedef.get() ) _shapedef->setReachable();
	if ( _bitmap.get() ) _bitmap->setReachable();
}
#endif // GNASH_USE_GC

} // namespace gnash
