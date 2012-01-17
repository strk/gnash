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

#ifndef GNASH_SWF_TAGLOADERSTABLE_H
#define GNASH_SWF_TAGLOADERSTABLE_H

#include "SWF.h"

#include <map>
#include <boost/noncopyable.hpp>

// Forward declarations
namespace gnash {
	class SWFStream;
	class movie_definition;
    class RunResources;
}

namespace gnash {
namespace SWF {

/// Table of SWF tags loaders
class TagLoadersTable : boost::noncopyable
{
public:

	/// Signature of an SWF tag loader
	//
	/// 'in' is the SWF input stream
	/// 't' is the tag type
	/// 'm' a pointer to the movie (or sprite) being read
	///
	typedef void (*TagLoader)(SWFStream& input, TagType type,
            movie_definition& m, const RunResources& r);

    typedef std::map<SWF::TagType, TagLoader> Loaders;

    /// Construct an empty TagLoadersTable
	TagLoadersTable() {}

    /// Construct a TagLoadersTable by copying another table
    TagLoadersTable(const Loaders& loaders)
        :
        _loaders(loaders)
    {}
	
    ~TagLoadersTable() {}

	/// Get the TagLoader for a specified TagType.
	//
	/// @return false if no loader is associated with the tag.
	///
	bool get(TagType t, TagLoader& lf) const;

	/// Register a loader for the specified SWF::TagType.
	//
    /// This is part of an API for allowing external applications
    /// to register custom tags, and is not used by Gnash itself.
	/// @return false if a loader is already registered
	///               for the given tag
	///
	bool registerLoader(TagType t, TagLoader lf);

private:

	Loaders _loaders;

};

} // namespace gnash::SWF
} // namespace gnash

#endif // GNASH_SWF_TAGLOADERSTABLE_H
