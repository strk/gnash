// 
//   Copyright (C) 2008 Free Software Foundation, Inc.
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

#ifndef __BUFFER_H__
#define __BUFFER_H__ 1

#include <boost/cstdint.hpp>
#include <string>

#include "getclocktime.hpp"
#include "amf.h"
#include "element.h"
#include "network.h"
#include "dsodefs.h"

// _definst_ is the default instance name
namespace amf
{

class DSOEXPORT Buffer 
{
public:
    Buffer();
    // Create with a size other than the default
    Buffer(size_t nbytes);
    
    // Delete the allocate memory
    ~Buffer();
    void clear();
    bool empty() { return (_seekptr)?true:false; };

    // Resize the buffer that holds the data
    void *resize();
    void *resize(size_t nbytes);

    // Put data into the buffer. This overwrites all data, and resets the seek ptr.
    void copy(gnash::Network::byte_t *data, size_t nbytes);
    void copy(gnash::Network::byte_t *data) { copy(data, _nbytes); };
    void copy(const std::string &str);
    void copy(double num);
    void copy(boost::uint16_t length);
    void copy(gnash::Network::byte_t byte);
//    void copy(bool);
//     void copy(boost::uint32_t val);
//     void copy(Element::amf_type_e type);

    // Append data to the existing data in the buffer. This assume the
    // buffer has been sized to hold the data as it is appended.
    gnash::Network::byte_t *append(Buffer *buf);
    gnash::Network::byte_t *append(Buffer &buf);
    gnash::Network::byte_t *append(boost::uint32_t val);
    gnash::Network::byte_t *append(bool);
    gnash::Network::byte_t *append(double num);
    gnash::Network::byte_t *append(Element::amf0_type_e type);
    gnash::Network::byte_t *append(boost::uint16_t length);
    gnash::Network::byte_t *append(gnash::Network::byte_t *data, size_t nbytes);
    gnash::Network::byte_t *append(gnash::Network::byte_t byte);
    gnash::Network::byte_t *append(const std::string &str);

    // Find a byte in the buffer
//    Network::byte_t *find(char c);
    gnash::Network::byte_t *find(gnash::Network::byte_t b);
    gnash::Network::byte_t *find(gnash::Network::byte_t *b, size_t size);
    
    // Drop a byte or range of characters without resizing
//    Network::byte_t *remove(char c);
    gnash::Network::byte_t *remove(gnash::Network::byte_t c);
    gnash::Network::byte_t *remove(int x);
    gnash::Network::byte_t *remove(int x, int y);
    
    // Accessors
    gnash::Network::byte_t *begin() { return _ptr ; };
    gnash::Network::byte_t *end() { return _ptr + _nbytes; };
    gnash::Network::byte_t *reference() { return _ptr; }
    size_t size() { return _nbytes; }
    void setSize(size_t nbytes) { _nbytes = nbytes; };
    
    // make ourselves be able to be copied.
    Buffer &operator=(Buffer *buf);
    Buffer &operator=(Buffer &buf);

    // Test against other buffers
    bool operator==(Buffer *buf);
    bool operator==(Buffer &buf);
    Buffer &operator+=(Buffer *buf);
    Buffer &operator+=(gnash::Network::byte_t byte);
    Buffer &operator+=(char byte);
    Buffer &operator+=(Buffer &buf);
    gnash::Network::byte_t operator[](int x) { return *(_ptr + x); };
    gnash::Network::byte_t *at(int x) { return _ptr + x; };
//    Buffer *hex2mem(const char *str);

    // How much room is left in the buffer past the seek pointer. This is
    // primarily used to see if the buffer is full populated with data.
    size_t spaceLeft() { return (_nbytes - (_seekptr - _ptr)); };
    
    // debug stuff, not need for running Cygnal
    void dump();
  protected:
    void *init(size_t nbytes);
    gnash::Network::byte_t *_seekptr;
    gnash::Network::byte_t *_ptr;
    size_t         _nbytes;
#ifdef USE_STATS_BUFFERS
    struct timespec _stamp;	// used for timing how long data stays in the queue.
#endif
};


} // end of amf namespace

#endif // end of __BUFFER_H__

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
