// BitsReader.cpp:  bits reader, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "BitsReader.h"

#include "log.h"

namespace gnash {
	
bool BitsReader::read_bit()
{
	bool ret = (*ptr&(128>>usedBits));
	if ( ++usedBits == 8 ) advanceToNextByte();
	return ret;
}

unsigned BitsReader::read_uint(unsigned short bitcount)
{
	assert(bitcount <= 32);

	boost::uint32_t value = 0;

	unsigned short bits_needed = bitcount;
	do
	{
		int unusedMask = 0xFF >> usedBits;
		int unusedBits = 8-usedBits;

		if (bits_needed == unusedBits)
		{
			// Consume all the unused bits.
			value |= (*ptr&unusedMask);
			advanceToNextByte();
			break;

		}
		else if (bits_needed > unusedBits)
		{
			// Consume all the unused bits.

			bits_needed -= unusedBits; // assert(bits_needed>0)

			value |= ((*ptr&unusedMask) << bits_needed);
			advanceToNextByte();
		}
		else
		{
			assert(bits_needed <= unusedBits);

			// Consume some of the unused bits.

			unusedBits -= bits_needed;

			value |= ((*ptr&unusedMask) >> unusedBits);

			usedBits += bits_needed;
			if ( usedBits >= 8 ) advanceToNextByte();

			// We're done.
			break;
		}
	}
	while (bits_needed > 0);

	//std::cerr << "Returning value: " << value << " unused bits: " << (int)m_unused_bits << std::endl;
	return value;

}


boost::int32_t BitsReader::read_sint(unsigned short bitcount)
{
	boost::int32_t	value = boost::int32_t(read_uint(bitcount));

	// Sign extend...
	if (value & (1 << (bitcount - 1))) 
		value |= -1 << bitcount;

	return value;
}


} // end namespace gnash

