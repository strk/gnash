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

#include "movie_instance.h" // for inheritance

// Forward declarations
namespace gnash
{
	class BitmapMovieDefinition;
    class GnashImage;
}

namespace gnash
{


/// Instance of a BitmapMovieDefinition
class BitmapMovieInstance : public movie_instance
{

public:

	BitmapMovieInstance(BitmapMovieDefinition* def, character* parent=0); 

	virtual ~BitmapMovieInstance() {}

    /// Render this MovieClip to a GnashImage using the passed transform
    //
    /// @return     The GnashImage with the MovieClip drawn onto it.
    virtual std::auto_ptr<GnashImage> drawToBitmap(
            const SWFMatrix& mat = SWFMatrix(), 
            const cxform& cx = cxform(),
            character::BlendMode bm = character::BLENDMODE_NORMAL,
            const rect& clipRect = rect(),
            bool smooth = false);

	/// Do nothing on restart. Especially don't trash the DisplayList 
	//
	/// TODO: this is needed due to the implementation detail of 
	///       using the DisplayList to store our bitmap-filled
	///       shape. Using the _drawable instead, or overriding
	///       ::display to simply display our definition is likely
	///	  the best way to go instead (we'd also reuse the same
	///       bitmap info rather then creating new instances..)
	void restart() {}

};

} // end of namespace gnash

#endif // GNASH_BITMAPMOVIEINSTANCE_H
