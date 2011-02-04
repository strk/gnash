// SystemClock.h -- system-time based VirtualClock for gnash core lib
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#ifndef GNASH_SYSTEM_CLOCK_H
#define GNASH_SYSTEM_CLOCK_H

#include "VirtualClock.h" // for inheritance

#include <boost/cstdint.hpp> // for boost::uint64_t typedef
#include "dsodefs.h"

namespace gnash
{

/// A system-clock based virtual clock
//
/// This class uses the system clock for computing elapsed time.
/// See http://en.wikipedia.org/wiki/System_time for capacity
///
class DSOEXPORT SystemClock: public VirtualClock
{
public:

    /// Construct the clock, starting it
    SystemClock();

    // see VirtualClock.h for dox
    unsigned long int elapsed() const;

    // see VirtualClock.h for dox
    void restart();

private:

    /// System time at time of start
    boost::uint64_t _startTime;
};


} // namespace gnash

#endif // GNASH_SYSTEM_CLOCK_H

