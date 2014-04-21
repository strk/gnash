// IOChannel.h - a virtual IO channel, for Gnash
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


#ifndef GNASH_IOCHANNEL_H
#define GNASH_IOCHANNEL_H

#include <string>
#include <iostream> // for std::streamsize
#include <boost/cstdint.hpp> // for boost int types

#include "dsodefs.h" // DSOEXPORT
#include "GnashException.h" // for IOException inheritance

namespace gnash {

/// Exception signalling an IO error
class DSOEXPORT IOException : public GnashException
{
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
    ///    returning it as a native-endian word.
    //
    /// Throw IOException on error
    ///
    boost::uint32_t read_le32();

    /// Read a 16-bit word from a little-endian stream.
    //
    /// Throw IOException on error
    ///
    boost::uint16_t read_le16();

    /// Read a single byte from the stream
    //
    /// Throw IOException on error
    ///
    boost::uint8_t read_byte();
    
    /// Read the given number of bytes from the stream
    //
    /// Return the number of bytes actually read. 
    /// EOF might cause it to be < num.
    ///
    /// Throw IOException on error
    ///
    virtual std::streamsize read(void* dst, std::streamsize num)=0;

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
    virtual std::streamsize readNonBlocking(void* dst, std::streamsize num)
    {
        return read(dst, num);
    }

    /// Write the given number of bytes to the stream
    //
    /// Throw IOException on error/unsupported op.
    ///
    virtual std::streamsize write(const void* src, std::streamsize num);

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
    int    read_string(char* dst, int max_length);
    
    /// Read a 32-bit float from a little-endian stream.
    //
    /// NOTE: this currently relies on host FP format being the
        ///       same as the Flash one (presumably IEEE 754).
    ///
    /// Throw IOException on error
    ///
    float read_float32();

    /// Return current stream position
    //
    /// Throw IOException on error
    ///
    virtual std::streampos tell() const = 0;

    /// Seek to the specified position
    //
    /// 
    /// Throw IOException on error
    ///
    /// @return true on success, or false on failure.
    ///
    virtual bool seek(std::streampos p) = 0;

    /// Seek to the end of the stream
    //
    /// Throw IOException on error
    ///
    virtual void go_to_end() = 0;

    /// Return true if the end of the stream has been reached.
    //
    /// Throw IOException on error
    ///
    virtual bool eof() const = 0;
    
    /// Return true if the stream is in an error state
    //
    /// When the stream is in an error state there's nothing
    /// you can do about it, just delete it and log the error.
    virtual bool bad() const = 0;
    
    /// Get the size of the stream (unreliably).
    //
    /// Size of stream is unreliable as not all input
    /// channels have a mechanism to advertise size,
    /// and some have one but isn't necessarely truthful
    /// (a few HTTP severs are bogus in this reguard).
    ///
    /// @return unreliable input size, (size_t)-1 if not known. 
    ///
    virtual size_t size() const { return static_cast<size_t>(-1); }
   
};

} // namespace gnash

#endif // GNASH_IOCHANNEL_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
