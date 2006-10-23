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

dnl Linking Gnash statically or dynamically with other modules is making a
dnl combined work based on Gnash. Thus, the terms and conditions of the GNU
dnl General Public License cover the whole combination.
dnl
dnl As a special exception, the copyright holders of Gnash give you
dnl permission to combine Gnash with free software programs or libraries
dnl that are released under the GNU LGPL and with code included in any
dnl release of Talkback distributed by the Mozilla Foundation. You may
dnl copy and distribute such a system following the terms of the GNU GPL
dnl for all but the LGPL-covered parts and Talkback, and following the
dnl LGPL for the LGPL-covered parts.
dnl
dnl Note that people who make modified versions of Gnash are not obligated
dnl to grant this special exception for their modified versions; it is their
dnl choice whether to do so. The GNU General Public License gives permission
dnl to release a modified version without this exception; this exception
dnl also makes it possible to release a modified version which carries
dnl forward this exception.
dnl  
dnl 

dnl $Id: gnashpkgtool.m4,v 1.4 2006/10/23 23:01:09 nihilus Exp $

dnl Generic macros for finding and setting include-paths and library-path
dnl for packages. Implements GNASH_PKG_INCLUDES() and GNASH_PKG_LIBS().

AC_DEFUN([GNASH_PKG_INCLUDES], dnl GNASH_PKG_INCLUDES(jpeg, [jpeglib.h], [jpeg images])
[
  AC_ARG_ENABLE($1, AC_HELP_STRING([--enable-$1], [Enable support for $3.]),
  [case "${enableval}" in
    yes) $1=yes ;;
    no)  $1=no ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for enable-$1 option]) ;;
  esac], $1=yes)

  if test x"$1" = x"yes"; then
    dnl Look for the header
  AC_ARG_WITH($1_incl, AC_HELP_STRING([--with-$1-incl], [Directory where $2 header is]), with_$1_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_$1_incl,[
    if test x"${with_$1_incl}" != x ; then
      if test -f ${with_$1_incl}/$2 ; then
	ac_cv_path_$1_incl=-I`(cd ${with_$1_incl}; pwd)`
      else
	AC_MSG_ERROR([${with_$1_incl} directory doesn't contain $2])
      fi
    fi
    ])

    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_$1_incl}" = x; then
      $PKG_CONFIG --exists $1 && ac_cv_path_$1_incl=`$PKG_CONFIG --cflags $1`
    fi

    dnl If the path hasn't been specified, go look for it.
    if test x"${ac_cv_path_$1_incl}" = x; then
      AC_CHECK_HEADERS($2, [ac_cv_path_$1_incl=""],[
      if test x"${ac_cv_path_$1_incl}" = x; then
        incllist="${prefix}/include /sw/include /usr/nekoware/include /usr/freeware/include /opt/local/include /usr/local/include /home/latest/include /opt/include /usr/include /usr/pkg/include .. ../.."

        for i in $incllist; do
	  if test -f $i/$2; then
	    if test x"$i" != x"/usr/include"; then
	      ac_cv_path_$1_incl="-I$i"
	      break
            else
	      ac_cv_path_$1_incl=""
	      break
	    fi
	  fi
        done
      fi])
    else
      if test x"${ac_cv_path_$1_incl}" != x"/usr/include"; then
	ac_cv_path_$1_incl="-I${ac_cv_path_$1_incl}"
       else
	ac_cv_path_$1_incl=""
      fi
    fi
      AC_MSG_CHECKING([for $2 header])
      AC_MSG_RESULT(${ac_cv_path_$1_incl})

    if test x"${ac_cv_path_$1_incl}" != x ; then
      $1_CFLAGS="${ac_cv_path_$1_incl}"
      AC_MSG_RESULT(${ac_cv_path_$1_incl})
    else
      $1_CFLAGS=""
    fi

  AM_CONDITIONAL(HAVE_$1, [test x$1 = xyes])

  AC_SUBST($1_CFLAGS)

])
