// Socket.cpp - an IOChannel for sockets
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
//
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <boost/cstdint.hpp>
#include <cstring>

#include "URL.h"
#include "Socket.h"
#include "log.h"
#include "GnashAlgorithm.h"

namespace gnash
{

Socket::Socket()
    :
    _socket(0),
    _size(0),
    _pos(0),
    _timedOut(false)
{}


void
Socket::close()
{
    if (_socket) ::close(_socket);
    _socket = 0;
    _size = 0;
}

bool
Socket::connect(const URL& url)
{

    if (connected()) {
        log_error("Connection attempt while already connected");
        return false;
    }

    const std::string& hostname = url.hostname();
    if (hostname.empty()) return false;

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;

    addr.sin_addr.s_addr = inet_addr(hostname.c_str());
    if (addr.sin_addr.s_addr == INADDR_NONE) {
        struct hostent* host = gethostbyname(hostname.c_str());
        if (!host || !host->h_addr) {
            return false;
        }
        addr.sin_addr = *reinterpret_cast<in_addr*>(host->h_addr);
    }

    const std::string& port = url.port();
    const int p = port.empty() ? 0 : boost::lexical_cast<int>(port);
    addr.sin_port = htons(p);

    _timedOut = false;

    _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_socket != -1) {
        const struct sockaddr* a = reinterpret_cast<struct sockaddr*>(&addr);

        if (::connect(_socket, a, sizeof(struct sockaddr)) < 0) {
            const int err = errno;
            log_error("Failed to connect socket: %s", std::strerror(err));
            close();
            return false;
        }

    }

    // Magic timeout number. Use rcfile ?
    struct timeval tv = { 120, 0 };

    if (setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO,
                reinterpret_cast<unsigned char*>(&tv), sizeof(tv))) {
        log_error("Setting socket timeout failed");
    }

    const boost::int32_t on = 1;
    setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));

    assert(_socket);
    return true;
}


std::streamsize
Socket::fillCache()
{

    // If there are no unprocessed bytes, start from the beginning.
    if (!_size) {
        _pos = _cache;
    }

    while (1) {


        // Read up to the end of the cache if possible.
        const int toRead = arraySize(_cache) - _size - (_pos - _cache);

        const int bytesRead = recv(_socket, _pos + _size, toRead, 0);
        
        if (bytesRead == -1) {
            const int err = errno;
            log_debug("Socket receive error %s", std::strerror(err));
            //if (err == EINTR) continue;
            
            if (err == EWOULDBLOCK || err == EAGAIN) {
                // Nothing read.
                _timedOut = true;
                return 0;
            }
        }
        _size += bytesRead;
        return bytesRead;
    }
}


// Do a single read and report how many bytes were read.
std::streamsize 
Socket::read(void* dst, std::streamsize num)
{
    
    int toRead = num;
    _timedOut = false;

    boost::uint8_t* ptr = static_cast<boost::uint8_t*>(dst);

    if (!_size) {
        if (fillCache() < 1) {
            if (_timedOut) {
                return -1;
            }
        }
    }

    const int thisRead = std::min(_size, toRead);
    if (thisRead > 0) {
        std::copy(_pos, _pos + thisRead, ptr);
        _pos += thisRead;
        _size -= thisRead;
    }
    return thisRead;
}

std::streamsize
Socket::readNonBlocking(void* dst, std::streamsize num)
{
    return read(dst, num);
}

std::streamsize
Socket::write(const void* src, std::streamsize num)
{
    assert(!bad());

    int bytesSent = 0;
    int toWrite = num;

    const boost::uint8_t* buf = static_cast<const boost::uint8_t*>(src);

    while (toWrite > 0) {
        bytesSent = ::send(_socket, buf, toWrite, 0);
        //log_debug("Bytes sent %s", bytesSent);
        if (bytesSent < 0) {
            const int err = errno;
            log_error("Socket send error %s", std::strerror(err));
            close();
            return -1;
            break;
        }
        if (bytesSent == 0) break;
        toWrite -= bytesSent;
        buf += bytesSent;
    }
    return num - toWrite;
}

std::streampos
Socket::tell() const
{
    log_error("tell() called for Socket");
    return static_cast<std::streamsize>(-1);
}

bool
Socket::seek(std::streampos)
{
    log_error("seek() called for Socket");
    return false;
}

void
Socket::go_to_end()
{
    log_error("go_to_end() called for Socket");
}

bool
Socket::eof() const
{
    log_error("eof() called for Socket");
    return false;
}

bool
Socket::bad() const
{
    return !_socket || _timedOut;
}

} // namespace gnash
