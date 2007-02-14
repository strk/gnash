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

	_shapedef = new shape_character_def();

	// Set its boundaries

	_shapedef->set_bound(_framesize);

	// Add the bitmap fill style (fill style 0)

	fill_style bmFill(_bitmap.get());
	line_style line(40, rgba(255, 0, 0, 255));
	_shapedef->add_fill_style(bmFill);
	_shapedef->add_line_style(line);

	// Define a rectangle filled with the bitmap style

	float w = _framesize.width();
	float h = _framesize.height();

	log_msg("Creating a shape_definition wrapping a %f x %f bitmap", w, h);

	path bmPath(w, h, 1, 0, 1);
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
	_framesize(0, 0, image->m_width*20, image->m_height*20),
	_framecount(1),
	_playlist(_framecount),
	_framerate(12),
	_url(url),
	_image(image)
{
	// Do not create shape_character_def now (why?)
}

} // namespace gnash
