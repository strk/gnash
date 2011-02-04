// GnashImageSystemNetHeaders.h: Compatibility Network header for Gnash.
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

/// This file should be included for:
//
/// htons()
/// ntohs()
/// gethostname()
/// gethostbyname()
/// send()
/// recv()
/// close() (actually an FD function)
/// socket()
/// fcntl() (POSIX only!)
/// inet_addr()
/// setsockopt()

#ifndef GNASH_NET_HEADERS_H
#define GNASH_NET_HEADERS_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

/// @todo A configure 'test' (a #define in gnashconfig.h) might be a better
///       way of checking for compiler.
#if defined(HAVE_WINSOCK_H) && !defined(__OS2__)
# include <winsock2.h>
# include <windows.h>
# include <io.h>
# include <ws2tcpip.h>
# include <fcntl.h>
#elif defined(__amigaos4__) //maybe HAVE_NETINET_IN_H
# include <netinet/in.h>
#else
# include <unistd.h>
# include <arpa/inet.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/times.h>
# include <unistd.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <fcntl.h>

// gethostbyname()
# include <netdb.h>
#endif

#endif

