// CodeStream.h A class which allows bounds-checked reading from a char array
//
//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

#ifndef GNASH_CODESTREAM_H
#define GNASH_CODESTREAM_H

#include <string>
#include <boost/utility.hpp>
#include <boost/cstdint.hpp>
#include <istream>
#include <sstream>

namespace gnash {

/// The exception which will be thrown by the CodeStream for access
/// violations.
class CodeStreamException { };

/// A checked read DisplayObject array
///
/// CodeStream provides a safe interface to read various things from a
/// DisplayObject array of known size.  Any attempt to access memory outside
/// of the given array will throw an exception of type CodeStreamException
class CodeStream : public std::istream, private boost::noncopyable
{
public:
	CodeStream(std::string data): std::istream(new std::stringbuf(data)){
		
	}

/// Read a variable length encoded 32 bit unsigned integer
boost::uint32_t read_V32();

/// Read an opcode for ActionScript 3
boost::uint8_t read_as3op();

/// Change the current position by a relative value.
void seekBy(int change);

/// Set the current position to an absolute value (relative to the start)
void seekTo(unsigned int set);

///Read a signed 24 bit interger.
boost::int32_t read_S24();

/// Read a signed 8-bit character.
int8_t read_s8();

/// Read an unsigned 8-bit character.
boost::uint8_t read_u8();

/// Same as read_V32(), but doesn't bother with the arithmetic for
/// calculating the value.
void skip_V32();

};

} // namespace gnash
#endif 
