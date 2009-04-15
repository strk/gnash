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


#ifndef GNASH_BITMAP_MOVIE_H
#define GNASH_BITMAP_MOVIE_H

#include "BitmapMovieDefinition.h"
#include "Movie.h" // for inheritance

// Forward declarations
namespace gnash
{
    class GnashImage;
    class DisplayObject;
}

namespace gnash
{


/// A top-level movie displaying a still bitmap.
//
/// It is uncomfortable that this inherits from MovieClip, as it
/// has no DisplayList, no actions, and no frames. But its sister class
/// SWFMovie is a kind of MovieClip (or Sprite), so this arrangement
/// is necessary for now.
//
/// It is a mouse entity, so can receive mouse events, and should also be
/// draggable.
//
/// The class should override all MovieClip methods that rely on the
/// DisplayList: currently point tests and display().
class BitmapMovie : public Movie
{

public:

	BitmapMovie(const BitmapMovieDefinition* const def, DisplayObject* parent); 

	virtual ~BitmapMovie() {}
    
    /// Return true if the point is within our bounds
    //
    /// MovieClip traverses the DisplayList for this.
    bool pointInVisibleShape(boost::int32_t x, boost::int32_t y) const {
        if (!visible()) return false;
        return pointInBounds(x, y);
    }

    /// This is a no-op for a BitmapMovie, as it never changes.
	virtual void advance() { }

    /// Return the bounds of this BitmapMovie.
    //
    /// MovieClip uses the DisplayList to calculate this. For us it is
    /// simpler.
    virtual rect getBounds() const {
        return _def->get_frame_size();
    }
 
    /// Render the BitmapMovie.
    //
    /// This draws the bitmap directly using the BitmapMovie's matrix.
    virtual void display();

    virtual float frameRate() const {
        return _def->get_frame_rate();
    }

    virtual float widthPixels() const {
        return _def->get_width_pixels();
    }

    virtual float heightPixels() const {
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
	
    /// Render this MovieClip to a GnashImage using the passed transform
    //
    /// @return     The GnashImage with the MovieClip drawn onto it.
    virtual std::auto_ptr<GnashImage> drawToBitmap(
            const SWFMatrix& mat = SWFMatrix(), 
            const cxform& cx = cxform(),
            DisplayObject::BlendMode bm = DisplayObject::BLENDMODE_NORMAL,
            const rect& clipRect = rect(),
            bool smooth = false);
private:
	
    const BitmapMovieDefinition* const _def;

};

} // end of namespace gnash

#endif // GNASH_BITMAPMOVIEINSTANCE_H
