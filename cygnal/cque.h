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

#ifndef __CQUE_H__
#define __CQUE_H__ 1

#include <boost/cstdint.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <deque>

#include "buffer.h"

// _definst_ is the default instance name
namespace cygnal
{

class CQue {
public:
    CQue();
    ~CQue();
    // Push data onto the que
    bool push(boost::uint8_t *data, int nbytes);
    bool push(Buffer *data);
    // Pop the first date element off the que
    Buffer *pop();
    // Peek at the first date element witjhout removing it from the que
    Buffer *peek();
    // Get the number of elements in the que
    size_t size();
    // Wait for a condition variable to trigger
    void wait();
    // Notify a condition variable to trigger
    void notify();
    // Empty the que of all data. 
    void clear();
    // Dump the data to the terminal
    void dump();
private:
    // The queue itself
    std::deque<Buffer *> _que;
    // A condition variable used to signal the other thread when the que has data
    boost::condition	_cond;
    // This is the mutex used by the condition variable. It needs to be separate from the
    // one used to lock access to the que.
    boost::mutex	_cond_mutex;
    // This is the mutex that control access to the que.
    boost::mutex	_mutex;
};
    
} // end of cygnal namespace

#endif // end of __CQUE_H__

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
