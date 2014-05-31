// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011. 2012
//   Free Software Foundation, Inc.
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

#ifndef GNASH_BITMAPMOVIEDEFINITION_H
#define GNASH_BITMAPMOVIEDEFINITION_H

#include "movie_definition.h" // for inheritance
#include "SWFRect.h" 
#include "GnashNumeric.h"

#include <boost/intrusive_ptr.hpp>
#include <string>
#include <memory> 

// Forward declarations
namespace gnash {
    class Renderer;
    namespace image {
        class GnashImage;
    }
}

namespace gnash
{

/// A "movie" definition for a bitmap file
//
/// The createMovie function will return a BitmapMovie
///
class BitmapMovieDefinition : public movie_definition
{
public:


	/// Construct a BitmapMovieDefinition for the given image (rgb)
	//
	/// Will be initialized with the following values
	///
	///  - SWF version 6
	///  - Framesize extracted from image 
	///  - Single frame (unlabeled)
	///  - 12 FPS
	///  - image->size() bytes (for get_bytes_loaded()/get_bytes_total())
	///  - provided url
	///
	BitmapMovieDefinition(std::unique_ptr<image::GnashImage> image,
            Renderer* renderer, std::string url);

    virtual DisplayObject* createDisplayObject(Global_as&, DisplayObject*)
        const;

	virtual int	get_version() const {
		return _version;
	}

	virtual size_t get_width_pixels() const {
		return std::ceil(twipsToPixels(_framesize.width()));
	}

	virtual size_t get_height_pixels() const {
		return std::ceil(twipsToPixels(_framesize.height()));
	}

	virtual size_t get_frame_count() const {
		return _framecount;
	}

	virtual float get_frame_rate() const {
		return _framerate;
	}

	virtual const SWFRect& get_frame_size() const {
		return _framesize;
	}

	/// Return number of bytes loaded
	//
	/// Since no progressive load is implemented yet
	/// we'll always return total bytes here..
	///
	virtual size_t get_bytes_loaded() const {
		return get_bytes_total();
	}

	/// Return total number of bytes which composed this movie
	//
	/// We actually cheat, and return the image size here...
	///
	virtual size_t get_bytes_total() const {
		return _bytesTotal;
	}
	
	/// Create a playable Movie from this def.
	virtual Movie* createMovie(Global_as& gl, DisplayObject* parent = nullptr);

	virtual const std::string& get_url() const {
		return _url;
	}

	// Inheritance from movie_definition requires this.
	// we always return 1 so MovieClip::construct()
	// doesn't skip our handling (TODO: check if it's correct to
	// skip handling of 0-frames movies..).
	size_t  get_loading_frame() const 
	{
		return 1;
	}

    const CachedBitmap* bitmap() const {
        return _bitmap.get();
    }

protected:

private:

	int _version;
	SWFRect _framesize;
	size_t _framecount;
	float _framerate;
	std::string _url;

	size_t _bytesTotal;

    boost::intrusive_ptr<CachedBitmap> _bitmap;
};

} // namespace gnash

#endif // GNASH_BITMAPMOVIEDEFINITION_H
