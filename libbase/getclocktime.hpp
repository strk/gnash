//
//   Copyright (C) 2008 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <ctime>

#ifndef HAVE_CLOCK_GETTIME

#include <sys/time.h>

#define  CLOCK_REALTIME 0 /* Dummy */

static int clock_gettime(int clk_id /* unused */, struct timespec *tp) {
    
    struct timeval now;
    int ret = gettimeofday(&now, NULL);
    
    if (ret != 0)
        return ret;
    
    tp->tv_sec = now.tv_sec;
    tp->tv_nsec = now.tv_usec * 1000;
    return 0;
}

#endif