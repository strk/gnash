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

#include <boost/thread/mutex.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
//#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
//#include <boost/date_time/time_zone_base.hpp>
#include <string>
#include <vector>
#include "log.h"
#include "cqueue.h"

using namespace gnash;
using namespace std;
using namespace boost;

namespace gnash
{

CQueue::CQueue()
{
//    GNASH_REPORT_FUNCTION;
}

CQueue::~CQueue()
{
//    GNASH_REPORT_FUNCTION;
}

#if 0
void
memcpy(boost::uint8_t *data, size_t size,
                 vector<uint8_t> *ptr)
{
//    GNASH_REPORT_FUNCTION;
    ptr->reserve(size);
    std::copy(data, data + size, ptr->begin());
}
    
void
memcpy(std::vector<uint8_t> *ptr,
                 uint8_t *data, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    data = new uint8_t[size + 1];
    std::copy(ptr->begin(), ptr->end(), data);
}
#endif

// Push bytes on the FIFO
bool
CQueue::push(uint8_t *data, int nbytes)
{
//    GNASH_REPORT_FUNCTION;
    vector<boost::uint8_t> *ptr = new vector<boost::uint8_t>;
    ptr->reserve(nbytes);
    std::copy(data, data + nbytes, ptr->begin());
    _queue.push_back(ptr);
}

bool
CQueue::push(vector<uint8_t> *data)
{
//    GNASH_REPORT_FUNCTION;
    _queue.push_back(data);
}

// Pop the first date element off the FIFO
vector<uint8_t> *
CQueue::pop()
{
//    GNASH_REPORT_FUNCTION;
}

// Peek at the first data element without removing it
vector <uint8_t> *
CQueue::peek()
{
//    GNASH_REPORT_FUNCTION;
}

// Dump internal data.
void
CQueue::dump()
{
//    GNASH_REPORT_FUNCTION;
}

} // end of gnash namespace


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
