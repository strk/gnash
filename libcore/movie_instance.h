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
#include "movie_definition.h" // for dtor visibility by smart ptr

// Forward declarations
namespace gnash {
	class movie_root; 
	class DisplayObject; 
}

namespace gnash
{

/// Stateful Movie object (a special kind of sprite)
class movie_instance : public MovieClip
{

public:

	// We take a generic movie_definition to allow
	// for subclasses for other then SWF movies
	movie_instance(movie_definition* def, DisplayObject* parent);

	virtual ~movie_instance() {}

	virtual void advance();

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

private:

	/// A map to track execution of init actions
	//
	/// Elements of this set are ids of DisplayObjects
	/// in our definition's CharacterDictionary.
	///
	std::set<int> _initializedCharacters;

    /// This should only be a top-level movie, not a sprite_definition.
	const boost::intrusive_ptr<const movie_definition> _def;
};


} // end of namespace gnash

#endif // GNASH_MOVIE_INSTANCE_H
