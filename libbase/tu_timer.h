// tu_timer.h	-- by Thatcher Ulrich <tu@tulrich.com>

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Utility/profiling timer.


#ifndef TU_TIMER_H

#include "tu_config.h"
#include "tu_types.h"


namespace tu_timer
{
	// General-purpose wall-clock timer.  May not be hi-res enough
	// for profiling. Time Unit: millisecond
	DSOEXPORT uint64_t get_ticks();

	// Time Unit: second
	DSOEXPORT double ticks_to_seconds(uint64_t ticks);
	
	// Hi-res timer for CPU profiling.

	// Return a hi-res timer value.  Time 0 is arbitrary, so
	// generally you want to call this at the start and end of an
	// operation, and pass the difference to
	// profile_ticks_to_seconds() to find out how long the
	// operation took.
	DSOEXPORT uint64_t	get_profile_ticks();

	// Convert a hi-res ticks value into seconds.
	DSOEXPORT double	profile_ticks_to_seconds(uint64_t profile_ticks);
}


#endif // TU_TIMER_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
