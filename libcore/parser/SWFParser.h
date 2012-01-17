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

// The SWFMovieDefinition is the 'root' definition of a SWF movie, including
// movies loaded into another SWF file. Each self-contained SWF file has exactly
// one SWFMovieDefinition.

#ifndef GNASH_SWFPARSER_H
#define GNASH_SWFPARSER_H

#include "SWF.h"

namespace gnash {
    class SWFStream;
    class movie_definition;
    class RunResources;
}

namespace gnash {


/// The SWFParser parses tags from a SWFStream.
//
/// The definitions created from the tags are added to one of the higher-level
/// definition types: either a sprite_definition or a SWFMovieDefinition.
//
/// SWFParser's interface deals only with the size of data it has to read, or
/// the number of bytes read. It does not expose the absolute stream position.
/// This is intended to make internal refactoring simpler. Users must tell
/// the SWFParser how many bytes it should read from the stream.
//
/// The SWFParser will only deal with ParserExceptions in an open tag.
/// Exceptions thrown when opening and closing tags signal a fatal error,
/// and will be left to the callers to deal with.
class SWFParser
{

public:
    SWFParser(SWFStream& in, movie_definition* md, const RunResources& runResources)
        :
        _stream(in),
        _md(md),
        _runResources(runResources),
        _bytesRead(0),
        _tagOpen(false),
        _endRead(0),
        _nextTagEnd(0)

    {
    }

    /// The number of bytes processed by this SWFParser.
    size_t bytesRead() const {
        return _bytesRead;
    }

    /// Parse a specified number of bytes from the stream.
    //
    /// This function will read as many complete tags as are in the specified
    /// number of bytes. Any incomplete tags will be left open and unparsed
    /// until the next call to read().
    //
    /// @param bytes    The number of bytes to read. Callers must ensure
    ///                 this is not past the end of the stream, or this
    ///                 function may never return.
    //
    /// @return         False if the end of a SWF is reached during parsing.
    ///                 This can be mean that a SWF::END tag appears before
    ///                 the end of the bytes to parse.
    bool read(std::streamsize bytes);
    
private:

    size_t openTag();

    void closeTag();

    SWFStream& _stream;
    
    movie_definition* _md;
    
    const RunResources& _runResources;
    
    size_t _bytesRead;
    
    bool _tagOpen;
    
    size_t _endRead;
    
    size_t _nextTagEnd;
    
    SWF::TagType _tag;

};

} // namespace gnash

#endif
