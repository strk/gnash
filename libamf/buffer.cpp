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
#include "GnashException.h"

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

// Initialize a Buffer's storage to the specified size
Buffer &
Buffer::init(size_t size)
{
//    GNASH_REPORT_FUNCTION;
    if (!_data) {
	_data.reset(new Network::byte_t[size]);
	_seekptr = _data.get();
    }
    _seekptr = _data.get();
    _nbytes = size;

    clear();			// FIXME; this is a perforance hitm but aids in debugging
#ifdef USE_STATS_BUFFERS
    clock_gettime (CLOCK_REALTIME, &_stamp);
#endif
    
    return *this;
}

Buffer::Buffer() 
    : _seekptr(0)
{
//    GNASH_REPORT_FUNCTION;
    _nbytes = gnash::NETBUFSIZE;
    init(gnash::NETBUFSIZE);
}
    
// Create with a size other than the default
Buffer::Buffer(size_t nbytes)
    : _seekptr(0)
{
//    GNASH_REPORT_FUNCTION;
    _nbytes = nbytes;
    init(nbytes);
}

// Delete the allocate memory
Buffer::~Buffer()
{
//    GNASH_REPORT_FUNCTION;
    if (_data) {
#ifdef USE_STATS_BUFFERS
	struct timespec now;
	clock_gettime (CLOCK_REALTIME, &now);
	log_debug("Buffer %x (%d) stayed in queue for %f seconds",
		  (void *)_data.get(), _nbytes,
		  (float)((now.tv_sec - _stamp.tv_sec) + ((now.tv_nsec - _stamp.tv_nsec)/1e9)));
#endif
        _seekptr = 0;
        _nbytes = 0;
    }
}

// Put data into the buffer
Buffer &
Buffer::copy(Network::byte_t *data, size_t nbytes)
{    
//    GNASH_REPORT_FUNCTION;
    if (_data) {
	std::copy(data, data + nbytes, _data.get());
	_seekptr = _data.get() + nbytes;
    } else {
	throw GnashException("Not enough storage was allocated to hold the data!");
    }
    return *this;
}


Buffer &
Buffer::append(gnash::Network::byte_t *data, size_t nbytes)
{
//    GNASH_REPORT_FUNCTION;
    if (_data) {
	if (spaceLeft() >= nbytes) {
	    std::copy(data, data + nbytes, _seekptr);
	    _seekptr += nbytes;
	} else {
	    throw GnashException("Not enough storage was allocated to hold the appended data!");
	}
    }

    return *this;
}

// make ourselves be able to appended too. If the source
// buffer is larger than the current buffer has room for,
// resize ourselves (which copies the data too) to make
// enough room.
Buffer &
Buffer::operator+=(Buffer &buf)
{
// //    GNASH_REPORT_FUNCTION;
    append(buf.reference(), buf.size());
    return *this;
}

Buffer &
Buffer::operator+=(amf::Element::amf0_type_e type)
{
//    GNASH_REPORT_FUNCTION;
    Network::byte_t nb = static_cast<Network::byte_t>(type);
    
    return operator+=(type);
}

Buffer &
Buffer::operator+=(char byte)
{
//    GNASH_REPORT_FUNCTION;
    Network::byte_t nb = static_cast<Network::byte_t>(byte);
    return operator+=(nb);
}

Buffer &
Buffer::operator+=(bool flag)
{
//    GNASH_REPORT_FUNCTION;
    Network::byte_t nb = static_cast<Network::byte_t>(flag);
    return operator+=(nb);
}

Buffer &
Buffer::operator+=(Network::byte_t byte)
{
//    GNASH_REPORT_FUNCTION;
    if ((_seekptr + 1) <= (_data.get() + _nbytes)) {
	*_seekptr = byte;
	_seekptr += sizeof(char);
    }
    return *this;
}

Buffer &
Buffer::operator+=(const char *str)
{
//    GNASH_REPORT_FUNCTION;
    Network::byte_t *ptr = const_cast<Network::byte_t *>(reinterpret_cast<const Network::byte_t *>(str));
    return append(ptr, strlen(str));
    
}

Buffer &
Buffer::operator+=(const std::string &str)
{
//    GNASH_REPORT_FUNCTION;
    Network::byte_t *ptr = const_cast<Network::byte_t *>(reinterpret_cast<const Network::byte_t *>(str.c_str()));
    return append(ptr, str.size());
    
}

Buffer &
Buffer::operator+=(double num)
{
//    GNASH_REPORT_FUNCTION;
    Network::byte_t *ptr = reinterpret_cast<Network::byte_t *>(&num);
    return append(ptr, AMF0_NUMBER_SIZE);
}

Buffer &
Buffer::operator+=(boost::uint16_t length)
{
//    GNASH_REPORT_FUNCTION;
    Network::byte_t *ptr = reinterpret_cast<Network::byte_t *>(&length);
    return append(ptr, sizeof(boost::uint16_t));
}


Buffer &
Buffer::operator=(Buffer &buf)
{
//    GNASH_REPORT_FUNCTION;
    if (buf.size() != _nbytes) {
	resize(buf.size());
    }
    copy(buf.reference(), buf.size());

    return *this;
}

Buffer &
Buffer::operator=(const std::string &str)
{
//    GNASH_REPORT_FUNCTION;
    Network::byte_t *ptr = const_cast<Network::byte_t *>(reinterpret_cast<const Network::byte_t *>(str.c_str()));
    return copy(ptr, str.size());
}

Buffer &
Buffer::operator=(const char *str)
{
//    GNASH_REPORT_FUNCTION;
    Network::byte_t *ptr = const_cast<Network::byte_t *>(reinterpret_cast<const Network::byte_t *>(str));
    return copy(ptr, strlen(str));
}

Buffer &
Buffer::operator=(double num)
{
//    GNASH_REPORT_FUNCTION;
    Network::byte_t *ptr = reinterpret_cast<Network::byte_t *>(&num);
    return copy(ptr, AMF0_NUMBER_SIZE);
}

Buffer &
Buffer::operator=(boost::uint16_t length)
{
//    GNASH_REPORT_FUNCTION;
    Network::byte_t *ptr = reinterpret_cast<Network::byte_t *>(&length);
    return copy(ptr, sizeof(boost::uint16_t));
}

Buffer &
Buffer::operator=(amf::Element::amf0_type_e type)
{
    Network::byte_t nb = static_cast<Network::byte_t>(type);
    return operator+=(nb);
}

Buffer &
Buffer::operator=(bool flag)
{
    Network::byte_t nb = static_cast<Network::byte_t>(flag);
    return operator=(nb);
}

Buffer &
Buffer::operator=(gnash::Network::byte_t byte)
{
//    GNASH_REPORT_FUNCTION;
    return copy(&byte, 1);
}

Buffer &
Buffer::operator=(gnash::Network::byte_t *data)
{
//    GNASH_REPORT_FUNCTION;
    if (data) {
	_data.reset(data);
    } else {
	throw ParserException("Passing invalid pointer!");
    }
    
    return *this;
}

// Check to see if two Buffer objects are identical
// bool
// Buffer::operator==(Buffer *buf)
// { 
// //    GNASH_REPORT_FUNCTION;
//     Network::byte_t *bufptr = buf->reference();
//     if (buf->size() == _nbytes) {
//         if (memcmp(bufptr, _data.get(), _nbytes) == 0)  {
//             return true;
//         }
//     }
//     return false;
// }

bool
Buffer::operator==(Buffer &buf)
{
//    GNASH_REPORT_FUNCTION;
     if (buf.size() == _nbytes){
         if (memcmp(buf.reference(), _data.get(), _nbytes) == 0) {
             return true;
         }
     }
     return false;
}

Network::byte_t *
Buffer::find(Network::byte_t *b, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    for (size_t i=0; i< _nbytes; i++) {
	if (memcmp((_data.get() + i), b, size) == 0) {
	    return _data.get() + i;
	}
    }
    return 0;
}

Network::byte_t *
Buffer::find(Network::byte_t c)
{
//    GNASH_REPORT_FUNCTION;
    for (size_t i=0; i< _nbytes; i++) {
	if (*(_data.get() + i) == c) {
	    return _data.get() + i;
	}
    }
    return 0;
}

Network::byte_t *
Buffer::remove(Network::byte_t c)
{
//    GNASH_REPORT_FUNCTION;
    Network::byte_t *start = find(c);

//    log_debug("Byte is at %x", (void *)start);
    
    if (start == 0) {
	return 0;
    }
    
    std::copy(start + 1, end(), start);
//    *(end()) = 0;
    _nbytes--;

    return _data.get();
}

Network::byte_t *
Buffer::remove(int start)
{
//    GNASH_REPORT_FUNCTION;
    std::copy((_data.get() + start + 1), end(), (_data.get() + start)),
//    *end() = 0;
    _nbytes--;
    return _data.get();
}

Network::byte_t *
Buffer::remove(int start, int stop)
{
//    GNASH_REPORT_FUNCTION;
    std::copy((_data.get() + stop + 1), end(), (_data.get() + start)),
//    *end() = 0;
    _nbytes -= stop-start;
    return _data.get();
}

// Just reset to having no data, but still having storage
void
Buffer::clear()
{
//    GNASH_REPORT_FUNCTION;
    if (_data.get()) {
        memset(_data.get(), 0, _nbytes);
    }
    _seekptr = _data.get();
}

// Resize the buffer that holds the data.
Buffer &
Buffer::resize()
{
//    GNASH_REPORT_FUNCTION;
    return resize(_seekptr - _data.get());
}

// Resize the block of memory used for this Buffer.
Buffer &
Buffer::resize(size_t size)
{
//    GNASH_REPORT_FUNCTION;
    boost::scoped_array<gnash::Network::byte_t> tmp;
    
    if (_nbytes == 0) {
	return init(size);
    } else {
	// Don't bother to resize without really changing anything
	if (size == _nbytes) {
	    return *this;
	}

	// Cache the number of bytes currently being held
	size_t used = _seekptr - _data.get();
	
	// Copy the existing data into the new block of memory. The data
	// held currently is moved to the temporary array, and then gets
	// deleted when this method returns.
	tmp.swap(_data);
	
	// We loose data if we resize smaller than the data currently held.
	if (size < used) {
	    log_error("Truncating data (%d bytes) while resizing!", used - size);
	    used = size;
	}
	_data.reset(new Network::byte_t[size]);
	std::copy(tmp.get(), tmp.get() + used, _data.get());
	
	// Make the seekptr point into the new space with the correct offset
	_seekptr = _data.get() + used;

	// Adjust the size
	_nbytes = size;
    }
    
    return *this;
}

void
Buffer::dump()
{
    cerr << "Buffer is " << _nbytes << " bytes at " << (void *)_data.get() << endl;
    if (_nbytes < 0xffff) {
	cerr << gnash::hexify((unsigned char *)_data.get(), _nbytes, false) << endl;
	cerr << gnash::hexify((unsigned char *)_data.get(), _nbytes, true) << endl;
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
