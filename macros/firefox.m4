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

# Configure paths for libfirefox
AC_DEFUN([AC_PATH_FIREFOX],
[dnl 
dnl Get the cflags and libraries
dnl
AC_ARG_WITH(firefox,[  --with-firefox=PFX   Prefix where firefox is installed (optional)], firefox_prefix="$withval", firefox_prefix="")
AC_ARG_WITH(firefox-libraries,[  --with-firefox-libraries=DIR   Directory where firefox library is installed (optional)], firefox_libraries="$withval", firefox_libraries="")
AC_ARG_WITH(firefox-includes,[  --with-firefox-includes=DIR   Directory where firefox header files are installed (optional)], firefox_includes="$withval", firefox_includes="")

  if test "x$firefox_libraries" != "x" ; then
    FIREFOX_LIBS="-L$firefox_libraries"
  elif test "x$firefox_prefix" != "x" ; then
    FIREFOX_LIBS="-L$firefox_prefix/lib"
  elif test "x$prefix" != "xNONE" ; then
    FIREFOX_LIBS="-L$libdir"
  fi

  if test "x$firefox_includes" != "x" ; then
    FIREFOX_CFLAGS="-I$firefox_includes"
  elif test "x$firefox_prefix" != "x" ; then
    FIREFOX_CFLAGS="-I$firefox_prefix/include"
  elif test "$prefix" != "NONE"; then
    FIREFOX_CFLAGS="-I$prefix/include"
  fi

  AC_MSG_CHECKING(for firefox)
  no_firefox=""

  if test "x$FIREFOX_CFLAGS" = "x" ; then
    FIREFOX_CFLAGS=`firefox-config --cflags`
  fi

  if test "x$FIREFOX_LIBS" = "x" ; then
    FIREFOX_LIBS=`firefox-config --libs`
  fi

  if test "x$FIREFOX_LIBS" != "x" ; then
    FIREFOX_DEFS=`firefox-config --defines`
    FIREFOX_PLUGINS=`echo $FIREFOX_LIBS | sed -e 's:-L::'`/plugins
  fi

  if test "x$FIREFOX_CFLAGS" != "x" -a  "x$FIREFOX_LIBS" != "x"; then
    AC_MSG_RESULT(yes)
    AC_DEFINE(HAVE_FIREFOX,1,[Define this if you have firefox support available])
  else
    AC_MSG_RESULT(no)
  fi

  AC_SUBST(FIREFOX_CFLAGS)
  AC_SUBST(FIREFOX_LIBS)
  AC_SUBST(FIREFOX_DEFS)
  AC_SUBST(FIREFOX_PLUGINS)
])
