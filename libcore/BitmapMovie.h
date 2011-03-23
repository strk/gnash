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


#ifndef GNASH_BITMAP_MOVIE_H
#define GNASH_BITMAP_MOVIE_H

#include <string>
#include "BitmapMovieDefinition.h"
#include "Movie.h" // for inheritance

// Forward declarations
namespace gnash {
    class DisplayObject;
}

namespace gnash
{


/// A top-level movie displaying a still bitmap.
//
///  A loaded BitmapMovie is tested in misc-ming.all/loadMovieTest.swf to
///  have a DisplayList, so it is appropriate that it inherits from MovieClip.
class BitmapMovie : public Movie
{

public:

	BitmapMovie(as_object* object, const BitmapMovieDefinition* def,
            DisplayObject* parent); 

	virtual ~BitmapMovie() {}
    
    /// BitmapMovies do need an advance method.
    //
    /// This may be for play() or other inherited methods.
	virtual void advance() { MovieClip::advance(); }

    virtual float frameRate() const {
        return _def->get_frame_rate();
    }

    virtual size_t widthPixels() const {
        return _def->get_width_pixels();
    }

    virtual size_t heightPixels() const {
        return _def->get_height_pixels();
    }

    virtual const std::string& url() const {
        return _def->get_url();
    }

    virtual int version() const {
        return _def->get_version();
    }

    virtual const movie_definition* definition() const {
        return _def;
    }
	
private:
	
    const BitmapMovieDefinition* const _def;

};

} // end of namespace gnash

#endif // GNASH_BITMAPMOVIEINSTANCE_H
