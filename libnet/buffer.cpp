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
#include "buffer.h"
#include "log.h"
#include "network.h"

using namespace std;
using namespace gnash;

namespace gnash
{

void *
Buffer::init(size_t nbytes)
{
//    GNASH_REPORT_FUNCTION;
    if (_ptr == 0) {
        _ptr = new Network::byte_t[nbytes];
        _nbytes = nbytes;
        // this could be a performance hit, but for debugging we leave it in so we get
        // easier to ready hex dumps in GDB,
        empty();
    }

#ifdef USE_STATS_BUFFERS
    clock_gettime (CLOCK_REALTIME, &_stamp);
#endif
    return _ptr;
}

Buffer::Buffer() 
{
//    GNASH_REPORT_FUNCTION;
    _ptr = 0;
    _nbytes = gnash::NETBUFSIZE;
    init(gnash::NETBUFSIZE);
}
    
// Create with a size other than the default
Buffer::Buffer(size_t nbytes)
{
//    GNASH_REPORT_FUNCTION;
    _ptr = 0;
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
        _ptr = 0;
        _nbytes = 0;
    }
}

// Put data into the buffer
void
Buffer::copy(Network::byte_t *data, size_t nbytes)
{    
//    GNASH_REPORT_FUNCTION;
    std::copy(data, data + nbytes, _ptr);
}

void
Buffer::copy(string &str)
{    
//    GNASH_REPORT_FUNCTION;
    std::copy(str.begin(), str.end(), _ptr);
}

// make ourselves be able to be copied.
Buffer &
Buffer::operator=(Buffer *buf)
{
//    GNASH_REPORT_FUNCTION;
    if (buf->size() != _nbytes) {
         resize(buf->size());
    }
    
    std::copy(buf->reference(), buf->reference() + _nbytes, _ptr);

    return *this;
}

Buffer &
Buffer::operator=(Buffer &buf)
{
//    GNASH_REPORT_FUNCTION;
    if (buf.size() != _nbytes) {
         resize(buf.size());
    }
    
    std::copy(buf.reference(), buf.reference() + _nbytes, _ptr);

    return *this;
}

// Check to see if two Buffer objects are identical
bool
Buffer::operator==(Buffer *buf)
{ 
//    GNASH_REPORT_FUNCTION;
   if (buf->size() == _nbytes) {
        if (memcmp(buf->reference(), _ptr, _nbytes) == 0)  {
            return true;
        }
    }
    return false;
}

bool
Buffer::operator==(Buffer &buf)
{
//    GNASH_REPORT_FUNCTION;
    if (buf.size() == _nbytes){
        if (memcmp(buf.reference(), _ptr, _nbytes) == 0)  {
            return true;
        }
    }
    return false;
}

Network::byte_t *
Buffer::find(Network::byte_t b, size_t start)
{
    GNASH_REPORT_FUNCTION;
    for (size_t i=start; i< _nbytes; i++) {
	if ( *(_ptr + i) == b) {
	    return _ptr + i;
	}
    }
    return 0;
}

// Find a byte in the buffer
// Network::byte_t *
// Buffer::find(char c)
// {
// //    GNASH_REPORT_FUNCTION;
//     return find(static_cast<Network::byte_t>(c), 0);
// }

Network::byte_t *
Buffer::find(Network::byte_t c)
{
//    GNASH_REPORT_FUNCTION;
    return find(static_cast<Network::byte_t>(c), 0);
}   

// // Drop a character or range of characters without resizing
// Network::byte_t
// Buffer::remove(char c)
// {
// //    GNASH_REPORT_FUNCTION;
//     return remove(reinterpret_cast<Network::byte_t>(c));
// }

Network::byte_t *
Buffer::remove(Network::byte_t c)
{
    GNASH_REPORT_FUNCTION;
    Network::byte_t *start = find(c, 0);
    log_debug("FRAME MARK is at %x", (void *)start);
    if (start == 0) {
	return 0;
    }
//    std::copy((start + 1), end(), start);
    *start = '*';
//    *end() = 0;
//    _nbytes--;

    return _ptr;
}

Network::byte_t *
Buffer::remove(int start)
{
//    GNASH_REPORT_FUNCTION;
    std::copy((_ptr + start + 1), end(), (_ptr + start)),
    *end() = 0;
    _nbytes--;
    return _ptr;
}

Network::byte_t *
Buffer::remove(int start, int stop)
{
//    GNASH_REPORT_FUNCTION;
    std::copy((_ptr + start), end(), (_ptr + stop)),
//    *end() = 0;
    _nbytes--;
    return _ptr;
}

// Just reset to having no data, but still having storage
void
Buffer::empty()
{
//    GNASH_REPORT_FUNCTION;
    if (_ptr) {
        memset(_ptr, 0, _nbytes);
    }
}

// Resize the buffer that holds the data
void *
Buffer::resize(size_t nbytes)
{
//    GNASH_REPORT_FUNCTION;
    // Allocate a new memory block
    Network::byte_t *tmp = new Network::byte_t[nbytes];
    // And copy ourselves into it
    if (nbytes > _nbytes) {
        std::copy(_ptr, _ptr + _nbytes, tmp);
    }
    
    if (nbytes < _nbytes) {
        std::copy(_ptr, _ptr + nbytes, tmp);
    }

    _nbytes = nbytes;

    // Delete the old block, it's unused now
    delete[] _ptr;

    // Make the memeory block use the new space
    _ptr = tmp;

    return tmp;
}

void
Buffer::dump()
{
    cerr << "Buffer is " << _nbytes << " bytes at " << (void *)_ptr << endl;
    cerr << gnash::hexify((unsigned char *)_ptr, _nbytes, true) << endl;
}

} // end of gnash namespace


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
