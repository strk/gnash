dnl Process this file with autoconf to produce a configure script.
dnl
dnl  Copyright (C) 2005 Free Software Foundation, Inc.
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
dnl
dnl  You should have received a copy of the GNU General Public License
dnl  along with this program; if not, write to the Free Software
dnl  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

AC_DEFUN([AM_PATH_JPEG],
[
  dnl Lool for the header
  AC_ARG_WITH(jpeg_incl, [  --with-jpeg-incl        directory where jpeg header is], with_jpeg_incl=${withval})
  AC_CACHE_VAL(ac_cv_path_jpeg_incl,[
  if test x"${with_jpeg_incl}" != x ; then
    if test -f ${with_jpeg_incl}/jpeglib.h ; then
      ac_cv_path_jpeg_incl=`(cd ${with_jpeg_incl}; pwd)`
    elif test -f ${with_jpeg_incl}/jpeglib.h ; then
      ac_cv_path_jpeg_incl=`(cd ${with_jpeg_incl}; pwd)`
    else
      AC_MSG_ERROR([${with_jpeg_incl} directory doesn't contain jpeglib.h])
    fi
  fi
  ])

  if test x"${ac_cv_path_jpeg_incl}" = x ; then
    AC_MSG_CHECKING([for jpeg header])
    incllist="/sw/include /usr/local/include /home/latest/include /opt/include /usr/include .. ../.."

    for i in $incllist; do
      if test -f $i/jpeglib.h; then
        ac_cv_path_jpeg_incl=$i
        break
      fi
    done

    JPEG_CFLAGS=""
    if test x"${ac_cv_path_jpeg_incl}" = x ; then
      AC_MSG_RESULT(none)
      AC_CHECK_HEADERS(jpeglib.h, [ac_cv_path_jpeg_incl=""])
    else
      AC_MSG_RESULT(${ac_cv_path_jpeg_incl})
      if test x"${ac_cv_path_jpeg_incl}" != x"/usr/include"; then
        ac_cv_path_jpeg_incl="-I${ac_cv_path_jpeg_incl}"
      else
        ac_cv_path_jpeg_incl=""
      fi
    fi
  fi

  if test x"${ac_cv_path_jpeg_incl}" != x ; then
    JPEG_CFLAGS="${ac_cv_path_jpeg_incl}"
  fi

  dnl Look for the library
  AC_ARG_WITH(jpeg_lib, [  --with-jpeg-lib         directory where jpeg library is], with_jpeg_lib=${withval})
  AC_MSG_CHECKING([for jpeg library])
  AC_CACHE_VAL(ac_cv_path_jpeg_lib,[
  if test x"${with_jpeg_lib}" != x ; then
    if test -f ${with_jpeg_lib}/libjpeg.a ; then
      ac_cv_path_jpeg_lib=`(cd ${with_jpeg_lib}; pwd)`
    elif test -f ${with_jpeg_lib}/libjpeg.a -o -f ${with_jpeg_lib}/libjpeg.so; then
      ac_cv_path_jpeg_lib=`(cd ${with_jpeg_incl}; pwd)`
    else
      AC_MSG_ERROR([${with_jpeg_lib} directory doesn't contain libjpeg.a])
    fi
  fi
  ])

  if test x"${ac_cv_path_jpeg_lib}" = x ; then
    liblist="/sw/lib /usr/local/lib /home/latest/lib /opt/lib /usr/lib .. ../.."

    for i in $liblist; do
    if test -f $i/libjpeg.a -o -f $i/libjpeg.so; then
       ac_cv_path_jpeg_lib=$i
       break
    fi
    done

    JPEG_LIBS=""
    if test x"${ac_cv_path_jpeg_lib}" = x ; then
      AC_MSG_RESULT(none)
      dnl if we can't find libjpeg via the path, see if it's in the compiler path
      AC_CHECK_LIB(jpeg, jpeg_mem_init, JPEG_LIBS="-ljpeg")
    else
      AC_MSG_RESULT(${ac_cv_path_jpeg_lib})
      if test x"${ac_cv_path_jpeg_lib}" != x"/usr/lib"; then
        ac_cv_path_jpeg_lib="-L${ac_cv_path_jpeg_lib} -ljpeg"
      else
        ac_cv_path_jpeg_lib="-ljpeg"
      fi
    fi
  fi

  if test x"${ac_cv_path_jpeg_lib}" != x ; then
    JPEG_LIBS="${ac_cv_path_jpeg_lib}"
  fi

  AC_SUBST(JPEG_CFLAGS)
  AC_SUBST(JPEG_LIBS)
])

dnl   AC_LINK_IFELSE([AC_LANG_PROGRAM([[void *f;
dnl 					  #libude <stdlib.h>
dnl 					  #libude <stdio.h>
dnl 					  #libude <jpeglib.h>]],
dnl 					[[f = jpeg_has_multiple_scans;]])],
dnl 		       [have_jpeg="yes"], [have_jpeg="no"])


