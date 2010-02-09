// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#ifndef GNASH_SHM_H
#define GNASH_SHM_H

#include <string>

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

class Shm
{
public:

    DSOEXPORT Shm();
    DSOEXPORT ~Shm();
    
    /// Initialize the shared memory segment
    //
    /// This is called by LocalConnection when either connect() or send()
    /// is called.
    bool attach();
    
    bool lock();
    bool unlock();

    // Accessors for testing
    char* getAddr()             { return _addr; };
    size_t getSize()            { return _size; };

    class Lock
    {
    public:
        Lock(Shm& s) : _s(s), _locked(s.lock()) {}
        ~Lock() { if (_locked) _s.unlock(); }
        bool locked() const {
            return _locked;
        }
    private:
        Shm& _s;
        bool _locked;
    };

private:

    char* _addr;
    size_t _size;

    // Semaphore ID.
    int _semid;

    // Shared memory ID.
    int _shmid;

#if !defined(HAVE_WINSOCK_H) || defined(__OS2__)
    key_t	_shmkey;
#else
    long	_shmkey;
    HANDLE      _shmhandle;
#endif
};

} // end of gnash namespace

#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
