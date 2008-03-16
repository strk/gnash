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

using namespace std;

namespace cygnal
{

// Adjust for the constant size
const size_t BUFFERSIZE = 128;

void *
Buffer::init(size_t nbytes)
{
//    GNASH_REPORT_FUNCTION;
    if (_ptr == 0) {
        _ptr = new boost::uint8_t[nbytes];
        _nbytes = nbytes;
        // this could be a performance hit, but for debugging we leave it in so we get
        // easier to ready hex dumps in GDB,
        empty();
    }
    
    return _ptr;
}

Buffer::Buffer() 
{
//    GNASH_REPORT_FUNCTION;
    _ptr = 0;
    _nbytes = BUFFERSIZE;
    init(BUFFERSIZE);
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
        delete[] _ptr;
        _ptr = 0;
        _nbytes = 0;
    }
}

// Put data into the buffer
void
Buffer::copy(boost::uint8_t *data, int nbytes)
{    
//    GNASH_REPORT_FUNCTION;
    std::copy(data, data + nbytes, _ptr);
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
    if (buf.size() == _nbytes){
        if (memcmp(buf.reference(), _ptr, _nbytes) == 0)  {
            return true;
        }
    }
    return false;
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
    // Allocate a new memory block
    boost::uint8_t *tmp = new boost::uint8_t[nbytes];
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
}

} // end of cygnal namespace


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
