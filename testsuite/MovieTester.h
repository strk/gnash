/* 
 *   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Linking Gnash statically or dynamically with other modules is making a
 * combined work based on Gnash. Thus, the terms and conditions of the GNU
 * General Public License cover the whole combination.
 *
 * As a special exception, the copyright holders of Gnash give you
 * permission to combine Gnash with free software programs or libraries
 * that are released under the GNU LGPL and with code included in any
 * release of Talkback distributed by the Mozilla Foundation. You may
 * copy and distribute such a system following the terms of the GNU GPL
 * for all but the LGPL-covered parts and Talkback, and following the
 * LGPL for the LGPL-covered parts.
 *
 * Note that people who make modified versions of Gnash are not obligated
 * to grant this special exception for their modified versions; it is their
 * choice whether to do so. The GNU General Public License gives permission
 * to release a modified version without this exception; this exception
 * also makes it possible to release a modified version which carries
 * forward this exception.
 *
 */ 

#ifndef _GNASH_MOVIETESTER_H
#define _GNASH_MOVIETESTER_H

#include <memory> // for auto_ptr
#include <string> // for auto_ptr

// Forward declarations
namespace gnash {
	class movie_definition;
	class sprite_instance;
	class character;
}

namespace gnash {

/// An utility class for testing movie playback
//
/// This is a just born implementation and doesn't
/// have much more then simply loading a movie and
/// providing a function to find DisplayItems by name
///
/// More functions will be added when needed.
///
class MovieTester
{
public:
	/// Fully load the movie at the specified location
	/// and create an instance of it.
	MovieTester(const std::string& filespec);

	/// Advance the movie by one frame
	void advance();

	/// Find a character in the display list of a sprite by name.
	//
	/// Return NULL if there's no character with that name in
	/// the sprite's display list.
	///
	const character* findDisplayItemByName(const sprite_instance& mc,
			const std::string& name);

	/// Get the topmost sprite instance of this movie
	gnash::sprite_instance* getRootMovie() {
		return _movie;
	}

private:

	gnash::movie_definition* _movie_def;

	gnash::sprite_instance* _movie;

};

} // namespace gnash

#endif // _GNASH_MOVIETESTER_H
