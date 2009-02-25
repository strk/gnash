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
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

/*
 * Header file for ensuring that C99 types ([u]int32_t and bool) are
 * available.
 */

#if defined(WIN32) || defined(OS2)
  /*
   * Win32 and OS/2 don't know C99, so define [u]int_32 here. The bool
   * is predefined tho, both in C and C++.
   */
  typedef int int32_t;
  typedef unsigned int uint32_t;
#elif defined(_AIX) || defined(__sun) || defined(__osf__) || defined(__sgi) || defined(HPUX)
  /*
   * AIX and SunOS ship a inttypes.h header that defines [u]int32_t,
   * but not bool for C.
   */
  #include <inttypes.h>

  #ifndef __cplusplus
    typedef int bool;
  #endif
#elif defined(bsdi) || defined(FREEBSD) || defined(OPENBSD)
  /*
   * BSD/OS, FreeBSD, and OpenBSD ship sys/types.h that define int32_t and 
   * u_int32_t.
   */
  #include <sys/types.h>

  /*
   * BSD/OS ships no header that defines uint32_t, nor bool (for C)
   * OpenBSD ships no header that defines uint32_t and using its bool macro is
   * unsafe.
   */
  #if defined(bsdi) || defined(OPENBSD)
  typedef u_int32_t uint32_t;

  #if !defined(__cplusplus)
    typedef int bool;
  #endif
  #else
  /*
   * FreeBSD defines uint32_t and bool.
   */
    #include <inttypes.h>
    #include <stdbool.h>
  #endif
#elif defined(BEOS)
  #include <inttypes.h>
#else
  /*
   * For those that ship a standard C99 stdint.h header file, include
   * it. Can't do the same for stdbool.h tho, since some systems ship
   * with a stdbool.h file that doesn't compile!
   */
#ifdef HAVE_STDINT_H
  #include <stdint.h>
#endif

  #ifndef __cplusplus
    #if !defined(__GNUC__) || (__GNUC__ > 2 || __GNUC_MINOR__ > 95)
      #include <stdbool.h>
    #else
      /*
       * GCC 2.91 can't deal with a typedef for bool, but a #define
       * works.
       */
      #define bool int
    #endif
  #endif
#endif
