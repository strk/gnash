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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <string>
#include <deque>

#include "log.h"
#include "buffer.h"
#include "cque.h"

using namespace gnash;
using namespace std;
using namespace boost;

namespace cygnal
{

CQue::CQue()
{
//    GNASH_REPORT_FUNCTION;
}

CQue::~CQue()
{
//    GNASH_REPORT_FUNCTION;
//    clear();
#if 0
    deque<Buffer *>::iterator it;
    boost::mutex::scoped_lock lock(_mutex);
    for (it = _que.begin(); it != _que.end(); it++) {
	Buffer *ptr = *(it);
	delete ptr;
    }
#endif
}

size_t
CQue::size()
{
//    GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lock(_mutex);
    return _que.size();
}

bool
CQue::push(Buffer *data)
{
//    GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lock(_mutex);
    _que.push_back(data);
    return true;
}

// Push bytes on the outgoing FIFO
bool
CQue::push(uint8_t *data, int nbytes)
{
//    GNASH_REPORT_FUNCTION;
    Buffer *buf = new Buffer;
    std::copy(data, data + nbytes, buf->reference());
}


// Pop the first date element off the FIFO
Buffer *
CQue::pop()
{
//    GNASH_REPORT_FUNCTION;
    Buffer *buf;
    boost::mutex::scoped_lock lock(_mutex);
    if (_que.size()) {
        buf = _que.front();
        _que.pop_front();
    }
    return buf;
}

// Peek at the first data element without removing it
Buffer *
CQue::peek()
{
//    GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lock(_mutex);
    if (_que.size()) {
        return _que.front();
    }
    return 0;	
}

// Return the size of the queues
void
CQue::clear()
{
//    GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lock(_mutex);
    _que.clear();
}
    
// Dump internal data.
void
CQue::dump()
{
//    GNASH_REPORT_FUNCTION;
    deque<Buffer *>::iterator it;
    boost::mutex::scoped_lock lock(_mutex);
    cerr << endl << "Outgoing queue has "<< _que.size() << " buffers." << endl;
    for (it = _que.begin(); it != _que.end(); it++) {
	Buffer *ptr = *(it);
        ptr->dump();
    }
}

} // end of cygnal namespace


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

