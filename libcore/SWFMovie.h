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


#ifndef GNASH_SWF_MOVIE_H
#define GNASH_SWF_MOVIE_H

#include <set>

#include "Movie.h" // for inheritance
#include "smart_ptr.h" // for composition
#include "SWFMovieDefinition.h" // for dtor visibility by smart ptr

// Forward declarations
namespace gnash {
	class movie_root; 
	class DisplayObject; 
}

namespace gnash
{

/// Stateful Movie object (a special kind of sprite)
class SWFMovie : public Movie
{

public:

	// We take a generic movie_definition to allow
	// for subclasses for other then SWF movies
	SWFMovie(const SWFMovieDefinition* const def, DisplayObject* parent);

	virtual ~SWFMovie() {}

	virtual void advance();

    virtual float frameRate() const {
        return _def->get_frame_rate();
    }

    virtual float widthPixels() const {
        return _def->get_width_pixels();
    }

    virtual float heightPixels() const {
        return _def->get_height_pixels();
    }

    virtual bool ensureFrameLoaded(size_t frameNo) const {
        return _def->ensure_frame_loaded(frameNo);
    }

	/// Handle a top-level movie on stage placement.
	//
	/// This method will just ensure first frame is loaded
	/// and then call MovieClip::stagePlacementCallback.
	///
	/// It's intended to be called by movie_root::setLevel().
	///
	void stagePlacementCallback(as_object* initObj = 0);

    const std::string& url() const {
        return _def->get_url();
    }

    int version() const {
        return _def->get_version();
    }

	/// Set a DisplayObject in the dictionary as initialized, returning
	/// true if not already initialized.
	bool setCharacterInitialized(int cid)
	{
		return _initializedCharacters.insert(cid).second;
	}

    const movie_definition* definition() const {
        return _def.get();
    }

private:

	/// A map to track execution of init actions
	//
	/// Elements of this set are ids of DisplayObjects
	/// in our definition's CharacterDictionary.
	///
	std::set<int> _initializedCharacters;

    /// This should only be a top-level movie, not a sprite_definition.
	const boost::intrusive_ptr<const SWFMovieDefinition> _def;
};


} // end of namespace gnash

#endif // GNASH_MOVIE_INSTANCE_H
