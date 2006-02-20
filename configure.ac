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
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.50)
AC_INIT(server/gnash.h)
AM_CONFIG_HEADER(config.h)

AC_CANONICAL_TARGET

dnl These are required by automake
AM_INIT_AUTOMAKE(gnash, 0.7)
AM_MAINTAINER_MODE
AC_PROG_MAKE_SET

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

dnl Install the headers to make Gnash an SDK
AC_ARG_ENABLE(sdkinstall, [  --enable-sdk-install  Enable installing the libraries and headers as an SDK],
[case "${enableval}" in
  yes) sdkinstall=yes ;;
  no)  sdkinstall=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for enable-i810-lod-bias option]) ;;
esac], sdkinstall=no)

AM_CONDITIONAL(SDKINSTALL, test x$sdkinstall = xyes)

dnl Add methods to enable testing
AC_ARG_ENABLE(testing, [  --enable-testing  Enable testing specific methods],
[case "${enableval}" in
  yes) testing=yes ;;
  no)  testing=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for enable-testing option]) ;;
esac], testing=yes)

if test x"${testing}" != x"no"; then
  AC_DEFINE([ENABLE_TESTING], [], [Unit testing support])
fi
AM_CONDITIONAL(TESTING, test x$testing = xyes)

dnl Use the Dom parser from libxml2, not the xmlReader one
dnl The default is to use the Dom parser
dom=yes
AC_ARG_ENABLE(dom, [  --enable-dom            Enable support for Dom parser],
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
esac], reader=no)

if test x$reader = xyes; then
    AC_DEFINE([USE_XMLREADER], [], [Use xmlReader parser])
fi

if test x$reader = xyes -a x$dom = xyes; then
   AC_ERROR([Can't specify a DOM and an xmlReader parser])
fi

dnl This enables or disables the support to make Gnash function as a
dnl small web server.
AC_ARG_ENABLE(http, [  --enable-http           Enable support for a web server],
[case "${enableval}" in
  yes) http=yes ;;
  no)  http=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for enable-http option]) ;;
esac], http=no)

if test x$http = xyes; then
    AC_DEFINE([HTTP_SUPPORT], [], [Add the mini web server])
fi
AM_CONDITIONAL(HTTP, test x$http = xtrue)

AC_ARG_ENABLE(net_conn, [  --enable-net-conn           Enable support ],
[case "${enableval}" in
  yes) net_conn=yes ;;
  no)  net_conn=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for enable-net_conn option]) ;;
esac], net_conn=no)

if test x$net_conn = xyes; then
    AC_DEFINE([NETWORK_CONN], [], [Use tcp/ip for local connections])
fi
AM_CONDITIONAL(NET_CONN, test x$net_conn = xtrue)

AC_PROG_CC
AC_PROG_CXX
AC_EXEEXT
AC_PROG_INSTALL
AM_COMPILER_LIB
GNASH_PATH_DMALLOC
AC_CHECK_FUNCS(mallinfo)
LIBS+="-L/usr/lib64"

AC_PATH_FIREFOX
AC_PATH_XTRA
AC_CHECK_HEADERS(dejagnu.h)
AC_CHECK_LIB(Xmu, XmuCvtStringToOrientation)
AC_CHECK_LIB(Xi, XInput_find_display)
AC_CHECK_LIB(X11, XDisableAccessControl)
AC_CHECK_LIB(rt, shm_unlink)
AC_CHECK_FUNCS(shm_open)
dnl AC_CHECK_LIB(c, mremap)
AM_PATH_ZLIB
AM_PATH_JPEG
AM_PATH_PNG
AC_PATH_MING
AM_CONDITIONAL(ENABLE_MING, [ test x"$MAKESWF" != x ])
AC_PATH_PROG(DOXYGEN, doxygen)
AM_CONDITIONAL(ENABLE_DOXYGEN, [ test x"$DOXYGEN" != x ])

AM_PATH_MP3
AM_PATH_OGG
AM_PATH_SDL
AM_PATH_SDL_MIXER
AM_PATH_OPENGL
GNASH_PATH_PTHREADS
GNASH_PATH_GHELP
AM_CONDITIONAL(GHELP, [test x$ghelp = xyes])
GNASH_DOCBOOK
AM_CONDITIONAL(DOCBOOK, test x$docbook = xyes)
AC_PROG_LIBTOOL

dnl Define winsock if we're on windows. We could do something complicated,
dnl but since AC_EXEEXT does it for us, we'll do this the easy way.
if test x"$EXEEXT" == "exe"; then
  AC_DEFINE(HAVE_WINSOCK,1,[This is defined is we are on Win32])
fi

dnl AC_CONFIG_LINKS(doc/C/images)

AC_OUTPUT(Makefile
libbase/Makefile
libgeometry/Makefile
server/Makefile
libamf/Makefile
backend/Makefile
utilities/Makefile
plugin/Makefile
plugin/mozilla-sdk/Makefile
doc/Makefile
doc/C/Makefile
doc/Doxyfile
testsuite/Makefile
testsuite/actionscript.all/Makefile
)
dnl config.mk
dnl plugin/test/Makefile
dnl plugin/test/unix
dnl /Makefile

echo ""
echo "Configurable options are:"

if test x"$i810lodbias" = x"yes"; then
  echo "        Intel 810 LOD bias hack enabled"
else
  echo "        Intel 810 LOD bias hack disabled (default)"
fi

if test x"$mp3" = x"yes"; then
  echo "        MP3 (libmad) support enabled"
else
  echo "        MP3 (libmad) support disabled (default). Use --enable-mp3 to enable"
fi

if test x"$pthreads" = x"yes"; then
  echo "        POSIX Threads support enabled"
else
  echo "        POSIX Thread support disabled"
fi

if test x"$http" = x"yes"; then
  echo "        Web 
server support enabled"
else
  echo "        Web server support disabled (default)"
fi

if test x"$dmalloc" = x"yes"; then
  echo "        DMalloc support enabled"
  echo "        For a list of the command-line options enter: dmalloc --usage"
else
  echo "        DMalloc support disabled (default)"
fi

if test x$enable_xml = xno; then
  echo "        XML and XMLSocket disabled"
else
  echo "        XML and XMLSocket enabled (default)"
fi

if test x"$plugin" = x"yes"; then
  echo "        Firefox plugin enabled (default)"
else
  echo "        Firefox plugin disabled"
fi

if test x"$ghelp" = x"yes"; then
  echo "        GNOME help enabled"
else
  echo "        GNOME help disabled (default)"
fi

if test x"$testing" = x"yes"; then
  echo "        Unit testing support enabled (default)"
else
  echo "        Unit testing support disabled"
fi


echo ""
echo "Configured paths for ${build} are:"

if test x"$docbook" = x"yes"; then
  echo "        DocBook document processing enabled"
  if test x"$docbook_styles" != x; then
    echo "        Docbook styles sheets in $docbook_styles"
  fi
else
  echo "        DocBook document processing disabled (default)"
fi

# set a variable if we shouldn't continue. This way we can print
# out everything that is missing in one pass, hopefully making it
# easy for new developers to get everything they need installed.
nogo=false

if test x$enable_xml != xno; then
  if test x"$LIBXML_CFLAGS" != x; then
    echo "        XML flags are: $LIBXML_CFLAGS"
    echo "        XML libs are: $LIBXML_LIBS"
  else
    echo "        ERROR: No libxml2 development package installed!"
    echo "               Reconfigure with --disable-xml to continue"
    nogo=true
  fi
fi

if test x"${png}" != x ; then
  if test x"${PNG_CFLAGS}" != x ; then
    echo "        PNG flags are: $PNG_CFLAGS"
  else
    echo "        PNG flags are: default include path"
  fi
  echo "        PNG libs are: $PNG_LIBS"
else
  echo "        ERROR: No PNG library development package installed!"
  nogo=true
fi

if test x"${JPEG_LIBS}" != x ; then
  if test x"${JPEG_CFLAGS}" != x ; then
    echo "        JPEG flags are: $JPEG_CFLAGS"
  else
    echo "        JPEG flags are: default include path"
  fi
  echo "        JPEG libs are: $JPEG_LIBS"
else
  echo "        ERROR: No JPEG library development package installed!"
  nogo=true
fi

if test x"$mp3" = xyes; then
  if test x"$MAD_LIBS" != x; then
    if test x"$MAD_CFLAGS" != x; then
      echo "        MAD flags are: $MAD_CFLAGS"
    else
      echo "        MAD flags are: default include path"
    fi
    echo "        MAD libs are: $MAD_LIBS"
  else
    echo "ERROR: No libmad (mp3) development package installed!\
 You need to have the libmad development package installed\
 to compile this project. You can either reconfigure without --enable-mp3,\
 or install libmad0-dev (using apt-get) or libmad (using yum)."

  fi
fi

if test x"$OGG_LIBS" != x; then
  if test x"$OGG_CFLAGS" != x; then
    echo "        OGG flags are: $OGG_CFLAGS"
  else
    echo "        OGG flags are: default include path"
  fi
  echo "        OGG libs are: $OGG_LIBS"
else
  echo "ERROR: No OGG Vorbis development package installed!"
  nogo=true
fi

dnl if test x"$VORBIS_LIBS" != x; then
dnl   if test x"$VORBIS_CFLAGS" != x; then
dnl     echo "        VORBIS flags are: $VORBIS_CFLAGS"
dnl   else
dnl     echo "        VORBIS flags are: default include path"
dnl   fi
dnl   echo "        VORBIS libs are: $VORBIS_LIBS"
dnl else
dnl   echo "ERROR: No VORBIS Vorbis development package installed!"
dnl   nogo=true
dnl fi

if test x"$opengl" = x"yes"; then
  if test x"$OPENGL_LIBS" != x; then
    if test x"$OPENGL_CFLAGS" != x; then
      echo "        OPENGL flags are: $OPENGL_CFLAGS"
    else
      echo "        OPENGL flags are: default include path"
    fi
      echo "        Opengl libs are: $OPENGL_LIBS"
    else
      echo "ERROR: No Opengl development package installed!\
 You need to have the libmesa development package installed\
 to compile this project or install libgl1-mesa-dev\
 (using apt-get) or libmesa-devel (using yum)."
    nogo=true
  fi
fi

if test x"$SDL_CFLAGS" != x; then
    echo "        SDL flags are: $SDL_CFLAGS"
    echo "        SDL libs are: $SDL_LIBS"
else
    echo "ERROR: No SDL development package installed!\
 You need to have the SDL development package installed\
 to compile, or install libsdl1.2-dev (using apt-get)\
 or SDL-devel (using yum)."
    nogo=true
fi

if test x"$SDL_MIXER_CFLAGS" != x; then
    echo "        SDL_Mixer flags are: $SDL_MIXER_CFLAGS"
    echo "        SDL_Mixer libs are: $SDL_MIXER_LIBS"
else
    echo "ERROR: No SDL_Mixer development package installed!\
 You need to have the SDL Mixer development package installed\
 to compile this project with SDL sound support or install\
 libsdl-mixer1.2-dev (using apt-get) or SDL_mixer-devel (using yum)."
fi

if test x"$plugin" = x"yes"; then
  if test x"$FIREFOX_CFLAGS" != x; then
    echo "        Firefox flags are: $FIREFOX_CFLAGS"
    echo "        Firefox libs are: $FIREFOX_LIBS"
    echo "        Plugin will be installed in ${FIREFOX_PLUGINS}"
  else
    echo "ERROR: No Firefox or Mozilla development package installed!"
    nogo=true
  fi
fi

if test x"$pthreads" = x"yes"; then
  if test x"$PTHREAD_LIBS" != x; then
    if test x"$PTHREAD_CFLAGS" != x; then
      echo "        POSIX Threads flags are: $PTHREAD_CFLAGS"
    else
      echo "        POSIX Threads flags are: default include path"
    fi
    echo "        POSIX Threads libs are: $PTHREAD_LIBS"
  else
    echo "ERROR: No pthread development package installed!"
    nogo=true
  fi
fi

if test x"$docbook" = x"yes"; then
  if test x"$DB2X_XSLTPROC" = x -o x"$DB2X_TEXIXML" = x -o x"$MAKEINFO" = x -o x"$XSLTPROC" = x; then
    echo "ERROR: No DocBook2X tools installed!\
 docbook, docbook-xml, docbook2Xm docbook-utils, docbook-xsl (using apt-get or yum)."
    nogo=true
  else
    echo "        You have all the DocBook2X tools installed."
    echo "        You have version $db2x_version of the DocBook2X tools."

  fi

fi


# If anything critical is missing, don't bother to continue
if test x"$nogo" = x"true"; then
echo ""
  AC_MSG_ERROR([Please install required packages])
fi

