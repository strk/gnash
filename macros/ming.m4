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
dnl

# Configure paths for Ming
# Author: Sandro Santilli <strk@keybit.net>
#
# This macro uses ming-config, which was
# not available as of Ming 0.3beta1
#
#
# Use: 
#	AC_PATH_MING
#
# Provides:
#	MING_VERSION	  - Ming version string (example: "0.4.1" or "0.4.0.beta2")
#	MING_VERSION_CODE - a 8digits number encoding Major, Minor, Patch and Beta numbers.
#	                    examples: 00040002 (0.4.0.beta2) 00040100 (0.4.1)
#	MING_CFLAGS
#	MING_LIBS
#	MAKESWF
#

AC_DEFUN([AC_PATH_MING], [
  MING_CFLAGS=""
  MING_LIBS=""

  AC_ARG_WITH(ming,[  --with-ming=[<ming-config>]    Path to the ming-config command], [
    case "${withval}" in
      yes|no) ;;
      *) MING_CONFIG=${withval} ;;
    esac
  ], MING_CONFIG="")

  if test x"$MING_CONFIG" = "x"; then
    AC_PATH_PROG(MING_CONFIG, ming-config, ,[${pathlist}])
  fi

  if test x"$MING_CONFIG" != "x"; then
    MING_VERSION=`$MING_CONFIG --version`
    major=`echo $MING_VERSION | cut -d '.' -f 1`
    minor=`echo $MING_VERSION | cut -d '.' -f 2`
    micro=`echo $MING_VERSION | cut -d '.' -f 3`
    beta=`echo $MING_VERSION | sed -ne 's/.*beta\([[0-9]]*\).*/\1/p'`
    dnl This is a little screwy. The previous Ming release was tagged beta5,
    dnl but the newer one is tagged rc1. This makes it look like rc1 is older the
    dnl beta 5 release, as it only looks at the last integer. So we fudge the
    dnl numbers so tc1 comes after beta 5. ie.. this looks like beta 6.
    if test -z $beta; then
      beta=`echo $MING_VERSION | sed -ne 's/.*rc\([[0-9]]*\).*/\1/p'`
      if test x"$beta" != x; then
        beta=`eval expr $beta + 5`
      fi
    fi
    MING_VERSION_CODE=`printf %2.2d%2.2d%2.2d%2.2d $major $minor $micro $beta`
    MING_CFLAGS=`$MING_CONFIG --cflags`
    MING_LIBS=`$MING_CONFIG --libs`
    MING_PATH=`$MING_CONFIG --bindir`
    AC_PATH_PROG([MAKESWF], [makeswf], , [$MING_PATH:$PATH])
  fi

  dnl if MING_LIBS is empty, no MING_CONFIG found.
  if test x"${MING_LIBS}" = x; then
    AC_ARG_WITH(ming_incl, AC_HELP_STRING([--with-ming-incl], [Directory where Ming header is]), with_ming_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_ming_incl, [
      if test x"${with_ming_incl}" != x ; then
        if test -f ${with_ming_incl}/ming.h ; then
	  ac_cv_path_ming_incl="-I`(cd ${with_ming_incl}; pwd)`"
        else
	  AC_MSG_ERROR([${with_ming_incl} directory doesn't contain minglib.h])
        fi
      fi
    ])

    dnl if they didn't successfully force it, see if it's not in /usr/include.
    if test x"${ac_cv_path_ming_incl}" = x; then
      for i in $incllist; do
	      if test -f $i/ming.h; then
      	  if test x"$i" != x"/usr/include"; then
      	    ac_cv_path_ming_incl="-I$i"
      	    break
          else
	          ac_cv_path_ming_incl=""
	          break
	        fi
        fi
      done
    fi

    dnl if it's not somewhere weird, make sure we can #include it already.
    if test x"${ac_cv_path_ming_incl}" = x; then
      AC_CHECK_HEADERS(ming.h, [ac_cv_path_ming_incl=""])
    fi

    AC_MSG_CHECKING([for Ming header])
    AC_MSG_RESULT(${ac_cv_path_ming_incl})

    dnl Look for the library
    AC_ARG_WITH(ming_lib, AC_HELP_STRING([--with-ming-lib], [directory where ming library is]), with_ming_lib=${withval})
    AC_CACHE_VAL(ac_cv_path_ming_lib, [
      if test x"${with_ming_lib}" != x ; then
        if test -f ${with_ming_lib}/libming.a -o -f ${with_ming_lib}/libming.${shlibext}; then
	        ac_cv_path_ming_lib="`(cd ${with_ming_lib}; pwd)`"
        else
          AC_MSG_ERROR([${with_ming_lib} directory doesn't contain libming.])
        fi
      fi
    ])

    dnl look for the library, but don't add -L if it's in a standard place
    for i in $libslist; do
      if test -f $i/libming.a -o -f $i/libming.${shlibext}; then
      	if test ! x"$i" = x"/usr/lib" -a ! x"$i" = x"/usr/lib64"; then
      	  ac_cv_path_ming_lib="-L$i -lming"
      	  break
        else
	        ac_cv_path_ming_lib="-lming"
      	  break
        fi
      fi
    done
    if test x"${ac_cv_path_ming_lib}" = x; then
      AC_CHECK_LIB(ming, ming_init_io, [ac_cv_path_ming_lib=""])
    fi
    AC_MSG_CHECKING([for libming library])
    AC_MSG_RESULT(${ac_cv_path_ming_lib})

    if test x"${ac_cv_path_ming_incl}" != x ; then
      MING_CFLAGS="${ac_cv_path_ming_incl}"
      AC_MSG_RESULT(${ac_cv_path_ming_incl})
    else
      MING_CFLAGS=""
    fi

    if test x"${ac_cv_path_ming_lib}" != x ; then
      MING_LIBS="${ac_cv_path_ming_lib}"
      AC_MSG_RESULT(${ac_cv_path_ming_lib})
    else
      MING_LIBS=""
    fi
  fi

  AC_SUBST(MING_VERSION_CODE)
  AC_SUBST(MING_VERSION)
  AC_SUBST(MING_CFLAGS)
  AC_SUBST(MING_LIBS)
  AC_SUBST(MAKESWF)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
