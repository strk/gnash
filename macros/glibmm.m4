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

AC_DEFUN([AM_PATH_GLIBMM],
[dnl 
dnl Get the cflags and libraries
dnl
AC_ARG_WITH(glibmm,[  --with-glibmm=PFX   Prefix where glibmm is installed (optional)], glibmm_prefix="$withval", glibmm_prefix="")
AC_ARG_WITH(glibmm-libraries,[  --with-glibmm-libraries=DIR   Directory where glibmm library is installed (optional)], glibmm_libraries="$withval", glibmm_libraries="")
AC_ARG_WITH(glibmm-includes,[  --with-glibmm-includes=DIR   Directory where glibmm header files are installed (optional)], glibmm_includes="$withval", glibmm_includes="")
dnl AC_ARG_ENABLE(glibmmtest, [  --disable-glibmmtest       Do not try to compile and run a test glibmm program],, enable_glibmmtest=yes)

  if test "x$glibmm_libraries" != "x" ; then
    GLIBMM_LIBS="-L$glibmm_libraries"
  elif test "x$glibmm_prefix" != "x" ; then
    GLIBMM_LIBS="-L$glibmm_prefix/lib"
  elif test "x$prefix" != "xNONE" ; then
    GLIBMM_LIBS="-L$libdir"
  fi

  if test "x$glibmm_includes" != "x" ; then
    GLIBMM_CFLAGS="-I$glibmm_includes"
  elif test "x$glibmm_prefix" != "x" ; then
    GLIBMM_CFLAGS="-I$glibmm_prefix/include"
  elif test "$prefix" != "xNONE"; then
    GLIBMM_CFLAGS="-I$prefix/include"
  fi

  AC_MSG_CHECKING(for glibmm)
  no_glibmm=""

  if test "x$GLIBMM_CFLAGS" = "x" ; then
    GLIBMM_CFLAGS=`pkg-config --cflags glibmm2`
  fi

  if test "x$GLIBMM_LIBS" = "x" ; then
    GLIBMM_LIBS=`pkg-config --libs glibmm2`
  fi

  if test "x$GLIBMM_CFLAGS" != "x" -a  "x$GLIBMM_LIBS" != "x"; then
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
  fi

  AC_SUBST(GLIBMM_CFLAGS)
  AC_SUBST(GLIBMM_LIBS)
])
