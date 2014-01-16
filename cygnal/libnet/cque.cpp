// 
//   Copyright (C) 2008, 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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
#include <vector>
#include <deque>

#include "cque.h"
#include "log.h"
#include "gmemory.h"
#include "buffer.h"

using std::deque;

namespace gnash
{

CQue::CQue()
{
//    GNASH_REPORT_FUNCTION;
#ifdef USE_STATS_QUEUE
    _stats.totalbytes = 0;
    _stats.totalin = 0;
    _stats.totalout = 0;
    clock_gettime (CLOCK_REALTIME, &_stats.start);
#endif
    _name = "default";
}

CQue::~CQue()
{
//    GNASH_REPORT_FUNCTION;
//    clear();
    que_t::iterator it;
    boost::mutex::scoped_lock lock(_mutex);
//     for (it = _que.begin(); it != _que.end(); it++) {
// 	boost::shared_ptr<cygnal::Buffer> ptr = *(it);
// 	if (ptr->size()) {	// FIXME: we probably want to delete ptr anyway,
// 	    delete ptr;		// but if we do, this will core dump.
// 	}
//     }
}

// Wait for a condition variable to trigger
void
CQue::wait()
{
//    GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lk(_cond_mutex);
#ifndef _WIN32
    _cond.wait(lk);
    log_unimpl(_("CQue::wait(win32)"));
#endif
//    log_debug("wait mutex released for \"%s\"", _name);
}

// Notify a condition variable to trigger
void
CQue::notify()
{
//    GNASH_REPORT_FUNCTION;
#ifndef _WIN32
    _cond.notify_one();
    log_unimpl(_("CQue::notify(win32)"));
#endif
//    log_debug("wait mutex triggered for \"%s\"", _name);
}

size_t
CQue::size()
{
//    GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lock(_mutex);
    return _que.size();
}

bool
CQue::push(boost::shared_ptr<cygnal::Buffer> data)
{
//     GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lock(_mutex);
    _que.push_back(data);
#ifdef USE_STATS_QUEUE
    _stats.totalbytes += data->size();
    _stats.totalin++;
#endif
    return true;
}

// Push data
bool
CQue::push(boost::uint8_t *data, int nbytes)
{
//    GNASH_REPORT_FUNCTION;
    boost::shared_ptr<cygnal::Buffer> buf(new cygnal::Buffer);
    std::copy(data, data + nbytes, buf->reference());
    return push(buf);
}


// Pop the first date element off the FIFO
boost::shared_ptr<cygnal::Buffer> 
CQue::pop()
{
//    GNASH_REPORT_FUNCTION;
    boost::shared_ptr<cygnal::Buffer> buf;
    boost::mutex::scoped_lock lock(_mutex);
    if (_que.size()) {
        buf = _que.front();
        _que.pop_front();
#ifdef USE_STATS_QUEUE
	_stats.totalout++;
#endif
    }
    return buf;
}

// Peek at the first data element without removing it
boost::shared_ptr<cygnal::Buffer> 
CQue::peek()
{
//    GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lock(_mutex);
    if (_que.size()) {
        return _que.front();
    }
    return boost::shared_ptr<cygnal::Buffer>();
}

// Return the size of the queues
void
CQue::clear()
{
//    GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lock(_mutex);
    _que.clear();
}

// Remove a range of elements
void
CQue::remove(boost::shared_ptr<cygnal::Buffer> begin, boost::shared_ptr<cygnal::Buffer> end)
{
    GNASH_REPORT_FUNCTION;
    deque<boost::shared_ptr<cygnal::Buffer> >::iterator it;
    deque<boost::shared_ptr<cygnal::Buffer> >::iterator start;
    deque<boost::shared_ptr<cygnal::Buffer> >::iterator stop;
    boost::mutex::scoped_lock lock(_mutex);
    boost::shared_ptr<cygnal::Buffer> ptr;
    for (it = _que.begin(); it != _que.end(); ++it) {
	ptr = *(it);
	if (ptr->reference() == begin->reference()) {
	    start = it;
	}
	if (ptr->reference() == end->reference()) {
	    stop = it;
	    break;
	}
    }
    _que.erase(start, stop);
}

// Remove an element
void
CQue::remove(boost::shared_ptr<cygnal::Buffer> element)
{
    GNASH_REPORT_FUNCTION;
    deque<boost::shared_ptr<cygnal::Buffer> >::iterator it;
    boost::mutex::scoped_lock lock(_mutex);
    for (it = _que.begin(); it != _que.end(); ) {
	boost::shared_ptr<cygnal::Buffer> ptr = *(it);
	if (ptr->reference() == element->reference()) {
	    it = _que.erase(it);
	} else {
	    ++it;
	}
    }
}

// Merge sucessive buffers into one single larger buffer. This is for some
// protocols, than have very long headers.
boost::shared_ptr<cygnal::Buffer> 
CQue::merge()
{
//     GNASH_REPORT_FUNCTION;
    
    return merge(_que.front());
}

boost::shared_ptr<cygnal::Buffer> 
CQue::merge(boost::shared_ptr<cygnal::Buffer> start)
{
//     GNASH_REPORT_FUNCTION;
    // Find iterator to first element to merge
    que_t::iterator from = std::find(_que.begin(), _que.end(), start); 
    if (from == _que.end()) {
        // Didn't find the requested Buffer pointer
        return start;		// FIXME:
    }

    // Find iterator to last element to merge (first with size < NETBUFSIZE)
    // computing total size with the same scan
    size_t totalsize = (*from)->size();
    que_t::iterator to=from; ++to;
    for (que_t::iterator e=_que.end(); to!=e; ++to) {
        size_t sz = (*to)->size();
        totalsize += sz;
// 	log_debug("%s: Totalsize is %s", __PRETTY_FUNCTION__, totalsize);
        if (sz < cygnal::NETBUFSIZE) {
	    break;
	}
    }
    totalsize += 24;
//     log_debug("%s: Final Totalsize is %s", __PRETTY_FUNCTION__, totalsize);
    
    // Merge all elements in a single buffer. We have totalsize now.
    boost::shared_ptr<cygnal::Buffer> newbuf(new cygnal::Buffer(totalsize));
    for (que_t::iterator i=from; i!=to; ++i) {
//  	log_debug("%s: copying %d bytes, space left is %d, totalsize is %d", __PRETTY_FUNCTION__,
//  		  (*i)->allocated(), newbuf->spaceLeft(), totalsize);
//  	(*i)->dump();
// 	if (newbuf->spaceLeft() >= (*i)->allocated()) {
	    *newbuf += *i;
// 	}
	
	//
	// NOTE: If we're the buffer owners, it is safe to delete
	//       the buffer now.
	// delete buf;
	//
    }

    // Finally erase all merged elements, and replace with the composite one
    _que.erase(from, to);
    //que_t::iterator nextIter = _que.erase(from, to);
//    _que.insert(nextIter, newbuf.get()); FIXME:

    return newbuf; //->release(); // ownership is transferred. TODO: return auto_ptr
}

// Dump internal data.
void
CQue::dump()
{
//    GNASH_REPORT_FUNCTION;
    deque<boost::shared_ptr<cygnal::Buffer> >::iterator it;
    boost::mutex::scoped_lock lock(_mutex);
    std::cerr << std::endl << "CQue \"" << _name << "\" has "<< _que.size()
              << " buffers." << std::endl;
    for (it = _que.begin(); it != _que.end(); ++it) {
	boost::shared_ptr<cygnal::Buffer> ptr = *(it);
        ptr->dump();
    }
#ifdef USE_STATS_QUEUE
    struct timespec now;
    clock_gettime (CLOCK_REALTIME, &now);
    cerr << "Que lifespan is " <<
	static_cast<float>((now.tv_sec - _stats.start.tv_sec) + ((now.tv_nsec - _stats.start.tv_nsec)/1e9)) << " seconds" << endl;
    cerr << "Total number of bytes is " << _stats.totalbytes << " bytes" << endl;
    cerr << "Total number of packets pushed to queue is: " << _stats.totalin << endl;
    cerr << "Total number of packets popped from queue is: " << _stats.totalout << endl;
#endif
}

} // end of gnash namespace


// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:

