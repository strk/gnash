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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#if !defined(HAVE_WINSOCK_H) && !defined(__riscos__) && !defined(__OS2__) && !defined(HAIKU_HOST)
# include <sys/mman.h>
# include <sys/shm.h>
# include <sys/ipc.h>
# include <sys/sem.h>
#elif !defined(__riscos__) && !defined(__OS2__)
# include <windows.h>
# include <process.h>
# include <io.h>
#endif
#include <string>
#include <vector>
#include <cerrno>

#include "log.h"
#include "SharedMem.h"

#if (defined(USE_SYSV_SHM) && defined(HAVE_SHMGET)) || defined(_WIN32)
# define ENABLE_SHARED_MEM 1
#else
# undef ENABLE_SHARED_MEM
#endif

namespace {
    gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
}

namespace gnash {

SharedMem::SharedMem(size_t size)
{
    log_unimpl(_("%s on Haiku"), __FUNCTION__);
}

SharedMem::~SharedMem()
{
    log_unimpl(_("%s on Haiku"), __FUNCTION__);
}

bool
SharedMem::lock()
{
    log_unimpl(_("%s on Haiku"), __FUNCTION__);
    return false;
}

bool
SharedMem::unlock()
{
    log_unimpl(_("%s on Haiku"), __FUNCTION__);
    return false;
}

bool
SharedMem::attach()
{
    log_unimpl(_("%s on Haiku"), __FUNCTION__);
    return false;
}

} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
