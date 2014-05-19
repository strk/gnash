// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#ifndef GNASH_DUMMYMOVIEDEFINITION_H
#define GNASH_DUMMYMOVIEDEFINITION_H

#include "SWFMovieDefinition.h" // for inheritance
#include "SWFRect.h" // for composition
#include "SWFMovie.h" // for createMovie
#include "Global_as.h"
#include "namedStrings.h"

#include <vector>
#include <string>
#include <memory> // for unique_ptr


namespace gnash
{

/// A dummy movie definition, for use by unit tests
//
/// This class provides implementation of all virtual
/// methods of movie_definition by returning user-defined
/// values for version/size/frame rate etc..
///
/// The createMovie function will return the same
/// object created by createEmptyMovieClip() calls
/// (an empty movieclip... still to be designed)
///
class DummyMovieDefinition : public SWFMovieDefinition
{
	int _version;
	SWFRect _framesize;
	size_t _framecount;
	std::vector<PlayList> _playlist;
	float _framerate;
	std::string _url;

public:


	/// Default constructor
	//
	/// Will be initialized with the following values
	///
	///  - SWF version 6
	///  - 640x480 size
	///  - Single frame (unlabeled)
	///  - 12 FPS
	///  - 0 bytes (for get_bytes_loaded()/get_bytes_total())
	///  - empty url
	///
	DummyMovieDefinition(const RunResources& ri)
		:
        SWFMovieDefinition(ri),
		_version(6),
		_framesize(0, 0, 640*20, 480*20),
		_framecount(1),
		_playlist(_framecount),
		_framerate(12),
		_url("http://www.gnu.org/software/gnash")
	{
	}

	/// Overloaded constructor for specifying target version
	//
	/// This is particularly useful for unit tests.
	/// All but the target version will be initialized
	/// exactly as with the default constructor.
	///
	DummyMovieDefinition(const RunResources& ri, int version)
		:
        SWFMovieDefinition(ri),
		_version(version),
		_framesize(0, 0, 640*20, 480*20),
		_framecount(1),
		_playlist(_framecount),
		_framerate(12),
		_url("http://www.gnu.org/software/gnash")
	{
	}

    virtual bool ensure_frame_loaded(size_t) const {
        return true;
    }

	virtual int	get_version() const {
		return _version;
	}

	virtual size_t get_width_pixels() const {
		return _framesize.width()/20;
	}

	virtual size_t get_height_pixels() const {
		return _framesize.height()/20;
	}

	virtual size_t	get_frame_count() const {
		return _framecount;
	}

	virtual float	get_frame_rate() const {
		return _framerate;
	}

	virtual const SWFRect& get_frame_size() const {
		return _framesize;
	}

	virtual size_t get_bytes_loaded() const {
		return 0;
	}

	virtual size_t get_bytes_total() const {
		return 0;
	}
	
	/// Create a playable movie instance from a def.
	virtual Movie* createMovie(Global_as& gl, DisplayObject* parent=NULL)
	{
        as_object* o = getObjectWithPrototype(gl, NSV::CLASS_MOVIE_CLIP);
		return new SWFMovie(o, this, parent);
	}
	
	virtual const PlayList& get_playlist(size_t frame_number) const
	{
		assert ( frame_number < _playlist.size() );
		return _playlist[frame_number];
	}

	//
	// For use during creation.
	//

	/// Returns 1 based index. Ex: if 1 then 1st frame as been fully loaded
	virtual size_t	get_loading_frame() const  {
		return 1;
	}

	virtual const std::string& get_url() const {
		return _url;
	}

};

} // namespace gnash

#endif // GNASH_DUMMYMOVIEDEFINITION_H
