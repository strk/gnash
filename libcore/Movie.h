// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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


#ifndef GNASH_MOVIE_H
#define GNASH_MOVIE_H

#include <string>
#include <set>

#include "MovieClip.h" // for inheritance

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

	Movie(as_object* object, const movie_definition* def,
            DisplayObject* parent)
        :
        MovieClip(object, def, this, parent)
    {}

	virtual ~Movie() {}

	virtual void advance() = 0;

    virtual float frameRate() const = 0;

    virtual size_t widthPixels() const = 0;

    virtual size_t heightPixels() const = 0;

    virtual bool ensureFrameLoaded(size_t /*frameNo*/) const {
        return true;
    }

    /// Get the URL the Movie was loaded from.
    virtual const std::string& url() const = 0;

    /// Get the version of the Movie
    //
    /// @return     Either the version of the Movie or -1 if the Movie is of
    ///             a type that has no version.
    virtual int version() const = 0;

    /// Get an exported character definition by its symbol name.
    //
    /// The character is only available after the ExportAssets tag has been
    /// executed.
    //
    /// @param symbol   The exported symbol of the character to retrieve.
    /// @return         The DefinitionTag of the requested character or 0
    ///                 if the character has not yet been exported.
    virtual SWF::DefinitionTag* exportedCharacter(const std::string& /*s*/) {
        return 0;
    }

    /// Add a character to the list of known characters
    //
    /// This makes the character known to ActionScript for initialization.
    /// Exported characters must both be in the definition's list of exports
    /// and added with this function before they are available.
    virtual void addCharacter(boost::uint16_t /*id*/) {}

    /// Attempt to mark a character as initialized.
    //
    /// The default is to return false. Only a SWFMovie can have a list of
    /// characters.
	virtual bool initializeCharacter(boost::uint16_t /*id*/) {
        return false;
    }

    virtual const movie_definition* definition() const = 0;

};


} // end of namespace gnash

#endif 
