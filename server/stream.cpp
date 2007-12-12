// stream.cpp - SWF stream reading clas, for Gnash
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


#include "stream.h"

#include "log.h"
#include "types.h"
#include "tu_file.h"
#include "swf.h"
#include "Property.h"

#include <cstring>
//#include <iostream> // debugging only

//#define USE_TU_FILE_BYTESWAPPING 1

namespace gnash {
	
stream::stream(tu_file* input)
	:
	m_input(input),
	m_current_byte(0),
	m_unused_bits(0)
{
}


stream::~stream()
{
}

unsigned stream::read(char *buf, unsigned count)
{
	align();
	return m_input->read_bytes(buf, count);
}

bool stream::read_bit()
{
	if (!m_unused_bits)
	{
		m_current_byte = m_input->read_byte(); // don't want to align here
		m_unused_bits = 7;
		return (m_current_byte&0x80);
	}
	else
	{
		return ( m_current_byte & (1<<(--m_unused_bits)) );
	}
}

unsigned stream::read_uint(unsigned short bitcount)
{
	//assert(bitcount <= 24);
	// should be 24, check why htf_sweet.swf fails this assertion
	assert(bitcount <= 32);

	// Optimization for multibyte read
	if ( bitcount > m_unused_bits )
	{
		typedef unsigned char byte;

		boost::uint32_t value = 0;

		if (m_unused_bits) // Consume all the unused bits.
		{
			int unusedMask = (1 << m_unused_bits)-1;
			bitcount -= m_unused_bits; 
			value |= ((m_current_byte&unusedMask) << bitcount);
		}

		int bytesToRead = bitcount/8;
		int spareBits = bitcount%8; // additional bits to read

		//std::cerr << "BytesToRead: " << bytesToRead << " spareBits: " << spareBits << " unusedBits: " << (int)m_unused_bits << std::endl;

		byte cache[4]; // at most 4 bytes in the cache

		if ( spareBits ) m_input->read_bytes(&cache, bytesToRead+1);
		else m_input->read_bytes(&cache, bytesToRead);

		for (int i=0; i<bytesToRead; ++i)
		{
			bitcount -= 8;
			value |= cache[i] << bitcount; 
		}

		//assert(bitcount == spareBits);
		if ( bitcount )
		{
			m_current_byte = cache[bytesToRead];
			m_unused_bits = 8-bitcount;
			value |= m_current_byte >> m_unused_bits;
		}
		else
		{
			m_unused_bits = 0;
		}

		return value;
		
	}

	if (!m_unused_bits)
	{
		m_current_byte = m_input->read_byte();
		m_unused_bits = 8;
	}

	// TODO: optimize unusedMask creation ?
	//       (it's 0xFF if ! m_unused_bits above)
	int unusedMask = (1 << m_unused_bits)-1;

	if (bitcount == m_unused_bits)
	{
		// Consume all the unused bits.
		m_unused_bits = 0;
		return (m_current_byte&unusedMask);
	}
	else
	{
		assert(bitcount < m_unused_bits);
		// Consume some of the unused bits.

		m_unused_bits -= bitcount;
		return ((m_current_byte&unusedMask) >> m_unused_bits);
	}

}


int	stream::read_sint(unsigned short bitcount)
{
	//assert(bitcount <= 32); // already asserted in read_uint

	boost::int32_t	value = boost::int32_t(read_uint(bitcount));

	// Sign extend...
	if (value & (1 << (bitcount - 1))) {
		value |= -1 << bitcount;
	}

//		IF_DEBUG(log_msg("stream::read_sint(%d) == %d\n", bitcount, value));

	return value;
}


float	stream::read_fixed()
{
	// align(); // read_u32 will align 
	return static_cast<float> (
		static_cast<double>(read_s32()) / 65536.0f
	);

}

// float is not large enough to hold a 32 bit value without doing the wrong thing with the sign.
// So we upgrade to double for the calculation and then resize when we know it's small enough.
float	stream::read_ufixed()
{
	// align(); // read_u32 will align 
	return static_cast<float> (
		static_cast<double>(read_u32()) / 65536.0f
	);
}

// Read a short fixed value, unsigned.
float   stream::read_short_ufixed()
{
	// align(); // read_u16 will align 
	return static_cast<float> ( read_u16() / 256.0f );
}

// Read a short fixed value, signed.
float	stream::read_short_sfixed()
{
	// align(); // read_s16 will align 
	return static_cast<float> ( read_s16() / 256.0f );
}

// Read a signed float value.
float	stream::read_float()
{
	// align(); // read_s16 will align
	return static_cast<float> ( read_s16() );
}

// Read a 64-bit double value
long double stream::read_d64()
{
#ifdef USE_TU_FILE_BYTESWAPPING 
	align();
	return m_input->read_le_double64();
#else
	using boost::uint32_t;

	unsigned char _buf[8]; read((char*)_buf, 8); // would align
	uint64_t low = _buf[0];
		 low |= _buf[1] << 8;
		 low |= _buf[2] << 16;
		 low |= _buf[3] << 24;

	uint64_t hi = _buf[4];
		 hi |= _buf[5] << 8;
		 hi |= _buf[6] << 16;
		 hi |= _buf[7] << 24;

	return static_cast<long double> ( low | (hi<<32) );
#endif
}

boost::uint8_t	stream::read_u8()
{
	align();
	return m_input->read_byte();
}

int8_t	stream::read_s8()
{
	// read_u8 will align
	return read_u8();
}

boost::uint16_t	stream::read_u16()
{
#ifdef USE_TU_FILE_BYTESWAPPING 
	align();
	return m_input->read_le16();
#else
	using boost::uint32_t;

	unsigned char _buf[2]; read((char*)_buf, 2); // would align
	uint32_t result = _buf[0];
		 result |= (_buf[1] << 8);

	return result;
#endif
}

boost::int16_t stream::read_s16()
{
	// read_u16 will align
	return read_u16();
}

boost::uint32_t	stream::read_u32()
{
#ifdef USE_TU_FILE_BYTESWAPPING 
	align();
	return m_input->read_le32();
#else
	using boost::uint32_t;

	unsigned char _buf[4]; read((char*)_buf, 4); // would align
	uint32_t result = _buf[0];
		 result |= _buf[1] << 8;
		 result |= _buf[2] << 16;
		 result |= _buf[3] << 24;

	return result;
#endif
}

boost::int32_t	stream::read_s32()
{
	// read_u32 will align
	return read_u32();
}


char*	stream::read_string()
{
	align();

	std::vector<char>	buffer;
	char	c;
	while ((c = read_u8()) != 0)
	{
		buffer.push_back(c);
	}
	buffer.push_back(0);

	if (buffer.size() == 0)
	{
		return NULL;
	}

	char*	retval = new char[buffer.size()];
	strcpy(retval, &buffer[0]);

	return retval;
}

void
stream::read_string(std::string& to)
{
	align();

	to.clear();

	char	c;
	while ((c = read_u8()) != 0)
	{
		to += c; 
	}

}


char*	stream::read_string_with_length()
{
	align();

	int	len = read_u8();
	//log_msg("String length: %d", len);
	if (len <= 0)
	{
		return NULL;
	}
	else
	{
		char*	buffer = new char[len + 1];
		int	i;
		for (i = 0; i < len; i++)
		{
			buffer[i] = read_u8();
		}
		buffer[i] = '\0';	// terminate.

		return buffer;
	}
}

void stream::read_string_with_length(std::string& to)
{
	align();

	unsigned int	len = read_u8();
	read_string_with_length(len, to);
}

void stream::read_string_with_length(unsigned len, std::string& to)
{
	align();

	to.resize(len);

	for (unsigned int i = 0; i < len; ++i)
	{
		to[i] = read_u8();
	}

}


unsigned long stream::get_position()
{
	return m_input->get_position();
}


bool	stream::set_position(unsigned long pos)
{
	align();

	// If we're in a tag, make sure we're not seeking outside the tag.
	if (_tagBoundsStack.size() > 0)
	{
		TagBoundaries& tb = _tagBoundsStack.back();
		unsigned long end_pos = tb.second;
		if ( pos > end_pos )
		{
			log_error("Attempt to seek past the end of an opened tag");
			// abort(); // ?
			// throw ParserException ?
			return false;
		}
		unsigned long start_pos = tb.first;
		if ( pos < start_pos )
		{
			log_error("Attempt to seek before start of an opened tag");
			// abort(); // ?
			// throw ParserException ?
			return false;
		}
	}

	// Do the seek.
	if ( m_input->set_position(pos) == TU_FILE_SEEK_ERROR )
	{
		// TODO: should we throw an exception ?
		//       we might be called from an exception handler
		//       so throwing here might be a double throw...
		log_swferror(_("Unexpected end of stream"));
		return false;
	}

	return true;
}


unsigned long stream::get_tag_end_position()
{
	assert(_tagBoundsStack.size() > 0);

	return _tagBoundsStack.back().second;
}


SWF::tag_type stream::open_tag()
{
	align();

	unsigned long tagStart=get_position();

	int	tag_header = read_u16();
	int	tag_type = tag_header >> 6;
	int	tag_length = tag_header & 0x3F;
	assert(m_unused_bits == 0);
	if (tag_length == 0x3F) {
		tag_length = read_u32();
	}

	if ( tag_length > 1024*64 )
	{
		log_debug("Tag %d has a size of %d bytes !!", tag_type, tag_length);
	}

	unsigned long tagEnd = get_position()+tag_length;

	if ( ! _tagBoundsStack.empty() )
	{
		// check that this tag doesn't cross containing tag bounds
		unsigned long containerTagEnd = _tagBoundsStack.back().second;
		if ( tagEnd > containerTagEnd )
		{
			unsigned long containerTagStart = _tagBoundsStack.back().first;
			std::stringstream ss;
			ss << "Tag " << tag_type << " starting at offset " << tagStart
			   << " is advertised to end at offset " << tagEnd
			   << " which is after end of previously opened tag starting "
			   << " at offset " << containerTagStart
			   << " and ending at offset " << containerTagEnd << "."
			   << " Making it end where container tag ends.";
			log_swferror("%s", ss.str().c_str());

			// what to do now ?
			tagEnd = containerTagEnd;
			//throw ParserException(ss.str());
		}
	}
		
	// Remember where the end of the tag is, so we can
	// fast-forward past it when we're done reading it.
	_tagBoundsStack.push_back(std::make_pair(tagStart, tagEnd));

	IF_VERBOSE_PARSE (
		log_parse("SWF[%lu]: tag type = %d, tag length = %d, end tag = %lu",
		tagStart, tag_type, tag_length, tagEnd);
	);

	return static_cast<SWF::tag_type>(tag_type);
}


void	stream::close_tag()
{
	assert(_tagBoundsStack.size() > 0);
	unsigned long end_pos = _tagBoundsStack.back().second;
	_tagBoundsStack.pop_back();

	if ( m_input->set_position(end_pos) == TU_FILE_SEEK_ERROR )
	{
		log_error("Could not seek to end position");
	}

	m_unused_bits = 0;
}

} // end namespace gnash

	
// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
