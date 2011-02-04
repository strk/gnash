// 
//   Copyright (C) 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

#ifndef __CQUE_H__
#define __CQUE_H__

#include <string>
#include <boost/cstdint.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <deque>

#include "getclocktime.hpp"
#include "buffer.h"
#include "network.h"
#include "dsodefs.h" //For DSOEXPORT.

// _definst_ is the default instance name
namespace gnash
{

class CQue {
public:
    typedef std::deque<boost::shared_ptr<cygnal::Buffer> > que_t;
#ifdef USE_STATS_QUEUE
    typedef struct {
	struct timespec start;
	int		totalbytes;
	int		totalin;
	int		totalout;
    } que_stats_t;
#endif
    CQue();
    CQue(const std::string &str) { _name = str; };
    ~CQue();
    // Push data onto the que
    bool push(boost::uint8_t *data, int nbytes);
    bool push(boost::shared_ptr<cygnal::Buffer> data);
    // Pop the first date element off the que
    boost::shared_ptr<cygnal::Buffer> DSOEXPORT pop();
    // Peek at the first date element witjhout removing it from the que
    boost::shared_ptr<cygnal::Buffer> DSOEXPORT peek();
    // Get the number of elements in the que
    size_t DSOEXPORT size();
    // Wait for a condition variable to trigger
    void wait();
    // Notify a condition variable to trigger
    void notify();
    // Empty the que of all data. 
    void clear();
    // Remove a range of elements
    void remove(boost::shared_ptr<cygnal::Buffer> begin, boost::shared_ptr<cygnal::Buffer> end);
//     // Remove an element
//    void remove(boost::shared_ptr<cygnal::Buffer> it);
    void remove(boost::shared_ptr<cygnal::Buffer> it);
    // Merge sucessive buffers into one single larger buffer. This is for some
    // protocols, than have very long headers.
    boost::shared_ptr<cygnal::Buffer> DSOEXPORT merge(boost::shared_ptr<cygnal::Buffer> begin);
    boost::shared_ptr<cygnal::Buffer> DSOEXPORT merge();

    boost::shared_ptr<cygnal::Buffer> operator[] (int index) { return _que[index]; };
    
    // Dump the data to the terminal
    void dump();
#ifdef USE_STATS_QUEUE
    que_stats_t *stats() { return &_stats; };
#endif
    void setName(const std::string &str) { _name = str; }
    const std::string &getName() { return _name; }
private:
    // an optional name for the queue, only used for debugging messages to make them unique
    std::string		_name;
    // The queue itself
    que_t		_que;

    // A condition variable used to signal the other thread when the que has data
    boost::condition	_cond;
    // This is the mutex used by the condition variable. It needs to be separate from the
    // one used to lock access to the que.
    boost::mutex	_cond_mutex;
    // This is the mutex that controls access to the que.
    boost::mutex	_mutex;
#ifdef USE_STATS_QUEUE
    que_stats_t		_stats;
#endif
};
    
} // end of gnash namespace

#endif // end of __CQUE_H__

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
