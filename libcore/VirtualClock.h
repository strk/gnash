// VirtualClock.h -- virtual clock for gnash core lib
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

#ifndef GNASH_VIRTUAL_CLOCK_H
#define GNASH_VIRTUAL_CLOCK_H

#include <cassert> // for InterruptableVirtualClock

namespace gnash
{

/// A class used to virtualize time flow
//
/// This class will be used to fetch current time
/// everytime it is needed by the core lib.
///
class VirtualClock
{
public:

    /// Return number of milliseconds elapsed since start.
    //
    /// Subclass this to provide time to the core lib.
    //
    /// NOTE:
    /// 32bit unsigned int has an upper limit of 4294967295
    /// which means about 49 days before overlflow.
    ///
    virtual unsigned long int elapsed() const=0;

    /// Restart the clock
    virtual void restart()=0;

    virtual ~VirtualClock() {}
};

/// A VirtualClock wrapper adding pause/resume capabilities 
class InterruptableVirtualClock : public VirtualClock
{

public:

	/// Construct an InterruptableVirtualClock from a VirtualClock source
	//
	/// The interruptable virtual clock starts in 'stop' mode,
    /// use resume() to start.
	///
	/// @param src
	///	A VirtualClock to use as source, ownership is retained by caller
    /// which should guarantee to keep the source alive for the whole
    /// lifetime of this instance.
	///
	InterruptableVirtualClock(VirtualClock& src)
		:
		_src(src),
		_elapsed(0),
		_offset(_src.elapsed()),
		_paused(true)
	{
	}

	/// Return elapsed time, taking interruptions in consideration
	unsigned long int elapsed() const
	{
		if ( ! _paused ) // query source if not stopped
			_elapsed = _src.elapsed()-_offset;
		return _elapsed;
	}

	void restart()
	{
		_elapsed = 0;
		_offset = _src.elapsed();
	}

	void pause()
	{
		if ( _paused ) return; // nothing to do
		_paused = true;
	}

	void resume()
	{
		if ( ! _paused ) return; // nothing to do
		_paused = false;

		unsigned long now = _src.elapsed();
		_offset = ( now - _elapsed );
		assert( now-_offset == _elapsed ); // check if we did the right thing
	}

private:

	VirtualClock& _src;

	mutable unsigned long int _elapsed;

	unsigned long int _offset;

	bool _paused;
};


} // namespace gnash

#endif // GNASH_VIRTUAL_CLOCK_H

