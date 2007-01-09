dnl 
dnl    Copyright (C) 2005, 2006 Free Software Foundation, Inc.
dnl  
dnl  This program is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU General Public License as published by
dnl  the Free Software Foundation; either version 2 of the License, or
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
dnl $Id: pthreads.m4,v 1.21 2007/01/09 00:43:43 rsavoye Exp $

AC_DEFUN([GNASH_PATH_PTHREADS],
[
AC_REQUIRE([AC_CANONICAL_HOST])
AC_LANG_SAVE
AC_LANG_C
pthreads=no

# We used to check for pthread.h first, but this fails if pthread.h
# requires special compiler flags (e.g. on True64 or Sequent).
# It gets checked for in the link test anyway.

# First of all, check if the user has set any of the PTHREAD_LIBS,
# etcetera environment variables, and if threads linking works using
# them:
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

# We must check for the threads library under a number of different
# names; the ordering is very important because some systems
# (e.g. DEC) have both -lpthread and -lpthreads, where one of the
# libraries is broken (non-POSIX).

# Create a list of thread flags to try.  Items starting with a "-" are
# C compiler flags, and other items are library names, except for "none"
# which indicates that we try without any flags at all, and "pthread-config"
# which is a program returning the flags for the Pth emulation library.

pthread_flags="pthreads none -Kthread -kthread lthread -pthread -pthreads -mthreads pthread --thread-safe -mt pthread-config pth-config"

# The ordering *is* (sometimes) important.  Some notes on the
# individual items follow:

# pthreads:	AIX (must check this before -lpthread)
# none:		in case threads are in libc; should be tried before -Kthread and
# 		other compiler flags to prevent continual compiler warnings
# -Kthread:	Sequent (threads in libc, but -Kthread needed for pthread.h)
# -kthread:	FreeBSD kernel threads (preferred to -pthread since SMP-able)
# lthread:	LinuxThreads port on FreeBSD (also preferred to -pthread)
# -pthread:	Linux/gcc (kernel threads), BSD/gcc (userland threads)
# -pthreads:	Solaris/gcc
# -mthreads:	Mingw32/gcc, Lynx/gcc
# -mt:		Sun Workshop C (may only link SunOS threads [-lthread], but it
#		doesn't hurt to check since this sometimes defines pthreads too;
#		also defines -D_REENTRANT)
#		... -mt is also the pthreads flag for HP/aCC
# pthread:	Linux, etcetera
# --thread-safe: KAI C++
# pth(read)-config: use pthread-config program (for GNU Pth library)

case "${host_cpu}-${host_os}" in
        *solaris*)

        # On Solaris (at least, for some versions), libc contains stubbed
        # (non-functional) versions of the pthreads routines, so link-based
        # tests will erroneously succeed.  (We need to link with -pthreads/-mt/
        # -lpthread.)  (The stubs are missing pthread_cleanup_push, or rather
        # a function called by this macro, so we could check for that, but
        # who knows whether they'll stub that too in a future libc.)  So,
        # we'll just look for -pthreads and -lpthread first:

        pthread_flags="-pthreads pthread -mt -pthread $pthread_flags"
        ;;
esac

if test x"$pthreads" = xno; then
for flag in $pthread_flags; do

        case $flag in
                none)
                AC_MSG_CHECKING([whether pthreads work without any flags])
                ;;

                -*)
                AC_MSG_CHECKING([whether pthreads work with $flag])
                PTHREAD_CFLAGS="$flag"
                ;;

                pth-config)
                AC_CHECK_PROG(pth_config, pth-config, yes, no)
                if test x"$pth_config" = xno; then continue; fi
                PTHREAD_CFLAGS="`pth-config --cflags`"
                PTHREAD_LIBS="`pth-config --ldflags` `pth-config --libs`"
                ;;

                pthread-config)
                AC_CHECK_PROG(pthread_config, pthread-config, yes, no)
                if test x"$pthread_config" = xno; then continue; fi
                PTHREAD_CFLAGS="`pthread-config --cflags`"
                PTHREAD_LIBS="`pthread-config --ldflags` `pthread-config --libs`"
                ;;

                *)
                AC_MSG_CHECKING([for the pthreads library -l$flag])
                PTHREAD_LIBS="-l$flag"
                ;;
        esac

        save_LIBS="$LIBS"
        save_CFLAGS="$CFLAGS"
        LIBS="$PTHREAD_LIBS $LIBS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"

        # Check for various functions.  We must include pthread.h,
        # since some functions may be macros.  (On the Sequent, we
        # need a special flag -Kthread to make this header compile.)
        # We check for pthread_join because it is in -lpthread on IRIX
        # while pthread_create is in libc.  We check for pthread_attr_init
        # due to DEC craziness with -lpthreads.  We check for
        # pthread_cleanup_push because it is one of the few pthread
        # functions on Solaris that doesn't have a non-functional libc stub.
        # We try pthread_create on general principles.
        AC_TRY_LINK([#include <pthread.h>],
                    [pthread_t th; pthread_join(th, 0);
                     pthread_attr_init(0); pthread_cleanup_push(0, 0);
                     pthread_create(0,0,0,0); pthread_cleanup_pop(0); ],
                    [pthreads=yes])

        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"

        AC_MSG_RESULT($pthreads)
        if test "x$pthreads" = xyes; then
                break;
        fi

        PTHREAD_LIBS=""
        PTHREAD_CFLAGS=""
done
fi

# Various other checks:
if test "x$pthreads" = xyes; then
        save_LIBS="$LIBS"
        LIBS="$PTHREAD_LIBS $LIBS"
        save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"

        # Detect AIX lossage: JOINABLE attribute is called UNDETACHED.
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

        # More AIX lossage: must compile with cc_r (or xlc_r)
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

# Finally, execute ACTION-IF-FOUND/ACTION-IF-NOT-FOUND:
if test x"$pthreads" = xyes; then
	if test "${PTHREAD_LIBS}x" = "x"; then
		dnl PTHREAD_LIBS="-lpthread"
		PTHREAD_LIBS=" "
	fi
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
