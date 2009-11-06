// BitmapMovieDefinition.cpp:  Bitmap movie definition, for Gnash.
// 
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "smart_ptr.h" // GNASH_USE_GC
#include "BitmapMovie.h"
#include "BitmapMovieDefinition.h"
#include "Geometry.h" // for class path and class edge
#include "GnashImage.h"
#include "log.h"
#include "Bitmap.h"
#include "Renderer.h"
#include "Global_as.h"
#include "namedStrings.h"
#include "GnashException.h"
#include <boost/shared_ptr.hpp>
#include <sstream>

namespace gnash {

Movie*
BitmapMovieDefinition::createMovie(Global_as& gl, DisplayObject* parent)
{
    as_object* o = getObjectWithPrototype(gl, NSV::CLASS_MOVIE_CLIP);
    return new BitmapMovie(o, this, parent);
}

BitmapMovieDefinition::BitmapMovieDefinition(
        boost::shared_ptr<IOChannel> imageData,
		Renderer* renderer, FileType type, const std::string& url)
	:
	_version(6),
	_framesize(0,0,512*20,512*20), // arbitrary default size (will change)
	_framecount(1),
	_framerate(12),
	_url(url),
	_bytesTotal(0),
	_bitmap(0),
    _inputStream(imageData),
    _inputFileType(type),
    _renderer(renderer)
{
}

BitmapMovieDefinition::BitmapMovieDefinition(std::auto_ptr<GnashImage> image,
		Renderer* renderer, const std::string& url)
	:
	_version(6),
	_framesize(0, 0, image->width()*20, image->height()*20),
	_framecount(1),
	_framerate(12),
	_url(url),
	_bytesTotal(image->size()),
	_bitmap(renderer ? renderer->createBitmapInfo(image) : 0)
{
}

DisplayObject*
BitmapMovieDefinition::createDisplayObject(Global_as& /*gl*/,
        DisplayObject* /*parent*/) const
{
    std::abort();
    return 0;
}

bool
BitmapMovieDefinition::moreToLoad() 
{
    // TODO: use a thread for background loading here

    // no more to load if we don't have an input stream
    if ( ! _inputStream ) return false;
    // ..... or it's in bad state .....
    if ( _inputStream->bad() ) return false;
    // ..... or it's over .....
    if ( _inputStream->eof() ) return false;

    // This one blocks... (and may throw ?)
    log_debug("BitmapMovieDefinition starting image loading");
    std::auto_ptr<GnashImage> im(
            ImageInput::readImageData(_inputStream, _inputFileType));

    log_debug("BitmapMovieDefinition finished image loading");
    _inputStream.reset(); // we don't need this anymore

    if (!im.get()) {
        std::stringstream ss;
        ss << _("Can't read image file from") << _url;
        throw ParserException(ss.str());
    }

	_framesize.set_to_rect(0, 0, im->width()*20, im->height()*20);
	_bytesTotal = im->size();

    if ( _renderer ) _bitmap = _renderer->createBitmapInfo(im);

    return false;
}

#ifdef GNASH_USE_GC
void
BitmapMovieDefinition::markReachableResources() const
{
	if (_bitmap.get()) _bitmap->setReachable();
}
#endif // GNASH_USE_GC

} // namespace gnash
