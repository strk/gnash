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

class Shm {
public:

    DSOEXPORT Shm();
    DSOEXPORT ~Shm();
    
    // Initialize the shared memory segment
    bool attach();
    bool DSOEXPORT attach(key_t key, bool nuke);
    
    // Resize the allocated memory segment
    bool resize(int bytes);
    bool resize();
    
    // Close the memory segment. This removes it from the system.
    bool DSOEXPORT closeMem();

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

} // end of gnash namespace

// end of __SHM_H__
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
