// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//

#ifndef GNASH_SWF_TAGLOADERSTABLE_H
#define GNASH_SWF_TAGLOADERSTABLE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "swf.h"

#include <map>

// Forward declarations
namespace gnash {
	class stream;
	class movie_definition;
}

namespace gnash {
namespace SWF {

/// Table of SWF tags loaders
class TagLoadersTable
{
public:

	/// Signature of an SWF tag loader
	//
	/// 'in' is the SWF input stream
	/// 't' is the tag type
	/// 'm' a pointer to the movie (or sprite) being read
	///
	typedef void (*loader_function)(
		stream* input, tag_type type, movie_definition* m);

	/// Default constructor
	TagLoadersTable()
		:
		_tag_loaders()
	{}

	/// \brief
	/// Get a pointer to the loader_function for the
	/// specified SWF::tag_type.
	//
	/// @return false if no loader is associated with the tag.
	///
	bool get(tag_type t, loader_function* lf);

	/// \brief
	/// Register a loader for the specified SWF::tag_type.
	//
	/// @return false if a loader is already registered
	///               for the given tag
	///
	bool register_loader(tag_type t, loader_function lf);
	

private:

	/// The container being used for the table
	typedef std::map<int, loader_function> container;

	container _tag_loaders;

};

} // namespace gnash::SWF
} // namespace gnash

#endif // GNASH_SWF_TAGLOADERSTABLE_H
