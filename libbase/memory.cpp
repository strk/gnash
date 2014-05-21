// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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
#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

// If we don't have support for mallinfo(), this code is useless
#if HAVE_MALLINFO

#include "gmemory.h"

#include <vector>
#include <iostream>

#include "log.h"
#include "getclocktime.hpp"

using namespace std;

namespace gnash {
  
RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();


const int DATALOG_SIZE = 1024;

Memory::Memory() 
    : _collecting(false),
      _info(nullptr),
      _size(DATALOG_SIZE),
      _index(0)
    , _checkpoint()
{
//    GNASH_REPORT_FUNCTION;
}


Memory::Memory(size_t size) 
    : _collecting(false)
    , _checkpoint()
{
//    GNASH_REPORT_FUNCTION;
    _size = size;
    _info = new struct small_mallinfo[_size];
    reset();
}

Memory::~Memory()
{
//    GNASH_REPORT_FUNCTION;
    if (_info) {
        delete[] _info;
    }
    _index = 0;
    _size = 0;
}

// Erase all collected data and reset collections.
void
Memory::reset()
{
//    GNASH_REPORT_FUNCTION;
    if (_info) {
        memset(_info, 0, _size);
    }
    _index = 0;
}


void
Memory::startStats()
{
//    GNASH_REPORT_FUNCTION;
    _collecting = true;
    if (_info == nullptr) {
        log_debug(_("Allocating buffer for %d data samples"), _size);
        _info = new struct small_mallinfo[_size];
        reset();
	addStats();
    }
}
    
int
Memory::addStats(int line)
{
//    GNASH_REPORT_FUNCTION;
    if (_info) {
        struct small_mallinfo *ptr = _info + _index;        
        addStats(ptr, line);
    }
    
    return _index;
}

int
Memory::addStats()
{
//    GNASH_REPORT_FUNCTION;
    if (_info) {
        struct small_mallinfo *ptr = _info + _index;        
        addStats(ptr, 0);
    }
    
    return _index;
}


int
Memory::addStats(struct small_mallinfo *ptr)
{
//    GNASH_REPORT_FUNCTION;
    return addStats(ptr, 0);
}

int
Memory::addStats(struct small_mallinfo *ptr, int line)
{
//    GNASH_REPORT_FUNCTION;
    struct mallinfo mal = mallinfo();
    int yy = static_cast<int>(_size);

//    dump(&mal);
    if ((ptr) && (_index < yy)) {
	ptr->line = line;
	clock_gettime (CLOCK_REALTIME, &ptr->stamp);
        ptr->arena = mal.arena;
        ptr->uordblks = mal.uordblks;
        ptr->fordblks = mal.fordblks;
        _index++;
    }
    
    return _index;
}

// return true if we haven't leaked any memory
bool
Memory::endCheckpoint()
{
//    GNASH_REPORT_FUNCTION;
    _checkpoint[1] = mallinfo();
    if (_checkpoint[1].uordblks == _checkpoint[0].uordblks) {
        return true;
    }

    return false;
}

// Dump the differences of bytes allocated between two samples
int
Memory::diffStats()
{
//    GNASH_REPORT_FUNCTION;
    return diffStats(_index - 1, _index - 2);
}

int
Memory::diffStats(int x, int y)
{
//    GNASH_REPORT_FUNCTION;
    int yy = static_cast<int>(_size);
    if ((_info) && (x < DATALOG_SIZE) && (y < yy)) {
        return (_info[x].uordblks - _info[y].uordblks);
    }
    return -1;
}
    
// Dump the differences between two samples's timestamp
int
Memory::diffStamp()
{
//    GNASH_REPORT_FUNCTION;
    return diffStamp(_index - 1, _index - 2);
}

int
Memory::diffStamp(int x, int y)
{
//    GNASH_REPORT_FUNCTION;
    int yy = static_cast<int>(_size);
    if ((_info) && (x < DATALOG_SIZE) && (y < yy)) {
        return (_info[x].stamp.tv_nsec - _info[y].stamp.tv_nsec);
    }
    return -1;
}    

// Analyze memory usage
bool
Memory::analyze()
{
//    GNASH_REPORT_FUNCTION;

    int accumulate_allocated = 0;
    int accumulate_freed = 0;

    // System memory is what we get from brk(), the lowest level
    // system call used by both malloc() or new().
    cerr << endl << "System memory allocated in bytes: "
         << _info->arena << endl;
    int diff_arena = (_info + _index - 1)->arena - _info->arena;
    if (diff_arena) {
        cerr << "System memory change in bytes: " << diff_arena << endl;
    }    

    int total_allocated = (_info + _index - 1)->uordblks - _info->uordblks;
    cerr << "Total bytes allocated: " << total_allocated << endl;

    if (_index > 1) {
        for (int i=1; i<_index; i++) {
            struct small_mallinfo *ptr = _info + i;

//	    // Get the time stamp
//            int diff_stamp_sec = (ptr->stamp.tv_sec) - (ptr - 1)->stamp.tv_sec;
            int diff_stamp_nsec = (ptr->stamp.tv_nsec) - (ptr - 1)->stamp.tv_nsec;
//             if ((diff_stamp_sec > 0) || (diff_stamp_nsec > 0)) {
// 		if (ptr->line && (ptr - 1)->line) {
// 		    if (diff_stamp_sec > 0) {
// 			cerr << "Difference in seconds is: " << diff_stamp_sec;
// 			cerr << ", nanoseconds is: "<< diff_stamp_nsec;
// 		    }
// 		    else {
// 			cerr << "Difference in nanoseconds is: "<< diff_stamp_nsec;
// 		    }
// 		    cerr << "\tbetween lines: " << (ptr - 1)->line
// 			 << " and " << ptr->line << endl;
// 		} else {
// 		    cerr << "Difference in seconds is: " << diff_stamp_sec
// 			 << ", nanoseconds is: "<< diff_stamp_nsec << endl;
// 		}
// 	    }
	    // See what was allocated between samples
            int diff_allocated = (ptr->uordblks) - (ptr - 1)->uordblks;
            if (diff_allocated > 0) {
                accumulate_allocated += diff_allocated;
                if (ptr->line && (ptr - 1)->line) {
                    cerr << "Allocated " << diff_allocated
                         << " bytes\tbetween lines: " << (ptr - 1)->line
                         << " and " << ptr->line;
                } else {
                    cerr << "Allocated bytes: " << diff_allocated;
                }
// same as diff_freed
//             } else {
//                 if (diff_allocated != 0) {
//                     cerr << "\tnew heap bytes: " << diff_allocated << endl;
//                 }
            }

            // See what was freed between samples
            int diff_freed = ptr->fordblks - (ptr - 1)->fordblks;
            if (diff_freed > 0) {
                accumulate_freed += diff_freed;
                if (ptr->line && (ptr - 1)->line) {
                    cerr << "Freed " << diff_freed
                         << " bytes between lines: " << (ptr - 1)->line
                         << " and " << ptr->line;
                } else {
                    cerr << "Freed bytes: " << diff_freed;
                }
// Same as diif_allocated
//              } else {
//                  if (diff_freed != 0) {
//                      cerr << "\tnuked heap bytes: " << diff_freed << endl;
//                  }
            }
	    if (diff_freed || diff_allocated) {
		cerr << ", and took " << diff_stamp_nsec << " nanoseconds";
	    } else {
		cerr << "no allocations, time difference is " << diff_stamp_nsec << " nanoseconds";
                if (ptr->line && (ptr - 1)->line) {
                         cerr << " between lines: " << (ptr - 1)->line
                         << " and " << ptr->line;
		}
	    }
	    cerr << endl;
        }
    } else {
        cerr << "Only have one sample" << endl;
        dump();
    }

    // Sanity check on our calculations
    if (total_allocated != (accumulate_allocated - accumulate_freed)) { 
        log_error(_("Calculations don't equal"));
    } else {
        log_debug(_("Zero memory leaks for this program"));
    }
    if ((_checkpoint[0].uordblks != 0) && (_checkpoint[1].uordblks != 0)) {
        if (_checkpoint[1].uordblks == _checkpoint[0].uordblks) {
            cerr << "The last checkpoint status was: "
                 << ((_checkpoint[1].uordblks == _checkpoint[0].uordblks)
                 ? "passed" : "failed") << endl;
        }
    }
    return true;
}
    

// Dump the vector of stored classes
void
Memory::dump(struct mallinfo *ptr)
{
//    GNASH_REPORT_FUNCTION;
    cerr << "\tstruct mallinfo: Non-mmapped space allocated from system is: \""
         << ptr->arena << "\"" << endl;
    cerr << "\tstruct mallinfo: Total allocated space  is: \""
         << ptr->uordblks << "\"" << endl;
    cerr << "\tstruct mallinfo: Total free space  is: \""
         << ptr->fordblks << "\"" << endl;
}

// Dump the vector of stored classes
void
Memory::dump(struct small_mallinfo *ptr)
{
//    GNASH_REPORT_FUNCTION;
    cerr << "\tLine number of sample: " << ptr->line << endl;
    cout.fill('0');
    cout.width(9);
    cerr << "\tTimestamp number of sample: " << ptr->stamp.tv_sec
	 << ":" << ptr->stamp.tv_nsec << endl;
    cout.fill(' ');
    cout.width(1);
    cerr << "\tNon-mmapped space allocated from system is: \""
         << ptr->arena << "\"" << endl;
    cerr << "\tTotal allocated space  is: \""
         << ptr->uordblks << "\"" << endl;
    cerr << "\tTotal free space  is: \""
         << ptr->fordblks << "\"" << endl;
}

void
Memory::dump()
{
//    GNASH_REPORT_FUNCTION;

    for (int i=0; i<_index; i++) {
        cerr << "Mallinfo index: " << i << endl;
        dump(_info + i);
    }
}

void
Memory::dumpCSV()
{
//    GNASH_REPORT_FUNCTION;

    struct small_mallinfo *ptr;
    cerr << "linenum,seconds,nanoseconds,arena,allocated,freed" << endl;
    for (int i=0; i<_index; i++) {
	ptr = _info + i;
        cerr << ptr->line << ","
	     << ptr->stamp.tv_sec << ","
	     << ptr->stamp.tv_nsec << ","
	     << ptr->arena << ","
	     << ptr->uordblks << ","
	     << ptr->fordblks << endl;
    }
}

} // end of gnash namespace

#endif // end of HAVE_MALLINFO

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
