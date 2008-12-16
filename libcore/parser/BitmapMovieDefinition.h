// 
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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

#include "smart_ptr.h" // GNASH_USE_GC
#include "movie_definition.h" // for inheritance
#include "rect.h" // for composition
#include "BitmapMovieInstance.h" // for create_movie_instance
#include "BitmapInfo.h" // for destructor visibility by intrusive_ptr
#include "DynamicShape.h" // for destructor visibility by intrusive_ptr
#include "GnashImage.h"

#include <string>
#include <memory> // for auto_ptr

// Forward declarations

namespace gnash
{

/// A "movie" definition for a bitmap file
//
/// The create_movie_instance function will return a BitmapMovieInstance
///
class BitmapMovieDefinition : public movie_definition
{
	int _version;
	rect _framesize;
	size_t _framecount;
	float _framerate;
	std::string _url;

	std::auto_ptr<GnashImage> _image;

	boost::intrusive_ptr<BitmapInfo> _bitmap;

	boost::intrusive_ptr<DynamicShape> _shapedef;

	/// Get the shape character definition for this bitmap movie
	//
	/// It will create the definition the first time it's called
	///
	shape_character_def* getShapeDef();

	size_t _bytesTotal;

protected:

#ifdef GNASH_USE_GC
	/// Mark reachable resources of a BitmapMovieDefinition
	//
	/// Reachable resources are:
	///	- dynamic shape (_shapedef)
	///	- bitmap (_bitmap)
	///
	void markReachableResources() const;
#endif // GNASH_USE_GC

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
	BitmapMovieDefinition(std::auto_ptr<GnashImage> image, const std::string& url);

	// Discard id, always return the only shape character we have 
	virtual character_def* get_character_def(int /*id*/)
	{
		return getShapeDef();
	}

	virtual int	get_version() const {
		return _version;
	}

	virtual float get_width_pixels() const {
		return std::ceil(TWIPS_TO_PIXELS(_framesize.width()));
	}

	virtual float get_height_pixels() const {
		return std::ceil(TWIPS_TO_PIXELS(_framesize.height()));
	}

	virtual size_t	get_frame_count() const {
		return _framecount;
	}

	virtual float	get_frame_rate() const {
		return _framerate;
	}

	virtual const rect& get_frame_size() const {
		return _framesize;
	}

	virtual const rect& get_bound() const {
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
	
	/// Create a playable movie_instance from this def.
	virtual movie_instance* create_movie_instance(character* parent=0)
	{
		return new BitmapMovieInstance(this, parent);
	}

	virtual const std::string& get_url() const {
		return _url;
	}

	// Inheritance from movie_definition requires this.
	// we always return 1 so MovieClip::stagePlacementCallback
	// doesn't skip our handling (TODO: check if it's correct to
	// skip handling of 0-frames movies..).
	size_t  get_loading_frame() const 
	{
		return 1;
	}
};

} // namespace gnash

#endif // GNASH_BITMAPMOVIEDEFINITION_H
