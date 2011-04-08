// BitmapMovieDefinition.cpp:  Bitmap movie definition, for Gnash.
// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

#include "BitmapMovie.h"
#include "BitmapMovieDefinition.h"
#include "Geometry.h" // for class path and class edge
#include "GnashImage.h"
#include "log.h"
#include "Bitmap.h"
#include "Renderer.h"
#include "Global_as.h"
#include "namedStrings.h"
#include "CachedBitmap.h"

namespace gnash {

Movie*
BitmapMovieDefinition::createMovie(Global_as& gl, DisplayObject* parent)
{
    as_object* o = getObjectWithPrototype(gl, NSV::CLASS_MOVIE_CLIP);
    return new BitmapMovie(o, this, parent);
}

BitmapMovieDefinition::BitmapMovieDefinition(
        std::auto_ptr<image::GnashImage> image,
		Renderer* renderer, const std::string& url)
	:
	_version(6),
	_framesize(0, 0, image->width()*20, image->height()*20),
	_framecount(1),
	_framerate(12),
	_url(url),
	_bytesTotal(image->size()),
	_bitmap(renderer ? renderer->createCachedBitmap(image) : 0)
{
}

DisplayObject*
BitmapMovieDefinition::createDisplayObject(Global_as& /*gl*/,
        DisplayObject* /*parent*/) const
{
    std::abort();
    return 0;
}

} // namespace gnash
