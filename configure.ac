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
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.50)
AC_INIT(server/gnash.h)
AM_CONFIG_HEADER(config.h)

AC_CANONICAL_TARGET

dnl These are required by automake
AM_INIT_AUTOMAKE(gnash, 0.7)
AM_MAINTAINER_MODE
AC_PROG_MAKE_SET

dnl Use the POSIX Threads library, if specified.
AC_ARG_ENABLE(pthreads, [  --enable-pthreads       Enable support for pthreads],
[case "${enableval}" in
  yes) pthreads=yes ;;
  no)  pthreads=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for enable-pthreads option]) ;;
esac])

if test x$pthreads = xyes; then
    AC_DEFINE([HAVE_PTHREADS], [], [Use POSIX Threads])
fi

dnl Add XML support, if specified.
AC_ARG_ENABLE(libxml, [  --disable-xml           Disable support for xml and xmlsocket],
[case "${enableval}" in
  yes) disablexml=yes ;;
  no)  disablexml=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for disable-xml option]) ;;
esac],disablexml=no)

if test x$enable_xml != xno; then
    AM_PATH_LIBXML2
fi
AM_CONDITIONAL(XML, test x$enable_xml = xyes)

dnl Fix the Intel 810 LOD bias problem
AC_ARG_ENABLE(i810-lod-bias, [  --enable-i810-lod-bias  Enable fix for Intel 810 LOD bias problem],
[case "${enableval}" in
  yes) i810lodbias=yes ;;
  no)  i810lodbias=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for enable-i810-lod-bias option]) ;;
esac])

if test x$i810lodbias = xyes; then
    AC_DEFINE([FIX_I810_LOD_BIAS], [], [Fix i810 LOD bias problem])
fi

dnl Use the DMalloc library, if specified.
AC_ARG_ENABLE(dmalloc, [  --enable-dmalloc        Enable support for DMalloc],
[case "${enableval}" in
  yes) dmalloc=yes ;;
  no)  dmalloc=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for enable-malloc option]) ;;
esac])

if test x$dmalloc = xyes; then
    AC_DEFINE([USE_DMALLOC], [], [Use DMalloc])
    DMALLOC_LIB="-ldmallocxx"
else
    DMALLOC_LIB=""
fi
AC_SUBST(DMALLOC_LIB)

dnl Use the Dom parser from libxml2, not the xmlReader one
dnl The default is to use the Dom parser
dom=yes
AC_ARG_ENABLE(dom, [  --enable-dom         Enable support for Dom parser],
[case "${enableval}" in
  yes) dom=yes ;;
  no)  dom=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for enable-malloc option]) ;;
esac], dom=yes)

if test x$dom = xyes; then
    AC_DEFINE([USE_DOM], [], [Use Dom parser])
fi

dnl Use an xmlReader parser, instead of the DOM one
AC_ARG_ENABLE(reader, [  --enable-xmlreader      Enable support for xmlReader parser],
[case "${enableval}" in
  yes) reader=yes ;;
  no)  reader=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for enable-xmlreader option]) ;;
esac])

if test x$reader = xyes; then
    AC_DEFINE([USE_XMLREADER], [], [Use xmlReader parser])
fi

if test x$reader = xyes -a x$dom = xyes; then
   AC_ERROR([Can't specify a DOM and an xmlReader parser])
fi

dnl This enables or disables the support to make Gnash function as a
dnl small web server.
AC_ARG_ENABLE(reader, [  --enable-http      Enable support for a web server],
[case "${enableval}" in
  yes) http=yes ;;
  no)  http=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for enable-http option]) ;;
esac], http=no)

if test x$http = xyes; then
    AC_DEFINE([HTTP_SUPPORT], [], [Add the mini web server])
fi
AM_CONDITIONAL(HTTP, test x$http = xtrue)

dnl Where are the Docbook stylesheets ?
AC_ARG_WITH(docbook, [  --with-docbook          specify the path to the Docbook
stylesheets],
[case "${withval}" in
  yes) docbook=yes ;;
  no)  docbook=no ;;
  *)   AC_MSG_ERROR([bad value ${withval} for with-docbook option]) ;;
esac])

if test x$docbook = xyes; then
    DOCBOOK_STYLES="${withval}"
else
   AC_DOCBOOK_STYLES
fi


AC_PATH_FIREFOX

AC_PROG_CC
AC_PROG_CXX
AC_EXEEXT
AM_COMPILER_LIB
AC_PROG_INSTALL

AC_PATH_XTRA
AM_PATH_SDL
AM_PATH_SDL_MIXER
AM_PATH_OPENGL
AM_PATH_ZLIB
AM_PATH_JPEG
AM_PATH_PNG
AC_PROG_LIBTOOL

dnl Define winsock if we're on widows. We could do something complicated,
dnl but since AC_EXEEXT does it for us, we'll do this the easy way.
if test x"$EXEEXT" == "exe"; then
  AC_DEFINE(HAVE_WINSOCK,1,[This is defined is we are on Win32])
fi

dnl Look for pthreads
if test x"$pthreads" = x"yes"; then
  AC_CHECK_LIB(pthread, main, PTHREAD_LIB=-lpthread,PTHREAD_LIB="")
else
  PTHREAD_LIB=""
fi
AC_SUBST(PTHREAD_LIB)


AC_OUTPUT(Makefile
libbase/Makefile
libgeometry/Makefile
server/Makefile
backend/Makefile
utilities/Makefile
plugin/Makefile
plugin/mozilla-sdk/Makefile
doc/Makefile
doc/C/Makefile
)
dnl config.mk
dnl plugin/test/Makefile
dnl plugin/test/unix/Makefile

echo ""
echo "Configurable options are:"

if test x"$pthreads" = x"yes"; then
  echo "        Using POSIX Threads"
else
  echo "        Not using POSIX Threads"
fi

if test x"$http" = x"yes"; then
  echo "        Building the web server support"
else
  echo "        Not building the web server support."
fi

if test x"$docbook" = x"yes"; then
  echo "        Docbook styles sheets in $DOCBOOK_STYLES"
fi


if test x"$i810lodbias" = x"yes"; then
  echo "        Using the Intel 810 LOD bias hack"
else
  echo "        Not using the Intel 810 LOD bias hack"
fi

if test x"$dmalloc" = x"yes"; then
  echo "        Using DMalloc, For a list of the command-line options enter: dmalloc --usage"
  dnl dmalloc
else
  echo "        Not using DMalloc"
fi

if test x$enable_xml = xno; then
  echo "        Not using XML or XMLSocket"
else
  echo "        Adding XML and XMLSocket"
fi

if test x"$plugin" = x"yes"; then
  echo "        Building the Firefox plugin"
else
  echo "        Not building the Firefox plugin"
fi

if test x"$FIREFOX_CFLAGS" != x; then
  echo "        Firefox C flags are: $FIREFOX_CFLAGS"
  echo "        Firefox libs are: $FIREFOX_LIBS"
else
  echo "        WARNING: No Firefox development package installed!"
fi
