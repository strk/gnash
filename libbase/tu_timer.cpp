// tu_timer.cpp	-- by Thatcher Ulrich <tu@tulrich.com>

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Utility/profiling timer.


#include "tu_timer.h"


#ifdef _WIN32

#include <windows.h>


uint64 tu_timer::get_ticks()
{
	return timeGetTime();
}


double tu_timer::ticks_to_seconds(uint64 ticks)
{
	return ticks * (1.0f / 1000.f);
}


uint64	tu_timer::get_profile_ticks()
{
	// @@ use rdtsc?

	LARGE_INTEGER	li;
	QueryPerformanceCounter(&li);

	return li.QuadPart;
}


double	tu_timer::profile_ticks_to_seconds(uint64 ticks)
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


uint64 tu_timer::get_ticks()
{
	return get_profile_ticks();
}


double tu_timer::ticks_to_seconds(uint64 ticks)
{
	return profile_ticks_to_seconds(ticks);
}


uint64	tu_timer::get_profile_ticks()
{
	// @@ TODO prefer rdtsc when available?

	// Return microseconds.
	struct timeval tv;
	uint64 result;
	
	gettimeofday(&tv, 0);

	result = tv.tv_sec * 1000000;
	result += tv.tv_usec;
	
	return result;
}


double	tu_timer::profile_ticks_to_seconds(uint64 ticks)
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
