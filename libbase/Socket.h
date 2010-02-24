// Socket.h - a virtual IO channel, for Gnash
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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


#ifndef GNASH_SOCKET_H
#define GNASH_SOCKET_H

#include "dsodefs.h"
#include <boost/cstdint.hpp>
#include "IOChannel.h"

namespace gnash {
    class URL;
}

namespace gnash {

/// A simple IOChannel subclass for reading and writing sockets.
//
/// The only important functions here are read and write.
class DSOEXPORT Socket : public IOChannel
{
public:

    Socket();

    virtual ~Socket() {}

    bool connect(const URL& url);

    void close();

    bool connected() const {
        return (_socket);
    }

    bool timedOut() const {
        return (_timedOut);
    }

    /// Read the given number of bytes from the stream
    //
    /// Return the number of bytes actually read. 
    virtual std::streamsize read(void* dst, std::streamsize num);

    /// The same as read().
    virtual std::streamsize readNonBlocking(void* dst, std::streamsize num);

    /// Write the given number of bytes to the stream
    virtual std::streamsize write(const void* src, std::streamsize num);
    
    /// Return current stream position
    //
    /// Meaningless for a Socket.
    virtual std::streampos tell() const;

    /// Seek to the specified position
    //
    /// Meaningless for a Socket.
    virtual bool seek(std::streampos p);

    /// Seek to the end of the stream
    //
    /// Not Socket.
    virtual void go_to_end();

    /// Return true if the end of the stream has been reached.
    //
    /// Not implemented for Socket.
    virtual bool eof() const;
    
    /// Return true if the stream is in an error state
    virtual bool bad() const;

private:

    /// Fill the cache.
    std::streamsize fillCache();

    /// A cache for received data.
    boost::uint8_t _cache[16384];

    /// The socket ID.
    int _socket;

    /// Unprocessed bytes.
    int _size;

    /// Current read position in cache.
    boost::uint8_t* _pos;

    bool _timedOut;
};

} // namespace gnash

#endif // GNASH_IOCHANNEL_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
