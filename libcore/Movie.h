// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

// Stateful live Movie instance 


#ifndef GNASH_MOVIE_INSTANCE_H
#define GNASH_MOVIE_INSTANCE_H

#include <vector>
#include <set>

#include "MovieClip.h" // for inheritance
#include "smart_ptr.h" // for composition

// Forward declarations
namespace gnash {
	class DisplayObject; 
    class movie_definition;
}

namespace gnash
{

/// A top-level, standalone Movie that can be loaded and played.
//
/// This is an abstract interface for any Movie that can be loaded directly
/// into Gnash, including SWFs and Bitmaps.
//
/// The interface is not especially clean because SWFs and Bitmaps are
/// treated the same as top-level movies despite having almost nothing
/// in common. As this is required by Flash, it seems unavoidable.
class Movie : public MovieClip
{

public:

	Movie(movie_definition* def, DisplayObject* parent)
        :
        MovieClip(def, this, parent, parent ? 0 : -1)
    {}

	virtual ~Movie() {}

	virtual void advance() = 0;

    virtual float frameRate() const = 0;

    virtual float widthPixels() const = 0;

    virtual float heightPixels() const = 0;

    virtual bool ensureFrameLoaded(size_t /*frameNo*/) const {
        return true;
    }

    virtual const std::string& url() const = 0;

    virtual int version() const = 0;

	/// Set a DisplayObject in the dictionary as initialized, returning
	/// true if not already initialized.
	virtual bool setCharacterInitialized(int /*cid*/) {
        return false;
    }

    virtual const movie_definition* definition() const = 0;

};


} // end of namespace gnash

#endif // GNASH_MOVIE_INSTANCE_H
