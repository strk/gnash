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

AC_DEFUN([AM_PATH_LIBXMLCPP],
[dnl 
dnl Get the cflags and libraries
dnl
AC_ARG_WITH(libxmlcpp,[  --with-libxmlcpp=PFX   Prefix where libxml++ is installed (optional)], libxmlcpp_prefix="$withval", libxmlcpp_prefix="")
AC_ARG_WITH(libxmlcpp-libraries,[  --with-libxmlcpp-libraries=DIR   Directory where libxml++ library is installed (optional)], libxmlcpp_libraries="$withval", libxmlcpp_libraries="")
AC_ARG_WITH(libxmlcpp-includes,[  --with-libxmlcpp-includes=DIR   Directory where libxmlc++ header files are installed (optional)], libxmlcpp_includes="$withval", libxmlcpp_includes="")
dnl AC_ARG_ENABLE(libxmlcpptest, [  --disable-libxmlcpptest       Do not try to compile and run a test libxmlcpp program],, enable_libxmlcpptest=yes)

  if test "x$libxmlcpp_libraries" != "x" ; then
    LIBXMLCPP_LIBS="-L$libxmlcpp_libraries"
  elif test "x$libxmlcpp_prefix" != "x" ; then
    LIBXMLCPP_LIBS="-L$libxmlcpp_prefix/lib"
  elif test "x$prefix" != "xNONE" ; then
    LIBXMLCPP_LIBS="-L$libdir"
  fi

  if test "x$libxmlcpp_includes" != "x" ; then
    LIBXMLCPP_CFLAGS="-I$libxmlcpp_includes"
  elif test "x$libxmlcpp_prefix" != "x" ; then
    LIBXMLCPP_CFLAGS="-I$libxmlcpp_prefix/include"
  elif test "$prefix" != "xNONE"; then
    LIBXMLCPP_CFLAGS="-I$prefix/include"
  fi

  AC_MSG_CHECKING(for libxmlcpp)
  no_libxmlcpp=""

  if test "x$LIBXMLCPP_CFLAGS" = "x" ; then
    LIBXMLCPP_CFLAGS=`pkg-config --cflags libxml++-2`
  fi

  if test "x$LIBXMLCPP_LIBS" = "x" ; then
    LIBXMLCPP_LIBS=`pkg-config --libs libxml++-2`
  fi

  if test "x$LIBXMLCPP_CFLAGS" != "x" -a  "x$LIBXMLCPP_LIBS" != "x"; then
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
  fi

  AC_SUBST(LIBXMLCPP_CFLAGS)
  AC_SUBST(LIBXMLCPP_LIBS)
])
