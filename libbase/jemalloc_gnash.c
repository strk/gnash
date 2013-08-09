/* 
 *   Copyright (C) 2013 Free Software Foundation, Inc.
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
 */ 

#include <stddef.h>
#include <jemalloc.h>

#ifdef HAVE_CONFIG_H
# include "gnashconfig.h"
#endif

#ifdef USE_STATS_MEMORY

/* Borrowed from malloc.h, as this is Linux specific. This has been
 * added to jemalloc so the existing memory profiling in Gnash will
 * continue to work. Most of these fields aren't used by the Gnash
 * memory profiling, but we leave them here for a semblance of
 * portability. The only fields Gnash uses are arena, uordblks. and
 * fordblks.
 */
struct mallinfo {
    int arena;    /* non-mmapped space allocated from system */
    int ordblks;  /* number of free chunks UNUSED */
    int smblks;   /* number of fastbin blocks UNUSED */
    int hblks;    /* number of mmapped regions UNUSED */
    int hblkhd;   /* space in mmapped regions UNUSED */
    int usmblks;  /* maximum total allocated space UNUSED */
    int fsmblks;  /* space available in freed fastbin blocks UNUSED */
    int uordblks; /* total allocated space */
    int fordblks; /* total free space */
    int keepcost; /* top-most, releasable space UNUSED */
};

struct mallinfo mallinfo (void);

struct mallinfo
mallinfo(void)
{
    struct mallinfo mi = {0,};

    size_t len = sizeof(mi.arena);
    mallctl("stats.mapped", &mi.arena, &len, NULL, 0);
    len = sizeof(mi.uordblks);
    mallctl("stats.allocated", &mi.uordblks, &len, NULL, 0);

    mi.fordblks = mi.arena - mi.uordblks;

    return mi;
}

#endif /* USE_STATS_MEMORY */
