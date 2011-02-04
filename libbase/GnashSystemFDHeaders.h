// GnashSystemFDHeaders.h: Compatibility FD header for Gnash.
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
//

// Include this file for file descriptor utilities:
//
// select()
// ioctl() / ioctlsocket (use ioctlSocket())

#ifndef GNASH_SYSTEM_FD_HEADERS
#define GNASH_SYSTEM_FD_HEADERS

#include "gnashconfig.h"

#ifdef HAVE_WINSOCK2_H
# include <winsock2.h>
namespace {

inline int ioctlSocket(int fd, int request, int* arg) {
    unsigned long p = *arg;
    const int ret = ::ioctlsocket(fd, request, &p);
    *arg = p;
    return ret;
}

}

#else

# include <sys/types.h>
# include <sys/stat.h>
# include <unistd.h>
# include <sys/ioctl.h>

namespace {

inline int ioctlSocket(int fd, int request, int* arg) {
    return ::ioctl(fd, request, arg);
}

}
#endif

#endif
