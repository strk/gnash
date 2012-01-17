/* 
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
 *   Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 */ 

#ifndef _GNASH_MANUAL_CLOCK_HH
#define _GNASH_MANUAL_CLOCK_HH

#include "VirtualClock.h" // for inheritance

namespace gnash {

/// A manually advanced clock
class ManualClock : public VirtualClock
{
public:

	/// Construct a manual clock
	ManualClock()
		:
		_elapsed(0)
	{}

	// see dox in VirtualClock.h
	unsigned long elapsed() const
	{
		return _elapsed;
	}

	// see dox in VirtualClock.h
	void restart()
	{
		_elapsed=0;
	}

	/// Advance the clock by the given amount of milliseconds
	void advance(unsigned long amount)
	{
		_elapsed += amount;
	}

private:

	unsigned long _elapsed;
};


} // namespace gnash

#endif // _GNASH_MANUAL_CLOCK_HH
