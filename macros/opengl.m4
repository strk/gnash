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

dnl Check for OpenGL & [Glut | GLU]
AC_DEFUN([AM_PATH_OPENGL], [

dnl Add XML support, if specified.
  AC_ARG_ENABLE(opengl, [  --disable-opengl           Disable support for OpenGL],
  [case "${enableval}" in
    yes) opengl=no ;;
    no)  pengl=yes ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for disable-opengl option]) ;;
  esac],opengl=yes)

  AM_CONDITIONAL(OPENGL, test x$opengl = xyes)

  AC_ARG_WITH(opengl,[  --with-opengl=PFX   Prefix where opengl is installed (optional)], opengl_prefix="$withval", opengl_prefix="")
  AC_ARG_WITH(opengl-libraries,[  --with-opengl-libraries=DIR   Directory where opengl library is installed (optional)], opengl_libraries="$withval", opengl_libraries="")
  AC_ARG_WITH(opengl-includes,[  --with-opengl-includes=DIR   Directory where opengl header files are installed (optional)], opengl_includes="$withval", opengl_includes="")

  if test "x$opengl_libraries" != "x" ; then
    OPENGL_LIBS="-L$opengl_libraries -lxml2"
  elif test "x$opengl_prefix" != "x" ; then
    OPENGL_LIBS="-L$opengl_prefix/lib -lxml2"
  fi

  if test "x$opengl_includes" != "x" ; then
    OPENGL_CFLAGS="-I$opengl_includes"
  elif test "x$opengl_prefix" != "x" ; then
    OPENGL_CFLAGS="-I$opengl_prefix/include"
  fi

  if test x"$opengl" = x"yes"; then
    LIBS_SAVE="$LIBS"
    LIBS="$LIBS $OPENGL_LIBS"
    AC_CHECK_LIB(GL, glBegin)
    LIBS="$LIBS_SAVE"
    AC_CHECK_HEADERS(GL/gl.h)
    AC_CHECK_LIB(GLU, gluPerspective)
    if test x"$HAVE_GL" = x; then
      AC_MSG_CHECKING(futher locations for OpenGL Header files)
      dirlist="/usr/X11R6 /usr /usr/local /opt /home/latest"
      for i in $dirlist; do
        if test -f $i/include/GL/gl.h; then
          OPENGL_CFLAGS=-I`(cd $i/include; pwd)`
          break
        fi
      done
      if test x"$OPENGL_CFLAGS" != x; then
	AC_MSG_RESULT(yes)
      else
	AC_MSG_RESULT(no)
      fi
      AC_MSG_CHECKING(futher locations for OpenGL library files)
      for i in $dirlist; do
        if test -f $i/lib/libGL.a; then
          OPENGL_LIBS="-L`(cd $i/lib; pwd)` -lGL -lGLU"
          break
        fi
      done
      if test x"$OPENGL_LIBS" != x; then
	AC_MSG_RESULT(yes)
      else
	AC_MSG_RESULT(no)
      fi
    fi 
  fi

  AC_SUBST(OPENGL_CFLAGS)
  AC_SUBST(OPENGL_LIBS)
  AM_CONDITIONAL(HAVE_OPENGL, [test x$ac_use_opengl = "xyes"])
])
