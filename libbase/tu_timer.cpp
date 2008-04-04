// tu_timer.cpp	-- by Thatcher Ulrich <tu@tulrich.com>

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Utility/profiling timer.


#include "tu_timer.h"


#if defined(_WIN32) || defined(WIN32)

#include <windows.h>
#include <mmsystem.h>


boost::uint64_t tu_timer::get_ticks()
{
	return timeGetTime();
}


double tu_timer::ticks_to_seconds(boost::uint64_t ticks)
{
	return ticks * (1.0f / 1000.f);
}

#else	// not _WIN32


#include <sys/time.h>


// The profile ticks implementation is just fine for a normal timer.


boost::uint64_t tu_timer::get_ticks()
{

	struct timeval tv;
	
	gettimeofday(&tv, 0);

	boost::uint64_t result = static_cast<boost::uint64_t>(tv.tv_sec) * 1000000L;

	result += tv.tv_usec;
	// Time Unit: microsecond

	return static_cast<boost::uint64_t>(result / 1000.0);
}


double tu_timer::ticks_to_seconds(boost::uint64_t ticks)
{
	return ticks / 1000000.0;
}

#endif	// not _WIN32


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
