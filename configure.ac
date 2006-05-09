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

AC_PREREQ(2.50)
AC_INIT(server/gnash.h)
AM_CONFIG_HEADER(config.h)

AC_CANONICAL_TARGET

dnl These are required by automake
AM_INIT_AUTOMAKE(gnash, 0.7)
AM_MAINTAINER_MODE
AC_PROG_MAKE_SET

dnl Add KFLASH support, if specified.
AC_ARG_ENABLE(klash, [  --enable-klash           Enable support for Konqueror plugin],
[case "${enableval}" in
  yes) klash=yes ;;
  no)  klash=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for --enable-klash option]) ;;
esac],klash=no)

AC_ARG_VAR(PKGCONFIGFOUND, [Path to pkg-config])
AC_CHECK_PROG(PKGCONFIGFOUND, pkg-config,[yes])
KDE_SET_PREFIX
AM_KDE_WITH_NLS
AC_PATH_KDE
KDE_USE_QT(3)
AM_CONDITIONAL(KLASH, test x$klash = xyes)

dnl Add KFLASH support, if specified.
AC_ARG_ENABLE(fork, [  --disable-fork           Use a thread instead forking the standalone player.],
[case "${enableval}" in
  yes) fork=yes ;;
  no)  fork=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for --disable-fork option]) ;;
esac],fork=yes)

if test x"$fork" = x"yes"; then
  AC_DEFINE([USE_FORK], [], [Fork player instead of thread])
fi

dnl Add XML support, if specified.
AC_ARG_ENABLE(xml, [  --disable-xml           Disable support for XML and XMLSocket],
[case "${enableval}" in
  yes) xml=yes ;;
  no)  xml=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for disable-xml option]) ;;
esac],xml=yes)

if test x$xml != xno; then
    AM_PATH_LIBXML2
fi
AM_CONDITIONAL(XML, test x$xml = xyes)

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

dnl dnl Add methods to enable gtk2
dnl AC_ARG_ENABLE(gtk2, [  --enable-gtk2  Enable GTK2 support for standalone player],
dnl [case "${enableval}" in
dnl   yes) gtk2=yes ;;
dnl   no)  gtk2=no ;;
dnl   *)   AC_MSG_ERROR([bad value ${enableval} for enable-gtk2 option]) ;;
dnl esac], gtk2=yes)

dnl if test x"${gtk2}" != x"no"; then
dnl   AC_DEFINE([ENABLE_GTK2], [], [Unit gtk2 support])
dnl fi
dnl AM_CONDITIONAL(GTK2, test x$gtk2 = xyes)

AC_ARG_ENABLE(
 renderer,
 [  --enable-renderer=engine Specify rendering engine:
                                 OpenGL (default)
                                 Cairo  (experimental)
                                 SDL    (unsupported)],
 [case "${enableval}" in
   opengl|OpenGL) renderer=opengl ;;
   cairo|Cairo)  renderer=cairo  ;;
   *)      AC_MSG_ERROR([invalid renderer ${enableval} given]) ;;
  esac],
 [renderer=opengl]
)

if test x$renderer = xopengl; then
  AC_DEFINE([RENDERER_OPENGL], [], [Use OpenGL renderer])
fi
AM_CONDITIONAL(USE_RENDERER_OPENGL, test x$renderer = xopengl)

if test x$renderer = xcairo; then
  AC_DEFINE([RENDERER_CAIRO], [], [Use cairo renderer])
fi
AM_CONDITIONAL(USE_RENDERER_CAIRO, test x$renderer = xcairo)

dnl Use the Dom parser from libxml2, not the xmlReader one
dnl The default is to use the Dom parser
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

dnl LIBS+="-L/usr/lib64"

GNASH_PATH_FIREFOX
AC_PATH_XTRA
AC_CHECK_HEADERS(dejagnu.h)
AC_CHECK_LIB(Xmu, XmuCvtStringToOrientation)
AC_CHECK_LIB(Xi, XInput_find_display)
AC_CHECK_LIB(X11, XDisableAccessControl)
AC_CHECK_LIB(rt, shm_unlink)
AC_CHECK_FUNCS(shm_open)
AC_CHECK_FUNCS(strcasecmp stricmp)
AC_CHECK_FUNCS(vsnprintf)
dnl AC_CHECK_FUNCS(finite isfinite)

AC_CACHE_CHECK([for finite], ac_cv_finite,
 [AC_TRY_LINK([#include <math.h>],
 [double x; int y; y = finite(x);],
 ac_cv_finite=yes,
 ac_cv_finite=no
)])
if test x"$ac_cv_finite" = x"yes"; then
  AC_DEFINE(HAVE_FINITE, [], [Has finite])
fi

AC_CACHE_CHECK([for isfinite], ac_cv_isfinite,
 [AC_TRY_LINK([#include <math.h>],
 [double x; int y; y = isfinite(x);],
 ac_cv_isfinite=yes,
 ac_cv_isfinite=no
)])
if test x"$ac_cv_isfinite" = x"yes"; then
  AC_DEFINE(HAVE_ISFINITE, [], [Has isfinite])
fi

AC_CHECK_FUNCS(sysconf)
AC_CHECK_FUNCS(shmget)
AC_CHECK_FUNCS(shmat)
AC_CHECK_FUNCS(mmap)
AC_CHECK_FUNCS(gettimeofday)
dnl Look for Win32 networking stuff
AC_CHECK_HEADERS(winsock.h)
AC_CHECK_FUNCS(socket)
AC_CHECK_FUNCS(CreateFileMappingA)

GNASH_HASHMAP
dnl AC_CHECK_LIB(c, mremap)
GNASH_PATH_ZLIB
GNASH_PATH_JPEG
GNASH_PATH_PNG
GNASH_PATH_MING
AM_CONDITIONAL(ENABLE_MING, [ test x"$MAKESWF" != x ])
AC_PATH_PROG(DOXYGEN, doxygen)
AM_CONDITIONAL(ENABLE_DOXYGEN, [ test x"$DOXYGEN" != x ])

GNASH_PATH_MP3
GNASH_PATH_OGG
GNASH_PATH_SDL
AM_CONDITIONAL(HAS_SDL, [ test x$has_sdl = xyes ])
GNASH_PATH_SDL_MIXER
GNASH_PATH_GSTREAMER
GNASH_PATH_OPENGL
GNASH_PATH_GLEXT
GNASH_PATH_GTK2
AM_CONDITIONAL(HAS_GTK2, [ test x$has_gtk2 = xyes ])
GNASH_PATH_FLTK
AM_CONDITIONAL(HAS_FLTK, [ test x$has_fltk = xyes ])
GNASH_PATH_XFT
AM_CONDITIONAL(HAS_XFT, [ test x$has_xft = xyes ])
GNASH_PATH_CAIRO
GNASH_PATH_GLIB
GNASH_PATH_PANGO
GNASH_PATH_ATK
GNASH_PATH_PTHREADS
GNASH_PATH_GHELP
AM_CONDITIONAL(GHELP, [test x$ghelp = xyes])
GNASH_DOCBOOK
AM_CONDITIONAL(DOCBOOK, test x$docbook = xyes)
GNASH_PROG_LIBTOOL

dnl Define winsock if we're on windows. We could do something complicated,
dnl but since AC_EXEEXT does it for us, we'll do this the easy way.
if test x"$EXEEXT" == "exe"; then
  AC_DEFINE(HAVE_WINSOCK,1,[This is defined is we are on Win32])
fi

dnl enable more warnings. These should be cleaned up to make Gnash a
dnl better project, but for now they generate too many messages,
dnl mostly due to the win32 and crufty C code pretending to be C++.
AC_ARG_ENABLE(lotsa-warnings, [  --enable-lotsa-warnings Turn on tons of GCC compiler warnings],
[case "${enableval}" in
  yes) lotsa=yes ;;
  no)  lotsa=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} --enable-lotsa-warnings for option]) ;;
esac],lotsa=no)

# We want warnings, lots of warnings  :-)
if test "$GCC" = "yes"; then
  CXXFLAGS="$CXXFLAGS \
    -ansi \
    -Wall \
    "

  CFLAGS="$CFLAGS \
    -ansi \
    -Wall \
    "

  if test x"$lotsa" = x"yes"; then
    # Source do not build with -ansi -pedantic yet
    CXXFLAGS="$CXXFLAGS \
      -W \
      -Wall \
      -Wcast-align \
      -Wcast-qual \
      -Wpointer-arith \
      -Wreturn-type \
      "
    CFLAGS="$CFLAGS \
      -W \
      -Wall \
      -Wcast-align \
      -Wcast-qual \
      -Wpointer-arith \
      -Wreturn-type \
      -Wmissing-declarations \
      -Wmissing-prototypes \
      -Wstrict-prototypes \
      "
  fi
fi

if test "$GCC" = "yes"; then
  AC_COMPILE_IFELSE([
  #if __GNUC__ < 4
  #error
  #endif],gccvisibility=yes,)
  if test x"$gccvisibility" = x"yes"; then
    CXXFLAGS="$CXXFLAGS -fvisibility-inlines-hidden"
  fi
fi



dnl AC_CONFIG_LINKS(doc/C/images)
plugmk=""
if test x"$plugin" = x"yes"; then
plugmk="plugin/Makefile plugin/mozilla-sdk/Makefile"
fi

AC_OUTPUT(Makefile
libbase/Makefile
libgeometry/Makefile
server/Makefile
libamf/Makefile
backend/Makefile
utilities/Makefile
doc/Makefile
doc/C/Makefile
doc/Doxyfile
testsuite/Makefile
testsuite/actionscript.all/Makefile
plugin/Makefile
plugin/klash/Makefile
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
  echo "        Intel 810 LOD bias hack disabled (default). Use --enable-i810-lod-bias to enable."
fi

if test x"$mp3" = x"yes"; then
  echo "        MP3 (libmad) support enabled"
else
  echo "        MP3 (libmad) support disabled (default). Use --enable-mp3 to enable"
fi

if test x"$pthreads" = x"yes"; then
  echo "        POSIX Threads support enabled (default)"
else
  echo "        POSIX Thread support disabled."
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
  echo "        DMalloc support disabled (default). Use --enable-dmalloc to enable."
fi

if test x$xml = xno; then
  echo "        XML and XMLSocket disabled"
else
  echo "        XML and XMLSocket enabled (default). Use --disable-xml to disable."
fi

if test x"$plugin" = x"yes"; then
  echo "        Firefox plugin enabled (default). Use --disable-plugin to disable."
else
  echo "        Firefox plugin disabled"
fi

if test x"$klash" = x"yes"; then
  echo "        Konqueror plugin enabled"
else
  echo "        Konqueror plugin disabled (default). Use --enable-klash to enable."
fi

if test x"$ghelp" = x"yes"; then
  echo "        GNOME help enabled"
else
  echo "        GNOME help disabled (default). Use --enable-ghelp to enable."
fi

if test x"$testing" = x"yes"; then
  echo "        Unit testing support enabled (default)"
else
  echo "        Unit testing support disabled"
fi

echo "        Renderer engine: "$renderer


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

if test x$xml != xno; then
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
      echo "        Opengl flags are: $OPENGL_CFLAGS"
    else
      echo "        Opengl flags are: default include path"
    fi
      echo "        Opengl libs are: $OPENGL_LIBS"
    else
      echo "ERROR: No Opengl development package installed!\
 You need to have the libmesa development package installed\
 to compile this project or install libgl1-mesa-dev\
 (using apt-get) or xorg-x11-Mesa-libGL (using yum)."
    nogo=true
  fi
fi

if test x"$glext" = x"yes"; then
  if test x"$GLEXT_LIBS" != x; then
    if test x"$GLEXT_CFLAGS" != x; then
      echo "        GtkGlext flags are: $GLEXT_CFLAGS"
    else
      echo "        GtkGLext flags are: default include path"
    fi
      echo "        GtkGLext libs are: $GLEXT_LIBS"
    else
      echo "ERROR: No GtkGLext development package installed!\
 You need to have the GtkGLext development package installed\
 to compile this project or install libgtkglext1-dev \
 (using apt-get) or gtkglext-devel (using yum)."
#     nogo=true
  fi
fi

if test x"$plugin" = x"yes" -a x"$glext" = x"yes"; then
  if test x"$GTK2_LIBS" != x; then
    if test x"$GTK2_CFLAGS" != x; then
      echo "        Gtk2 flags are: $GTK2_CFLAGS"
    else
      echo "        Gtk2 flags are: default include path"
    fi
      echo "        Gtk2 libs are: $GTK2_LIBS"
    else
      echo "ERROR: No Gtk2 development package installed!\
 You need to have the Gtk2 development package installed\
 to compile this project or install libgtk2-dev \
 (using apt-get) or gtk2-devel (using yum)."
dnl    nogo=true
  fi
fi

if test x"$glext" = x"yes"; then
  if test x"$PANGO_LIBS" != x; then
    if test x"$PANGO_CFLAGS" != x; then
      echo "        Pango flags are: $PANGO_CFLAGS"
    else
      echo "        Pango flags are: default include path"
    fi
      echo "        Pango libs are: $PANGO_LIBS"
    else
      echo "ERROR: No Pango development package installed!\
 You need to have the Pango development package installed\
 to compile this project or install libpango1.0-dev \
 (using apt-get) or pango-devel (using yum)."
dnl    nogo=true
  fi
fi

if test x"$glext" = x"yes"; then
  if test x"$GLIB_LIBS" != x; then
    if test x"$GLIB_CFLAGS" != x; then
      echo "        Glib flags are: $GLIB_CFLAGS"
    else
      echo "        Glib flags are: default include path"
    fi
      echo "        Glib libs are: $GLIB_LIBS"
    else
      echo "ERROR: No Glib development package installed!\
 You need to have the Glib development package installed\
 to compile this project or install libglib2.0-dev \
 (using apt-get) or glib2-devel (using yum)."
dnl    nogo=true
  fi
fi

if test x"$glext" = x"yes"; then
  if test x"$CAIRO_LIBS" != x; then
    if test x"$CAIRO_CFLAGS" != x; then
      echo "        Cairo flags are: $CAIRO_CFLAGS"
    else
      echo "        Cairo flags are: default include path"
    fi
      echo "        Cairo libs are: $CAIRO_LIBS"
    else
    echo "ERROR: No Cairo development package installed!\
 You need to have the Cairo development package installed\
 to compile this project or install libcairo-dev \
 (using apt-get) or cairo-devel (using yum)."
dnl    nogo=true
  fi
fi

if test x"$glext" = x"yes"; then
  if test x"$ATK_LIBS" != x; then
    if test x"$ATK_CFLAGS" != x; then
      echo "        Atk flags are: $ATK_CFLAGS"
    else
      echo "        Atk flags are: default include path"
    fi
      echo "        Atk libs are: $ATK_LIBS"
    else
      echo "ERROR: No Atk development package installed!\
 You need to have the Atk development package installed\
 to compile this project or install libatk1.0-dev \
 (using apt-get) or atk-devel (using yum)."
dnl    nogo=true
  fi
fi

if test x"$fltk" = x"yes"; then
  if test x"$FLTK_LIBS" != x; then
    if test x"$FLTK_CFLAGS" != x; then
      echo "        Fltk flags are: $FLTK_CFLAGS"
    else
      echo "        Fltk flags are: default include path"
    fi
      echo "        Fltk libs are: $FLTK_LIBS"
dnl     else
dnl       echo "ERROR: No Fltk development package installed!\
dnl  You need to have the fltk development package installed\
dnl  to compile this project or install libfltk1.1-dev \
dnl  (using apt-get) or fltk-devel (using yum)."
dnl    nogo=true
  fi
fi

if test x"$fltk" = x"yes"; then
  if test x"$XFT_LIBS" != x; then
    if test x"$XFT_CFLAGS" != x; then
      echo "        Xft flags are: $XFT_CFLAGS"
    else
      echo "        Xft flags are: default include path"
    fi
      echo "        Xft libs are: $XFT_LIBS"
dnl     else
dnl       echo "ERROR: No Xft development package installed!\
dnl  You need to have the xft development package installed\
dnl  to compile this project or install libxft-dev \
dnl  (using apt-get) or xft-devel (using yum)."
dnl    nogo=true
  fi
fi

if test x"$SDL_LIBS" != x; then
    echo "        SDL flags are: $SDL_CFLAGS"
    echo "        SDL libs are: $SDL_LIBS"
else
    echo "ERROR: No SDL development package installed!\
 You need to have the SDL development package installed\
 to compile, or install libsdl1.2-dev (using apt-get)\
 or SDL-devel (using yum)."
    nogo=true
fi

if test x"$SDL_MIXER_LIBS" != x; then
    echo "        SDL_Mixer flags are: $SDL_MIXER_CFLAGS"
    echo "        SDL_Mixer libs are: $SDL_MIXER_LIBS"
else
    echo "ERROR: No SDL_Mixer development package installed!\
 You need to have the SDL Mixer development package installed\
 to compile this project with SDL sound support or install\
 libsdl-mixer1.2-dev (using apt-get) or SDL_mixer-devel (using yum)."
fi

dnl As the GStreamer support isn;t used, but is to support a
dnl work-in-progress, we don't want to confuse people by printing a
dnl warnign if it's not installed.
if test x"$GSTREAMER_LIBS" != x; then
    echo "        Gstreamer flags are: $GSTREAMER_CFLAGS"
    echo "        Gstreamer libs are: $GSTREAMER_LIBS"
dnl else
dnl     echo "ERROR: No Gstreamer development package installed!."
fi

if test x"$plugin" = x"yes"; then
dnl  if test x"$FIREFOX_CFLAGS" != x; then
dnl    echo "        Firefox flags are: $FIREFOX_CFLAGS"
dnl    echo "        Firefox libs are: $FIREFOX_LIBS"
    echo "        Plugin will be installed in ${FIREFOX_PLUGINS}"
dnl  else
dnl    echo "ERROR: No Firefox or Mozilla development package installed!"
#    nogo=true
dnl  fi
fi

if test x"$pthreads" = x"yes"; then
  if test x"$PTHREAD_LIBS" != x; then
    if test x"$PTHREAD_CFLAGS" != x; then
      echo "        POSIX Threads flags are: $PTHREAD_CFLAGS"
    else
      echo "        POSIX Threads flags are: default include path"
    fi
    echo "        POSIX Threads lib is: $PTHREAD_LIBS"
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

if test x"$fork" = x"no"; then
  AC_MSG_ERROR([Currently only forking the standalone player works!])
fi

