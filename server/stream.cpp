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

	
	int	stream::read_uint(int bitcount)
	{
		assert(bitcount <= 32 && bitcount >= 0);
			
		uint32_t	value = 0;

		int	bits_needed = bitcount;
		while (bits_needed > 0)
		{
			if (m_unused_bits) {
				if (bits_needed >= m_unused_bits) {
					// Consume all the unused bits.
					value |= (m_current_byte << (bits_needed - m_unused_bits));

					bits_needed -= m_unused_bits;

					m_current_byte = 0;
					m_unused_bits = 0;

				} else {
					// Consume some of the unused bits.
					value |= (m_current_byte >> (m_unused_bits - bits_needed));

					// mask off the bits we consumed.
					m_current_byte &= ((1 << (m_unused_bits - bits_needed)) - 1);

					m_unused_bits -= bits_needed;

					// We're done.
					bits_needed = 0;
				}
			} else {
				m_current_byte = m_input->read_byte();
				m_unused_bits = 8;
			}
		}

		assert(bits_needed == 0);

		return value;
	}


	int	stream::read_sint(int bitcount)
	{
		assert(bitcount <= 32 && bitcount >= 0);

		int32_t	value = (int32_t) read_uint(bitcount);

		// Sign extend...
		if (value & (1 << (bitcount - 1))) {
			value |= -1 << bitcount;
		}

//		IF_DEBUG(log_msg("stream::read_sint(%d) == %d\n", bitcount, value));

		return value;
	}


	float	stream::read_fixed()
	{
		m_unused_bits = 0;
		int32_t	val = m_input->read_le32();
		return (float) val / 65536.0f;
	}

	void	stream::align() { m_unused_bits = 0; m_current_byte = 0; }

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


	char*	stream::read_string_with_length()
	{
		align();

		int	len = read_u8();
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


	int	stream::get_position()
	{
		return m_input->get_position();
	}


	void	stream::set_position(int pos)
	{
		align();

		// If we're in a tag, make sure we're not seeking outside the tag.
		if (m_tag_stack.size() > 0)
		{
			int	end_pos = m_tag_stack.back();
			assert(pos <= end_pos);
			end_pos = end_pos;	// inhibit warning
			// @@ check start pos somehow???
		}

		// Do the seek.
		m_input->set_position(pos);
	}


	int	stream::get_tag_end_position()
	{
		assert(m_tag_stack.size() > 0);

		return m_tag_stack.back();
	}


	SWF::tag_type stream::open_tag()
	{
		align();
		int	tag_header = read_u16();
		int	tag_type = tag_header >> 6;
		int	tag_length = tag_header & 0x3F;
		assert(m_unused_bits == 0);
		if (tag_length == 0x3F) {
			tag_length = m_input->read_le32();
		}

		IF_VERBOSE_PARSE (
			log_parse("SWF[%u]: tag type = %d, tag length = %d",
			get_position(), tag_type, tag_length);
		);
			
		// Remember where the end of the tag is, so we can
		// fast-forward past it when we're done reading it.
		m_tag_stack.push_back(get_position() + tag_length);

		return static_cast<SWF::tag_type>(tag_type);
	}


	void	stream::close_tag()
	{
		assert(m_tag_stack.size() > 0);
		int	end_pos = m_tag_stack.back();
		m_tag_stack.pop_back();
		m_input->set_position(end_pos);

		m_unused_bits = 0;
	}

} // end namespace gnash

	
// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
