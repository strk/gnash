// stream.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A class to handle reading the elements of SWF files.


#ifndef GNASH_STREAM_H
#define GNASH_STREAM_H

#include "container.h"
#include "swf.h"
#include "tu_config.h"
#include "GnashException.h"

#include <string>

class tu_file;

namespace gnash {

	/// stream is used to encapsulate bit-packed file reads.
	class DSOEXPORT stream
	{
	public:
		stream(tu_file* input);
		~stream();

		/// \brief
		/// Reads a bit-packed unsigned integer from the stream
		/// and returns it.  The given bitcount determines the
		/// number of bits to read.
		unsigned read_uint(unsigned short bitcount);

		/// \brief
		/// Reads a bit-packed little-endian signed integer
		/// from the stream.  The given bitcount determines the
		/// number of bits to read.
		int	read_sint(unsigned short bitcount);

		float	read_fixed();
		void	align();

		unsigned read(char *buf, unsigned count);
		uint8_t  read_u8();
		int8_t   read_s8();
		uint16_t read_u16();
		int16_t  read_s16();
		uint32_t read_u32();
		int32_t  read_s32();
		unsigned read_variable_count()
		{
			unsigned count = read_u8();
			if (count == 0xFF)
				count = read_u16();
			return count;
		};

		/// \brief
		/// Reads *and new[]'s* the string from the given file.
		/// Ownership passes to the caller; caller must delete[] the
		/// string when it is done with it.
		char*	read_string();	

		/// \brief
		/// Reads a null-terminated string from the given file and
		/// assigns it to the given std::string, overriding any
		/// previous value of it.
		///
		void	read_string(std::string& to);

		/// \brief
		/// Reads *and new[]'s* the string from the given file.
		/// Ownership passes to the caller; caller must delete[] the
		/// string when it is done with it.
		/// Length of string is read from the first byte.
		///
		char*	read_string_with_length();

		/// Reads a sized string into a provided std::string.
		//
		/// Length of string is read from the first byte.
		///
		/// @param to
		/// 	Output argument. Any previous value will be overriden.
		///
		void	read_string_with_length(std::string& to);

		/// Reads a sized string into a provided std::string.
		//
		///
		/// @param len
		///	Length of string to read.
		///
		/// @param to
		/// 	Output argument. Any previous value will be overriden.
		///
		void	read_string_with_length(unsigned len, std::string& to);

		/// Return our current (byte) position in the input stream.
		unsigned long get_position();

		/// Set the file position to the given value.
		//
		///
		/// If we're scanning a tag, don't allow seeking past
		/// the tag end. Ideally we shouldn't also allow seeking
		/// before tag start but this is currently unimplemented.
		///
		/// @return true on success, false on failure
		/// 	Possible failures:
		///	- given position is after end of stream.
		///	- given position is after end of current tag, if any.
		///
		bool set_position(unsigned long pos);

		/// Return the file position of the end of the current tag.
		unsigned long get_tag_end_position();

		/// Return the length of the current tag.
		//
		/// should return a  'long' ?
		///
		unsigned get_tag_length() {
			return _current_tag_length;
		}

		/// Return the tag type.
		SWF::tag_type	open_tag();

		/// Seek to the end of the most-recently-opened tag.
		void	close_tag();

		tu_file*	get_underlying_stream() { return m_input; }

		/// Discard given number of bytes
		//
		///
		/// @return true on success, false on failure
		/// 	Possible failures:
		///	- skipping given number of bytes reaches end of stream.
		///	- skipping given number of bytes reaches end of
		///	  current tag, if any.
		///
		bool skip_bytes(unsigned num)
		{
			// there's probably a better way, but
			// it's the interface that counts atm
			size_t curpos = get_position();
			return set_position(curpos+num);
		}

		/// Discard all bytes up to end of tag
		void skip_to_tag_end()
		{
			set_position(get_tag_end_position());
		}

		/// \brief
		/// Ensure the requested number of bytes are available in the
		/// currently opened tag.
		//
		/// Throws an exception on a short count.
		/// This method should be called before any attempt to read
		/// fields from the SWF.
		///
		void ensureBytes(unsigned long needed)
		{
			if ( get_tag_end_position() - get_position() < needed )
			{
				throw ParserException("premature end of tag");
			}
		}

	private:
		// should this be long ?
		unsigned _current_tag_length;

		tu_file*	m_input;
		uint8_t	m_current_byte;
		uint8_t	m_unused_bits;

		std::vector<unsigned long> m_tag_stack;	// position of end of tag
	};


}	// end namespace gnash


#endif // GNASH_STREAM_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
