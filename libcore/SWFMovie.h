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

// Stateful live Movie instance 


#ifndef GNASH_SWF_MOVIE_H
#define GNASH_SWF_MOVIE_H

#include <boost/intrusive_ptr.hpp>
#include <string>
#include <map>

#include "Movie.h" // for inheritance
#include "SWFMovieDefinition.h" // for dtor visibility by smart ptr
#include "dsodefs.h" // for DSOTEXPORT

// Forward declarations
namespace gnash {
	class DisplayObject; 
}

namespace gnash
{

/// Stateful Movie object (a special kind of sprite)
//
/// The tasks of the Movie include:
//
/// 1. Keep a 'dictionary' of parsed characters.
///     This is a container of characters defined in previous frames. It
///     acts like a genuine runtime dictionary of characters, although Gnash
///     actually stores the definitions in the SWFMovieDefinition as it is
///     parsed.
class SWFMovie : public Movie
{

    /// A container to track known characters and whether they are initialized.
    typedef std::map<std::uint16_t, bool> Characters;

public:

	DSOTEXPORT SWFMovie(as_object* object, const SWFMovieDefinition* def,
            DisplayObject* parent);

	virtual ~SWFMovie() {}

	virtual void advance();

    virtual float frameRate() const {
        return _def->get_frame_rate();
    }

    virtual size_t widthPixels() const {
        return _def->get_width_pixels();
    }

    virtual size_t heightPixels() const {
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

    /// Get the URL of the SWFMovie's definition.
    const std::string& url() const {
        return _def->get_url();
    }

    /// Get the version of the SWFMovie.
    //
    /// @return     the version of the SWFMovie.
    int version() const {
        return _def->get_version();
    }
    
    /// Get an exported character definition by its symbol name.
    //
    /// The character is only available after the ExportAssets tag has been
    /// executed.
    //
    /// @param symbol   The exported symbol of the character to retrieve.
    /// @return         The DefinitionTag of the requested character or 0
    ///                 if the character has not yet been exported.
    virtual SWF::DefinitionTag* exportedCharacter(const std::string& symbol);

    /// Add a character to the list of known characters
    //
    /// This makes the character known to ActionScript for initialization.
    /// Exported characters must both be in the definition's list of exports
    /// and added with this function before they are available.
    //
    /// If a duplicated character is added, it will not be marked
    /// uninitialized, as SWF::DoInitAction tags are only executed once
    /// for each id.
    void addCharacter(std::uint16_t id);

    /// Attempt to mark a character as initialized.
    //
    /// A character can be initialized once, but only after it is known to this
    /// Movie.
    //
    /// @param id   The id of the character to initialize.
    /// @return     false if the character cannot be initialized. This can mean
    ///             1. The character is not yet present (either not exported
    ///                or has not yet been placed on stage).
    ///             2. The character has already been initialized.
    ///             true if the character was marked initialized.
	bool initializeCharacter(std::uint16_t id);

    const movie_definition* definition() const {
        return _def.get();
    }

private:

    /// Tracks known characters and whether they have been initialized.
	Characters _characters;

    /// This should only be a top-level movie, not a sprite_definition.
	const boost::intrusive_ptr<const SWFMovieDefinition> _def;
};


} // end of namespace gnash

#endif // GNASH_MOVIE_INSTANCE_H
