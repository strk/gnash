// CodeStream.cpp A class which allows bounds-checked reading from a char array
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

#include "CodeStream.h"
#include <iostream>

namespace gnash {

/// Read a variable length encoded 32 bit unsigned integer
boost::uint32_t
CodeStream::read_V32()
{
	char data;

	read(&data,1);
	boost::uint32_t result = data;
	if (!(result & 0x00000080))	return result;

	read(&data,1);
	result = (result & 0x0000007F) | data << 7;
	if (!(result & 0x00004000)) return result;
	
	read(&data,1);
	result = (result & 0x00003FFF) | data << 14;
	if (!(result & 0x00200000)) return result;

	read(&data,1);
	result = (result & 0x001FFFFF) | data << 21;
	if (!(result & 0x10000000)) return result;

	read(&data,1);
	return (result & 0x0FFFFFFF) | data << 28;

}

/// Read an opcode for ActionScript 3
boost::uint8_t
CodeStream::read_as3op()
{
	char data;
	read(&data,1);
	if(eof()){
		return 0;
	}
	else{
		return static_cast<boost::uint8_t> (data);
	}
}

/// Change the current position by a relative value.
void
CodeStream::seekBy(int change)
{
	seekg(change,ios_base::cur);
}

/// Set the current position to an absolute value (relative to the start)
void
CodeStream::seekTo(unsigned int set)
{
	seekg(set);
}

//TODO: Is there a better way to read a 24 bit signed int?
boost::int32_t
CodeStream::read_S24()
{
	char buffer[3];
	read(buffer,3);
	uint32_t result = buffer[0] & 0xFF;
	result |= buffer[1] & 0xFF << 8;
	result |= buffer[2] & 0xFF << 16;
	if (result & (1 << 23)) {
	       	result |= -1 << 24;
   	}

	return static_cast<boost::int32_t>(result);
}
	
/// Read a signed 8-bit character.
int8_t
CodeStream::read_s8()
{
	char data;
	read(&data,1);
	return static_cast<int8_t> (data);
}

/// Read an unsigned 8-bit character.
boost::uint8_t
CodeStream::read_u8()
{
	char data;
	read(&data,1);
	return static_cast<boost::uint8_t> (data);
}

/// Same as read_V32(), but doesn't bother with the arithmetic for
/// calculating the value.
void 
CodeStream::skip_V32()
{
	// shortcut evalution is mandated as standard.
	//TODO: Make this more efficient.
// 	if ((*mCurrent++ & 0x80) && (*mCurrent++ & 0x80) && (*mCurrent++ &0x80)
// 		&& (*mCurrent++ & 0x80) && (*mCurrent++ & 0x80)){
// 		return;
// 	}
	read_V32();
	return;

}

} // namespace gnash

