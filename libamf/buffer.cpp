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

#include <boost/cstdint.hpp>
#include <iostream>
#include "buffer.h"
#include "amf.h"
#include "log.h"
#include "network.h"

using namespace std;
using namespace gnash;

namespace amf
{

#if 0
// convert an ascii hex digit to a number.
//      param is hex digit.
//      returns a decimal digit.
Network::byte_t
hex2digit (Network::byte_t digit)
{  
    if (digit == 0)
        return 0;
    
    if (digit >= '0' && digit <= '9')
        return digit - '0';
    if (digit >= 'a' && digit <= 'f')
        return digit - 'a' + 10;
    if (digit >= 'A' && digit <= 'F')
        return digit - 'A' + 10;
    
    // shouldn't ever get this far
    return -1;
}

// Convert the hex array pointed to by buf into binary to be placed in mem
Buffer *
Buffer::hex2mem(const char *str)
{
    size_t count = strlen(str);
    Network::byte_t ch = 0;
    _ptr = new Buffer((count/3)+1);
//    buf->clear();

    Network::byte_t *strdata = const_cast<Network::byte_t *>(reinterpret_cast<const Network::byte_t *>(str));
    
    for (size_t i=0; i<count; i++) {
        if (*strdata == ' ') {      // skip spaces.
            strdata++;
            continue;
        }
        ch = hex2digit(*strdata++) << 4;
        ch |= hex2digit(*strdata++);
        append(ch);
    }
    return _ptr;
}
#endif

void *
Buffer::init(size_t nbytes)
{
//    GNASH_REPORT_FUNCTION;
    if (_ptr == 0) {
        _ptr = new Network::byte_t[nbytes];
	_seekptr = _ptr;
	if (_ptr == 0) {
	    return _ptr;
	}
        _nbytes = nbytes;
    }

#ifdef USE_STATS_BUFFERS
    clock_gettime (CLOCK_REALTIME, &_stamp);
#endif
    return _ptr;
}

Buffer::Buffer() 
    : _seekptr(0),
      _ptr(0)
{
//    GNASH_REPORT_FUNCTION;
    _nbytes = gnash::NETBUFSIZE;
    init(gnash::NETBUFSIZE);
}
    
// Create with a size other than the default
Buffer::Buffer(size_t nbytes)
    : _seekptr(0),
      _ptr(0)
{
//    GNASH_REPORT_FUNCTION;
    _nbytes = nbytes;
    init(nbytes);
}

// Delete the allocate memory
Buffer::~Buffer()
{
//    GNASH_REPORT_FUNCTION;
    if (_ptr) {
#ifdef USE_STATS_BUFFERS
	struct timespec now;
	clock_gettime (CLOCK_REALTIME, &now);
	log_debug("Buffer %x (%d) stayed in queue for %f seconds",
		  (void *)_ptr, _nbytes,
		  (float)((now.tv_sec - _stamp.tv_sec) + ((now.tv_nsec - _stamp.tv_nsec)/1e9)));
#endif
        delete[] _ptr;
        _seekptr = _ptr = 0;
        _nbytes = 0;
    }
}

// Put data into the buffer
void
Buffer::copy(Network::byte_t *data, size_t nbytes)
{    
//    GNASH_REPORT_FUNCTION;
    std::copy(data, data + nbytes, _ptr);
    _seekptr = _ptr + nbytes;
}

void
Buffer::copy(const string &str)
{    
//    GNASH_REPORT_FUNCTION;
    std::copy(str.begin(), str.end(), _ptr);
    _seekptr = _ptr + str.size();
}

void
Buffer::copy(boost::uint16_t length)
{
    Network::byte_t *data = reinterpret_cast<Network::byte_t *>(&length);
    std::copy(data, data + sizeof(boost::uint16_t), _ptr);    
    _seekptr = _ptr + sizeof(boost::uint16_t);
}

void
Buffer::copy(double num)
{
//    GNASH_REPORT_FUNCTION;
    Network::byte_t *ptr = reinterpret_cast<Network::byte_t *>(&num);
    std::copy(ptr, ptr + amf::AMF0_NUMBER_SIZE, _ptr);    
    _seekptr = _ptr + amf::AMF0_NUMBER_SIZE;
}

void
Buffer::copy(Network::byte_t val)
{
    GNASH_REPORT_FUNCTION;
    *_ptr = val;
    _seekptr = _ptr + sizeof(bool);
}

#if 0
void
Buffer::copy(bool val)
{
    GNASH_REPORT_FUNCTION;
    return copy(static_cast<Network::byte_t>(val));
}
#endif

Network::byte_t *
Buffer::append(boost::uint16_t length)
{
//    GNASH_REPORT_FUNCTION;
    
    if ((_seekptr + sizeof(boost::uint16_t)) <= (_ptr + _nbytes)) {
	Network::byte_t *data = reinterpret_cast<Network::byte_t *>(&length);
	std::copy(data, data + sizeof(boost::uint16_t), _seekptr);
	_seekptr += sizeof(boost::uint16_t);
	return _seekptr;
    }
    return 0;
}

Network::byte_t *
Buffer::append(gnash::Network::byte_t *data, size_t nbytes)
{
//    GNASH_REPORT_FUNCTION;

    if ((_seekptr + nbytes) <= (_ptr + _nbytes)) {
	std::copy(data, data + nbytes, _seekptr);    
	_seekptr += nbytes;
	return _seekptr;
    }
    return 0;
}

Network::byte_t *
Buffer::append(double num)
{
//    GNASH_REPORT_FUNCTION;
    if ((_seekptr + sizeof(double)) <= (_ptr + _nbytes)) {
	Network::byte_t *ptr = reinterpret_cast<Network::byte_t *>(&num);
	std::copy(ptr, ptr + amf::AMF0_NUMBER_SIZE, _seekptr);    
	_seekptr += amf::AMF0_NUMBER_SIZE;
	return _seekptr;
    }
    return 0;
}

Network::byte_t *
Buffer::append(bool val)
{
//    GNASH_REPORT_FUNCTION;
    if ((_seekptr + sizeof(bool)) <= (_ptr + _nbytes)) {
	*_seekptr = val;
	_seekptr += sizeof(bool);
	return _seekptr;
    }
    return 0;
}

Network::byte_t *
Buffer::append(boost::uint32_t num)
{
//    GNASH_REPORT_FUNCTION;
    Network::byte_t *ptr = reinterpret_cast< Network::byte_t *>(&num);    
    return append(ptr, sizeof(boost::uint32_t));
}

Network::byte_t *
Buffer::append(Network::byte_t byte)
{
//    GNASH_REPORT_FUNCTION;
    if ((_seekptr + sizeof(gnash::Network::byte_t)) <= (_ptr + _nbytes)) {
	*_seekptr = byte;
	_seekptr += sizeof(gnash::Network::byte_t);
	return _seekptr;
    }
    return 0;
}

Network::byte_t *
Buffer::append(const std::string &str)
{
//    GNASH_REPORT_FUNCTION;
    if ((_seekptr + str.size()) <= (_ptr + _nbytes)) {
	std::copy(str.begin(), str.end(), _seekptr);    
	_seekptr += str.size();
	return _seekptr;
    }
    return 0;
}

Network::byte_t *
Buffer::append(amf::Element::amf0_type_e type)
{
    return append(static_cast<Network::byte_t>(type));
}

Network::byte_t *
Buffer::append(Buffer &buf)
{
//    GNASH_REPORT_FUNCTION;
    return append(&buf);
}

Network::byte_t *
Buffer::append(Buffer *buf)
{
//    GNASH_REPORT_FUNCTION;
    size_t diff = _seekptr - _ptr;
    
    if (buf->size() > (_nbytes - diff)) {
         resize(buf->size() + diff);
    }
    
    std::copy(buf->begin(), buf->end(), _seekptr);
    _seekptr += buf->size();

    return _seekptr;
}

// make ourselves be able to appended too. If the source
// buffer is larger than the current buffer has room for,
// resize ourselves (which copies the data too) to make
// enough room.
// note that using this may have a performance hit due to
// the resize operation, which has to copy data.
Buffer &
Buffer::operator+=(Buffer &buf)
{
    return operator+=(&buf);
}

Buffer &
Buffer::operator+=(Buffer *buf)
{
//    GNASH_REPORT_FUNCTION;
    size_t diff = 0;
    if (buf->size() >= _nbytes) {
	diff = _seekptr - _ptr;
	resize(buf->size() + diff);
    }
    
    if ((_seekptr + buf->size()) <= (_ptr + _nbytes)) {
	std::copy(buf->begin(), buf->end(), _seekptr);
	_seekptr += buf->size();
    }
    return *this;
}

Buffer &
Buffer::operator+=(char byte)
{
//    GNASH_REPORT_FUNCTION;
    return operator+=(static_cast<Network::byte_t>(byte));
}

Buffer &
Buffer::operator+=(Network::byte_t byte)
{
//    GNASH_REPORT_FUNCTION;
    if ((_seekptr + 1) <= (_ptr + _nbytes)) {
	*_seekptr = byte;
	_seekptr += sizeof(char);
    }
    return *this;
}

// make ourselves be able to be copied.
Buffer &
Buffer::operator=(Buffer *buf)
{
//    GNASH_REPORT_FUNCTION;
    if (buf->size() > _nbytes) {
         resize(buf->size());
    }
    
    std::copy(buf->begin(), buf->end(), _ptr);
    _seekptr += buf->size();

    return *this;
}

Buffer &
Buffer::operator=(Buffer &buf)
{
//    GNASH_REPORT_FUNCTION;
    if (buf.size() != _nbytes) {
         resize(buf.size());
    }
    
    std::copy(buf.begin(), buf.end(), _ptr);

    return *this;
}

// Check to see if two Buffer objects are identical
bool
Buffer::operator==(Buffer *buf)
{ 
//    GNASH_REPORT_FUNCTION;
    Network::byte_t *bufptr = buf->reference();
    if (buf->size() == _nbytes) {
        if (memcmp(bufptr, _ptr, _nbytes) == 0)  {
            return true;
        }
    }
    return false;
}

bool
Buffer::operator==(Buffer &buf)
{
//    GNASH_REPORT_FUNCTION;
//     Network::byte_t *bufptr = buf.reference();
//     size_t max = 0;
    
//     if (buf.size() == _nbytes){
//         if (memcmp(bufptr, _ptr, _nbytes) == 0) {
//             return true;
//         }
//     }
//     return false;
    return operator==(&buf);
}

Network::byte_t *
Buffer::find(Network::byte_t *b, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    for (size_t i=0; i< _nbytes; i++) {
	if (memcmp((_ptr + i), b, size) == 0) {
	    return _ptr + i;
	}
    }
    return 0;
}

Network::byte_t *
Buffer::find(Network::byte_t c)
{
//    GNASH_REPORT_FUNCTION;
    for (size_t i=0; i< _nbytes; i++) {
	if (*(_ptr + i) == c) {
	    return _ptr + i;
	}
    }
    return 0;
}

Network::byte_t *
Buffer::remove(Network::byte_t c)
{
//    GNASH_REPORT_FUNCTION;
    Network::byte_t *start = find(c);

    log_debug("Byte is at %x", (void *)start);
    
    if (start == 0) {
	return 0;
    }
    
    std::copy(start + 1, end(), start);
//    *(end()) = 0;
    _nbytes--;

    return _ptr;
}

Network::byte_t *
Buffer::remove(int start)
{
//    GNASH_REPORT_FUNCTION;
    std::copy((_ptr + start + 1), end(), (_ptr + start)),
//    *end() = 0;
    _nbytes--;
    return _ptr;
}

Network::byte_t *
Buffer::remove(int start, int stop)
{
//    GNASH_REPORT_FUNCTION;
    std::copy((_ptr + stop + 1), end(), (_ptr + start)),
//    *end() = 0;
    _nbytes -= stop-start;
    return _ptr;
}

// Just reset to having no data, but still having storage
void
Buffer::clear()
{
//    GNASH_REPORT_FUNCTION;
    if (_ptr) {
        memset(_ptr, 0, _nbytes);
    }
    _seekptr = _ptr;
}

// Resize the buffer that holds the data.
// Resize the buffer that holds the data.
void *
Buffer::resize()
{
//    GNASH_REPORT_FUNCTION;
    return resize(_seekptr - _ptr);
}

void *
Buffer::resize(size_t size)
{
//    GNASH_REPORT_FUNCTION;
    // Allocate a new memory block
    if (_nbytes == 0) {
	init(size);
    } else {
	size_t diff =_seekptr - _ptr;
	Network::byte_t *tmp = new Network::byte_t[size];
	// The size is the same, don't do anything.
	if (size == _nbytes) {
	    return _ptr;
	}
	// And copy ourselves into it
	if (size > _nbytes) {
	    std::copy(_ptr, _ptr + _nbytes, tmp);
	    // Delete the old block, it's unused now
	    delete[] _ptr;
	    // Make the memory block use the new space
	    _ptr = tmp;
	    // Make the seekptr point into the new space with the correct offset
	    _seekptr = tmp + diff;
	}
	if (size < _nbytes) {
	    std::copy(_ptr, _ptr + size, tmp);
	    // Delete the old block, it's unused now
	    delete[] _ptr;
	    // Make the memory block use the new space
	    _ptr = tmp;
	    // Make the seekptr point into the new space with the correct offset
	    _seekptr = _ptr + size;
	}
    }
    // Adjust the size
    _nbytes = size;
    
    return _ptr;
}

void
Buffer::dump()
{
    cerr << "Buffer is " << _nbytes << " bytes at " << (void *)_ptr << endl;
    if (_nbytes < 0xffff) {
	cerr << gnash::hexify((unsigned char *)_ptr, _nbytes, false) << endl;
	cerr << gnash::hexify((unsigned char *)_ptr, _nbytes, true) << endl;
    } else {
	cerr << "ERROR: Buffer size out of range!" << endl;
	abort();
    }
}

} // end of amf namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
