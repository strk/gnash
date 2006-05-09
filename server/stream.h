// stream.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A class to handle reading the elements of SWF files.


#ifndef GNASH_STREAM_H
#define GNASH_STREAM_H

#include "container.h"
#include "swf.h"

class tu_file;

namespace gnash {
	// stream is used to encapsulate bit-packed file reads.
	class stream
	{
	public:
		stream(tu_file* input);
		~stream();

		int	read_uint(int bitcount);
		int	read_sint(int bitcount);
		float	read_fixed();
		void	align();

		uint8_t	read_u8();
		int8_t	read_s8();
		uint16_t	read_u16();
		int16_t	read_s16();
		uint32_t	read_u32();
		int32_t	read_s32();
		int     read_variable_count()
		{
			int count = read_u8();
			if (count == 0xFF)
				count = read_u16();
			return count;
		};

		// For null-terminated string.
		char*	read_string();	// reads *and new[]'s* the string -- ownership passes to caller!

		// For string that begins with an 8-bit length code.
		char*	read_string_with_length();	// reads *and new[]'s* the string -- ownership passes to caller!

		int	get_position();
		void	set_position(int pos);
		int	get_tag_end_position();
		SWF::tag_type	open_tag();
		void	close_tag();

		tu_file*	get_underlying_stream() { return m_input; }

	private:
		tu_file*	m_input;
		uint8_t	m_current_byte;
		uint8_t	m_unused_bits;

		std::vector<int>	m_tag_stack;	// position of end of tag
	};


};	// end namespace gnash


#endif // GNASH_STREAM_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
