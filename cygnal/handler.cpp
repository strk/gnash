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

#include <boost/thread/mutex.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
//#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
//#include <boost/date_time/time_zone_base.hpp>
#include <string>
#include <deque>

#include "log.h"
#include "buffer.h"
#include "handler.h"

using namespace gnash;
using namespace std;
using namespace boost;

namespace cygnal
{

Handler::Handler()
{
//    GNASH_REPORT_FUNCTION;
}

Handler::~Handler()
{
//    GNASH_REPORT_FUNCTION;
#if 0
    deque<Buffer *>::iterator it;
    for (it = _incoming.begin(); it != _incoming.end(); it++) {
	Buffer *ptr = *(it);
//	delete ptr;
    }
    for (it = _outgoing.begin(); it != _outgoing.end(); it++) {
	Buffer *ptr = *(it);
	delete ptr;
    }
#endif
}

bool
Handler::push(Buffer *data, fifo_e direction)
{
//    GNASH_REPORT_FUNCTION;
    if (direction == Handler::OUTGOING) {
	_outgoing.push_back(data);
	return true;
    }
    if (direction == Handler::INCOMING) {
	_incoming.push_back(data);
	return true;
    }
    
    return false;
}

// Push bytes on the outgoing FIFO
bool
Handler::push(uint8_t *data, int nbytes, fifo_e direction)
{
//    GNASH_REPORT_FUNCTION;
    Buffer *ptr = new Buffer;
    return push(ptr, direction);
}

// Pop the first date element off the FIFO
Buffer *
Handler::pop(fifo_e direction)
{
//    GNASH_REPORT_FUNCTION;
    Buffer *buf;
    
    if (direction == Handler::OUTGOING) {
	if (_outgoing.size()) {
	    buf = _outgoing.front();
	    _outgoing.pop_front();
	}
    }
    if (direction == Handler::INCOMING) {
	if (_incoming.size()) {
	    buf = _incoming.front();
	    _incoming.pop_front();
	}
    }

    return buf;
}

// Peek at the first data element without removing it
Buffer *
Handler::peek(fifo_e direction)
{
//    GNASH_REPORT_FUNCTION;
    if (direction == Handler::OUTGOING) {
	if (_outgoing.size()) {
	    return _outgoing.front();
	}
    }
    if (direction == Handler::INCOMING) {
	if (_incoming.size()) {
	    return _incoming.front();
	}
    }    
}

// Return the size of the queues
size_t
Handler::size(fifo_e direction)
{
//    GNASH_REPORT_FUNCTION;
    if (direction == Handler::OUTGOING) {
	return _outgoing.size();
    }
    if (direction == Handler::INCOMING) {
	return _incoming.size();
    }
    
    return 0;			// we should never actually get to here
}

// Return the size of the queues
void
Handler::clear(fifo_e direction)
{
//    GNASH_REPORT_FUNCTION;
    if (direction == Handler::OUTGOING) {
	_outgoing.clear();
    }
    if (direction == Handler::INCOMING) {
	_incoming.clear();
    }    
}

// Dump internal data.
void
Handler::dump()
{
//    GNASH_REPORT_FUNCTION;
    deque<Buffer *>::iterator it;
    cerr << "Incoming queue has "<< _incoming.size() << " buffers." << endl;
    for (it = _incoming.begin(); it != _incoming.end(); it++) {
	Buffer *ptr = *(it);
        ptr->dump();
    }
    cerr << endl << "Outgoing queue has "<< _outgoing.size() << " buffers." << endl;
    for (it = _outgoing.begin(); it != _outgoing.end(); it++) {
	Buffer *ptr = *(it);
        ptr->dump();
    }
}

} // end of cygnal namespace


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
