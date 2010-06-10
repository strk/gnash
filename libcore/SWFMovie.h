// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#include "Movie.h" // for inheritance
#include "SWFMovieDefinition.h" // for dtor visibility by smart ptr

#include <boost/intrusive_ptr.hpp>
#include <string>
#include <map>

// Forward declarations
namespace gnash {
	class DisplayObject; 
}

namespace gnash
{

/// Stateful Movie object (a special kind of sprite)
class SWFMovie : public Movie
{

    typedef std::map<boost::uint16_t, bool> Characters;

public:

	SWFMovie(as_object* object, const SWFMovieDefinition* def,
            DisplayObject* parent);

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
	/// and then call MovieClip::construct
	///
	/// It's intended to be called by movie_root::setLevel().
    void construct(as_object* init = 0);

    const std::string& url() const {
        return _def->get_url();
    }

    int version() const {
        return _def->get_version();
    }

    /// Add a character to the list of known characters
    //
    /// This makes the character known to ActionScript for initialization.
    /// Exported characters must both be in the definition's list of exports
    /// and added with this function before they are available.
    void addCharacter(boost::uint16_t id);

    /// Returns true if character can be initialized.
    //
    /// A character can be initialized once, but only if it is known.
    /// @return     false if the character cannot be initialized. This can mean
    ///             1. The character is not yet present (either not exported
    ///                or has not yet been placed on stage).
    ///             2. The character has already been initialized.
	bool initializeCharacter(boost::uint16_t id);

    const movie_definition* definition() const {
        return _def.get();
    }

private:

	/// A map to track execution of init actions
	//
	/// Elements of this set are ids of DisplayObjects
	/// in our definition's CharacterDictionary.
	Characters _initializedCharacters;

    /// This should only be a top-level movie, not a sprite_definition.
	const boost::intrusive_ptr<const SWFMovieDefinition> _def;
};


} // end of namespace gnash

#endif // GNASH_MOVIE_INSTANCE_H
