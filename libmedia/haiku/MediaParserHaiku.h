// MediaParserHaiku.h: Haiku media parsers, for Gnash
// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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

#ifndef GNASH_MEDIAPARSER_HAIKU_H
#define GNASH_MEDIAPARSER_HAIKU_H

#include "MediaParser.h" // for inheritance

#include <boost/scoped_array.hpp>
#include <memory>

// Forward declaration
namespace gnash {
	class IOChannel;
}

namespace gnash {
namespace media {
namespace haiku {

/// Haiku media kit based MediaParser
class MediaParserHaiku : public MediaParser
{
public:

	/// Construct a haiku media kit based media parser for given stream
	//
	/// Can throw a GnashException if input format couldn't be detected
	///
	MediaParserHaiku(std::auto_ptr<IOChannel> stream);

	~MediaParserHaiku();

	// See dox in MediaParser.h
	virtual bool seek(boost::uint32_t&);

	// See dox in MediaParser.h
	virtual bool parseNextChunk();

	// See dox in MediaParser.h
	virtual boost::uint64_t getBytesLoaded() const;
};


} // gnash.media.haiku namespace 
} // gnash.media namespace 
} // namespace gnash

#endif // __MEDIAPARSER_HAIKU_H__
