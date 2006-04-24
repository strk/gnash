// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
//

#ifndef __SHM_H__
#define __SHM_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>

#include "log.h"
#include "impl.h"
#include <sys/types.h>
#ifndef HAVE_WINSOCK_H
# include <sys/ipc.h>
# include <sys/shm.h>
#else
# include <windows.h>
# include <process.h>
# include <fcntl.h>
# include <io.h>
#endif

namespace gnash {

#ifndef MAP_INHERIT
const int MAP_INHERIT = 0;
#endif
#ifndef MAP_HASSEMAPHORE
const int MAP_HASSEMAPHORE = 0;
#endif

class Shm {
public:
    Shm();
    ~Shm();
    
    // Initialize the shared memory segment
    bool attach(char const *filespec, bool nuke);
    
    // Resize the allocated memory segment
    bool resize(int bytes);
    bool resize();
    
    // Allocate a memory from the shared memory segment
    void *brk(int bytes);
    
    // Close the memory segment. This removes it from the system.
    bool closeMem();
    
    Shm *cloneSelf(void);

#ifdef ENABLE_TESTING 
    // Accessors for testing
    std::string getName()       { return _filespec; };
    size_t getSize()            { return _size; };
    int getAllocated()          { return _alloced; };
    bool exists();
#endif

protected:
    char        *_addr;
    long        _alloced;
    size_t      _size;
    std::string _filespec;
#ifndef HAVE_WINSOCK_H
    key_t	_shmkey;
#else
    long	_shmkey;
    HANDLE      _shmhandle;
#endif
    int		_shmfd;
};

// Custome memory allocator for the shared memory segment
template<typename _Tp>
class ShmAlloc
{
    Shm *mmptr;
    Shm mem;
public:
    typedef size_t     size_type;
    typedef ptrdiff_t  difference_type;
    typedef _Tp*       pointer;
    typedef const _Tp* const_pointer;
    typedef _Tp&       reference;
    typedef const _Tp& const_reference;
    typedef _Tp        value_type;
    
    template<typename _Tp1>
    struct rebind
    { typedef ShmAlloc<_Tp1> other; };
    
    ShmAlloc() throw() { }
    
    ShmAlloc(const ShmAlloc& other) throw() 
        : mem(other.mem)
        { }
    
    template<typename _Tp1>
    ShmAlloc(const ShmAlloc<_Tp1>& other) throw() 
        : mem(other.mem)
        { }
    
    ~ShmAlloc() throw() { }
    
    pointer
    address(reference __x) const        { return &__x; }
    
    const_pointer
    address(const_reference __x) const { return &__x; }
    
    // Allocate memory
    _Tp*
    allocate(size_type n, const void* p = 0) {
        // If the memory manager has no blocks, it hasn't been
        // initialized.
//         if (mminit == false) {
//             mmptr = mem.initMemManager(true);
//             mminit = true;
//         } else {
//             mmptr = mem.initMemManager(false);
//         }
        
        _Tp* ret = 0;
        if (n) {
            ret = (_Tp*)mmptr->brk(n * sizeof(_Tp));
            if (ret == 0)
                throw std::bad_alloc();
        }
        return ret;
    }
    
    // Deallocate memory
    void
    deallocate(pointer __p, size_type __n) {
        //mmptr->free(__p);
    }
    
    void construct(pointer __p, const _Tp& __val) {
        new(__p) _Tp(__val);
    }
    void destroy(pointer __p)   { __p->~_Tp(); }
};

struct shm_as_object : public as_object
{
    Shm obj;
};

#ifdef ENABLE_TESTING 
void shm_getname(const fn_call& fn);
void shm_getsize(const fn_call& fn);
void shm_getallocated(const fn_call& fn);
void shm_exists(const fn_call& fn);
#endif

} // end of gnash namespace

// end of __SHM_H__
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
