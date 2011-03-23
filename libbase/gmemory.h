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

//
// This class is a memory allocation tracker used to optimize
// the memory usage and find memory leaks.
//
#ifndef __MEMORY_H__
#define __MEMORY_H__

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

// If we don't have support for mallinfo(), this code is useless
#ifdef HAVE_MALLINFO

#include <cstdlib>
#include <malloc.h>
#include <ctime>
#include "dsodefs.h" // DSOEXPORT

namespace gnash {
  
class DSOEXPORT Memory {
public:

    // Borrowed from malloc.h and trimmed down.
    struct small_mallinfo {
        int line;     // line number of this data sample
	struct timespec stamp;	// the time stamp of this sample
        int arena;    // non-mmapped space allocated from system
        int uordblks; // total allocated space
        int fordblks; // total free space
    };
    Memory();
    Memory(size_t size);
    ~Memory();

    // Start collecting statistics. This can effect performance
    void startStats();
    
    // Stop collecting statistics
    void endStats() { addStats();_collecting = false; };

    // Erase all collected data and reset collections.
    void reset();

    // checkpoint()
    void startCheckpoint() { _checkpoint[0] = mallinfo(); };
    bool endCheckpoint();
    
    // Add or retrieve mallinfo data
    int addStats();
    int addStats(int line);
    int addStats(struct small_mallinfo *x);
    int addStats(struct small_mallinfo *x, int line);
    struct small_mallinfo *getStats() { return _info; };
    struct small_mallinfo *operator[](int x) { return _info + x; };
    int totalStats() { return _index; };

    // Analyze memory usage
    bool analyze();

    // Dump the differences of bytes allocated between two samples
    int diffStats();
    int diffStats(int x, int y);

    // Dump the differences in the timestamp between two samples
    int diffStamp();
    int diffStamp(int x, int y);
    
    // Dump the vector of stored classes
    void dump(struct mallinfo *x);
    void dump(struct small_mallinfo *x);
    void dump();
    void dumpCSV();
private:
    bool                _collecting;
    // For data logging, we want to store as little as possible
    // so we don't impact the system too hard. Data logging memory
    // allocations can generate a huge amount of data, so we have
    // to be careful. We also can't use STL, as that does more
    // memory allocations, which will confuse our statistics
    // gathering.
    struct small_mallinfo *_info;
    // Since we aren't using STL, we have to store how much
    // data storage we have ourselves.
    size_t              _size;
    int                 _index;
    // For checkpoints, we want the all the data
    struct mallinfo     _checkpoint[2];
};

} // end of gnash namespace

#endif // end of HAVE_MALLINFO

// end of __MEMORY_H__
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
