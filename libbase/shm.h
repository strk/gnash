// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifndef __SHM_H__
#define __SHM_H__

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <string>

#include "as_object.h" // for inheritance

#include <sys/types.h>
#if !defined(HAVE_WINSOCK_H) && !defined(__riscos__) && !defined(__OS2__)
# include <sys/ipc.h>
# include <sys/shm.h>
#elif !defined(__riscos__) && !defined(__OS2__)
# include <windows.h>
# include <process.h>
# include <fcntl.h>
# include <io.h>
#endif

// Forward declarations
namespace gnash {
	class fn_call;
}

namespace gnash {

#ifndef MAP_INHERIT
const int MAP_INHERIT = 0;
#endif
#ifndef MAP_HASSEMAPHORE
const int MAP_HASSEMAPHORE = 0;
#endif

const int MAX_SHM_NAME_SIZE = 48;

class Shm {
public:

    DSOEXPORT Shm();
    DSOEXPORT ~Shm();
    
    // Initialize the shared memory segment
    bool attach();
    bool DSOEXPORT attach(char const *filespec, bool nuke);
    bool DSOEXPORT attach(key_t key, bool nuke);
    
    // Resize the allocated memory segment
    bool resize(int bytes);
    bool resize();
    
    // Allocate a memory from the shared memory segment
    void *brk(int bytes);
    
    // Close the memory segment. This removes it from the system.
    bool DSOEXPORT closeMem();
    
    Shm *cloneSelf(void);

    // Accessors for testing
    char *getAddr()             { return _addr; };
    char *getName()             { return _filespec; };
    size_t getSize()            { return _size; };
    int getAllocated()          { return _alloced; };
    bool exists();
protected:
    char        *_addr;
    long        _alloced;
    size_t      _size;
    char        _filespec[MAX_SHM_NAME_SIZE];
#if !defined(HAVE_WINSOCK_H) || defined(__OS2__)
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
private:
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

class shm_as_object : public as_object
{
public:
    Shm obj;
};

as_value shm_getname(const fn_call& fn);
as_value shm_getsize(const fn_call& fn);
as_value shm_getallocated(const fn_call& fn);
as_value shm_exists(const fn_call& fn);

} // end of gnash namespace

// end of __SHM_H__
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
