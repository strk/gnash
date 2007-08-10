// tu_timer.cpp	-- by Thatcher Ulrich <tu@tulrich.com>

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Utility/profiling timer.


#include "tu_timer.h"


#if defined(_WIN32) || defined(WIN32)

#include <windows.h>
#include <mmsystem.h>


uint64_t tu_timer::get_ticks()
{
	return timeGetTime();
}


double tu_timer::ticks_to_seconds(uint64_t ticks)
{
	return ticks * (1.0f / 1000.f);
}


uint64_t	tu_timer::get_profile_ticks()
{
	// @@ use rdtsc?

	LARGE_INTEGER	li;
	QueryPerformanceCounter(&li);

	return li.QuadPart;
}


double	tu_timer::profile_ticks_to_seconds(uint64_t ticks)
{
	LARGE_INTEGER	freq;
	QueryPerformanceFrequency(&freq);

	double	seconds = (double) ticks;
	seconds /= (double) freq.QuadPart;

	return seconds;
}


#else	// not _WIN32


#include <sys/time.h>


// The profile ticks implementation is just fine for a normal timer.


uint64_t tu_timer::get_ticks()
{
	return static_cast<uint64_t>(get_profile_ticks()/1000.0);
}


double tu_timer::ticks_to_seconds(uint64_t ticks)
{
	return profile_ticks_to_seconds(ticks);
}


uint64_t	tu_timer::get_profile_ticks()
{
	// @@ TODO prefer rdtsc when available?

	// Return microseconds.
	struct timeval tv;
	
	gettimeofday(&tv, 0);

	uint64_t result = tv.tv_sec * 1000000;

	result += tv.tv_usec;
	// Time Unit: microsecond

	return result;
}


double	tu_timer::profile_ticks_to_seconds(uint64_t ticks)
{
	// ticks is microseconds.  Convert to seconds.
	return ticks / 1000000.0;
}

#endif	// not _WIN32


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
