// utility.cpp --	Various little utility functions, macros & typedefs.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/cstdint.hpp> // for boost::int16_t
#include <cstring> // for memcpy


#ifdef HAVE_DMALLOC

#ifdef HAVE_MALLINFO
	#include <cstdio>
#endif

#include "utility.h"
#include "dlmalloc.h"
#include "tu_config.h"

// Overrides of new/delete that use Doug Lea's malloc.  Very helpful
// on certain lame platforms.

void*	operator new(size_t size)
{
	return dlmalloc(size);
}

void	operator delete(void* ptr)
{
	if (ptr) dlfree(ptr);
}

void*	operator new[](size_t size)
{
	return dlmalloc(size);
}

void	operator delete[](void* ptr)
{
	if (ptr) dlfree(ptr);
}
// end of HAVE_DMALLOC
#endif

// This does not work with DMALLOC, since the internal data structures
// differ.
#ifdef HAVE_DMALLOC
#ifdef HAVE_MALLINFO
#define CAN_DUMP_MEMORY_STATS
#endif
#endif

#ifndef CAN_DUMP_MEMORY_STATS
void dump_memory_stats(const char*, int, const char *)  {}
#else
void dump_memory_stats(const char* from, int line, const char *label) 
{

	static int allocated = 0;
	static int freeb = 0;

	struct mallinfo mi = mallinfo();  

	if (label != 0) {
		printf("Malloc Statistics from %s() (line #%d): %s\n", from, line, label);
	} else { 
		printf("Malloc Statistics from %s() (line #%d):\n", from, line);
	}
  
	//printf("\tnon-mapped space from system:  %d\n", mi.arena);
	printf("\ttotal allocated space:         %d\n", mi.uordblks);
	printf("\ttotal free space:              %d\n", mi.fordblks);
	//printf("\tspace in mmapped regions:      %d\n", mi.hblkhd);
	//printf("\ttop-most, releasable space:    %d\n", mi.keepcost); // Prints 78824
	if (freeb != mi.fordblks) {
		printf("\t%d bytes difference in free space.\n", freeb - mi.fordblks);
		freeb = mi.fordblks;
	}

	//if (allocated != mi.uordblks) {
	//  printf("\t%d bytes difference in allocated space.\n", mi.uordblks - allocated);
	//  allocated = mi.uordblks;
	//}  

}
#endif // CAN_DUMP_MEMORY_STATS

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
