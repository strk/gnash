// stream.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A class to handle reading the elements of SWF files.


#ifndef GNASH_STREAM_H
#define GNASH_STREAM_H

#include "swf.h"
#include "tu_config.h"
#include "GnashException.h"

#include <string>
#include <sstream>
#include <vector> // for composition

// Define the following macro if you want to want Gnash parser
// to assume the underlying SWF is well-formed. It would make
// parsing faster, but might result in horrible behaviour with
// malformed SWFs (like taking up all system memory, keeping
// CPU busy for a long long time, or simply corrupting memory)
//
// This might be eventually set by a --disable-swf-checks or similar
// configure switch...
//
//#define GNASH_TRUST_SWF_INPUT

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
		/// Reads a single bit off the stream
		/// and returns it.  
		bool read_bit();

		/// \brief
		/// Reads a bit-packed little-endian signed integer
		/// from the stream.  The given bitcount determines the
		/// number of bits to read.
		int	read_sint(unsigned short bitcount);

		/// \brief
		/// Reads a little-endian decimal point value in the
		/// format that the first half is before the decimal
		/// point and the second half is after the decimal.
		/// _fixed is 32 bits, short_fixed is 16. The _sfixed
		/// versions read a signed fixed value.
		float	read_fixed();
		float	read_ufixed();
                float   read_short_ufixed();
		float	read_short_sfixed();

		/// \brief
		/// Read floating point values, not in the fixed format.
		float	read_float();

		/// \brief
		/// Read 64-bit double values.
		long double read_d64();

		/// \brief
		/// Discard any left-over bits from previous bit reads
		void	align()
		{
			m_unused_bits=0;
			// m_current_byte = 0; // this is not needed
		}

		/// \brief
		/// Read <count> bytes from the source stream and copy that data to <buf>.
		/// Implicitely aligns to the next byte.
		unsigned read(char *buf, unsigned count);
		
		/// \brief
		/// Read a aligned unsigned 8-bit value from the stream.		
		uint8_t  read_u8();

		/// \brief
		/// Read a aligned signed 8-bit value from the stream.		
		int8_t   read_s8();

		/// \brief
		/// Read a aligned unsigned 16-bit value from the stream.		
		uint16_t read_u16();

		/// \brief
		/// Read a aligned signed 16-bit value from the stream.		
		int16_t  read_s16();

		/// \brief
		/// Read a aligned unsigned 32-bit value from the stream.		
		uint32_t read_u32();

		/// \brief
		/// Read a aligned signed 32-bit value from the stream.		
		int32_t  read_s32();

		/// \brief
		/// Read a variable length unsigned 32-bit value from the stream.
		/// These values continue until either the high bit is not set or
		/// until 5 bytes have been read.
		uint32_t read_V32()
		{
			uint32_t res = read_u8();
			if (!(res & 0x00000080))
				return res;
			res = (res & 0x0000007F) | read_u8() << 7;
			if (!(res & 0x00004000))
				return res;
			res = (res & 0x00003FFF) | read_u8() << 14;
			if (!(res & 0x00200000))
				return res;
			res = (res & 0x001FFFFF) | read_u8() << 21;
			if (!(res & 0x10000000))
				return res;
			res = (res & 0x0FFFFFFF) | read_u8() << 28;
			return res;
		}

		/// \brief
		/// Skip a variable length unsigned 32-bit value in the stream.
		/// This is faster than doing the bitwise arithmetic of full reading.
		void skip_V32()
		{
			if (!(read_u8() & 0x80))
				return;
			if (!(read_u8() & 0x80))
				return;
			if (!(read_u8() & 0x80))
				return;
			if (!(read_u8() & 0x80))
				return;
			static_cast<void> (read_u8());
		}

		/// \brief
		/// Read a length in a byte or three.
		//
		/// If the byte == 0xff, read the lenght in 
		/// next two bytes.
		//
		/// Takes care of integrity check (ensureByte)
		///
		unsigned read_variable_count()
		{
			ensureBytes(1);
			unsigned count = read_u8();
			if (count == 0xFF)
			{
				ensureBytes(2);
				count = read_u16();
			}
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
		/// the end or before start of it.
		///
		/// @return true on success, false on failure
		/// 	Possible failures:
		///	- given position is after end of stream.
		///	- given position is after end of current tag, if any.
		///	- given position is before start of current tag, if any.
		///
		bool set_position(unsigned long pos);

		/// Return the file position of the end of the current tag.
		unsigned long get_tag_end_position();

		/// Return the tag type.
		SWF::tag_type	open_tag();

		/// Seek to the end of the most-recently-opened tag.
		void	close_tag();

		//tu_file*	get_underlying_stream() { return m_input; }

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
		/// Throws a ParserException on a short count.
		/// This method should be called before any attempt to read
		/// fields from the SWF.
		///
		/// NOTE: if GNASH_TRUST_SWF_INPUT is defined this function is a no-op 
		///
		void ensureBytes(unsigned long needed)
		{
#ifndef GNASH_TRUST_SWF_INPUT
			unsigned long int left = get_tag_end_position() - get_position();
			if ( left < needed )
			{
				std::stringstream ss;
				ss << "premature end of tag: need to read " << needed << " bytes, but only " << left << " left in this tag";
				throw ParserException(ss.str());
			}
#endif
		}

	private:

		tu_file*	m_input;
		uint8_t	m_current_byte;
		uint8_t	m_unused_bits;

		typedef std::pair<unsigned long,unsigned long> TagBoundaries;
		// position of start and end of tag
		std::vector<TagBoundaries> _tagBoundsStack;
	};


}	// end namespace gnash


#endif // GNASH_STREAM_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
