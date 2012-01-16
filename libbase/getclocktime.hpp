//
//   Copyright (C) 2008, 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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

#ifndef GETCLOCKTIMER_HPP
#define GETCLOCKTIMER_HPP 1

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <ctime>

// TODO: why doesn't OS2 define HAVE_CLOCK_GETTIME?
#if !defined(HAVE_CLOCK_GETTIME) && !defined(__OS2__)

#include <sys/time.h>

#ifdef HAVE_PTHREADS
#include <pthread.h>
#endif

#if defined WIN32 && !defined(HAVE_STRUCT_TIMESPEC)
struct timespec {
	time_t	tv_sec; 	/* seconds */
	long	tv_nsec;	/* nanoseconds */
};
#endif

#define CLOCK_REALTIME 0 /* Dummy */

static int clock_gettime(int, struct timespec *tp) {
    
    struct timeval now;
    int ret = gettimeofday(&now, NULL);
    
    if (ret != 0)
        return ret;
    
    tp->tv_sec = now.tv_sec;
    tp->tv_nsec = now.tv_usec * 1000;
    return 0;
}

#endif
#endif 
