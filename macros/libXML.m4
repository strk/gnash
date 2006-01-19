dnl Process this file with autoconf to produce a configure script.
dnl
dnl  Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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

AC_DEFUN([AM_PATH_LIBXML2],
[dnl 
dnl Get the cflags and libraries
dnl
AC_ARG_WITH(libxml,[  --with-libxml=PFX   Prefix where libxml is installed (optional)], libxml_prefix="$withval", libxml_prefix="")
AC_ARG_WITH(libxml-libraries,[  --with-libxml-libraries=DIR   Directory where libxml library is installed (optional)], libxml_libraries="$withval", libxml_libraries="")
AC_ARG_WITH(libxml-includes,[  --with-libxml-includes=DIR   Directory where libxml header files are installed (optional)], libxml_includes="$withval", libxml_includes="")
dnl AC_ARG_ENABLE(libxmltest, [  --disable-libxmltest       Do not try to compile and run a test libxml program],, enable_libxmltest=yes)

  if test "x$libxml_libraries" != "x" ; then
    LIBXML_LIBS="-L$libxml_libraries -lxml2"
  elif test "x$libxml_prefix" != "x" ; then
    LIBXML_LIBS="-L$libxml_prefix/lib -lxml2"
  fi

  if test "x$libxml_includes" != "x" ; then
    LIBXML_CFLAGS="-I$libxml_includes"
  elif test "x$libxml_prefix" != "x" ; then
    LIBXML_CFLAGS="-I$libxml_prefix/include"
  fi

  no_libxml=""
  AC_PATH_PROG(PKG_CONFIG, pkg-config, , ,[$PATH])
  AC_MSG_CHECKING(for libxml2)
  if test "x$PKG_CONFIG" != "x" ; then
    if test "x$LIBXML_CFLAGS" = "x" ; then
      LIBXML_CFLAGS=`$PKG_CONFIG --cflags libxml-2.0`
    fi

    if test "x$LIBXML_LIBS" = "x" ; then
      LIBXML_LIBS=`$PKG_CONFIG --libs libxml-2.0`
    fi
  else
    dirlist="/usr /usr/local /opt /home/latest"
    for i in $dirlist; do
      for j in `ls -dr $i/include/libxml2* 2>/dev/null ` ; do
         if test -f $j/libxml/parser.h; then
           LIBXML_CFLAGS=-I`(cd $j; pwd)`
           break
         fi
      done
      for j in `ls -dr $i/lib 2>/dev/null ` ; do
         if test -f $j/libxml2.so; then
           LIBXML_LIBS="-L`(cd $j; pwd)` -lxml2"
           break
         fi
      done
    done
  fi

  if test "x$LIBXML_CFLAGS" != "x" -a  "x$LIBXML_LIBS" != "x"; then
    AC_MSG_RESULT(yes)
    AC_DEFINE(HAVE_LIBXML,1,[Define this if you have libxml2 support available])
  else
    AC_MSG_RESULT(no)
  fi

  AC_SUBST(LIBXML_CFLAGS)
  AC_SUBST(LIBXML_LIBS)
])
