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
dnl  
dnl  Linking Gnash statically or dynamically with other modules is making
dnl  a combined work based on Gnash. Thus, the terms and conditions of
dnl  the GNU General Public License cover the whole combination.
dnl  
dnl  In addition, as a special exception, the copyright holders of Gnash give
dnl  you permission to combine Gnash with free software programs or
dnl  libraries that are released under the GNU LGPL and/or with Mozilla, 
dnl  so long as the linking with Mozilla, or any variant of Mozilla, is
dnl  through its standard plug-in interface. You may copy and distribute
dnl  such a system following the terms of the GNU GPL for Gnash and the
dnl  licenses of the other code concerned, provided that you include the
dnl  source code of that other code when and as the GNU GPL requires
dnl  distribution of source code. 
dnl  
dnl  Note that people who make modified versions of Gnash are not obligated
dnl  to grant this special exception for their modified versions; it is
dnl  their choice whether to do so.  The GNU General Public License gives
dnl  permission to release a modified version without this exception; this
dnl  exception also makes it possible to release a modified version which
dnl  carries forward this exception.
dnl 

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
