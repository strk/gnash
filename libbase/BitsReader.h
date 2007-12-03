// BitsReader.h:  bits reader, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
//

// $Id: BitsReader.h,v 1.5 2007/12/03 08:35:34 strk Exp $

#ifndef BITSREADER_H
#define BITSREADER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "swf.h"
#include "tu_config.h"
#include "GnashException.h"
#include "log.h"

#include <string>
#include <sstream>

namespace gnash {

/// BitsReader is used to encapsulate bit-packed buffer reads.
//
/// TODO: implement boundary checking, maybe by throwing an exception
///       when attempts are made to read past end of buffer
///
class DSOEXPORT BitsReader 
{
public:
	typedef unsigned char byte;

	/// Ownership of buffer left to caller
	BitsReader(byte* input, size_t len)
		:
		start(input),
		ptr(input),
		end(ptr+len),
		usedBits(0)
	{
	}

	~BitsReader() {}

	size_t size() const
	{
		return end-start;
	}

	/// Set a new buffer to work with
	void setBuffer(byte* input, size_t len)
	{
		start = ptr = input;
		end = start+len;
		usedBits = 0;
	}

	/// \brief
	/// Reads a bit-packed unsigned integer from the stream
	/// and returns it.  The given bitcount determines the
	/// number of bits to read.
	unsigned read_uint(unsigned short bitcount);

	/// \brief
	/// Reads a single bit off the stream
	/// and returns it.  
	bool read_bit();

	/// \brief
	/// Reads a bit-packed little-endian signed integer
	/// from the stream.  The given bitcount determines the
	/// number of bits to read.
	int32_t read_sint(unsigned short bitcount);

	/// Read a byte as an unsigned int (aligned)
	uint8_t  read_u8()
	{
		align();
		return *ptr++;
	}

	/// Read one bytes as a signed int (aligned)
	int8_t	read_s8()
	{
		return static_cast<int8_t>(read_u8());
	}

	/// Read two bytes as an unsigned int (aligned)
	uint16_t read_u16()
	{
		align();
		assert(ptr+2 < end);
		uint16_t result = *ptr++;
		result |= *ptr++ << 8;
		return result ;
	}

	/// Read two bytes as a signed int (aligned)
	int16_t	read_s16()
	{
		return static_cast<int16_t>(read_u16());
	}

	/// Read four bytes as an unsigned int (aligned)
	uint32_t read_u32()
	{
		align();
		assert(ptr+4 < end);
		uint32_t result = *ptr++;
		result |= *ptr++ << 8;
		result |= *ptr++ << 16;
		result |= *ptr++ << 24;
		return(result);
	}

	/// Read four bytes as an signed int (aligned)
	int32_t read_s32()
	{
		return static_cast<int32_t>(read_u32());
	}

	/// \brief
	/// Discard any unused bit in the current byte
	/// and advance to next
	void align()
	{
		if ( usedBits ) advanceToNextByte();
	}

	/// Checks if the stream contains X bits
	bool gotBits(uint32_t nbits)
	{
		uint32_t gotbits = 8-usedBits +8*(end-ptr-1);
		if (gotbits > nbits) return true;
		else return false;
	}

private:

	void advanceToNextByte()
	{
		if ( ++ptr == end )
		{
			log_debug("Going round");
			ptr=start;
		}
		usedBits=0;
	}

	/// Pointer to first byte
	byte* start;

	/// Pointer to current byte
	byte* ptr;

	/// Pointer to one past last byte
	byte* end;

	/// Number of used bits in current byte
	unsigned usedBits;

};


}	// end namespace gnash


#endif // BITSREADER_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
