dnl 
dnl  Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
dnl  2011 Free Software Foundation, Inc.
dnl  
dnl  This program is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU General Public License as published by
dnl  the Free Software Foundation; either version 3 of the License, or
dnl  (at your option) any later version.
dnl  
dnl  This program is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl  GNU General Public License for more details.
dnl  You should have received a copy of the GNU General Public License
dnl  along with this program; if not, write to the Free Software
dnl  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

dnl Copyright (C) 2006 Steven G. Johnson <stevenj@alum.mit.edu>.

AC_DEFUN([GNASH_PATH_PTHREADS],
[
AC_REQUIRE([AC_CANONICAL_HOST])
AC_LANG_SAVE
AC_LANG_C
PTHREAD_LIBS=""
PTHREAD_CFLAGS=""

dnl We used to check for pthread.h first, but this fails if pthread.h
dnl requires special compiler flags (e.g. on True64 or Sequent).
dnl It gets checked for in the link test anyway.

dnl First of all, check if the user has set any of the PTHREAD_LIBS,
dnl etcetera environment variables, and if threads linking works using
dnl them:
if test x"$PTHREAD_LIBS$PTHREAD_CFLAGS" != x; then
  save_CFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
  save_LIBS="$LIBS"
  LIBS="$PTHREAD_LIBS $LIBS"
  AC_MSG_CHECKING([for pthread_join in LIBS=$PTHREAD_LIBS with CFLAGS=$PTHREAD_CFLAGS])
  AC_TRY_LINK_FUNC(pthread_join, pthreads=yes)
  AC_MSG_RESULT($pthreads)
  if test x"$pthreads" = xno; then
    PTHREAD_LIBS=""
    PTHREAD_CFLAGS=""
  fi
  LIBS="$save_LIBS"
  CFLAGS="$save_CFLAGS"
fi

dnl We must check for the threads library under a number of different
dnl names; the ordering is very important because some systems
dnl (e.g. DEC) have both -lpthread and -lpthreads, where one of the
dnl libraries is broken (non-POSIX).

dnl Create a list of thread flags to try.  Items starting with a "-" are
dnl C compiler flags, and other items are library names, except for "none"
dnl which indicates that we try without any flags at all, and "pthread-config"
dnl which is a program returning the flags for the Pth emulation library.

pthread_flags="pthreads none -Kthread -kthread lthread -pthread -pthreads -mthreads pthread --thread-safe -mt pthread-config pth-config"

dnl When cross configuring, we're always using GCC, and we always have a platform
dnl with pthreads in that case, but it's often sonewhere non-standard, so
dnl unless this is a problem, assume we don't need any special flags.
if test x$cross_compiling = xyes; then
    pthread_flags="none"
fi

dnl The ordering *is* (sometimes) important.  Some notes on the
dnl individual items follow:

dnl pthreads:	AIX (must check this before -lpthread)
dnl none:	in case threads are in libc; should be tried before -Kthread and
dnl 		other compiler flags to prevent continual compiler warnings
dnl -Kthread:	Sequent (threads in libc, but -Kthread needed for pthread.h)
dnl -kthread:	FreeBSD kernel threads (preferred to -pthread since SMP-able)
dnl lthread:	LinuxThreads port on FreeBSD (also preferred to -pthread)
dnl -pthread:	Linux/gcc (kernel threads), BSD/gcc (userland threads)
dnl -pthreads:	Solaris/gcc
dnl -mthreads:	Mingw32/gcc, Lynx/gcc
dnl -mt:		Sun Workshop C (may only link SunOS threads [-lthread], but it
dnl		doesn't hurt to check since this sometimes defines pthreads too;
dnl		also defines -D_REENTRANT)
dnl		... -mt is also the pthreads flag for HP/aCC
dnl pthread:	Linux, etcetera
dnl --thread-safe: KAI C++
dnl pth(read)-config: use pthread-config program (for GNU Pth library)

case "${host_os}" in
  *linux* | *bsd*)
    pthread_flags="-pthread"
    ;;
  *darwin*)
    pthread_flags="none"
    ;;
  *mingw* | *cygwin*)
    pthread_flags="-mthreads"
    ;;
  *solaris*)

  dnl On Solaris (at least, for some versions), libc contains stubbed
  dnl (non-functional) versions of the pthreads routines, so link-based
  dnl tests will erroneously succeed.  (We need to link with -pthreads/-mt/
  dnl -lpthread.)  (The stubs are missing pthread_cleanup_push, or rather
  dnl a function called by this macro, so we could check for that, but
  dnl who knows whether they'll stub that too in a future libc.)  So,
  dnl we'll just look for -pthreads and -lpthread first:

    pthread_flags="-pthreads pthread -mt -pthread $pthread_flags"
    ;;
esac

for flag in $pthread_flags; do\
    case $flag in
      none)
        AC_MSG_CHECKING([whether pthreads work without any flags])
        PTHREAD_CFLAGS=""
        PTHREAD_LIBS=""
        ;;
      -*)
        AC_MSG_CHECKING([whether pthreads work with $flag])
        PTHREAD_CFLAGS="$flag"
	      PTHREAD_LIBS=""
        ;;

      pth-config)
       AC_CHECK_PROG(pth_config, pth-config, yes, no)
       if test x"$pth_config" = xno; then
         continue;
       fi
       PTHREAD_CFLAGS="`pth-config --cflags`"
       PTHREAD_LIBS="`pth-config --ldflags` `pth-config --libs`"
       ;;

      pthread-config)
       AC_CHECK_PROG(pthread_config, pthread-config, yes, no)
       if test x"$pthread_config" = xno; then
         continue;
       fi
       PTHREAD_CFLAGS="`pthread-config --cflags`"
       PTHREAD_LIBS="`pthread-config --ldflags` `pthread-config --libs`"
       ;;

      *)
        AC_MSG_CHECKING([for the pthreads library -l$flag])
        PTHREAD_LIBS="-l$flag"
	      PTHREAD_CFLAGS=""
        ;;
    esac

    save_LIBS="$LIBS"
    save_CFLAGS="$CFLAGS"
    save_CXXFLAGS="$CXXFLAGS"
    LIBS="$PTHREAD_LIBS $LIBS"
    CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
    CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"

    dnl Check for various functions.  We must include pthread.h,
    dnl since some functions may be macros.  (On the Sequent, we
    dnl need a special flag -Kthread to make this header compile.)
    dnl We check for pthread_join because it is in -lpthread on IRIX
    dnl while pthread_create is in libc.  We check for pthread_attr_init
    dnl due to DEC craziness with -lpthreads.  We check for
    dnl pthread_cleanup_push because it is one of the few pthread
    dnl functions on Solaris that doesn't have a non-functional libc stub.
    dnl We try pthread_create on general principles.
    AC_TRY_LINK([#include <pthread.h>],
      [pthread_t th; pthread_join(th, 0);
      pthread_attr_init(0); pthread_cleanup_push(0, 0);
      pthread_create(0,0,0,0); pthread_cleanup_pop(0);],
      [pthreads=yes])

    LIBS="$save_LIBS"
    CFLAGS="$save_CFLAGS"
    CXXFLAGS="$save_CXXFLAGS"

    AC_MSG_RESULT($pthreads)
    if test "x${pthreads}" = xyes; then
       break;
    fi
done

dnl Try a manual search, useful for cross-compiling
if test "x${pthreads}" = xyes; then
  AC_MSG_CHECKING([searching for pthread library])
  for i in $libslist; do
    if test -f $i/libpthread.a -o -f $i/libpthread.${shlibext} -o -f $i/libpthread.dylib; then
      pthreads=yes
      if test ! x"$i" = x"/usr/lib" -a ! x"$i" = x"/usr/lib64"; then
        PTHREAD_LIBS="-L$i -lpthread"
        AC_MSG_RESULT([using $PTHREAD_LIBS])
        break
      else
        dnl If using Mingw, we have to use the pthreadGCE2 library,as it has
        dnl has exception handling support for C++.
        if test x"${host_os}" = x"mingw32"; then
          PTHREAD_LIBS="-lpthreadGCE2"
        else
          PTHREAD_LIBS="-lpthread"
        fi
        AC_MSG_RESULT([using $PTHREAD_LIBS])
        break
      fi
    fi
  done
  if test x"${PTHREAD_LIBS}" = "x"; then
    AC_MSG_RESULT(not found)
    pthreads=no
  fi
fi

dnl Various other checks:
if test "x$pthreads" = xyes; then
  save_LIBS="$LIBS"
  LIBS="$PTHREAD_LIBS $LIBS"
  save_CFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS $PTHREAD_CFLAGS"

  dnl Detect AIX lossage: JOINABLE attribute is called UNDETACHED.
  AC_MSG_CHECKING([for joinable pthread attribute])
  attr_name=unknown
  for attr in PTHREAD_CREATE_JOINABLE PTHREAD_CREATE_UNDETACHED; do
    AC_TRY_LINK([#include <pthread.h>], [int attr=$attr; return attr;],
      [attr_name=$attr; break])
  done
  AC_MSG_RESULT($attr_name)
  if test "$attr_name" != PTHREAD_CREATE_JOINABLE; then
    AC_DEFINE_UNQUOTED(PTHREAD_CREATE_JOINABLE, $attr_name,
      [Define to necessary symbol if this constant
      uses a non-standard name on your system.])
  fi

  AC_MSG_CHECKING([if more special flags are required for pthreads])
  flag=no
  case "${host_cpu}-${host_os}" in
    *-aix* | *-freebsd* | *-darwin*) flag="-D_THREAD_SAFE";;
    *solaris* | *-osf* | *-hpux*) flag="-D_REENTRANT";;
    *irix*) flag="-D_SGI_MP_SOURCE";;
  esac
  AC_MSG_RESULT(${flag})
  if test "x$flag" != xno; then
    PTHREAD_CFLAGS="$flag $PTHREAD_CFLAGS"
  fi

  LIBS="$save_LIBS"
  CFLAGS="$save_CFLAGS"

  dnl More AIX lossage: must compile with cc_r (or xlc_r)
  case "${host_os}" in
    aix* )
      case "$CC" in
    *xlc )
      AC_CHECK_PROG(PTHREAD_CC, xlc_r, xlc_r, ${CC}) ;;
    *cc )
      AC_CHECK_PROG(PTHREAD_CC, cc_r, cc_r, ${CC}) ;;
  esac
  case "$CXX" in
    *xlC )
      AC_CHECK_PROG(PTHREAD_CXX, xlC_r, xlC_r, ${CXX}) ;;
  esac
     ;;
  esac
fi

if test "${PTHREAD_CC}x" = "x"; then
  PTHREAD_CC="$CC"
fi
if test "${PTHREAD_CXX}x" = "x"; then
  PTHREAD_CXX="$CXX"
fi

AC_SUBST(PTHREAD_LIBS)
AC_SUBST(PTHREAD_CFLAGS)
AC_SUBST(PTHREAD_CC)
AC_SUBST(PTHREAD_CXX)

dnl Finally, execute ACTION-IF-FOUND/ACTION-IF-NOT-FOUND:
if test x"$pthreads" = xyes; then
  ifelse([$1],,AC_DEFINE(HAVE_PTHREADS,1,[Define if you have POSIX threads libraries and header files.]),[$1])
        :
else
  pthreads=no
  $2
fi
  AC_LANG_RESTORE
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
