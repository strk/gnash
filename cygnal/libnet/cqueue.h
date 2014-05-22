// http.cpp:  HyperText Transport Protocol handler for Cygnal, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#ifndef __CQUEUE_H__
#define __CQUEUE_H__ 1


#include <cstdint>
#include <mutex>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
#include <vector>
#include "log.h"

namespace gnash
{

class CQueue 
{
public:
    CQueue();
    ~CQueue();

    // Push bytes on the FIFO
    bool push(std::uint8_t *data, int nbytes);
    bool push(std::vector<std::uint8_t> *data);
    // Pop the first date element off the FIFO
    std::vector<uint8_t> *pop();
    // Peek at the first data element without removing it
    std::vector<uint8_t> *peek();
//     void memcpy(std::uint8_t *data, size_t size,
//                  std::vector<std::uint8_t> *ptr);
//     void memcpy(std::vector<std::uint8_t> *ptr,
//                  std::uint8_t *data, size_t size);
    size_t size() { return _queue.size(); };
    // Dump internal data.
    void dump();
private:
    std::vector<std::vector<std::uint8_t> *> _queue;
};


} // end of gnash namespace

#endif // end of __CQUEUE_H__

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
