// Socket.h - a virtual IO channel, for Gnash
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
/// The Socket class will give you years of satisfaction provided you observe
/// the following points:
/// 1. A socket is active only when both connected() and not bad().
/// 2. The only accurate way of determining an error is to check bad().
/// 3. read() and write() should not be called until connected() is true.
class DSOEXPORT Socket : public IOChannel
{
public:

    /// Create a non-connected socket.
    Socket();

    virtual ~Socket() {}

    /// Initiate a connection
    //
    /// A return of true does not mean the connection has succeeded, only that
    /// it did not fail. The connection attempt is only complete when
    /// connected() returns true.
    //
    /// @return         false if the connection fails. In this case, the
    ///                 Socket is still in a closed state and is ready for
    ///                 a new connection attempt. Otherwise true.
    bool connect(const std::string& hostname, boost::uint16_t port);

    /// Close the Socket.
    //
    /// A closed Socket is in a state where another connection attempt can
    /// be made. Any errors are reset.
    void close();

    /// Whether a connection attempt is complete.
    //
    /// This is true as soon as the socket is ready for reading and writing.
    /// But beware! This function may still return true if the Socket is
    /// in error condition. Always check bad() if you care about this.
    bool connected() const;
    
    /// True if the Socket is in an error condition.
    //
    /// An error condition is fatal and can only be reset when the Socket
    /// is closed. Any read or write failure other than EAGAIN or EWOULDBLOCK
    /// causes a fatal error.
    virtual bool bad() const {
        return _error;
    }

    /// Read exactly the given number of bytes from the Socket or none at all.
    //
    virtual std::streamsize read(void* dst, std::streamsize num);

    /// Read up to the given number of bytes from the Socket.
    virtual std::streamsize readNonBlocking(void* dst, std::streamsize num);

    /// Write the given number of bytes to the stream
    //
    /// If you call write() before connected() is true, it may put the Socket
    /// into an error condition.
    //
    /// Calling write() when the Socket is bad has no effect.
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
    /// EOF is when remote end closed the connection and
    /// we have no more cached data to read
    ///
    virtual bool eof() const;

private:

    /// Fill the cache.
    void fillCache();

    mutable bool _connected;

    /// A cache for received data.
    char _cache[16384];

    /// The socket ID.
    int _socket;

    /// Unprocessed bytes.
    int _size;

    /// Current read position in cache.
    size_t _pos;

    mutable bool _error;
};

} // namespace gnash

#endif // GNASH_IOCHANNEL_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
