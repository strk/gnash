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

AC_DEFUN([GNASH_PATH_ZLIB],
[
  dnl Lool for the header
  AC_ARG_WITH(zlib_incl, [  --with-zlib-incl        directory where zlib header is], with_zlib_incl=${withval})
  AC_CACHE_VAL(ac_cv_path_zlib_incl,[
  if test x"${with_zlib_incl}" != x ; then
    if test -f ${with_zlib_incl}/zlib.h ; then
      ac_cv_path_zlib_incl=`(cd ${with_zlib_incl}; pwd)`
    elif test -f ${with_zlib_incl}/zlib.h ; then
      ac_cv_path_zlib_incl=`(cd ${with_zlib_incl}; pwd)`
    else
      AC_MSG_ERROR([${with_zlib_incl} directory doesn't contain zliblib.h])
    fi
  fi
  ])

  if test x"${ac_cv_path_zlib_incl}" = x ; then
    AC_MSG_CHECKING([for zlib header])
    incllist="${prefix}/include /sw/include /usr/local/include /home/latest/include /opt/include /usr/include .. ../.."

    for i in $incllist; do
      if test -f $i/zlib.h; then
        ac_cv_path_zlib_incl=$i
        break
      fi
    done

    ZLIB_CFLAGS=""
    if test x"${ac_cv_path_zlib_incl}" = x ; then
      AC_MSG_RESULT(none)
      AC_CHECK_HEADERS(zlib.h, [ac_cv_path_zlib_incl=""])
    else
      AC_MSG_RESULT(${ac_cv_path_zlib_incl})
      if test x"${ac_cv_path_zlib_incl}" != x"/usr/include"; then
        ac_cv_path_zlib_incl="-I${ac_cv_path_zlib_incl}"
      else
        ac_cv_path_zlib_incl=""
      fi
    fi
  fi

  if test x"${ac_cv_path_zlib_incl}" != x ; then
    ZLIB_CFLAGS="${ac_cv_path_zlib_incl}"
  fi

  dnl Look for the library
  AC_ARG_WITH(zlib_lib, [  --with-zlib-lib         directory where zlib library is], with_zlib_lib=${withval})
  AC_MSG_CHECKING([for zlib library])
  AC_CACHE_VAL(ac_cv_path_zlib_lib,[
  if test x"${with_zlib_lib}" != x ; then
    if test -f ${with_zlib_lib}/libz.a ; then
      ac_cv_path_zlib_lib=`(cd ${with_zlib_lib}; pwd)`
    elif test -f ${with_zlib_lib}/libz.a -o -f ${with_zlib_lib}/libz.so -o -f ${with_zlib_lib}/libz.dylib; then
      ac_cv_path_zlib_lib=`(cd ${with_zlib_incl}; pwd)`
    else
      AC_MSG_ERROR([${with_zlib_lib} directory doesn't contain libz.a])
    fi
  fi
  ])

  if test x"${ac_cv_path_zlib_lib}" = x ; then
    liblist="${prefix}/lib64 ${prefix}/lib /usr/lib64 /usr/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib .. ../.."

    for i in $liblist; do
    if test -f $i/libz.a -o -f $i/libz.so -o -f $i/libz.dylib; then
       ac_cv_path_zlib_lib=$i
       break
    fi
    done

    ZLIB_LIBS=""
    if test x"${ac_cv_path_zlib_lib}" = x ; then
      AC_MSG_RESULT(none)
      dnl if we can't find libzlib via the path, see if it's in the compiler path
      AC_CHECK_LIB(zlib, compress2, ${ac_cv_path_zlib_lib}="-lz")
    else
      AC_MSG_RESULT(${ac_cv_path_zlib_lib})
      if test x"${ac_cv_path_zlib_lib}" != x"/usr/lib"; then
        ac_cv_path_zlib_lib="-L${ac_cv_path_zlib_lib} -lz"
      else
        ac_cv_path_zlib_lib="-lz"
      fi
    fi
  fi

  if test x"${ac_cv_path_zlib_lib}" != x ; then
    ZLIB_LIBS="${ac_cv_path_zlib_lib}"
  fi

  AC_SUBST(ZLIB_CFLAGS)
  AC_SUBST(ZLIB_LIBS)
])
