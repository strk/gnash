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

#ifndef GNASH_SWF_TAGLOADERSTABLE_H
#define GNASH_SWF_TAGLOADERSTABLE_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "swf.h"

#include <map>
#include <boost/noncopyable.hpp>

// Forward declarations
namespace gnash {
	class SWFStream;
	class movie_definition;
    class RunInfo;
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
	typedef void (*loader_function)(
        SWFStream& input, TagType type, movie_definition& m, const RunInfo& r);

	/// \brief
	/// Get a pointer to the loader_function for the
	/// specified SWF::TagType.
	//
	/// @return false if no loader is associated with the tag.
	///
	bool get(TagType t, loader_function* lf) const;

	/// \brief
	/// Register a loader for the specified SWF::TagType.
	//
	/// @return false if a loader is already registered
	///               for the given tag
	///
	bool register_loader(TagType t, loader_function lf);

	/// \brief
	/// Return a reference to the singleton instance
	/// of this class.
	static TagLoadersTable& getInstance();

private:

	/// The container being used for the table
	typedef std::map<SWF::TagType, loader_function> container;

	container _tag_loaders;

	/// Use getInstance()
	TagLoadersTable()
		:
		_tag_loaders()
	{}

	~TagLoadersTable() {}

};

} // namespace gnash::SWF
} // namespace gnash

#endif // GNASH_SWF_TAGLOADERSTABLE_H
