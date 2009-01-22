// IOChannel.h - a virtual IO channel, for Gnash
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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


#ifndef GNASH_IOCHANNEL_H
#define GNASH_IOCHANNEL_H

#include "dsodefs.h" // DSOEXPORT

#include "GnashException.h" // for IOException inheritance

#include <boost/cstdint.hpp> // for boost int types

// temp hack to avoid changing all callers
// TODO: update all callers ...
enum
{
    TU_FILE_NO_ERROR = 0,
    TU_FILE_OPEN_ERROR = -1,
    TU_FILE_READ_ERROR = -1,
    TU_FILE_WRITE_ERROR = -1,
    TU_FILE_SEEK_ERROR = -1,
    TU_FILE_CLOSE_ERROR = -1
};

namespace gnash {

/// Exception signalling an IO error
class IOException : public GnashException {
public:
	IOException(const std::string& s) : GnashException(s) {}
	IOException() : GnashException("IO error") {}
};

/// A virtual IO channel
class DSOEXPORT IOChannel
{
public:

	virtual ~IOChannel() {}

	/// \brief Read a 32-bit word from a little-endian stream.
	///	returning it as a native-endian word.
	//
	/// Throw IOException on error
	///
	boost::uint32_t read_le32();

	/// \brief Read a 64-bit word from a little-ending stream,
	/// returning it as a native-endian word.
	//
	/// Throw IOException on premature EOF
	///
	/// TODO: define a platform-neutral type for 64 bits.
	///
	long double read_le_double64();

	/// Read a 16-bit word from a little-endian stream.
	//
	/// Throw IOException on error
	///
	boost::uint16_t read_le16();

	/// Write a 32-bit word to a little-endian stream.
	//
	/// Throw IOException on error
	///
	void write_le32(boost::uint32_t u);

	/// \brief Write a 16-bit word to a little-endian stream.
	//
	/// Throw IOException on error
	///
	void write_le16(boost::uint16_t u);

	/// Read a single byte from the stream
	//
	/// Throw IOException on error
	///
	boost::uint8_t read_byte();

	/// write a single byte to the stream
	//
	/// Throw IOException on error
	///
	void write_byte(boost::uint8_t u);

	/// Read the given number of bytes from the stream
	//
	/// Return the number of bytes actually read. 
	/// EOF might cause it to be < num.
	///
	/// Throw IOException on error
	///
	virtual int read(void* dst, int num)=0;

	/// Read at most the given number of bytes w/out blocking
	//
	/// Throw IOException on error
	///
	/// @return The number of bytes actually read.
	///         A short count may mean EOF was hit or 
	///         data didn't arrive yet.
	///
	/// Default implementation proxies the call to the
	/// blocking version.
	///
	virtual int readNonBlocking(void* dst, int num)
	{
		return read(dst, num);
	}

	/// Write the given number of bytes to the stream
	//
	/// Throw IOException on error/unsupported op.
	///
	virtual int write(const void* src, int num);

	/// \brief Write a 0-terminated string to a stream.
	//
	/// Throw IOException on error
	///
	void write_string(const char* src);

	/// \brief
	/// Read up to max_length characters, returns the number of characters 
	/// read, or -1 if the string length is longer than max_length.
	//
	/// Stops at the first \0 character if it comes before max_length.
	///
	/// Guarantees termination of the string.
	///
	/// @return the number of characters read, or -1 no null-termination
	///         was found within max_length
	///
	/// Throw IOException on error
	///
	int	read_string(char* dst, int max_length);

	/// Write a 32-bit float to a stream in little-endian order.
	//
	/// NOTE: this currently relies on host FP format being the same as the Flash one
	///       (presumably IEEE 754).
	///
	/// Throw IOException on error
	///
	void	write_float32(float value);

	/// Read a 32-bit float from a little-endian stream.
	//
	/// NOTE: this currently relies on host FP format being the same as the Flash one
	/// (presumably IEEE 754).
	///
	/// Throw IOException on error
	///
	float	read_float32();

	/// Return current stream position
	//
	/// Throw IOException on error
	///
	virtual int tell() const=0;

	/// Seek to the specified position
	//
	/// 
	/// Throw IOException on error
	///
	/// @return 0 on success, or -1 on failure.
	///
	virtual int seek(int p)=0;

	/// Seek to the end of the stream
	//
	/// Throw IOException on error
	///
	virtual void go_to_end()=0;

	/// Return true if the end of the stream has been reached.
	//
	/// Throw IOException on error
	///
	virtual bool eof() const=0;
    
	/// Return non-zero if the stream is in an error state
	//
	/// When the stream is in an error state there's nothing
	/// you can do about it, just delete it and log the error.
	///
	/// There are some rough meaning for possible returned values
	/// but I don't think they make much sense currently.
	///
	virtual int get_error() const=0;
    
	/// Get the size of the stream (unreliably).
	//
	/// Size of steram is unreliable as not all input
	/// channels have a mechanism to advertise size,
	/// and some have one but isn't necessarely truthful
	/// (a few HTTP severs are bogus in this reguard).
	///
	/// @return unreliable input size, -1 if not known. 
	///
	virtual int size() const { return -1; }
   
};

} // namespace gnash

#endif // GNASH_IOCHANNEL_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
