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

dnl $Id: configure.ac,v 1.266 2007/02/23 16:18:41 rsavoye Exp $

AC_PREREQ(2.50)
AC_INIT(gnash, cvs)
AC_CONFIG_SRCDIR(server/gnash.h)
AC_CONFIG_LIBOBJ_DIR(libbase)
AM_CONFIG_HEADER(config.h)
AC_CONFIG_SUBDIRS(libltdl)

dnl AC_CANONICAL_TARGET
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

dnl Some things you can only do by looking at the platform name.
case "${host}" in
  powerpc-apple-darwin*)
    AC_DEFINE([__powerpc64__], [], [this is a 64 bit powerpc])
    ;;
  i386-*-openbsd*)
    openbsd_os=openbsd
    ;;
  *64-*-openbsd*)
    openbsd_os=openbsd 
    AC_DEFINE([WORDSIZE], [64], [this is a 64 platform])
    ;;
esac

AM_CONDITIONAL(PLUGIN_LINK_UNDEFINED, test x$openbsd_os = xopenbsd)

dnl Get build date for helping us debugging
BUILDDATE="`date +%Y%m%d`"

dnl These are required by automake
AM_INIT_AUTOMAKE(gnash, "cvs-$BUILDDATE")
AM_MAINTAINER_MODE
AC_PROG_MAKE_SET

AC_DISABLE_STATIC dnl Disable building static libs.

AC_ARG_ENABLE(fork, AC_HELP_STRING([--disable-fork],[Use a thread instead forking the standalone player.]),
[case "${enableval}" in
  yes) fork=yes ;;
  no)  fork=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for --disable-fork option]) ;;
esac],fork=yes)

if test x"$fork" = x"yes"; then
  AC_DEFINE([USE_FORK], [], [Fork player instead of thread])
fi

AC_ARG_ENABLE(debugger, AC_HELP_STRING([--disable-debugger],[Disable the Flash debugger]),
[case "${enableval}" in
  yes) debugger=yes ;;
  no)  debugger=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for --disable-debugger option]) ;;
esac],debugger=yes)

if test x"$debugger" = x"yes"; then
  AC_DEFINE([USE_DEBUGGER], [], [Flash Debugger support])
fi
AM_CONDITIONAL(DEBUGGER, test x$debugger = xyes)

dnl Write the file to disk in the plugin, if specified.
AC_ARG_ENABLE(write, AC_HELP_STRING([--enable-write], [Makes the Mozilla plugin write the currently playing SWF movie to /tmp.]),
[case "${enableval}" in
  yes) write=yes ;;
  no)  write=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for --enable-write option]) ;;
esac],write=no)

if test x"$write" = x"yes"; then
  AC_DEFINE([WRITE_FILE], [], [Write files while streaming])
fi

dnl Don't build the extensions if specified.
AC_ARG_ENABLE(extensions, AC_HELP_STRING([--enable-extensions], [Enable support for Gnash EXTENSIONS]),
[case "${enableval}" in
  yes) extensions=yes ;;
  no)  extensions=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for enable-extensions option]) ;;
esac],extensions=no)

AM_CONDITIONAL(EXTENSIONS, test x$extensions = xyes)

dnl Don't build the cygnal server if specified.
AC_ARG_ENABLE(cygnal, AC_HELP_STRING([--disable-cygnal], [Disable support for the Cygnal server]),
[case "${enableval}" in
  yes) cygnal=yes ;;
  no)  cygnal=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for disable-cygnal option]) ;;
esac],cygnal=yes)

AM_CONDITIONAL(CYGNAL, test x$cygnal = xyes)

dnl Fix the Intel 810 LOD bias problem
AC_ARG_ENABLE(i810-lod-bias, AC_HELP_STRING([--enable-i810-lod-bias], [Enable fix for Intel 810 LOD bias problem]),
[case "${enableval}" in
  yes) i810lodbias=yes ;;
  no)  i810lodbias=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for enable-i810-lod-bias option]) ;;
esac])

if test x$i810lodbias = xyes; then
    AC_DEFINE([FIX_I810_LOD_BIAS], [], [Fix i810 LOD bias problem])
fi

dnl Install the headers to make Gnash an SDK
AC_ARG_ENABLE(sdkinstall, AC_HELP_STRING([--enable-sdk-install], [Enable installing the libraries and headers as an SDK]),
[case "${enableval}" in
  yes) sdkinstall=yes ;;
  no)  sdkinstall=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for enable-i810-lod-bias option]) ;;
esac], sdkinstall=no)

AM_CONDITIONAL(SDKINSTALL, test x$sdkinstall = xyes)

dnl Add KFLASH support, if specified.
AC_ARG_ENABLE(klash, AC_HELP_STRING([--disable-klash], [Disable support for Konqueror plugin]),
[case "${enableval}" in
  yes) klash=yes ;;
  no)  klash=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for --disable-klash option]) ;;
esac],klash=yes)

AC_ARG_ENABLE(
 gui,
 AC_HELP_STRING([--enable-gui=TOOLKIT], [Specify GUI toolkit: GTK, FLTK, RISCOS, FB (framebuffer), KDE or SDL[[GTK]] ]),
 [case "${enableval}" in
   gtk|GTK) gui=gtk ;;
   sdl|SDL) gui=sdl  ;;
   kde|KDE) gui=kde  ;;
   fb|FB) gui=fb  ;;
   fltk|FLTK) gui=fltk  ;;
   riscos|RISCOS) gui=riscos ;;
   *)      AC_MSG_ERROR([invalid gui ${enableval} given]) ;;
  esac],
 [gui=gtk]
)

dnl if test x"$klash" = xyes -a x"$gui" != xkde; then
dnl   AC_MSG_ERROR([You must use the kde GUI ( --enable-gui=KDE ) if you enable klash.])
dnl fi

AC_ARG_ENABLE(
 renderer,
 AC_HELP_STRING([--enable-renderer=engine], [Specify rendering engine: OpenGL, Agg (experimental) or Cairo (experimental) [[OpenGL]] ]),
 [case "${enableval}" in
   opengl|OpenGL) renderer=opengl ;;
   cairo|Cairo)  renderer=cairo  ;;
   agg|Agg|AGG)  renderer=agg  ;;
   *)      AC_MSG_ERROR([invalid renderer ${enableval} given]) ;;
  esac],
 [renderer=opengl]
)

if test x$renderer = xopengl; then
  if test x"$gui" = xfb -o x"$gui" = xfltk; then
    AC_MSG_ERROR([opengl renderer is not supported by the $gui gui]);
  fi
  AC_DEFINE([RENDERER_OPENGL], [], [Use OpenGL renderer])
fi
AM_CONDITIONAL(USE_RENDERER_OPENGL, test x$renderer = xopengl)

if test x$renderer = xcairo; then
  if test x"$gui" = xfb; then
    AC_MSG_ERROR([cairo renderer is not supported by fb gui]);
  fi
  AC_DEFINE([RENDERER_CAIRO], [], [Use cairo renderer])
fi
AM_CONDITIONAL(USE_RENDERER_CAIRO, test x$renderer = xcairo)

if test x$renderer = xagg; then
  if test x"$gui" != xfb -a x"$gui" != xgtk -a x"$gui" != xsdl -a x"$gui" != xriscos -a x"$gui" != xfltk; then
    AC_MSG_ERROR([agg renderer is only supported by fb, gtk, riscos, sdl and fltk GUIs]);
  fi
  AC_DEFINE([RENDERER_AGG], [], [Use AntiGrain renderer])
fi
AM_CONDITIONAL(USE_RENDERER_AGG, test x$renderer = xagg)

dnl 16 bit: RGB555, RGB565
dnl 24 bit: RGB24, BGR24
dnl 32 bit: RGBA32, BGRA32
pixelformat=all
AC_ARG_WITH(
  pixelformat,
  AC_HELP_STRING([--with-pixelformat=], [Use the specified pixel format for AGG]),
  if test -n ${withval}; then
    pixelformat="${withval}"
    withval=`echo ${withval} | tr '\054' ' ' `
  fi
  while test -n "${withval}" ; do
    val=`echo ${withval} | cut -d ' ' -f 1`
    [case "${val}" in
      rgba32|RGBA32)
        AC_DEFINE(PIXELFORMAT_RGBA32, [1], [RGBA32])
        ;;
      bgra32|BGRA32)
        AC_DEFINE(PIXELFORMAT_BGR32, [1], [BGRA32])
        ;;
      rgb24|RGB24)
        AC_DEFINE(PIXELFORMAT_RGB24, [1], [RGB24])
        ;;
      bgr24|BGR24)
        AC_DEFINE(PIXELFORMAT_BGR24, [1], [BGR24])
        ;;
      rgb555|RGB555)
        AC_DEFINE(PIXELFORMAT_RGB555, [1], [RGB555])
        ;;
      rgb565|RGB565)
        AC_DEFINE(PIXELFORMAT_RGB565, [1], [RGB565])
        ;;
      *) AC_MSG_ERROR([invalid pixel format ${withval} given (accept: RGB555|RG
B565|RGB24|BGR24|BGRA32|RGBA32)])
         ;;
      esac]
    withval=`echo ${withval} | cut -d ' ' -f 2-6`
    if test x$val = x$withval; then
      break;
    fi
  done
)

if test x$pixelformat = xall; then
  if test x$renderer = xagg; then
    if test x"$gui" = x"gtk"; then
      AC_DEFINE(PIXELFORMAT_RGB24, [1], [RGB24 pixel format])
      pixelformat="RGB24"
    else
      AC_DEFINE(PIXELFORMAT_RGB555, [1], [RGB555 pixel format])
      AC_DEFINE(PIXELFORMAT_RGB565, [1], [RGB565 pixel format])
      AC_DEFINE(PIXELFORMAT_RGB24,  [1], [RGB24 pixel format])
      AC_DEFINE(PIXELFORMAT_BGR24,  [1], [BGR24 pixel format])
      AC_DEFINE(PIXELFORMAT_RGBA32, [1], [RGB32 pixel format])
      AC_DEFINE(PIXELFORMAT_BGRA32, [1], [BGRA32 pixel format])
    fi
  fi
fi
dnl --------------------------------------------------------
dnl  Select sound streamer
dnl --------------------------------------------------------

AC_ARG_ENABLE(
 sound,
 AC_HELP_STRING([--enable-sound=handler], [Enable sound support using the specified handler: SDL, GST (Gstreamer) or none (no sound) [[SDL]] ]),
 [case "${enableval}" in
   GST|gst) sound_handler=gst ;;
   sdl|SDL) sound_handler=sdl  ;;
   no|NO|none) sound_handler=none ;;
   *) AC_MSG_ERROR([bad value ${enableval} for --enable-sound option]) ;;
  esac],
 [sound_handler=sdl]
)

dnl --------------------------------------------------------
dnl  Select mp3 decoder (only for 'sdl' sound)
dnl --------------------------------------------------------

AC_ARG_WITH(
 mp3-decoder,
 AC_HELP_STRING([--with-mp3-decoder=DECODER], [Use the specified mp3 decoder: ffmpeg, mad (libmad) or none [[none]]]),
 [case "${withval}" in
   ffmpeg|FFMPEG) mp3_decoder=ffmpeg ;;
   mad|MAD) mp3_decoder=mad  ;;
   no|none) mp3_decoder=none ;;
   *) AC_MSG_ERROR([invalid mp3_decoder ${withval} given (accept: ffmpeg|mad)]) ;;
  esac],
 [mp3_decoder=ffmpeg]
)

if test x"$sound_handler" != xsdl -a x"$mp3_decoder" != xnone; then
mp3_decoder=none;
	dnl AC_MSG_ERROR([--with-mp3-decoder is only valid with --enable-sound=sdl])
fi

dnl --------------------------------------------------------
dnl  ...
dnl --------------------------------------------------------

AC_PROG_CC
AC_PROG_CXX
AC_EXEEXT
AC_PROG_INSTALL
AM_COMPILER_LIB

dnl When cross compiling, limit the search directories cause otherwise
dnl we may get the host headers or libraries by accident. These values
dnl are exported, so all the other configure tests in macros/*.m4 use
dnl these same settings rather than duplicating them like we used to.
dnl To override thise, use the --with-*-incl= and --with-*-libs=
dnl options to configure.
if test x$cross_compiling = xyes; then
  AC_MSG_NOTICE([Configuring Gnash for cross compilation])
  export incllist="${prefix}/${host_alias}/include ${prefix}/include"
  export libslist="${prefix}/${host_alias}/lib ${prefix}/lib64 ${prefix}/lib32 ${prefix}/lib"
  export pathlist="${prefix}/${host_alias}/bin:${prefix}/bin"
  LDFLAGS="${LDFLAGS} -Wl,--as-needed"
  plugin=no
else
  AC_MSG_NOTICE([Configuring Gnash for native compilation])
  export incllist=`cat ${srcdir}/macros/incllist`
  export libslist=`cat ${srcdir}/macros/libslist`
  export pathlist=$PATH
fi

dnl GNASH_PKG_FIND(dmalloc, [dmalloc.h], [dmalloc], mallinfo)
AM_CONDITIONAL(HAVE_DMALLOC, [ test x$has_dmalloc = xyes ])

AC_CHECK_FUNCS(mallinfo)
PKG_PROG_PKG_CONFIG
dnl LIBS+="-L/usr/lib64"
AC_PATH_TOOL([AUTOTRACE], [autotrace])
AC_HEADER_DIRENT

dnl
dnl !! This has been moved here to make --enable-plugin work
dnl !! All of plugin-related macro calls could be moved into
dnl !! a specialized macros/plugin.m4
dnl
AC_ARG_ENABLE(plugin,
  AC_HELP_STRING([--disable-plugin], [Disable plugins build]),
  [case "${enableval}" in
    yes) plugin=yes ;;
    no)  plugin=no ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for disable-plugin option]) ;;
  esac],
  plugin=yes
)

GNASH_PATH_FIREFOX

dnl IRIX-hack.
case $host in
  *-*-irix*)
  dnl ABI-check
  save_LIBS=$LIBS
  dir="/usr/lib /usr/lib32 /usr/lib/lib64"
  for i in $dir; do
    LIBS=-L$i
    AC_SEARCH_LIBS(XDisableAccessControl, X11, [
      xpathed=$i
      break
    ])
    unset ac_cv_search_XDisableAccessControl
  done
  ac_x_libraries=$xpathed
  x_libraries=$xpathed
  LIBS=$save_LIBS
  ;;
esac
dnl AC_PATH_XTRA
GNASH_PATH_X11

AC_CHECK_HEADERS(dejagnu.h)
AC_CHECK_HEADERS(malloc.h)
AC_CHECK_HEADERS(getopt.h)
AC_CHECK_HEADERS(libgen.h)

dnl seems unused
dnl AC_CHECK_LIB(Xmu, XmuCvtStringToOrientation)
AC_CHECK_LIB(m, sqrt)
AC_CHECK_LIB(Xi, XInput_find_display)
AC_CHECK_LIB(X11, XDisableAccessControl)
AC_CHECK_LIB(rt, shm_unlink)
AC_CHECK_FUNCS(shm_open shm_unlink)
AC_TRY_COMPILE([#include <strings.h>], [
  char *p1 = (char *)"Hello";
  char *p2 = (char *)"World";
  strcasecmp(p1, p2); ],
  AC_DEFINE(HAVE_STRINGCASECMP, [1], [Has strcasecmp])
)

dnl AC_CHECK_FUNCS(strcasecmp stricmp)
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

AC_LANG_PUSH(C++)
AC_CACHE_CHECK([whether $CXX implements __PRETTY_FUNCTION__], ac_cv_implements___PRETTY_FUNCTION__, [
        AC_TRY_LINK([
#include <cstdio>
], 
                [ std::printf("%s", __PRETTY_FUNCTION__); ], 
                [ ac_cv_implements___PRETTY_FUNCTION__="yes" ],
                [ ac_cv_implements___PRETTY_FUNCTION__="no" ]
        )
])
if test "x$ac_cv_implements___PRETTY_FUNCTION__" = "xyes" ; then
        AC_DEFINE(HAVE_PRETTY_FUNCTION, [1], [__PRETTY_FUNCTION__ is defined])
fi

AC_CACHE_CHECK([whether $CXX implements __FUNCTION__], ac_cv_implements___FUNCTION__, [
        AC_TRY_LINK([
#include <cstdio>
], 
                [ std::printf("%s", __FUNCTION__); ], 
                [ ac_cv_implements___FUNCTION__="yes" ],
                [ ac_cv_implements___FUNCTION__="no" ]
        )
])
if test "x$ac_cv_implements___FUNCTION__" = "xyes" ; then
        AC_DEFINE(HAVE_FUNCTION, [1], [__FUNCTION__ is defined])
fi

AC_CACHE_CHECK([whether $CXX implements __func__], ac_cv_implements___func__, [
        AC_TRY_LINK([
#include <cstdio>
], 
                [ std::printf("%s", __func__); ], 
                [ ac_cv_implements___func__="yes" ],
                [ ac_cv_implements___func__="no" ]
        )
])
if test "x$ac_cv_implements___func__" = "xyes" ; then
        AC_DEFINE(HAVE_func, [1], [__func__ is defined])
fi
AC_LANG_POP(C++)
AC_REPLACE_FUNCS(getopt)

AC_CHECK_FUNCS(sysconf)
AC_CHECK_FUNCS(shmget shmat shmdt mmap)
AC_CHECK_FUNCS(gettimeofday)
AC_CHECK_FUNCS(ftime)
AC_CHECK_FUNCS(tzset)
AC_CHECK_FUNCS(localtime_r)
AC_CHECK_FUNCS(memmove)
AC_CHECK_FUNCS(strlcpy, AC_DEFINE(HAVE_STRLCPY_PROTO, [1],[Define if you have the strlcpy prototype]))
AC_CHECK_FUNCS(strlcat, AC_DEFINE(HAVE_STRLCAT_PROTO, [1],[Define if you have the strlcat prototype]))
dnl Look for Win32 networking stuff
AC_CHECK_HEADERS(winsock.h)
AC_CHECK_HEADERS(sys/endian.h)
AC_CHECK_HEADERS(machine/endian.h)
AC_CHECK_HEADERS(endian.h)
AC_CHECK_FUNCS(socket)
AC_CHECK_FUNCS(CreateFileMappingA)

dnl GNASH_HASHMAP
dnl Let's get rid of this small file and inline the function.

AC_LANG_PUSH(C++)
	AC_CHECK_HEADER(ext/hash_map, [
		AC_DEFINE([GNU_HASH_MAP], [1], [Using GNU ext/hash_map.h])
		AC_DEFINE([HASH_MAP_NS], [__gnu_cxx], [Using GNU __gnu_cxx::])
	], [
		AC_CHECK_HEADER(hash_map, [
			AC_DEFINE([WIN32_HASH_MAP], [1], [Using Win32 hash_map.h])
			AC_DEFINE([HASH_MAP_NS], [stdext], [Using win32 stdext::])
		])
	])
AC_LANG_POP(C++)

dnl AC_CHECK_LIB(c, mremap)
GNASH_PKG_FIND(z, [zlib.h], [zlib compression library], compress)
GNASH_PKG_FIND(jpeg, [jpeglib.h], [jpeg images], jpeg_mem_init)

dnl if we're not building the extensions, then the _FLAGS and _LIBS settings
dnl found in these tests are blank. This way they can be used in
dnl Makefiles, but won't add bloat to the Makefiles.
if test x$extensions = xyes; then
  GNASH_PATH_MYSQL
  AC_DEFINE(USE_EXTENSIONS, [], [Defined if you enable building the exetensions])
fi
AM_CONDITIONAL(ENABLE_MYSQL, [ test x"$mysql" = xyes ])

AC_PATH_MING
AM_CONDITIONAL(ENABLE_MING, [ test x"$MAKESWF" != x ])

AC_PATH_PROG(DOXYGEN, doxygen)
AM_CONDITIONAL(ENABLE_DOXYGEN, [ test x"$DOXYGEN" != x ])

AC_PATH_MTASC
AM_CONDITIONAL(ENABLE_MTASC, [ test x"$MTASC" != x ])

dnl currently unused
dnl GNASH_PKG_FIND(ogg, [ogg.h], [decode ogg streams], ogg_stream_init)

if test x$gui = xsdl -o x$sound_handler = xsdl; then
  GNASH_PATH_SDL
fi

AM_CONDITIONAL(HAS_SDL, [ test x$has_SDL = xyes ])

GNASH_PATH_BOOST
GNASH_PATH_LIBXML

dnl
dnl NOTE: make sure to check for GLIB and LIBXML2 
dnl before checking for gstreamer
dnl

AC_CHECK_SIZEOF(size_t) dnl set SIZEOF_SIZE_T.
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(short)

dnl # NO plugin ? NO klash !
if test x"$plugin" = xno; then
  echo "WARNING: plugin disabled, klash disabled accordingly"
  klash=no
fi

dnl # Check KDE if kde gui or klash were requested
if test x"$gui" = xkde -o x$klash = xyes; then
  kde=yes
  qt=yes
  GNASH_PATH_KDE
  GNASH_PATH_QT
fi
AM_CONDITIONAL(HAVE_KDE, [test x$has_kde = xyes])
AM_CONDITIONAL(HAVE_QT, [test x$has_qt = xyes])

dnl # NO kde ? NO klash !
if test x$has_kde != xyes; then
  if test x$klash = xyes; then
    echo "WARNING: No KDE development package installed, disabling klash"
    klash=no
  fi
fi

AM_CONDITIONAL(KLASH, [test x$klash = xyes])

AC_SUBST(KDE_CFLAGS)
AC_SUBST(KDE_LIBS)
if test x$renderer = xopengl; then
  GNASH_PATH_OPENGL
fi

AM_CONDITIONAL(opengl, [test x$opengl = xyes])

dnl Need GLIB for both GTK and GST
if test x${gui} = "xgtk" -o x${sound_handler} = "xgst"; then
  GNASH_PATH_GLIB
fi

AM_CONDITIONAL(HAVE_GLIB, [ test x$has_glib = xyes ])

if test x$gui = xgtk; then
  if test x$renderer = xopengl; then
    GNASH_PATH_GLEXT
  fi
  GNASH_PATH_GTK2
  GNASH_PATH_PANGO
  GNASH_PKG_FIND(atk, [atk/atk.h], [atk library], atk_focus_tracker_init, [1.0])

  AC_ARG_ENABLE(ghelp,
    AC_HELP_STRING([--enable-ghelp], [Enable support for the GNOME help system]),
    [case "${enableval}" in
      yes) ghelp=yes ;;
      no)  ghelp=no ;;
      *)   AC_MSG_ERROR([bad value ${enableval} for enable-ghelp option]) ;;
    esac], ghelp=no
  )

  if test x"$ghelp" = x"yes" ; then
    AC_PATH_PROG(SCROLLKEEPER, scrollkeeper-config, [], [$PATH:/usr/bin/X11:/usr/local/bin/X11:/opt/X11])
    AC_PATH_PROG(SCROLLUPDATE, scrollkeeper-update, [],	[$PATH:/usr/bin/X11:/usr/local/bin/X11:/opt/X11])
    AC_PATH_PROG(SCROLLINSTALL, scrollkeeper-preinstall, [], [$PATH:/usr/bin/X11:/usr/local/bin/X11:/opt/X11])

    if test x"$SCROLLKEEPER" = x -o x"$SCROLLUPDATE" = x -o x"$SCROLLINSTALL" = x ; then
      ghelp=no
      AC_MSG_WARN([You need to install scrollkeeper for gnome help])
    fi
  fi
fi
AM_CONDITIONAL(GHELP, [test x$ghelp = xyes])
AM_CONDITIONAL(HAVE_AGG, [test x${ac_cv_path_agg_lib} != x])

if test x"$gui" = x"fltk"; then
  GNASH_PKG_FIND(Xft, [Xft.h], [xft library], XftGlyphRender)
fi

dnl Some systems have a pervered set of dependencies. Although Gnash itself uses libXML,
dnl Fedora Core 6 appears to have a dependency on expat for fontconfig. We only need the
dnl library, but this is the easy wind to find it.
GNASH_PKG_FIND(expat, [expat.h], [Expat library], XML_ErrorString)

dnl these conditionals were moved out of kde.m4
AM_CONDITIONAL(HAS_KDE, [test x$kde = xyes])
# used to disable x11-specific stuff on special platforms
AM_CONDITIONAL(include_x11, test "$kde_use_qt_emb" = "no" && test "$kde_use_qt_mac" = "no")
AM_CONDITIONAL(include_ARTS, test "$build_arts" '!=' "no")
AM_CONDITIONAL(unsermake_enable_pch, test "$kde_use_pch" = "yes" && test "$kde_gcc_supports_pch" = "yes")

AM_CONDITIONAL(HAVE_GTK2, [ test x$has_gtk2 = xyes ])
AM_CONDITIONAL(HAVE_GLEXT, [test x$glext = xyes])

dnl We don't have GTKGLExt, so default to SDL, and don't build the Firefox plugin
if test x$glext = xno -a x$renderer = xopengl; then
  if test x$gtk2 = xyes -a x$gui = xgtk; then
    AC_ERROR([You have GTK installed, but not GtkGLExt. You need GtkGLExt to use the OpenGL renderer. Attempting to build SDL version])
  fi
  gui=sdl
  plugin=no
  AC_MSG_WARN([GTK2 specified for the GUI, but GtkGlExt is not present. Trying SDL instead.])
fi

AM_CONDITIONAL(PLUGIN, [test x$plugin = xyes])
AM_CONDITIONAL(USE_GUI_GTK, test x$gui = xgtk)
AM_CONDITIONAL(USE_GUI_SDL, test x$gui = xsdl)
AM_CONDITIONAL(USE_GUI_KDE, test x$gui = xkde)
AM_CONDITIONAL(USE_GUI_FLTK, test x$gui = xfltk)
AM_CONDITIONAL(USE_GUI_FB, test x$gui = xfb)
AM_CONDITIONAL(USE_GUI_RISCOS, test x$gui = xriscos)

case "${gui}" in
  gtk)  AC_DEFINE([GUI_GTK],  [1], [Use GTK gui toolkit]) ;;
  sdl)  AC_DEFINE([GUI_SDL],  [1], [Use SDL gui toolkit]) ;;
  kde)  AC_DEFINE([GUI_KDE],  [1], [Use KDE gui toolkit]) ;;
  fltk) AC_DEFINE([GUI_FLTK], [1], [Use FLTK gui toolkit]) ;;
  fb) AC_DEFINE([GUI_FB], [1], [Use Framebuffer, no gui toolkit]) ;;
  riscos) AC_DEFINE([GUI_RISCOS], [1], [Use RISC OS gui toolkit]) ;;
  *)
esac

if test x"$mp3_decoder" = x"ffmpeg"; then
  GNASH_PATH_FFMPEG
  if test x"${ac_cv_path_ffmpeg_lib}" = x ; then
    AC_MSG_WARN([ffmpeg specified as decoder, but it's not present, disabling sound])
    mp3_decoder=none
    sound_handler=none
  fi
fi

if test x"$mp3_decoder" = x"mad"; then
  dnl Will set MAD_LIBS and MAD_CFLAGS.
  dnl Handles --with-mad-incl and --with-mad-lib
  GNASH_PKG_FIND(mad, [mad.h], [mad library], mad_copyright)
  if test x"${ac_cv_path_mad_lib}" = x ; then
    mp3_decoder=none
    sound_handler=none
    AC_MSG_WARN([Libmad specified as decoder, but it's not present, disabling sound])
  fi
fi
dnl I'm kinda lazy, get rid of this later... //Markus
AM_CONDITIONAL(HAVE_MAD, test x$mp3_decoder = xmad)

AM_CONDITIONAL(USE_SOUND_GST, test x$sound_handler = xgst)
AM_CONDITIONAL(USE_SOUND_SDL, test x$sound_handler = xsdl)
AM_CONDITIONAL(USE_FFMPEG_ENGINE, test x$mp3_decoder = xffmpeg)
AM_CONDITIONAL(USE_MAD_ENGINE, test x$mp3_decoder = xmad)

dnl AM_CONDITIONAL(HAVE_FFMPEG, [test x"${FFMPEG_LIBS}" != x])

case "${sound_handler}" in
  gst) AC_DEFINE([SOUND_GST],  [1], [Use GSTREAMER for sound]) ;;
  sdl) AC_DEFINE([SOUND_SDL],  [1], [Use SDL for sound]) ;;
  *)
esac

case "${mp3_decoder}" in
  ffmpeg)  AC_DEFINE([USE_FFMPEG],  [1], [Use FFMPEG for mp3 decoding]) ;;
  mad)  AC_DEFINE([USE_MAD],  [1], [Use MAD for mp3 decoding]) ;;
  *)
esac

if test "$sound_handler" = "gst"; then
  GNASH_PKG_FIND(gstreamer, [gst/gst.h], [gstreamer library], gst_init, [0.10])
fi

dnl I'm kinda lazy, get rid of this later... //Markus
AM_CONDITIONAL(HAVE_GST, test x$sound_handler = xgst)

if test x$gui = xfltk; then
  GNASH_PKG_FIND(fltk2, [fltk/FL_API.h], [Fast Light Toolkit], fl_window_flush)
fi

AM_CONDITIONAL(HAS_FLTK2, [ test x$has_fltk2 = xyes ])
AM_CONDITIONAL(HAS_XFT, [ test x$has_xft = xyes ])


if test x$renderer = xagg; then
  GNASH_PATH_AGG
fi

if test x$renderer = xcairo -o x$gui = xgtk; then
  dnl Cairo renderer and GTK gui always need Cairo
  GNASH_PKG_FIND(cairo, [cairo.h], [cairo render library], cairo_status)
fi

GNASH_DOCBOOK
AM_CONDITIONAL(DOCBOOK, test x$docbook = xyes)

GNASH_PATH_CURL
AC_LIBTOOL_DLOPEN
dnl AC_LIBLTDL_CONVENIENCE
AC_LIBLTDL_INSTALLABLE
AC_PROG_LIBTOOL
AC_CHECK_LIB(ltdl, lt_dlmutex_register, AC_DEFINE(LT_DLMUTEX, 1, [Has lt_dlmutex_register]))

DLOPEN="-dlopen"
DLPREOPEN="-dlpreopen"
AC_SUBST(DLOPEN)
AC_SUBST(DLPREOPEN)

dnl Substitute INCLTDL and LIBLTDL in the Makefiles
AC_SUBST(INCLTDL)
AC_SUBST(LIBLTDL)
AC_SUBST(LIBTOOL_DEPS)

dnl Define winsock if we're on windows. We could do something complicated,
dnl but since AC_EXEEXT does it for us, we'll do this the easy way.
if test x"$EXEEXT" == "exe"; then
  AC_DEFINE(HAVE_WINSOCK,1,[This is defined is we are on Win32])
fi

dnl ****************************************
dnl *** Check for ELF visibility support ***
dnl ****************************************

AC_ARG_ENABLE([visibility], AC_HELP_STRING([--enable-visibility], [Use ELF visibility attributes]), [], [enable_visibility=no])

if test x"$enable_visibility" != x"no"; then
  dnl Check whether the compiler supports the visibility attribute
  save_CFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS -Wall -Werror"
  AC_MSG_CHECKING([whether $CC supports the GNUC visibility attribute])
  AC_COMPILE_IFELSE(AC_LANG_SOURCE(
    [
      void __attribute__ ((visibility("default"))) test_default (void) {}
      void __attribute__ ((visibility("hidden"))) test_hidden (void) {}
      int main (int argc, char **argv) { test_default (); test_hidden (); return 0; }
    ]),
    [
      AC_DEFINE([HAVE_GNUC_VISIBILITY], [1], [Define this for GCC-visibility.])
      AC_MSG_RESULT([yes])
    ],
    [
      AC_MSG_RESULT([no])
    ]
  )
  CFLAGS="$save_CFLAGS"
fi


GNASH_PATH_PTHREADS
if test x"$pthreads" = x"yes"; then
  if test x"$PTHREAD_CFLAGS" != x; then
    CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"
  fi
  if test "$CC" != "$PTHREAD_CC"; then
    AC_MSG_WARN([Replacing compiler $CC with compiler $PTHREAD_CC to support pthreads.])
    CC="$PTHREAD_CC"
  fi
  if test "$CXX" != "$PTHREAD_CXX"; then
    AC_MSG_WARN([Replacing compiler $CXX with compiler $PTHREAD_CXX to support pthreads.])
    CXX="$PTHREAD_CXX"
  fi
fi

AX_GCC_ARCHFLAG(no)

AC_ARG_ENABLE([strict], AC_HELP_STRING([--enable-strict],[Accept only standards compliant code (GCC only)]),
  [case "${enableval}" in
    yes) strict=yes ;;
    no) strict=no ;;
    *) AC_MSG_ERROR([bad value ${enableval} for --enable-strict option]) ;;
  esac],
  [strict=no]
)

if test x"$strict" = x"yes" -a x"$renderer" = x"agg"; then
  AC_MSG_ERROR([agg renderer will fail with --enable-strict.]);	
fi

# We want warnings, lots of warnings  :-)
if test x"$GCC" = x"yes"; then
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
  if test x"$strict" = x"yes"; then
    CXXFLAGS="$CXXFLAGS \
      -ansi \
      -pedantic \
      -Wno-long-long \
      "

    CFLAGS="$CFLAGS \
      -ansi \
      -pedantic \
      -Wno-long-long \
      "
  fi
fi

dnl FIXME: this GCC feature isn't fully debugged yet, so we don't want
dnl to use it by default.
dnl if test "$GCC" = "yes"; then
dnl   AC_COMPILE_IFELSE([
dnl   #if __GNUC__ < 4
dnl   #error
dnl   #endif],gccvisibility=yes,)
dnl   if test x"$gccvisibility" = x"yes"; then
dnl     CXXFLAGS="$CXXFLAGS -fvisibility-inlines-hidden"
dnl   fi
dnl fi

dnl Define convienience constants so Gnash can print out the
dnl default configuration of the build.
RENDERER_CONFIG=$renderer
AC_SUBST(RENDERER_CONFIG)
GUI_CONFIG=$gui
AC_SUBST(GUI_CONFIG)
SOUND_CONFIG=$sound_handler
AC_SUBST(SOUND_CONFIG)
DECODER_CONFIG=$mp3_decoder
AC_SUBST(DECODER_CONFIG)

dnl AC_CONFIG_LINKS(doc/C/images)
AC_CONFIG_LINKS(testsuite/libbase/gnashrc:testsuite/libbase/gnashrc.in)

AC_OUTPUT(Makefile
libbase/Makefile
libgeometry/Makefile
server/Makefile
server/asobj/Makefile
server/vm/Makefile
server/parser/Makefile
libamf/Makefile
backend/Makefile
utilities/Makefile
doc/Makefile
doc/C/Makefile
doc/Doxyfile
testsuite/Makefile
testsuite/libbase/Makefile
testsuite/libgeometry/Makefile
testsuite/actionscript.all/Makefile
testsuite/samples/Makefile
testsuite/misc-ming.all/Makefile
testsuite/misc-mtasc.all/Makefile
testsuite/movies.all/Makefile
testsuite/server/Makefile
gui/Makefile
extensions/Makefile
extensions/dejagnu/Makefile
extensions/mysql/Makefile
extensions/fileio/Makefile
extensions/gtk2/Makefile
plugin/Makefile
plugin/klash/Makefile
cygnal/Makefile
)

echo ""
echo "Configurable options are:"

if test x"$i810lodbias" = x"yes"; then
  echo "        Intel 810 LOD bias hack enabled"
else
  echo "        Intel 810 LOD bias hack disabled (default). Use --enable-i810-lod-bias to enable."
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

if test x"$plugin" = x"yes"; then
  echo "        Firefox plugin enabled (default). Use --disable-plugin to disable."
else
  echo "        Firefox plugin disabled."
fi

if test x"$klash" = x"yes"; then
  echo "        Konqueror plugin enabled (default). Use --disable-klash to disable"
else
  echo "        Konqueror plugin disabled."
fi

if test x"$ghelp" = x"yes"; then
  echo "        GNOME help enabled"
else
  echo "        GNOME help disabled (default). Use --enable-ghelp to enable."
fi

if test x"$extensions" = x"yes"; then
  echo "        Building extensions enabled (default). Use --disable-extensions to disable."
else
  echo "        Building Gnash extensions disabled."
fi

if test x"$cygnal" = x"yes"; then
  echo "        Building Cygnal media server enabled (default). Use --disable-cygnal to disable."
else
  echo "        Building Cygnal media server disabled."
fi

if test x"$debugger" = x"yes"; then
  echo "        Building Flash debugger support (default). Use --disable-debugger to disable."
else
  echo "        Building the Flash debugger is disabled."
fi

echo "        Renderer engine: "$renderer
echo "        GUI: "$gui
echo "        Sound handler: "$sound_handler
if test x"$sound_handler" = xsdl; then
echo "        MP3 decoder: "$mp3_decoder
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

if test x$xml != xno; then
  if test x"$LIBXML_CFLAGS" != x; then
    echo "        XML flags are: $LIBXML_CFLAGS"
    echo "        XML libs are: $LIBXML_LIBS"
  else
    echo "        ERROR: No libxml2 development package installed!\
 To compile this project install libxml2-dev (using apt-get)\
 or libxml2-devel (using yum or rpm)."
    nogo=true
  fi
fi

if test x$extensions = xyes; then
  if test x$mysql != xno; then
    if test x"$MYSQL_LIBS" != x; then
      echo "        MYSQL flags are: $MYSQC_CFLAGS"
      echo "        MYSQL libs are: $MYSQL_LIBS"
    else
      echo "        ERROR: No MySQL development package installed!"
    fi
  fi
fi

if test x$kde = xyes -o x$klash = xyes; then
  if test x"$has_kde" = xyes; then
    echo "        KDE flags are: $KDE_CFLAGS"
    echo "        KDE libs are: $KDE_LIBS"
  else
    if test x$gui = xkde -o x$klash = xyes ; then
      echo "        ERROR: No kde development package installed!"
      echo "               Reconfigure with --disable-klash to continue"
      echo "               and/or change to a different gui"
      nogo=true
    fi
  fi
  if test x"$has_qt" = xyes; then
    echo "        QT flags are: $QT_CFLAGS"
    echo "        QT libs are: $QT_LIBS"
  else
    if test x$gui = xkde -o x$klash = xyes ; then
      echo "        ERROR: No QT development package installed!"
      echo "               Reconfigure with --disable-klash to continue"
      echo "               and/or change to a different gui"
      nogo=true
    fi
  fi
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

dnl if test x"$OGG_LIBS" != x; then
dnl   if test x"$OGG_CFLAGS" != x; then
dnl     echo "        Ogg flags are: $OGG_CFLAGS"
dnl   else
dnl     echo "        Ogg flags are: default include path"
dnl   fi
dnl   echo "        Ogg libs are: $OGG_LIBS"
dnl else
dnl   echo "ERROR: No Ogg Vorbis development package installed!"
dnl   nogo=true
dnl fi

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
      echo "        OpenGL flags are: $OPENGL_CFLAGS"
    else
      echo "        OpenGL flags are: default include path"
    fi
    echo "        OpenGL libs are: $OPENGL_LIBS"
    else
      echo "ERROR: No OpenGL development package installed!\
 You need to have the libmesa development package installed\
 to compile this project or install libgl1-mesa-dev\
 (using apt-get) or xorg-x11-Mesa-libGL (using yum)."
    nogo=true
  fi
fi

dnl GLEXT is only needed for GTK/OpenGL
if test x"$gui" = x"gtk" -a x"$renderer" = xopengl ; then
  if test x"$GLEXT_LIBS" != x; then
    if test x"$GLEXT_CFLAGS" != x; then
      echo "        GtkGLExt flags are: $GLEXT_CFLAGS"
    else
      echo "        GtkGLExt flags are: default include path"
    fi
      echo "        GtkGLExt libs are: $GLEXT_LIBS"
  else
    if test x"$gui" = x"gtk"; then
      if test x"$renderer" = x"opengl"; then
        echo "ERROR: No GtkGLExt development package installed!\
 You need to have the GtkGLExt development package installed\
 to build the GTK gui / opengl renderer combination.\
 Try --enable-gui=sdl or --enable-renderer=cairo or install libgtkglext1-dev\
 (using apt-get) or gtkglext-devel (using yum)."
      nogo=true
      fi
    fi
  fi
fi

if test x"$gui" = x"gtk"; then #{

  if test x"$GTK2_LIBS" != x; then
    if test x"$GTK2_CFLAGS" != x; then
      echo "        GTK2 flags are: $GTK2_CFLAGS"
    else
      echo "        GTK2 flags are: default include path"
    fi
      echo "        GTK2 libs are: $GTK2_LIBS"
  else
    echo "ERROR: No GTK2 development package installed!\
 You need to have the GTK2 development package installed\
 to compile this project or install libgtk2.0-dev\
 (using apt-get) or gtk2-devel (using yum)."
dnl    nogo=true
  fi

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

  if test x"$GLIB_LIBS" != x; then
    if test x"$GLIB_CFLAGS" != x; then
      echo "        GLib flags are: $GLIB_CFLAGS"
    else
      echo "        GLib flags are: default include path"
    fi
    echo "        GLib libs are: $GLIB_LIBS"
  else
    echo "ERROR: No GLib development package installed!\
 You need to have the GLib development package installed\
 to compile this project or install libglib2.0-dev\
 (using apt-get) or glib2-devel (using yum)."
dnl    nogo=true
  fi

  if test x"$ATK_LIBS" != x; then
    if test x"$ATK_CFLAGS" != x; then
      echo "        ATK flags are: $ATK_CFLAGS"
    else
      echo "        ATK flags are: default include path"
    fi
      echo "        ATK libs are: $ATK_LIBS"
  else
    echo "ERROR: No ATK development package installed!\
 You need to have the ATK development package installed\
 to compile this project or install libatk1.0-dev\
 (using apt-get) or atk-devel (using yum)."
dnl    nogo=true
  fi

fi #}

if test "$sound_handler" = "gst"; then
  if test x"$GSTREAMER_LIBS" != x; then
    if test x"$GSTREAMER_CFLAGS" != x; then
      echo "        Gstreamer flags are: $GSTREAMER_CFLAGS"
    else
      echo "        Gstreamer flags are: default include path"
    fi
      echo "        Gstreamer libs are: $GSTREAMER_LIBS"
  else
    echo "ERROR: gstreamer sound requested, but no gstreamer 0.10+ found"
    nogo=true
  fi
fi

if test "$sound_handler" = "sdl"; then
  if test x"$mp3_decoder" = x"mad"; then
    echo "        MP3 support enabled through libmad"
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
 to enable mp3 decoding. You can either reconfigure using \
 --with-mp3-decoder=ffmpeg to use ffmpeg or install libmad0-dev\
 (using apt-get) or libmad (using yum)."
      nogo=true
    fi
  else
    if test x"$mp3_decoder" = x"ffmpeg"; then
      if test x${ffmpeg_version} != xok; then
         echo "ERROR: You have version ${ffmpeg_version} of ffmpeg installed. \
This version of ffmpeg may not work! \
Version 51.27.0 or newer is highly recommended"
        nogo=true
      fi
      if test x"$FFMPEG_LIBS" != x; then
        echo "        MP3 support enabled through ffmpeg"
        if test x"$FFMPEG_CFLAGS" != x; then
          echo "        ffmpeg flags are: $FFMPEG_CFLAGS"
        else
          echo "        ffmpeg flags are: default include path"
        fi
          echo "        ffmpeg libs are: $FFMPEG_LIBS"
      else
        echo "ERROR: No ffmpeg development package installed!\
 You need to have the ffmpeg development package installed\
 to enable mp3 decoding. You can either reconfigure using \
 --with-mp3-decoder=mad to use libmad or install libavcodec-dev \
 and libdts-dev (using apt-get) or <package_names_here> (using yum).".
        nogo=true
      fi
    fi
  fi
fi

if test "$renderer" = "cairo"; then
  if test x"$CAIRO_LIBS" != x; then
    if test x"$CAIRO_CFLAGS" != x; then
      echo "        Cairo flags are: $CAIRO_CFLAGS"
    else
      echo "        Cairo flags are: default include path"
    fi
    echo "        Cairo libs are: $CAIRO_LIBS"
  else
    echo "WARNING: No Cairo development package installed!\
 You need to have the Cairo development package installed\
 if you have used --enable-render=cairo to configure\
 To compile this project, install libcairo-dev\
 (using apt-get) or cairo-devel (using yum).\
 Current Cairo is only being used by developers."
    nogo=true
  fi
fi

if test x$gui = xfltk; then
  if test x"$FLTK2_LIBS" != x; then
    if test x"$FLTK2_CFLAGS" != x; then
      echo "        Fltk flags are: $FLTK2_CFLAGS"
    else
      echo "        Fltk flags are: default include path"
    fi
      echo "        Fltk libs are: $FLTK2_LIBS"
  else
    echo "ERROR: No Fltk2 development package installed!\
 You need to have the fltk2 development package installed\
 to compile this project."
dnl or install libfltk1.1-dev (using apt-get) or fltk-devel (using yum).
    nogo=true
  fi
fi

if test x"$gui" = x"fltk"; then
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

if test x"$gui" = x"sdl"; then
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
fi

if test x"$sound_handler" = x"sdl"; then
  if test x"$SDL_LIBS" != x; then
      echo "        SDL flags are: $SDL_CFLAGS"
      echo "        SDL libs are: $SDL_LIBS"
  else
      echo "ERROR: sdl sound requested, but no SDL development package installed!\
 You need to have the SDL development package installed\
 if you wish to have sdl-based sound support for Gnash.\
 To compile this project with SDL sound support, install\
 libsdl-1.2-dev (using apt-get) or SDL-devel (using yum).
 In alternative build with --enable-sound=gst for gstreamer use"
   nogo=true
  fi
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
  if test x"$PTHREAD_CFLAGS" != x; then
    echo "        POSIX Threads flags are: $PTHREAD_CFLAGS"
  fi
  if test x"$PTHREAD_LIBS" != x; then
    echo "        POSIX Threads lib is: $PTHREAD_LIBS"
  else
    echo "ERROR: No pthread development package installed!"
    nogo=true
  fi
fi

if test x"$docbook" = x"yes"; then
  if test x"$DB2X_XSLTPROC" = x -o x"$DB2X_TEXIXML" = x -o x"$MAKEINFO" = x -o x"$XSLTPROC" = x; then
    echo "ERROR: No DocBook2X tools installed!\
 docbook, docbook-xml, docbook2Xm, texinfo, docbook-utils, docbook-xsl (using apt-get or yum)."
    nogo=true
  else
    echo "        You have all the DocBook2X tools installed."
    echo "        You have version $db2x_version of the DocBook2X tools."
  fi
fi

if test x"$CURL_LIBS" != x; then
  if test x"$rtmp" = xyes; then
    echo "	This version of libCurl has RTMP support"
  else
    echo "	This version of libCurl doesn't have RTMP support"
  fi
  if test x"$CURL_CFLAGS" != x; then
    echo "        CURL flags are: $CURL_CFLAGS"
  else
    echo "        CURL flags are: default include path"
  fi
    echo "        CURL libs are: $CURL_LIBS"
fi

if test x$renderer = xagg; then
  echo "        AGG Pixel format is: $pixelformat"
  if test x"${agg25}" = xyes; then
    if test x"$AGG_LIBS" != x; then
      if test x"$AGG_CFLAGS" != x; then
        echo "        AGG flags are: $AGG_CFLAGS"
      else
        echo "        AGG flags are: default include path"
      fi
      echo "        AGG libs are: $AGG_LIBS"
dnl      echo "          needs compatibility layer: $agg_need_compatibility_layer"
    else
      echo "ERROR: No AGG development package installed!\
 You need to have the AGG development package installed\
 to compile this project or install libagg-dev\
 (using apt-get) or agg-devel (using yum)."
dnl    nogo=true
    fi
  else
    echo "Your installation of AGG appears to be a version older than 2.4. \
Please upgrade to AGG 2.4 or greater."
  fi
fi

if test x"$BOOST_LIBS" != x -a x"$boost_date_time" = xyes; then
    echo "        BOOST flags are: $BOOST_CFLAGS"
    echo "        BOOST libs are: $BOOST_LIBS"
else
    echo "ERROR: No BOOST development package installed!\
 You need to have the BOOST development package installed\
 to compile, or install libboost-thread-dev and\
 libboost-date-time-dev (using apt-get) or boost-devel\
 (using yum)."
 nogo=true
fi

if test x"$JPEG_LIBS" != x; then
    echo "        JPEG flags are: $JPEG_CFLAGS"
    echo "        JPEG libs are: $JPEG_LIBS"
else
    echo "ERROR: No JPEG development package installed!\
 You need to have the libjpeg package installed\
 to compile, or install libjpeg-dev (using apt-get)\
 or libjpeg-devel (using yum)."
 nogo=true
fi

if test x"$MING_LIBS" != x; then
    echo "        MING version code is $MING_VERSION_CODE"
    echo "        MING flags are $MING_CFLAGS"
    echo "        MING libs are $MING_LIBS"
    echo "        MAKESWF is $MAKESWF"
else
    echo "Warning:\
 You need to have the Ming development and utilities packages installed\
 to run most of the tests in Gnash testsuite. Some of the tests will be\
 run anyway."
fi

if test x"$MTASC" != x; then
    echo "        MTASC is $MTASC"
    echo "        MTASC CLASSPATH is $MTASC_CLASSPATH"
else
    echo "Warning:\
 You need to have the MTASC compiler packages installed\
 to run some of the tests in Gnash testsuite."
fi

if test x"$Z_LIBS" != x; then
  if test x"$Z_CFLAGS" != x; then
    echo "        Z flags are: $Z_CFLAGS"
  else
    echo "        Z flags are: default include path"
  fi
  echo "        Z libs are: $Z_LIBS"
else
  echo "Warning:\
 You need to have the zlib development packages installed\
 to play compressed SWF (most of them from version 6 up). "
fi

if test x$kde = xyes -o x$klash = xyes; then
  if test x"$has_kde" = xyes; then
    echo "        KDE flags are: $KDE_CFLAGS"
    echo "        KDE libs are: $KDE_LIBS"
  else
    if test x$gui = xkde -o x$klash = xyes ; then
      echo "        ERROR: No kde development package installed!"
      echo "               Reconfigure with --disable-klash to continue"
      echo "               and/or change to a different gui"
      nogo=true
    fi
  fi
  if test x"$has_qt" = xyes; then
    echo "        QT flags are: $QT_CFLAGS"
    echo "        QT libs are: $QT_LIBS"
  else
    if test x$gui = xkde -o x$klash = xyes ; then
      echo "        ERROR: No QT development package installed!"
      echo "               Reconfigure with --disable-klash to continue"
      echo "               and/or change to a different gui"
      nogo=true
    fi
  fi
fi

if test x$extensions = xyes; then
  if test x$mysql != xno; then
    if test x"$MYSQL_LIBS" != x; then
      echo "        MYSQL flags are: $MYSQC_CFLAGS"
      echo "        MYSQL libs are: $MYSQL_LIBS"
    else
      echo "        ERROR: No MySQL development package installed!"
    fi
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

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
