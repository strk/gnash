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


#ifndef GNASH_BITMAPMOVIEINSTANCE_H
#define GNASH_BITMAPMOVIEINSTANCE_H

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


/// Instance of a BitmapMovieDefinition
class BitmapMovie : public Movie
{

public:

	BitmapMovie(BitmapMovieDefinition* def, DisplayObject* parent=0); 

	virtual ~BitmapMovie() {}

	virtual void advance() {};

    virtual float frameRate() const {
        return _def->get_frame_rate();
    };

    virtual float widthPixels() const {
        return _def->get_width_pixels();
    }

    virtual float heightPixels() const {
        return _def->get_height_pixels();
    }

    virtual const std::string& url() const {
        return _def->get_url();
    }

    /// The SWF version of a loaded BitmapMovie is -1
    virtual int version() const {
        return -1;
    }

    virtual const movie_definition* definition() const {
        return _def;
    }
	
    /// Set a DisplayObject in the dictionary as initialized, returning
	/// true if not already initialized.
	bool setCharacterInitialized(int cid)
	{
		return _initializedCharacters.insert(cid).second;
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
	
    /// A map to track execution of init actions
	//
	/// Elements of this set are ids of DisplayObjects
	/// in our definition's CharacterDictionary.
	///
	std::set<int> _initializedCharacters;

    const BitmapMovieDefinition* const _def;

};

} // end of namespace gnash

#endif // GNASH_BITMAPMOVIEINSTANCE_H
