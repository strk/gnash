// stream.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// SWF stream wrapper class, for loading variable-length data from a
// stream, and keeping track of SWF tag boundaries.


#include "stream.h"

#include "log.h"
#include "types.h"
#include "tu_file.h"
#include "swf.h"
#include <cstring>
//#include <iostream> // debugging only
#include "Property.h"

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
		return m_input->read_bytes(buf, count);
	}

// @@ better?
// 	int	stream::read_uint(int bitcount)
// 	{
//		assert(bitcount <= 24);
// 		while (m_unused_bits < bitcount)
// 		{
// 			// Get more data.
// 			m_current_bits |= m_input->read_byte() << m_unused_bits;
// 			m_unused_bits += 8;
// 		}

// 		int	result = m_current_bits & ((1 << bitcount) - 1);
// 		m_current_bits >>= bitcount;
// 		m_unused_bits -= bitcount;
		
// 		return result;
// 	}

	
	bool stream::read_bit()
	{
		if (!m_unused_bits)
		{
			m_current_byte = m_input->read_byte();
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


#define OPTIMIZE_FOR_MULTIBYTE_BITS_READ 1

#ifdef OPTIMIZE_FOR_MULTIBYTE_BITS_READ

		// Optimization for multibyte read
		if ( bitcount > m_unused_bits )
		{
			typedef unsigned char byte;

			uint32_t value = 0;

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

#else // ndef OPTIMIZE_FOR_MULTIBYTE_BITS_READ

		uint32_t value = 0;

		unsigned short bits_needed = bitcount;
		do
		{
			int unusedMask = (1 << m_unused_bits)-1;

			if (bits_needed == m_unused_bits)
			{
				// Consume all the unused bits.
				value |= (m_current_byte&unusedMask);
				m_unused_bits = 0;
				break;

			}
			else if (bits_needed > m_unused_bits) // TODO: obsolete this !!
			{
				// Consume all the unused bits.

				bits_needed -= m_unused_bits; // assert(bits_needed>0)

				value |= ((m_current_byte&unusedMask) << bits_needed);

				m_current_byte = m_input->read_byte();
				m_unused_bits = 8;

			}
			else
			{
				assert(bits_needed <= m_unused_bits);
				// Consume some of the unused bits.

				m_unused_bits -= bits_needed;

				value |= ((m_current_byte&unusedMask) >> m_unused_bits);

				// We're done.
				break;
			}
		}
		while (bits_needed > 0);

		//std::cerr << "Returning value: " << value << " unused bits: " << (int)m_unused_bits << std::endl;
		return value;
#endif // ndef OPTIMIZE_FOR_MULTIBYTE_BITS_READ

	}


	int	stream::read_sint(unsigned short bitcount)
	{
		//assert(bitcount <= 32); // already asserted in read_uint

		int32_t	value = int32_t(read_uint(bitcount));

		// Sign extend...
		if (value & (1 << (bitcount - 1))) {
			value |= -1 << bitcount;
		}

//		IF_DEBUG(log_msg("stream::read_sint(%d) == %d\n", bitcount, value));

		return value;
	}


	float	stream::read_fixed()
	{
		align();
		return static_cast<float> (static_cast<double> (static_cast<long int> (m_input->read_le32())) / 65536.0f);
	}

	// float is not large enough to hold a 32 bit value without doing the wrong thing with the sign.
	// So we upgrade to double for the calculation and then resize when we know it's small enough.
	float	stream::read_ufixed()
	{
		align();
		return static_cast<float> (static_cast<double> (static_cast<unsigned long int> (m_input->read_le32())) / 65536.0f);
        }

	// Read a short fixed value, unsigned.
        float   stream::read_short_ufixed()
        {
		align();
		return static_cast<float> (static_cast<uint16_t> (m_input->read_le16())) / 256.0f;
        }

	// Read a short fixed value, signed.
	float	stream::read_short_sfixed()
	{
		align();
		return static_cast<float> (static_cast<int16_t> (m_input->read_le16())) / 256.0f;
	}

	// Read a signed float value.
	float	stream::read_float()
	{
		align();
		return static_cast<float> (m_input->read_le32());
	}

	// Read a 64-bit double value
	long double stream::read_d64()
	{
		align();
		return m_input->read_le_double64();
	}

	uint8_t	stream::read_u8() { align(); return m_input->read_byte(); }
	int8_t	stream::read_s8() { align(); return m_input->read_byte(); }
	uint16_t	stream::read_u16()
	{
		align();
//		IF_DEBUG(printf("filepos = %d ", SDL_RWtell(m_input)));
		int	val = m_input->read_le16();
//		IF_DEBUG(log_msg("val = 0x%X\n", val));
		return val;
	}
	int16_t	stream::read_s16() { align(); return m_input->read_le16(); }
	uint32_t	stream::read_u32()
	{
		align();
		uint32_t	val = m_input->read_le32();
		return val;
	}
	int32_t	stream::read_s32() { align(); return m_input->read_le32(); }


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
			tag_length = m_input->read_le32();
		}

		if ( tag_length > 1024*62 )
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
