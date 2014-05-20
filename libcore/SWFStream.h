// stream.h - SWF stream reading class, for Gnash
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#ifndef GNASH_STREAM_H
#define GNASH_STREAM_H

#include "SWF.h"
#include "dsodefs.h" // still neded ?
#include "GnashException.h"

#include <string>
#include <sstream>
#include <vector> // for composition
#include <cstdint> // for boost::?int??_t

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

namespace gnash {
	class IOChannel;
}

namespace gnash {

/// SWF stream wrapper class
//
/// This class is used for loading variable-length data
/// from a stream, and keeping track of SWF tag boundaries.
///
/// Provides 'aligned' and 'bitwise' read functions:
/// - aligned reads always start on a byte boundary
/// - bitwise reads can cross byte boundaries
/// 
class DSOEXPORT SWFStream
{
public:
	SWFStream(IOChannel* input);
	~SWFStream();

	/// \brief
	/// Reads a bit-packed unsigned integer from the stream
	/// and returns it.  The given bitcount determines the
	/// number of bits to read.
	//
	/// bitwise read
	///
	unsigned read_uint(unsigned short bitcount);

	/// \brief
	/// Reads a single bit off the stream
	/// and returns it.  
	//
	/// bitwise read
	///
	bool read_bit();

	/// \brief
	/// Reads a bit-packed little-endian signed integer
	/// from the stream.  The given bitcount determines the
	/// number of bits to read.
	//
	/// bitwise read
	///
	int	read_sint(unsigned short bitcount);

	/// Read a 16.16 fixed point signed value
	//
	/// aligned read
	///
	float	read_fixed();

	/// Read a 16.16 fixed point unsigned value
	//
	/// aligned read
	///
	float	read_ufixed();

	/// Read a 8.8 fixed point unsigned value
	//
	/// aligned read
	///
	float   read_short_ufixed();

	/// Read a 8.8 fixed point signed value
	//
	/// aligned read
	///
	float	read_short_sfixed();

	/// Read a 32bit (1:sign 8:exp 23:mantissa) floating point value
	//
	/// aligned read
	///
	float	read_long_float();

	/// Read a 64-bit double value
	//
	/// aligned read
	///
	double read_d64();

	/// Consume all bits of current byte
	//
	/// This method is implicitly called by all 'aligned' reads.
	///
	/// NOTE:
	/// The position returned by tell() won't be changed
	/// by calls to this function, altought any subsequent reads
	/// will start on next byte. See tell() for more info.
	///
	void	align()
	{
		m_unused_bits=0;
		// m_current_byte = 0; // this is not needed
	}

	/// Read <count> bytes from the source stream and copy that data to <buf>.
	//
	/// aligned read
	///
	unsigned read(char *buf, unsigned count);
	
	/// Read a aligned unsigned 8-bit value from the stream.		
	//
	/// aligned read
	///
	std::uint8_t  read_u8();

	/// Read a aligned signed 8-bit value from the stream.		
	//
	/// aligned read
	///
    std::int8_t read_s8();

	/// Read a aligned unsigned 16-bit value from the stream.		
	//
	/// aligned read
	///
	std::uint16_t read_u16();

	/// Read a aligned signed 16-bit value from the stream.		
	//
	/// aligned read
	///
	std::int16_t  read_s16();

	/// Read a aligned unsigned 32-bit value from the stream.		
	//
	/// aligned read
	///
	std::uint32_t read_u32();

	/// \brief
	/// Read a aligned signed 32-bit value from the stream.		
	//
	/// aligned read
	///
	std::int32_t  read_s32();

	/// \brief
	/// Read a variable length unsigned 32-bit value from the stream.
	/// These values continue until either the high bit is not set or
	/// until 5 bytes have been read.
	//
	/// aligned read
	///
	std::uint32_t read_V32()
	{
        ensureBytes(1);
		std::uint32_t res = read_u8();
		if (!(res & 0x00000080)) return res;
        
        ensureBytes(1);
		res = (res & 0x0000007F) | read_u8() << 7;
		if (!(res & 0x00004000)) return res;
        
        ensureBytes(1);
		res = (res & 0x00003FFF) | read_u8() << 14;
		if (!(res & 0x00200000)) return res;
        
        ensureBytes(1);
		res = (res & 0x001FFFFF) | read_u8() << 21;
		if (!(res & 0x10000000)) return res;
        
        ensureBytes(1);
		res = (res & 0x0FFFFFFF) | read_u8() << 28;
		return res;
	}

	/// \brief
	/// Skip a variable length unsigned 32-bit value in the stream.
	/// This is faster than doing the bitwise arithmetic of full reading.
	///
	/// aligned read
	///
	void skip_V32()
	{
        ensureBytes(1);
		if (!(read_u8() & 0x80)) return;
        ensureBytes(1);
		if (!(read_u8() & 0x80)) return;
        ensureBytes(1);
		if (!(read_u8() & 0x80)) return;
        ensureBytes(1);
		if (!(read_u8() & 0x80)) return;
        ensureBytes(1);
		static_cast<void> (read_u8());
	}

	/// \brief
	/// Read a length in a byte or three.
	//
	/// If the byte == 0xff, read the lenght in 
	/// next two bytes.
	///
	/// Takes care of integrity check (ensureByte)
	///
	/// aligned read
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
	/// Reads a null-terminated string from the given file and
	/// assigns it to the given std::string, overriding any
	/// previous value of it.
	///
	/// aligned read
	///
	/// Will throw ParserException if no terminating null is found within
    /// tag boundaries
	void read_string(std::string& to);

	/// Reads a sized string into a provided std::string.
	//
	/// Length of string is read from the first byte.
	///
	/// @param to
	/// 	Output argument. Any previous value will be overriden.
	///
	/// aligned read
	///
	/// Will throw ParserException if advertised length crosses tag boundaries
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
	/// aligned read
	///
	/// Will throw ParserException if len crosses tag boundaries
	///
	void	read_string_with_length(unsigned len, std::string& to);

	/// Return our current (byte) position in the input stream.
	//
	/// NOTE:
	/// This is not necessarely the byte you'll read on next read.
	/// - For bitwise reads the currenty byte will be used only if not
	///   completely consumed. See align().
	/// - For aligned reads the current byte will not be used
	///   (already used)
	///
	unsigned long tell();

	/// Set the file position to the given value (byte aligned)
	//
	/// If we're scanning a tag, don't allow seeking past
	/// the end or before start of it.
	///
	/// @return true on success, false on failure
	/// 	Possible failures:
	///	- given position is after end of stream.
	///	- given position is after end of current tag, if any.
	///	- given position is before start of current tag, if any.
	///
	bool seek(unsigned long pos);

	/// Return the file position of the end of the current tag.
	unsigned long get_tag_end_position();

	/// Open an SWF tag and return it's type.
	//
	/// aligned read
	///
	SWF::TagType	open_tag();

	/// Seek to the end of the most-recently-opened tag.
	void	close_tag();

	/// Discard given number of bytes
	//
	///
	/// @return true on success, false on failure
	/// 	Possible failures:
	///	- skipping given number of bytes reaches end of stream.
	///	- skipping given number of bytes reaches end of
	///	  current tag, if any.
	///
	/// WARNING: alignment is not specified here, the method uses
	///          tell() which is known NOT to consider
	///          a fully-read byte as already "skipped"
	///	     TODO: force alignment and see what happens !!
	///
	bool skip_bytes(unsigned num)
	{
		// there's probably a better way, but
		// it's the interface that counts atm
		size_t curpos = tell();
		return seek(curpos+num);
	}

	/// Discard all bytes up to end of tag
	void skip_to_tag_end()
	{
		// seek will call align...
		seek(get_tag_end_position());
	}

	/// \brief
	/// Ensure the requested number of bytes are available for an aligned read
	/// in the currently opened tag.
	//
	/// Throws a ParserException on a short count.
	/// This method should be called before any attempt to read
	/// fields from the SWF.
	///
	/// NOTE: if GNASH_TRUST_SWF_INPUT is defined this function is a no-op 
	///
	void ensureBytes(unsigned long needed);

	/// \brief
	/// Ensure the requested number of bits are available for a bitwise read
	/// in currently opened tag.
	//
	/// Throws a ParserException on a short count.
	/// This method should be called before any attempt to read
	/// bits from the SWF.
	///
	/// NOTE: if GNASH_TRUST_SWF_INPUT is defined this function is a no-op 
	///
	void ensureBits(unsigned long needed)
	{
#ifndef GNASH_TRUST_SWF_INPUT
		if ( _tagBoundsStack.empty() ) return; // not in a tag (should we check file length ?)
		unsigned long int bytesLeft = get_tag_end_position() - tell();
		unsigned long int bitsLeft = (bytesLeft*8)+m_unused_bits;
		if ( bitsLeft < needed )
		{
			std::stringstream ss;
			ss << "premature end of tag: need to read " << needed << " bytes, but only " << bitsLeft << " left in this tag";
			throw ParserException(ss.str());
		}
#endif
	}

	/// Consume any pending input
	//
	/// This method is useful to force full consumption
	/// of the SWFStream's underlying IOChannel for the 
	/// cases in which it is a pipe and a writer would
	/// hang on it unless someone is reading.
	///
	/// This method will NOT be called automatically
	/// on SWFStream destruction as in the current 
	/// design SWFStream does NOT own the underlying 
	/// IOChannel. TODO: rethink about ownership.
	///
	/// NOTE: the stream position will be updated by
	///       this call, so that ::tell will basically
	///       return the full input size.
	///
	void consumeInput();

private:

	IOChannel*	m_input;
	std::uint8_t	m_current_byte;
	std::uint8_t	m_unused_bits;

	typedef std::pair<unsigned long,unsigned long> TagBoundaries;
	// position of start and end of tag
	std::vector<TagBoundaries> _tagBoundsStack;
};


} // namespace gnash


#endif // GNASH_STREAM_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
