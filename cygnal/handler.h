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

#ifndef __HANDLER_H__
#define __HANDLER_H__ 1

#include <boost/cstdint.hpp>
#include <boost/thread/condition.hpp>
#include <string>
#include <deque>

#include "log.h"
#include "buffer.h"

// _definst_ is the default instance name
namespace cygnal
{

class Handler 
{
public:
    Handler();
    ~Handler();

    // This is used to pass parameters to a thread using boost::bind
    typedef struct {
	int netfd;
	int port;
	std::string filespec;
    } thread_params_t ;
    
    // Specify which queue should be used
    typedef enum { INCOMING, OUTGOING } fifo_e;
    
    // Push bytes on the incoming FIFO, which is the default
    bool push(boost::uint8_t *data, int nbytes, fifo_e direction);
    bool push(Buffer *data, fifo_e direction);
    bool push(Buffer *data)
	{ return push(data, INCOMING); };
    bool push(boost::uint8_t *data, int nbytes)
	{ return push(data, nbytes, INCOMING); };
    bool pushin(boost::uint8_t *data, int nbytes)
	{ return push(data, nbytes, INCOMING); };
    bool pushin(Buffer *data)
	{ return push(data, INCOMING); };
    
    // Push bytes on the incoming FIFO, which must be specified
    bool pushout(boost::uint8_t *data, int nbytes)
	{ return push(data, nbytes, OUTGOING); };
    bool pushout(Buffer *data)
	{ return push(data, OUTGOING); };
    
    // Pop the first date element off the incoming FIFO
    Buffer *pop(fifo_e direction);
    Buffer *pop()
    	{ return pop(INCOMING); };
    Buffer *popin()
    	{ return pop(INCOMING); };
    // Pop the first date element off the outgoing FIFO
    Buffer *popout()
    	{ return pop(OUTGOING); };
    
    // Peek at the first data element without removing it
    Buffer *peek(fifo_e direction);
    Buffer *peek()
    	{ return peek(INCOMING); };
    Buffer *peekin()
    	{ return peek(INCOMING); };
    // Pop the first date element off the outgoing FIFO
    Buffer *peekout()
    	{ return peek(OUTGOING); };    

    // Removes all the buffers from the queues
    void clear() { _incoming.clear(); };
    void clear(fifo_e direction);
    void clearin() { _incoming.clear(); };
    void clearout() { _outgoing.clear(); };
    void clearall() { _outgoing.clear(); _incoming.clear(); };
    
    // Return the size of the queues, default to the incoming queue
    size_t size(fifo_e direction);
    size_t size() { return size(INCOMING); };
    size_t insize() { return _incoming.size(); };
    size_t outsize() { return _outgoing.size(); };
    
    // Dump internal data.
    void dump();
private:
    int _netfd;
    boost::condition _inmutex;
    std::deque<Buffer *> _incoming;
    
    std::deque<Buffer *> _outgoing;
    boost::condition _outmutex;
};

// This is the thread for all incoming network connections, which
// has to be in C.
extern "C" {
    void nethandler(Handler::thread_params_t *args);
}

} // end of cygnal namespace

#endif // end of __HANDLER_H__

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
