// SystemClock.cpp -- system-time based VirtualClock for gnash core lib
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

#include "SystemClock.h"

#include "ClockTime.h" // for getting system time

#include <cstdint> // for std::uint64_t typedef

namespace gnash {

namespace {

std::uint64_t
fetchSystemTime() 
{
    // ClockTime::getTicks always returns milliseconds
    return clocktime::getTicks();
}

}

SystemClock::SystemClock() 
	:
	_startTime(fetchSystemTime())
{
}

unsigned long int
SystemClock::elapsed() const
{
    return fetchSystemTime() - _startTime;
}

void
SystemClock::restart()
{
    _startTime = fetchSystemTime();
}

} // namespace gnash

