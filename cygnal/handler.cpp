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

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>
#include <string>
#include <deque>

#include "log.h"
#include "network.h"
#include "buffer.h"
#include "handler.h"

#include "http.h"

using namespace gnash;
using namespace std;
using namespace boost;

namespace cygnal
{

Handler::Handler()
    : _die(false), _netfd(0)
{
    GNASH_REPORT_FUNCTION;
}

Handler::~Handler()
{
    GNASH_REPORT_FUNCTION;
    closeNet();
    _die = true;
    notifyout();
    notifyin();
}

bool
Handler::push(Buffer *data, fifo_e direction)
{
//    GNASH_REPORT_FUNCTION;
    if (direction == Handler::OUTGOING) {
	_outgoing.push(data);
	return true;
    }
    if (direction == Handler::INCOMING) {
	_incoming.push(data);
	return true;
    }
    
    return false;
}

// Push bytes on the outgoing FIFO
bool
Handler::push(gnash::Network::byte_t *data, int nbytes, fifo_e direction)
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
	    buf = _outgoing.pop();
	    return buf;	
	}
    }
    if (direction == Handler::INCOMING) {
	if (_incoming.size()) {
	    buf = _incoming.pop();
	    return buf;	
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
	    return _outgoing.peek();
	}
    }
    if (direction == Handler::INCOMING) {
	if (_incoming.size()) {
	    return _incoming.peek();
	}
    }    
    return 0;
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

// start the two thread handlers for the queues
bool
Handler::start(thread_params_t *args)
{
//    GNASH_REPORT_FUNCTION;
    int retries = 10;

    _incoming.setName("Incoming");
    _outgoing.setName("Outgoing");
//    toggleDebug(true);		// FIXME:
    createServer(args->port);
    while (retries-- > 0) {
	log_debug(_("%s: Starting Handlers for port %d"), __PRETTY_FUNCTION__, args->port);
	newConnection(true);
	args->netfd = getFileFd();
	args->handle = this;

	log_debug("Starting thread 1");
	boost::thread handler(boost::bind(&httphandler, args));

#if 1
	log_debug("Starting thread 2");
	boost::thread outport(boost::bind(&netout_handler, args));
#endif
	
	log_debug("Starting thread 3");
	boost::thread inport(boost::bind(&netin_handler, args));
	inport.join();    
//	outport.join();
	handler.join();
	if (_die) {
	    break;
	}
    }
}
    
// Dump internal data.
void
Handler::dump()
{
//    GNASH_REPORT_FUNCTION;
    _incoming.dump();
    _outgoing.dump();    
}

extern "C" {
void
netin_handler(Handler::thread_params_t *args)
{
    GNASH_REPORT_FUNCTION;

    Handler *hand = reinterpret_cast<Handler *>(args->handle);

    int retries = 3;
    while (retries-- >  0) {
	Buffer *buf = new Buffer;
	int ret = hand->readNet(buf->reference(), buf->size());
	if (ret >= 0) {
	    if (ret != buf->size()) {
		buf->resize(ret);
	    }
	    hand->push(buf);
//  	    string str = (const char *)buf->reference();
//  	    cerr << str << endl;
	    hand->notify();
	} else {
	    log_debug("exiting, no data");
	    hand->die();
	    break;
	}
    }
    hand->notify();
    hand->clearall();
//    hand->dump();
}
void
netout_handler(Handler::thread_params_t *args)
{
    GNASH_REPORT_FUNCTION;
    int retries = 10;
    int ret;
    Handler *hand = reinterpret_cast<Handler *>(args->handle);
    do {
	hand->waitout();
	while (hand->outsize()) {
	    Buffer *buf = hand->popout();
//	    log_debug("FIXME: got data in Outgoing que");
//	    buf->dump();
	    ret = hand->writeNet(buf);
	    delete buf;
	}
	if (hand->timetodie()) {
	    break;
	}
    } while (ret >= 0);    
}

} // end of extern C

} // end of cygnal namespace


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

