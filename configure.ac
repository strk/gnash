dnl  
dnl    Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
dnl  
dnl  This program is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU General Public License as published by
dnl  the Free Software Foundation; either version 3 of the License, or
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

dnl $Id: configure.ac,v 1.371 2007/07/27 03:28:56 strk Exp $

AC_PREREQ(2.50)
AC_INIT(gnash, cvs)
AC_CONFIG_SRCDIR(server/gnash.h)
AM_CONFIG_HEADER(config.h)

dnl AC_CANONICAL_TARGET
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

dnl Some things you can only do by looking at the platform name.
case "${host}" in
  powerpc-apple-darwin*)
    AC_DEFINE([__powerpc64__], [], [this is a 64 bit powerpc])
    ;;
  i[34567]86-apple-darwin*)
    darwin=yes
    ;;
  i[34567]86-*-openbsd*)
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
dnl AM_INIT_AUTOMAKE(gnash, "cvs$BUILDDATE")
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE
AC_PROG_MAKE_SET

dnl AC_DISABLE_STATIC dnl Disable building static libs.

AM_GNU_GETTEXT([external])
AM_CONDITIONAL(HAS_GETTEXT, test x$ac_cv_path_XGETTEXT != x)
dnl AM_GNU_GETTEXT_VERSION(0.15)

dnl AC_CHECK_SIZEOF([size_t], 4, AC_INCLUDES_DEFAULT)

dnl AC_DEFINE(SIZET_FMT, ["%z"], [size_t printf modifier is z])

dnl NOTE: Using AC_TRY_COMPILE is bogus when cross-compiling !
dnl       We need to know the size of size_t in the *target* host, 
dnl       not the build host (right?)
dnl
AC_LANG_PUSH(C++)
save_CXXFLAGS="$CXXFLAGS"
CXXFLAGS="$CXXFLAGS -Werror -Wformat"

AC_TRY_COMPILE([#include <sys/types.h>
  #include <stdio.h>], [
  size_t num = 0;
  printf("%d", num); ],
  AC_DEFINE(SIZET_FMT, ["%d"], [size_t printf modifier is d])
)
AC_TRY_COMPILE([#include <sys/types.h>
  #include <stdio.h>], [
  size_t num = 0;
  printf("%u", num); ],
   AC_DEFINE(SIZET_FMT, ["%u"], [size_t printf modifier is u])
)
AC_TRY_COMPILE([#include <sys/types.h>
  #include <stdio.h>], [
  size_t num = 0;
  printf("%lu", num); ],
  AC_DEFINE(SIZET_FMT, ["%lu"], [size_t printf modifier is lu])
)
AC_TRY_COMPILE([#include <sys/types.h> 
  #include <stdio.h>], [
  size_t num = 0;
  printf("%ld", num); ],
  AC_DEFINE(SIZET_FMT, ["%ld"], [size_t printf modifier is ld])
)

if test x"$ac_env_CXXFLAGS_set" = "xset"; then
	CXXFLAGS="$save_CXXFLAGS"
else
	unset CXXFLAGS; 
fi
AC_LANG_POP(C++)

dnl This is primarily used when compiling for a similar architecture,
dnl like pentium->geode, which can use the same compiler, but have
dnl different development
AC_ARG_WITH(top_level, AC_HELP_STRING([--with-top-level], [top level directory for cross compiling files]), with_top_level=${withval})

AC_ARG_ENABLE(debugger,
  AC_HELP_STRING([--enable-debugger],[Enable the Flash debugger]),
[case "${enableval}" in
  yes) debugger=yes ;;
  no)  debugger=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debugger option]) ;;
esac],debugger=no)

if test x"$debugger" = x"yes"; then
  AC_DEFINE([USE_DEBUGGER], [], [Flash Debugger support])
fi
AM_CONDITIONAL(DEBUGGER, test x$debugger = xyes)

AC_ARG_WITH(top_level, AC_HELP_STRING([--with-top-level], [top level directory for cross compiling files]), with_top_level=${withval})

AC_ARG_ENABLE(fps-debug,
  AC_HELP_STRING([--enable-fps-debug],[Enable FPS debugging code]),
[case "${enableval}" in
  yes) AC_DEFINE([GNASH_FPS_DEBUG], [1], [Enable FPS debugging code])
esac])

AC_ARG_WITH(cpu,
  AC_HELP_STRING([--with-cpu],[specify a cpu when cross compiling.]),
  [case "${withval}" in
    geode) with_cpu=geode ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for --with-cpu option]) ;;
   esac],with_cpu=none)

if test x"$with_cpu" != x"none"; then
dnl   $CXXFLAGS="$CXXFLAGS -march=${with_cpu}"
   cross_compiling=yes
fi

dnl Don't add the GUI menu. Some educational systems think this adds
dnl clutter and confusion, like on the OLPC.
AC_ARG_ENABLE(menus,
  AC_HELP_STRING([--disable-menus],[Disable the GUI menus]),
[case "${enableval}" in
  yes) menus=yes ;;
  no)  menus=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for --disable-menus option]) ;;
esac],menus=yes)

if test x"$menus" = x"yes"; then
  AC_DEFINE([USE_MENUS], [], [GUI Menu support])
fi
AM_CONDITIONAL(MENUS, test x$menus = xyes)

dnl Enable building the gui support even when statically
dnl linking. Normally this is only disabled when building a statically
dnl linked gnash executable.
AC_ARG_ENABLE(dynamic-gui,
  AC_HELP_STRING([--enable-dynamic-gui],[Enable building dynamically loadable GUIs and renderers ]),
[case "${enableval}" in
  yes) dynamic_gui=yes ;;
  no)  dynamic_gui=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for --enable-dynamic-gui option]) ;;
esac],dynamic_gui=no)
if test x"${dynamic_gui}" = x"yes"; then
  AC_DEFINE([USE_DYNAMIC_GUI], [], [Dynamically loadable GUI and renderer support])
fi
AM_CONDITIONAL(DYNAMIC_GUI, test x${dynamic_gui} = xyes)

dnl Write the file to disk in the plugin, if specified.
AC_ARG_ENABLE(write,
  AC_HELP_STRING([--enable-write], [Makes the Mozilla plugin write the currently playing SWF movie to /tmp.]),
[case "${enableval}" in
  yes) write=yes ;;
  no)  write=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for --enable-write option]) ;;
esac],write=no)

if test x"$write" = x"yes"; then
  AC_DEFINE([WRITE_FILE], [], [Write files while streaming])
fi

dnl Don't build the cygnal server if specified.
AC_ARG_ENABLE(cygnal,
  AC_HELP_STRING([--disable-cygnal], [Disable support for the Cygnal server]),
[case "${enableval}" in
  yes) cygnal=yes ;;
  no)  cygnal=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for disable-cygnal option]) ;;
esac],cygnal=no)

AM_CONDITIONAL(CYGNAL, test x$cygnal = xyes)

dnl Fix the Intel 810 LOD bias problem
AC_ARG_ENABLE(i810-lod-bias,
  AC_HELP_STRING([--enable-i810-lod-bias], [Enable fix for Intel 810 LOD bias problem]),
[case "${enableval}" in
  yes) i810lodbias=yes ;;
  no)  i810lodbias=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for enable-i810-lod-bias option]) ;;
esac])

if test x$i810lodbias = xyes; then
    AC_DEFINE([FIX_I810_LOD_BIAS], [], [Fix i810 LOD bias problem])
fi

dnl Install the headers to make Gnash an SDK
AC_ARG_ENABLE(sdkinstall,
  AC_HELP_STRING([--enable-sdk-install], [Enable installing the libraries and headers as an SDK]),
[case "${enableval}" in
  yes) sdkinstall=yes ;;
  no)  sdkinstall=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for enable-i810-lod-bias option]) ;;
esac], sdkinstall=no)

AM_CONDITIONAL(SDKINSTALL, test x$sdkinstall = xyes)

dnl Add KFLASH support, if specified.
AC_ARG_ENABLE(klash,
  AC_HELP_STRING([--disable-klash], [Disable support for Konqueror plugin]),
[case "${enableval}" in
  yes) klash=yes ;;
  no)  klash=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for --disable-klash option]) ;;
esac],klash=yes)

addgui=none
build_fb=no
build_fltk=no
build_kde=no
build_gtk=no
build_sdl=no
build_riscos=no
build_aqua=no
AC_ARG_ENABLE(gui,
  AC_HELP_STRING([--enable-gui=], [Use the specified GUI toolkit (default=gtk,kde)]),
  if test -n ${enableval}; then
    addgui="${enableval}"
    enableval=`echo ${enableval} | tr '\054' ' ' `
  fi
  while test -n "${enableval}" ; do
    val=`echo ${enableval} | cut -d ' ' -f 1`
    [case "${val}" in
      gtk|GTK|gtk2|GTK2)
        build_gtk=yes
        ;;
      kde|KDE)
        build_kde=yes
        ;;
      qt|QT)
        build_qt=yes
        ;;
      sdl|SDL)
        build_sdl=yes
        ;;
      aqua|AQUA|Aqua)
        build_aqua=yes
        ;;
      riscos|RISCOS|RiscOS)
        build_riscos=yes
        ;;
      fltk|FLTK|fltk2|FLTK2)
        build_fltk=yes
        ;;
      fb|FB)
        build_fb=yes
        ;;
      all|ALL)
        build_fb=yes
        build_fltk=yes
        build_kde=yes
        build_gtk=yes
        build_sdl=yes
dnl        build_riscos=yes
        ;;
      *) AC_MSG_ERROR([invalid gui ${enableval} given (accept: gtk|kde|fltk|sdl|riscos|aqua|fb)])
         ;;
      esac]
    enableval=`echo ${enableval} | cut -d ' ' -f 2-6`
    if test x$val = x$enableval; then
      break;
    fi
  done
)

if test x$addgui = xnone; then
   build_gtk=yes
   build_kde=yes
dnl   addgui="gtk,kde"
fi

add_renderer=
build_ogl=no
build_agg=no
build_cairo=no
AC_ARG_ENABLE(renderer,
  AC_HELP_STRING([--enable-renderer=], [Use the specified GUI toolkit (default=ogl,agg)]),
  if test -n ${enableval}; then
    add_renderer="${enableval}"
    enableval=`echo ${enableval} | tr '\054' ' ' `
  fi
  while test -n "${enableval}" ; do
    val=`echo ${enableval} | cut -d ' ' -f 1`
    [case "${val}" in
      ogl|OGL|gl|GL|opengl|OPENGL|OpenGL)
        build_ogl=yes
	add_renderer="ogl"
        ;;
      agg|AGG|Agg)
        build_agg=yes
	add_renderer="agg"
        ;;
      cairo|CAIRO|Cairo)
        build_cairo=yes
	add_renderer="cairo"
        ;;
      all|ALL|All)
        build_ogl=yes
        build_agg=yes
        build_cairo=yes
	add_renderer="ogl,agg,cairo"
        ;;
      *) AC_MSG_ERROR([invalid renderer ${enableval} given (accept: ogl,agg,cairo)])
         ;;
      esac]
    enableval=`echo ${enableval} | cut -d ' ' -f 2-6`
    if test x$val = x$enableval; then
      break;
    fi
  done
)

if test x$add_renderer = x; then
   build_ogl=yes
   build_agg=yes
dnl   add_renderer="ogl,agg"
fi

if test x$build_ogl = xyes; then
  if test x$build_fb = xyes -o x$build_fltk = xyes; then
    AC_MSG_ERROR([OpenGL renderer is not supported by the this gui. Try using AGG ( --enable-renderer=AGG ).]);
  fi
  AC_DEFINE([RENDERER_OPENGL], [], [Use OpenGL renderer])
fi

dnl 16 bit: RGB555, RGB565
dnl 24 bit: RGB24, BGR24
dnl 32 bit: RGBA32, BGRA32
pixelformat=all
AC_ARG_WITH(pixelformat,
  AC_HELP_STRING([--with-pixelformat=], [Use the specified pixel format for AGG (default=all)]),
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
        AC_DEFINE(PIXELFORMAT_BGRA32, [1], [BGRA32])
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
  if test x$build_agg = xyes; then
    ### The fact that we're building GTK doesn't mean we're not also
    ### building KDE or SDL, each needing its own pixel format !
    #if test x$build_gtk = xyes; then
    #  AC_DEFINE(PIXELFORMAT_RGB24, [1], [RGB24 pixel format])
    #  pixelformat="RGB24"
    #else
      AC_DEFINE(PIXELFORMAT_RGB555, [1], [RGB555 pixel format])
      AC_DEFINE(PIXELFORMAT_RGB565, [1], [RGB565 pixel format])
      AC_DEFINE(PIXELFORMAT_RGB24,  [1], [RGB24 pixel format])
      AC_DEFINE(PIXELFORMAT_BGR24,  [1], [BGR24 pixel format])
      AC_DEFINE(PIXELFORMAT_RGBA32, [1], [RGBA32 pixel format])
      AC_DEFINE(PIXELFORMAT_BGRA32, [1], [BGRA32 pixel format])
    #fi
  fi
fi

dnl --------------------------------------------------------
dnl  Select media handler
dnl --------------------------------------------------------

media_handler_specified=false
AC_ARG_ENABLE(media,
 AC_HELP_STRING([--enable-media=handler], [Enable media handling support using the specified handler: ffmpeg, mad, GST (Gstreamer) or none (no sound) [[ffmpeg]] ]),
 [case "${enableval}" in
   GST|gst)
     media_handler=gst
     media_handler_specified=true
     ;;
   ffmpeg|FFMPEG)
     media_handler=ffmpeg 
     media_handler_specified=true
     ;;
   mad|MAD)
     media_handler=mad 
     media_handler_specified=true
     ;;
   no|NO|none)
     media_handler=none
     media_handler_specified=true
     ;;
   *) AC_MSG_ERROR([bad value ${enableval} for --enable-media option]) ;;
  esac],
 [media_handler=ffmpeg]
)

dnl --------------------------------------------------------
dnl Figure out which extensions to build.
dnl --------------------------------------------------------

extensions_list=
ext_dejagnu=no
ext_mysql=no
ext_fileio=no
ext_gtk=no
ext_all=no
AC_ARG_WITH(extensions,
  AC_HELP_STRING([--with-extensions=], [Specify which extensions to build]),
  if test -n ${withval}; then
    if test "x${withval}" != "xno"; then
      extlist="${withval}"
      withval=`echo ${withval} | tr '\054' ' ' `
    else
      extlist=""
      withval=""
    fi
  fi
  nextensions=0
  while test -n "${withval}" ; do
    val=`echo ${withval} | cut -d ' ' -f 1`
    extensions_list="${extensions_list} ${val}"
    [case "${val}" in
      dejagnu|DEJAGNU|dj|DJ)
        AC_DEFINE(USE_DEJAGNU_EXT, [1], [Build the DejaGnu extension])
        AC_MSG_NOTICE([Adding DejaGnu extension])
        ext_dejagnu=yes
        nextensions=$((nextensions+1))
        ;;
      mysql|MYSQL|sql|SQL)
        AC_DEFINE(USE_MYSQL_EXT, [1], [Build the MySQL extension])
        AC_MSG_NOTICE([Adding MySql extension])
        ext_mysql=yes
        nextensions=$((nextensions+1))
        ;;
      fileio|FILEIO|io|IO)
        AC_DEFINE(USE_FILEIO_EXT, [1], [Build the FileIO extension])
        AC_MSG_NOTICE([Adding FileIO extension])
        ext_fileio=yes
        nextensions=$((nextensions+1))
        ;;
      gtk|GTK|gtk2|GTK2)
        AC_DEFINE(USE_GTK_EXT, [1], [Build the GTK extension])
        ext_gtk=yes
        nextensions=$((nextensions+1))
        ;;
      all|ALL)
        AC_DEFINE(USE_GTK_EXT, [1], [Build all the extensions])
        ext_dejagnu=yes
        ext_mysql=yes
        ext_fileio=yes
        ext_gtk=yes
        ext_all=yes
        nextensions=5
        ;;
      *) AC_MSG_ERROR([invalid extension specified: ${withval} given (accept: MYSQL|DEJAGNU|FILEIO|GTK|ALL)])
         ;;
      esac]
    withval=`echo ${withval} | cut -d ' ' -f 2-6`
    if test x$val = x$withval; then
      break;
    fi
  done
  if test "${nextensions}" -gt 0; then
        AC_DEFINE(USE_EXTENSIONS, [1], [Use extensions])
  fi
  EXTENSIONS_LIST="$extensions_list"
  AC_SUBST(EXTENSIONS_LIST)
)

AM_CONDITIONAL(BUILD_DEJAGNU_EXT, [ test x$ext_dejagnu = xyes ])
AM_CONDITIONAL(BUILD_FILEIO_EXT, [ test x$ext_fileio = xyes ])
AM_CONDITIONAL(BUILD_MYSQL_EXT, [ test x$ext_mysql = xyes ])
AM_CONDITIONAL(BUILD_GTK_EXT, [ test x$ext_gtk = xyes ])
if test x$ext_mysql = xyes; then
  GNASH_PATH_MYSQL
fi

dnl --------------------------------------------------------
dnl Figure out development tool stuff
dnl --------------------------------------------------------

AC_PROG_CC
AC_PROG_CXX
AC_EXEEXT
AC_PROG_INSTALL
AM_COMPILER_LIB

AC_LIBLTDL_INSTALLABLE
AC_LIBTOOL_DLOPEN
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
AC_C_CONST
AC_C_INLINE
AC_SUBST([LIBTOOL_DEPS])
AC_LIB_LTDL

AM_CONDITIONAL(ENABLE_SHARED, [test x"${enable_shared}" = xyes])
AM_CONDITIONAL(ENABLE_STATIC, [test x"${enable_shared}" = xno])
AM_CONDITIONAL(STATIC_GUI, test x"${dynamic_gui}" = xno -o x"${enable_shared}" = xno)

dnl build a statically linked executable
dnl We don't have anything special to do in this case
if false; then # test x"${enable_shared}" = xno; then

   g=`echo  $addgui | grep -c '\,'`
   r=`echo  $add_renderer | grep -c '\,'`
  if test $g -gt 0 -o $r -gt 0 ; then
     AC_MSG_ERROR([When statically linking, you must pick only one GUI toolkit and one renderer])
  fi
  dnl If statically linking, and no gui is specified, force gtk
  if test x$addgui = xnone; then
     build_gtk=yes
     build_kde=no
     addgui=gtk
     AC_MSG_NOTICE([Defaulting to GTK as the GUI, use --enable-gui= to change])
  fi
  dnl If statically linking, and no renderer is specified, force opengl
  if test x$add_renderer = x; then
     build_agg=no
     build_ogl=yes
     add_renderer=ogl
     AC_MSG_NOTICE([Defaulting to OpenGL as the renderer, use --enable-renderer= to change])
  fi

else dnl building a dynamically linked executable without loadable GUI plugins

  dnl If not dynamic loading, and no gui is specified, force gtk
  if test x"${dynamic_gui}" = xno; then
    if test x$addgui = xnone; then
      build_gtk=yes
      build_kde=no
      addgui=gtk
      AC_MSG_NOTICE([Defaulting to GTK as the GUI, use --enable-gui= to change])
    fi
    dnl If not dynamic loading, and no renderer is specified, force opengl
    if test x$add_renderer = x; then
      build_agg=no
      build_ogl=yes
      add_renderer=ogl
      AC_MSG_NOTICE([Defaulting to OpenGL as the renderer, use --enable-renderer= to change])
    fi
  else
    dnl If dynamically loading and no gui is specified, force gtk and kde
    if test x$addgui = xnone; then
      build_gtk=yes
      build_kde=yes
      addgui="gtk,kde"
      AC_MSG_NOTICE([Defaulting to GTK and KDE as the GUIs, use --enable-gui= to change])
    fi
    dnl If dynamically loading and no renderer is specified, force opengl and agg
    if test x$add_renderer = x; then
      build_ogl=yes
      build_agg=yes
      add_renderer="ogl,agg"
      AC_MSG_NOTICE([Defaulting to OpenGL and AGG as the renderers, use --enable-renderer= to change])
    fi
  fi

fi

AM_CONDITIONAL(BUILD_OGL_RENDERER, [ test x$build_ogl = xyes ])
AM_CONDITIONAL(BUILD_AGG_RENDERER, [ test x$build_agg = xyes ])
AM_CONDITIONAL(BUILD_CAIRO_RENDERER, [ test x$build_cairo = xyes ])

dnl AC_CHECK_LIB(ltdl, lt_dlmutex_register, AC_DEFINE(LT_DLMUTEX, 1, [Has lt_dlmutex_register]),
dnl                                         AC_DEFINE(LT_DLMUTEX, 0, [doesn't have lt_dlmutex_register]) )

if test x$build_agg = xyes; then
  AC_DEFINE([RENDERER_AGG], [], [Use AntiGrain renderer])
fi

DLOPEN="-dlopen"
DLPREOPEN="-dlpreopen"
AC_SUBST(DLOPEN)
AC_SUBST(DLPREOPEN)

dnl dnl Substitute INCLTDL and LIBLTDL in the Makefiles
AC_SUBST(INCLTDL)
AC_SUBST(LIBLTDL)
AC_SUBST(LIBTOOL_DEPS)

dnl Darwin uses libtool instead of ar to produce libraries. We determine which one
dnl we have here now. For some reason on Darwin, we don't get the
dnl count from grep via stdin correctly. Writing a temp file does the
dnl trick, so although it's ugly, that's what we gotta do...
AC_MSG_CHECKING([which type of library archiver we have])
rm -f .tmp
libtool > .tmp 2>&1
archiver=`grep -c dynamic .tmp 2>&1`
rm -f .tmp
dnl is there any good way to report what we found here?
AC_MSG_RESULT()
if test $archiver -eq 1; then
dnl   if test "$enable_shared" = no; then
dnl     AR="libtool -static"
dnl   else
dnl     AR="libtool -dynamic"
dnl     CXXFLAGS="${CXXFLAGS} -dynamic"
   export MACOSX_DEPLOYMENT_TARGET="10.3"
dnl   fi
dnl   AR_FLAGS="-o"
   darwin=yes
else
   darwin=no
fi

dnl When cross compiling, limit the search directories cause otherwise
dnl we may get the host headers or libraries by accident. These values
dnl are exported, so all the other configure tests in macros/*.m4 use
dnl these same settings rather than duplicating them like we used to.
dnl To override thise, use the --with-*-incl= and --with-*-libs=
dnl options to configure.
if test x$cross_compiling = xyes; then
  AC_MSG_NOTICE([Configuring Gnash for cross compilation])
  export pkgroot="`$CXX -print-search-dirs | grep "install:" | sed -e 's/install: //' -e 's:/lib/gcc/.*::'`"
  export incllist="`eval echo ${with_top_level}/include ${pkgroot}/${host_alias}/include ${pkgroot}/include`"
  export libslist="`eval echo ${with_top_level}/lib ${pkgroot}/${host_alias}/lib ${pkgroot}/lib64 ${pkgroot}/lib32 ${pkgroot}/lib`"
  export pathlist="`eval echo ${pkgroot}/${host_alias}/bin:${pkgroot}/bin`"
  plugin=no
else
  AC_MSG_NOTICE([Configuring Gnash for native compilation])
  export incllist="`eval cat ${srcdir}/macros/incllist`"
  export libslist="`eval cat ${srcdir}/macros/libslist`"
  export pathlist=$PATH
fi

dnl Try to ignore stupid dependencies

AC_MSG_CHECKING(linker --as-needed support)
gcc_cv_ld_as_needed=no
# Check if linker supports --as-needed and --no-as-needed options
if $LD --help 2>/dev/null | grep as-needed > /dev/null; then
	gcc_cv_ld_as_needed=yes
fi
if test x"$gcc_cv_ld_as_needed" = xyes; then
	LDFLAGS="${LDFLAGS} -Wl,--as-needed"
fi
AC_MSG_RESULT($gcc_cv_ld_as_needed)

dnl GNASH_PKG_FIND(dmalloc, [dmalloc.h], [dmalloc], mallinfo)
AM_CONDITIONAL(HAVE_DMALLOC, [ test x$has_dmalloc = xyes ])

AC_CHECK_FUNCS(mallinfo)
PKG_PROG_PKG_CONFIG
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

AC_CHECK_HEADERS(malloc.h)
AC_CHECK_HEADERS(getopt.h)
AC_CHECK_HEADERS(libgen.h)
AC_CHECK_LIB(m, sqrt)

dnl don't look for X11 when using a raw framebuffer for the GUI
if test x$build_gtk = xyes -o x$build_kde = xyes -o x$build_fltk = xyes -o x$build_sdl = xyes; then
dnl AC_PATH_XTRA
dnl AC_CHECK_LIB(Xmu, XmuCvtStringToOrientation)
dnl AC_CHECK_LIB(gmp, _gmp_get_memory_functions)
  GNASH_PATH_X11
  AC_CHECK_LIB(Xi, XInput_find_display)
  AC_CHECK_LIB(X11, XDisableAccessControl)
fi
AM_CONDITIONAL(HAVE_X11, [test x$x11 = xyes])

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
  AC_TRY_LINK([#include <cstdio>
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
  AC_TRY_LINK([#include <cstdio>
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
  AC_TRY_LINK([#include <cstdio>
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

dnl Date portability stuff, used in server/asobj/Date.cpp
AC_CHECK_FUNCS(gettimeofday)
AC_CHECK_FUNCS(ftime)
AC_CHECK_FUNCS(tzset)
AC_CHECK_FUNCS(localtime_r)

AC_CACHE_CHECK([whether struct tm has tm_gmtoff], ac_cv_tm_gmtoff, [
        AC_TRY_LINK([
/* ctime(1) says "The glibc version of struct tm has additional fields
 * defined  when _BSD_SOURCE was set before including <time.h>"
 * In practice, you don't need to define it yourself (tested on glibc-2.2.1 
 * and 2.3.6) but if you *do* define it yourself, it makes *all* functions
 * favour BSD-like behaviour over of GNU/POSIX, which seems dangerous.
 */
// #define _BSD_SOURCE 1
#include <time.h>
], 
                [ struct tm tm; long l = tm.tm_gmtoff; ], 
                [ ac_cv_tm_gmtoff="yes" ],
                [ ac_cv_tm_gmtoff="no" ]
        )
])
if test "x$ac_cv_tm_gmtoff" = "xyes" ; then
        AC_DEFINE(HAVE_TM_GMTOFF, [1], [struct tm has member tm_gmtoff])
fi

AC_CACHE_CHECK([whether timezone is a long], ac_cv_long_timezone, [
        AC_TRY_LINK([
/* On Linux/glibc, tzset(3) says "extern long timezone;" (seconds West of GMT)
 * but on BSD char *timezone(int,int) is a function returning a string name.
 * The BSD function timegm() may be the equivalent, but this should
 * not be necessary because on BSD the code should use tm.tm_gmtoff instead
 * (use of long timezone is a fallback strategy for when tm_gmtoff exists not).
 */
#include <stdio.h>
#include <time.h>
extern long timezone;
  ], 
    [ printf("%ld", timezone); ], 
    [ ac_cv_long_timezone="yes" ],
    [ ac_cv_long_timezone="no" ]
   )
])
if test "x$ac_cv_long_timezone" = "xyes" ; then
  AC_DEFINE(HAVE_LONG_TIMEZONE, [1], [extern timezone is a long integer, not a function])
fi

AC_CHECK_FUNCS(sysconf)
AC_CHECK_FUNCS(shmget shmat shmdt mmap)
AC_CHECK_FUNCS(memmove)
AC_CHECK_FUNCS(strlcpy, AC_DEFINE(HAVE_STRLCPY_PROTO, [1],[Define if you have the strlcpy prototype]))
AC_CHECK_FUNCS(strlcat, AC_DEFINE(HAVE_STRLCAT_PROTO, [1],[Define if you have the strlcat prototype]))
dnl Look for Win32 networking stuff
AC_CHECK_HEADERS(winsock.h)
AC_CHECK_HEADERS(winsock2.h)
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
GNASH_PKG_INCLUDES([dejagnu], [dejagnu.h])

dnl Find freetype and fontconfig
GNASH_PKG_FIND(freetype2, [freetype/freetype.h], [freetype2 font library], FT_Load_Char)
GNASH_PKG_FIND(fontconfig, [fontconfig/fontconfig.h], [fontconfig library], FcFontMatch)

if test x$cross_compiling = xno; then
  AC_PATH_MING
fi
AM_CONDITIONAL(ENABLE_MING, [ test x"$MAKESWF" != x ])
AM_CONDITIONAL(MAKESWF_SUPPORTS_PREBUILT_CLIPS,
        [ test x"$MAKESWF" != x && test $MING_VERSION_CODE -ge 00040002  ])
AM_CONDITIONAL(MING_SUPPORTS_INIT_ACTIONS,
        [ test x"$MAKESWF" != x && test $MING_VERSION_CODE -ge 00040004  ])
AM_CONDITIONAL(MING_SUPPORTS_REPLACE_TAG,
        [ test x"$MAKESWF" != x && test $MING_VERSION_CODE -ge 00040005 ])

if test x$cross_compiling = xno; then
  AC_ARG_WITH([swfdec_testsuite],
	  AC_HELP_STRING([--with-swfdec-testsuite],
		  [directory where swfdec testsuite (the 'test' dir) is]),
	SWFDEC_TESTSUITE=${withval})
  AC_SUBST(SWFDEC_TESTSUITE)
fi
AM_CONDITIONAL(ENABLE_SWFDEC_TESTSUITE, [ test x"$SWFDEC_TESTSUITE" != x ])

dnl
dnl See if we can use the swfmill and mtasc based testsuites 
dnl
if test x$cross_compiling = xno; then
  AC_PATH_PROG(SWFMILL, swfmill)
  AC_PATH_MTASC
fi
AM_CONDITIONAL(ENABLE_SWFMILL, [ test x"$SWFMILL" != x ])
AM_CONDITIONAL(ENABLE_MTASC, [ test x"$MTASC" != x ])

AC_PATH_PROG(DOXYGEN, doxygen)
AM_CONDITIONAL(ENABLE_DOXYGEN, [ test x"$DOXYGEN" != x ])

dnl currently unused
dnl GNASH_PKG_FIND(ogg, [ogg.h], [decode ogg streams], ogg_stream_init)

if test x$build_sdl = xyes -o x"$media_handler" = xffmpeg -o x"$media_handler" = xmad; then
  GNASH_PATH_SDL
fi

AM_CONDITIONAL(HAVE_SDL, [ test x$has_SDL = xyes ])

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
if test x$build_kde = xyes -o x$klash = xyes -o x$plugin = xno; then
  kde=yes
  qt=yes
  build_kde=yes
  addgui="${addgui},kde"
  GNASH_PATH_KDE
  GNASH_PATH_QT
fi
AM_CONDITIONAL(HAVE_KDE, [test x$has_kde = xyes])
AM_CONDITIONAL(HAVE_QT, [test x$has_qt = xyes])

AM_CONDITIONAL(BUILD_GTK_GUI, [ test x$build_gtk = xyes ])
AM_CONDITIONAL(BUILD_KDE_GUI, [ test x$build_kde = xyes ])
dnl AM_CONDITIONAL(BUILD_QT_GUI, [ test x$build_qt = xyes ])
AM_CONDITIONAL(BUILD_FLTK_GUI, [ test x$build_fltk = xyes ])
AM_CONDITIONAL(BUILD_SDL_GUI, [ test x$build_sdl = xyes ])
AM_CONDITIONAL(BUILD_FB_GUI, [ test x$build_fb = xyes ])
AM_CONDITIONAL(BUILD_AQUA_GUI, [ test x$build_aqua = xyes ])
AM_CONDITIONAL(BUILD_RISCOS_GUI, [ test x$build_riscos = xyes ])


dnl # NO kde ? NO klash !
if test x$has_kde != xyes; then
  if test x$klash = xyes; then
    echo "WARNING: No KDE development package installed, disabling klash"
    klash=no
  fi
fi
if test x$has_qt != xyes; then
  if test x$klash = xyes; then
    echo "WARNING: No QT development package installed, disabling klash"
    klash=no
  fi
fi

AM_CONDITIONAL(KLASH, [test x$klash = xyes])

AC_SUBST(KDE_CFLAGS)
AC_SUBST(KDE_LIBS)
if test x$build_ogl = xyes; then
  GNASH_PATH_OPENGL
fi

dnl Need GLIB for both GTK and GST
if test x$build_gtk = xyes -o x${media_handler} = "xgst"; then
  GNASH_PATH_GLIB
fi

AM_CONDITIONAL(HAVE_GLIB, [ test x$has_glib = xyes ])

if test x$build_ogl = xyes -a x$build_gtk = xyes; then
   GNASH_PATH_GLEXT
fi
if test x$build_gtk = xyes; then
   GNASH_PATH_GTK2
   GNASH_PATH_PANGO
   GNASH_PKG_FIND(atk, [atk/atk.h], [atk library], atk_focus_tracker_init, [1.0])
fi

if test x$build_gtk = xyes; then
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

if test x$build_fltk = xyes; then
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
if test x$glext = xno -a x$build_ogl = xyes; then
  if test x$gtk2 = xyes -a x$build_gtk = xyes; then
    AC_ERROR([You have GTK installed, but not GtkGLExt. You need GtkGLExt to use the OpenGL renderer. Attempting to build SDL version])
  fi
  gui=sdl
  plugin=no
  AC_MSG_WARN([GTK2 specified for the GUI, but GtkGlExt is not present. Trying SDL instead.])
fi
AM_CONDITIONAL(PLUGIN, [test x$plugin = xyes])

if test x"$media_handler" = x"ffmpeg"; then
  GNASH_PATH_FFMPEG
  if test x"${media_handler_specified}" = xfalse; then
     # If the library is not found, or its version is not ok, we'll try gst
     if test x"${ac_cv_path_ffmpeg_lib}" = x -o x"${ffmpeg_version}" != xok; then
       AC_MSG_WARN([No appropriate ffmpeg library found, will try using gstreamer.])
       media_handler=gst
     fi
  fi
fi

if test "$media_handler" = "gst"; then
  GNASH_PKG_FIND(gstreamer, [gst/gst.h], [gstreamer library], gst_init, [0.10])
  if test x"${media_handler_specified}" = xfalse; then
    # If the library is not found, or its version is not ok, we'll try mad
    if test x"$GSTREAMER_LIBS" = x; then
       AC_MSG_WARN([No appropriate gstreamer library found, will try using mad.])
       media_handler=mad
    fi
  else
    AC_MSG_CHECKING([for modern pbutils])
    save_cflags=$CFLAGS
    CFLAGS=$GSTREAMER_CFLAGS
    AC_TRY_COMPILE([#include <gst/pbutils/install-plugins.h>],[void* pointer = &gst_install_plugins_sync],
      GSTREAMER_LIBS="$GSTREAMER_LIBS -lgstpbutils-0.10",
      has_modern_gstpbutils='no')]
      CFLAGS=$save_cflags
    if test x$has_modern_gstpbutils = xno; then
      AC_MSG_RESULT([no {GSTREAMER_CFLAGS=$GSTREAMER_CFLAGS}])
    else
      AC_DEFINE([GST_HAS_MODERN_PBUTILS], [], [GStreamer has pbutils with missing codec install routines installed])
      AC_MSG_RESULT([ok])
    fi
  fi
fi

if test x"$media_handler" = x"mad"; then
  dnl Will set MAD_LIBS and MAD_CFLAGS.
  dnl Handles --with-mad-incl and --with-mad-lib
  GNASH_PKG_FIND(mad, [mad.h], [mad library], mad_copyright)
  if test x"${media_handler_specified}" = xfalse; then
    # If the library is not found, we'll disable media handling
    if test x"${ac_cv_path_mad_lib}" = x ; then
      AC_MSG_WARN([No libmad found, disabling media handling.])
      media_handler=none
    fi
  fi
fi

if test x$build_cairo = xyes; then
  GNASH_PKG_FIND(cairo, [cairo.h], [cairo render library], cairo_status)
dnl   if test x"${CAIRO_CFLAGS}" != x; then
dnl      CAIRO_CFLAGS=`echo ${CAIRO_CFLAGS} | sed -e 's/ *//'`
dnl      CAIRO_CFLAGS="${CAIRO_CFLAGS} ${CAIRO_CFLAGS}/cairo"
dnl   fi
  if test x$build_fb = xyes; then
    AC_MSG_ERROR([cairo renderer is not supported by fb gui]);
  fi
  AC_DEFINE([RENDERER_CAIRO], [], [Use cairo renderer])
fi

AM_CONDITIONAL(HAVE_CAIRO, [test x"${CAIRO_LIBS}" != x])

dnl I'm kinda lazy, get rid of this later... //Markus
AM_CONDITIONAL(HAVE_MAD, test x"$media_handler" = xmad)

AM_CONDITIONAL(USE_SOUND_GST, test x$media_handler = xgst)
AM_CONDITIONAL(USE_SOUND_SDL, test x"$media_handler" = xmad -o x"$media_handler" = xffmpeg)
AM_CONDITIONAL(USE_FFMPEG_ENGINE, test x"$media_handler" = xffmpeg)
AM_CONDITIONAL(USE_MAD_ENGINE, test x"$media_handler" = xmad)
AM_CONDITIONAL(HAVE_OPENGL, test x"${OPENGL_LIBS}" != x)

dnl AM_CONDITIONAL(HAVE_FFMPEG, [test x"${FFMPEG_LIBS}" != x])

case "${media_handler}" in
  gst) AC_DEFINE([SOUND_GST],  [1], [Use GSTREAMER for media handling]) ;;
  ffmpeg) AC_DEFINE([SOUND_SDL],  [1], [Use SDL for sound handing]) ;;
  mad) AC_DEFINE([SOUND_SDL],  [1], [Use SDL for sound handling]) ;;
  *)
esac

case "${media_handler}" in
  ffmpeg)  AC_DEFINE([USE_FFMPEG],  [1], [Use FFMPEG for media decoding]) ;;
  mad)  AC_DEFINE([USE_MAD],  [1], [Use MAD for media decoding]) ;;
  *)
esac

dnl I'm kinda lazy, get rid of this later... //Markus
AM_CONDITIONAL(HAVE_GST, test x$media_handler = xgst)

if test x$build_fltk = xyes; then
  GNASH_PKG_FIND(fltk2, [fltk/FL_API.h], [Fast Light Toolkit], fl_window_flush)
fi

AM_CONDITIONAL(HAVE_FLTK2, [ test x$has_fltk2 = xyes ])
AM_CONDITIONAL(HAS_XFT, [ test x$has_xft = xyes ])

if test x$build_agg = xyes; then
   GNASH_PATH_AGG
fi

GNASH_DOCBOOK
AM_CONDITIONAL(DOCBOOK, test x$docbook = xyes)
AM_CONDITIONAL(HAVE_AGG, [test x"${AGG_LIBS}" != x])

GNASH_PATH_CURL

dnl Define winsock if we're on windows. We could do something complicated,
dnl but since AC_EXEEXT does it for us, we'll do this the easy way.
if test x"$EXEEXT" = "exe"; then
  AC_DEFINE(HAVE_WINSOCK,1,[This is defined is we are on Win32])
fi

dnl ****************************************
dnl *** Check for ELF visibility support ***
dnl ****************************************

AC_ARG_ENABLE([visibility],
  AC_HELP_STRING([--enable-visibility], [Use ELF visibility attributes]), [], [enable_visibility=no])

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

AC_ARG_ENABLE([strict],
  AC_HELP_STRING([--enable-strict],[Accept only standards compliant code (GCC only)]),
  [case "${enableval}" in
    yes) strict=yes ;;
    no) strict=no ;;
    *) AC_MSG_ERROR([bad value ${enableval} for --enable-strict option]) ;;
  esac],
  [strict=no]
)

if test x"$strict" = x"yes" -a x$build_agg = xyes; then
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
RENDERER_CONFIG=${add_renderer}
RENDERER_LIBS=
for rend in `echo "${add_renderer}" | tr ',' ' '`; do
	RENDERER_LIBS="${RENDERER_LIBS} \$(top_builddir)/backend/libgnash${rend}.la"
done
AC_SUBST(RENDERER_LIBS)
AC_SUBST(RENDERER_CONFIG)
GUI_CONFIG=${addgui}
AC_SUBST(GUI_CONFIG)
DEFAULT_GUI=`echo "${GUI_CONFIG}" | cut -d',' -f1`
AC_SUBST(DEFAULT_GUI)
MEDIA_CONFIG=${media_handler}
AC_SUBST(MEDIA_CONFIG)

dnl AC_CONFIG_LINKS(doc/C/images)
AC_CONFIG_LINKS(testsuite/libbase/gnashrc:testsuite/libbase/gnashrc.in)

AC_OUTPUT(Makefile
po/Makefile
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
doc/C/asspec/Makefile
doc/Doxyfile
testsuite/Makefile
testsuite/libbase/Makefile
testsuite/libgeometry/Makefile
testsuite/actionscript.all/Makefile
testsuite/samples/Makefile
testsuite/swfdec/Makefile
testsuite/misc-ming.all/Makefile
testsuite/misc-mtasc.all/Makefile
testsuite/misc-swfmill.all/Makefile
testsuite/movies.all/Makefile
testsuite/server/Makefile
testsuite/libamf.all/Makefile
gui/Makefile
extensions/Makefile
extensions/dejagnu/Makefile
extensions/mysql/Makefile
extensions/fileio/Makefile
extensions/gtk2/Makefile
plugin/Makefile
plugin/klash/Makefile
cygnal/Makefile
cygnal/ACT/Makefile
cygnal/IO/Makefile
cygnal/HTTP/Makefile
cygnal/Net/Makefile
cygnal/unit_tests/Makefile
)

###
########################## Final report begins... ############################
###

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

if test x"${dynamic_gui}" = x"yes"; then
  echo "        Loadable GUI & renderer enabled."
else
  echo "        Loadable GUI & renderer disabled (default). Use --enable-dynamic-gui to enable."
fi

if test x"$ghelp" = x"yes"; then
  echo "        GNOME help enabled"
else
  echo "        GNOME help disabled (default). Use --enable-ghelp to enable."
fi

dnl if test x"$extensions" = x"yes"; then
dnl   echo "        Building extensions enabled (default). Use --disable-extensions to disable."
dnl else
dnl   echo "        Building Gnash extensions disabled."
dnl fi

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

if test x"$with-top_level" != x; then
  echo "        Top level for cross compiling support files is: $with_top_level"
fi

if test x$build_gtk = xyes -a x$pixelformat = xrgb565; then
  echo "        WARNING: Pixel format RGB565 selected in combination with the"
  echo "                 GTK GUI. Only a hacked GTK will work (e.g. on the OLPC)."
fi

if test x"$extensions_list" != x; then
  echo "        Building extensions: ${extensions_list}"
fi
echo "        GUI toolkits supported: ${GUI_CONFIG} (default gui: ${DEFAULT_GUI})"
echo "        Renderers supported: ${add_renderer}"
echo "        Media handler: "$media_handler
if test x"$media_handler" = xffmpeg -o x"$media_handler" = xmad; then
echo "        Using SDL for sound handling"
fi

echo ""
echo "Configured paths for ${build} are:"

if test x"$docbook" = x"yes"; then
  echo '        DocBook document processing enabled (for "make html" and "make pdf")'
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
    echo "               To compile this project install"
    echo "               or .deb users: apt-get install libxml2-dev"
    echo "               or .rpm users: yum install libxml2-devel"
    nogo=true
  fi
fi

if test x$kde = xyes -o x$klash = xyes; then
  if test x"$has_kde" = xyes; then
    echo "        KDE flags are: $KDE_CFLAGS"
    echo "        KDE libs are: $KDE_LIBS"
  else
    if test x$build_kde = xyes -o x$klash = xyes ; then
      echo "        ERROR: No KDE development package installed!"
      echo "               Reconfigure with --disable-klash and/or"
      echo "               change to a different gui with --enable-gui=..."
      echo "               or install the KDE development environment from http://kde.org"
      echo "               or .deb users: apt-get install kdelibs-dev"
      echo "               or .rpm users: yum install <something-else>."
      nogo=true
    fi
  fi
  if test x"$has_qt" = xyes; then
    echo "        QT flags are: $QT_CFLAGS"
    echo "        QT libs are: $QT_LIBS"
  else
    if test x$build_kde = xyes -o x$klash = xyes ; then
      if test x"$has_kde" != xyes; then
	# We have already moaned about KDE absence, so presumably they'll get
	# QT-dev when they install KDE-dev
	:
      else
	# This should never happen unless they have a very odd version of KDE
        echo "        ERROR: No QT development package installed!"
        echo "               Install a QT development environment from http://trolltech.com"
        echo "               or .deb users: apt-get install libqt3-mt-dev"
	if test x$klash = xyes; then
          echo "               or reconfigure with --disable-klash"
	fi
	if test x$build_kde = xyes; then
          echo "               or change to a different gui with --enable-gui=..."
	fi
      fi
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
  echo "               Install it from http://ijg.org"
  echo "               or .deb users: apt-get install libjpeg-dev"
  echo "               or .rpm users: yum install libjpeg-devel"
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
dnl   echo "        ERROR: No Ogg Vorbis development package installed!"
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
dnl   echo "        ERROR: No VORBIS Vorbis development package installed!"
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
      echo "        ERROR: No OpenGL development package installed!"
      echo "               You need to install the libmesa development package"
      echo "               or .deb users: apt-get install libgl1-mesa-dev"
      echo "               or .rpm users: yum install xorg-x11-Mesa-libGL"
      echo "               or use a different renderer with --enable-renderer="
    nogo=true
  fi
fi

dnl GLEXT is only needed for GTK/OpenGL
if test x$build_gtk = xyes -a x$build_ogl = xyes ; then
  if test x"$GLEXT_LIBS" != x; then
    if test x"$GLEXT_CFLAGS" != x; then
      echo "        GtkGLExt flags are: $GLEXT_CFLAGS"
    else
      echo "        GtkGLExt flags are: default include path"
    fi
      echo "        GtkGLExt libs are: $GLEXT_LIBS"
  else
    if test x$build_gtk = xyes; then
      if test x$build_ogl = xyes; then
        echo "        ERROR: No GtkGLExt development package installed!"
	echo "               It is needed to build the GTK/OpenGL GUI/renderer combination."
	echo "               Either install it from http://gtkglext.sourceforge.net"
        echo "               or .deb users: apt-get install libgtkglext1-dev"
        echo "               or .rpm users: yum install gtkglext-devel"
        echo "               or --enable-gui=sdl or --enable-renderer=agg"
      nogo=true
      fi
    fi
  fi
fi

if test x$build_gtk = xyes; then #{
  if test x"$GTK2_LIBS" != x; then
    if test x"$GTK2_CFLAGS" != x; then
      echo "        GTK2 flags are: $GTK2_CFLAGS"
    else
      echo "        GTK2 flags are: default include path"
    fi
      echo "        GTK2 libs are: $GTK2_LIBS"
  else
    echo "        ERROR: No GTK2 development package installed!"
    echo "               Install it from http://gtk.org"
    echo "               or .deb users: apt-get install libgtk2.0-dev"
    echo "               or .rpm users: yum install gtk2-devel"
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
    echo "        ERROR: No Pango development package installed!"
    echo "               Install it from http://pango.org"
    echo "               or .deb users: apt-get install libpango1.0-dev"
    echo "               or .rpm users: yum install pango-devel"
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
    echo "        ERROR: No GLib development package installed!"
    echo "               Install it from http://gtk.org"
    echo "               or .deb users: apt-get install libglib2.0-dev"
    echo "               or .rpm users: yum install glib2-devel"
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
    echo "        ERROR: No ATK development package installed!"
    echo "               Install it from http://gtk.org"
    echo "               or .deb users: apt-get install libatk1.0-dev"
    echo "               or .rpm users: yum install atk-devel"
dnl    nogo=true
  fi

fi

if test "$media_handler" = "gst"; then
  if test x"$GSTREAMER_LIBS" != x; then
    if test x"$GSTREAMER_CFLAGS" != x; then
      echo "        Gstreamer flags are: $GSTREAMER_CFLAGS"
    else
      echo "        Gstreamer flags are: default include path"
    fi
      echo "        Gstreamer libs are: $GSTREAMER_LIBS"
  else
    echo "        ERROR: GST media handling requested but gstreamer-0.10+ not found"
    echo "               Install it from http://www.gstreamer.net"
    echo "               or reconfigure using --enable-media=ffmpeg or =mad"
    echo "               or .deb users: apt-get install libgstreamer0.10-dev"
    echo "               or .rpm users: yum install gstreamer-devel"
    nogo=true
  fi
fi

  if test x"$media_handler" = x"mad"; then
    echo "        MP3 support enabled through libmad"
    if test x"$MAD_LIBS" != x; then
      if test x"$MAD_CFLAGS" != x; then
        echo "        MAD flags are: $MAD_CFLAGS"
      else
        echo "        MAD flags are: default include path"
      fi
      echo "        MAD libs are: $MAD_LIBS"
    else
      echo "        ERROR: No libmad development package installed!"
      echo "               You can install it from http://mad.sourceforge.net"
      echo "               or .deb users: apt-get install libmad0-dev"
      echo "               or .rpm users: yum install libmad"
      echo "               or reconfigure using --enable-media=ffmpeg"
      nogo=true
    fi
  fi

  if test x"$media_handler" = x"ffmpeg"; then
    if test x"$FFMPEG_LIBS" != x; then
      echo "        MP3 and video support enabled through ffmpeg"
      if test x"${ffmpeg_version}" != xok; then
        echo "        ERROR: You have version ${ffmpeg_version} of ffmpeg installed."
        echo "               This version of ffmpeg may not work!"
        echo "               Version 51.27.0 or newer is highly recommended."
        nogo=true
      else
	avformat_h=`echo $avcodec_h | sed 's/avcodec.h/avformat.h/'`
	if test ! -f "$avformat_h"; then
	  echo "        ERROR: FFMPEG's libavcodec header is installed but not libavformat."
          echo "               You can install FFMPEG from http://ffmpeg.mplayerhq.hu"
          echo "               or .deb users: apt-get install libavformat-dev"
          echo "               or reconfigure with --enable-media=mad"
	  nogo=true
	else
          if test x"$FFMPEG_CFLAGS" != x; then
            echo "        FFMPEG flags are: $FFMPEG_CFLAGS"
          else
            echo "        FFMPEG flags are: default include path"
          fi
          echo "        FFMPEG libs are: $FFMPEG_LIBS"
	fi
      fi
    else
      echo "        ERROR: No FFMPEG development package installed!"
      echo "               You can install FFMPEG from http://ffmpeg.mplayerhq.hu"
      echo "               or .deb users: apt-get install libavformat-dev"
      echo "               or .rpm users: yum install ffmpeg-devel"
      echo "               or reconfigure with --enable-media=mad"
      nogo=true
    fi
  fi

if test x$build_cairo = xyes; then
  if test x"$CAIRO_LIBS" != x; then
    if test x"$CAIRO_CFLAGS" != x; then
      echo "        Cairo flags are: $CAIRO_CFLAGS"
    else
      echo "        Cairo flags are: default include path"
    fi
    echo "        Cairo libs are: $CAIRO_LIBS"
  else
    echo "        ERROR: No Cairo development package installed!"
    echo "               You need to have the Cairo development package installed"
    echo "               if you have used --enable-render=cairo to configure."
    echo "               Install it from http://cairographics.org"
    echo "               or .deb users: apt-get install libcairo-dev"
    echo "               or .rpm users: yum install cairo-devel"
    nogo=true
  fi
fi

if test x$build_fltk = xyes; then
  if test x"$FLTK2_LIBS" != x; then
    if test x"$FLTK2_CFLAGS" != x; then
      echo "        FLTK flags are: $FLTK2_CFLAGS"
    else
      echo "        FLTK flags are: default include path"
    fi
      echo "        FLTK libs are: $FLTK2_LIBS"
  else
    echo "        ERROR: No FLTK2 development package installed!"
    echo "               There are currently no Debian or RPM packages for FLTK2;"
    echo "		 see http://www.fltk.org to install it from source."
dnl What it was for FLTK 1:
dnl    echo "               or .deb users: apt-get install fltk-1.1-dev"
dnl    echo "               or .rpm users: yum install fltk-devel"
    nogo=true
  fi
fi

if test x$build_fltk = xyes; then
  if test x"$XFT_LIBS" != x; then
    if test x"$XFT_CFLAGS" != x; then
      echo "        Xft flags are: $XFT_CFLAGS"
    else
      echo "        Xft flags are: default include path"
    fi
      echo "        Xft libs are: $XFT_LIBS"
dnl     else
dnl       echo "        ERROR: No Xft development package installed!"
dnl       echo "               Install the xft development package"
dnl       echo "               or .deb users: apt-get install libxft-dev"
dnl       echo "               or .rpm users: yum install xft-devel"
dnl    nogo=true
  fi
fi

# See whether SDL is required
need_sdl=false
test x$build_sdl = xyes			&& need_sdl=true
test x"$media_handler" = x"ffmpeg"	&& need_sdl=true
test x"$media_handler" = x"mad"		&& need_sdl=true

if $need_sdl; then
  if test x"$SDL_LIBS" != x; then
    echo "        SDL flags are: $SDL_CFLAGS"
    echo "        SDL libs are: $SDL_LIBS"
  else
    echo "        ERROR: No SDL development package installed!"
    echo "               Install it from http://www.libsdl.org/download-1.2.php"
    echo "               or .deb users: apt-get install libsdl1.2-dev"
    echo "               or .rpm users: yum install SDL-devel"
    [ x$build_sdl == xyes ] &&
	echo "               or select a different GUI with --enable-gui= "
    [ x"$media_handler" = x"ffmpeg" -o x"$media_handler" = x"mad" ] &&
	echo "               or use --enable-media=gst"
    nogo=true
  fi
fi
unset need_sdl

if test x"$plugin" = x"yes"; then
dnl  if test x"$FIREFOX_CFLAGS" != x; then
dnl    echo "        Firefox flags are: $FIREFOX_CFLAGS"
dnl    echo "        Firefox libs are: $FIREFOX_LIBS"
    echo "        Plugin will be installed in ${FIREFOX_PLUGINS}"
dnl  else
dnl    echo "        ERROR: No Firefox or Mozilla development package installed!"
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
    echo "        ERROR: No DocBook2X tools installed!"
    echo "               Either install it from http://docbook2x.sourceforge.net"
    echo "               or .deb users: apt-get install docbook docbook2x docbook-utils"
    echo "                              docbook-xml docbook-xsl texinfo xsltproc"
    echo "               or configure without --enable-docbook"
    nogo=true
  else
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
else
  echo "        WARNING: CURL library not found."
  echo "                 Gnash will be built without support for streaming from URLs."
  echo "                 Why not install libcurl from http://curl.haxx.se/libcurl"
  echo "                 or .deb users: apt-get install libcurl3-dev"
  echo "                 or .rpm users: yum install curl-devel"
fi

if test x$build_agg = xyes; then # {
  echo "        AGG Pixel format is: $pixelformat"
    if test x"$AGG_LIBS" != x; then # {
      if test x"${agg25}" != xyes; then # {
        echo "        WARNING: Your installation of AGG appears to be version 2.4 or older."
        echo "                 Please upgrade to AGG 2.5 or greater"
        echo "                 or .deb users: apt-get install libagg-dev"
        echo "                 or .rpm users: yum install agg-devel"
        nogo=true
      else # }{
        if test x"$AGG_CFLAGS" != x; then # {
          echo "        AGG flags are: $AGG_CFLAGS"
        else # }{
          echo "        AGG flags are: default include path"
        fi # }
        echo "        AGG libs are: $AGG_LIBS"
      fi # }
    else # }{
      echo "        ERROR: No AGG development package installed!"
      echo "               Install it from http://www.antigrain.com"
      echo "               or .deb users: apt-get install libagg-dev"
      echo "               or .rpm users: yum install agg-devel"
      nogo=true
    fi # }
fi # }

if test x"$BOOST_LIBS" != x; then
    # Only cygnal requires date_time at present, so it's OK if either
    # you don't want cygnal or do have date_time installed.
    if test x"$cygnal" = xno -o x"$boost_date_time" = xyes; then
	echo "        BOOST flags are: $BOOST_CFLAGS"
	echo "        BOOST libs are: $BOOST_LIBS"
    else
	# They have some boost libs but no date_time and want to compile cygnal.
	echo "        ERROR: The BOOST date_time package is needed to compile cygnal!"
	echo "               Install it from http://boost.org"
	echo "               or .deb users: apt-get install libboost-date-time-dev"
	echo "               or configure --disable-cygnal"
	nogo=true
    fi
else
    echo "        ERROR: No BOOST development package installed!"
    echo "               Install it from http://www.boost.org"
    echo "               or .deb users: apt-get install libboost-thread-dev"
    if test x"$cygnal" = xyes; then
    echo "                              and libboost-date-time-dev"
    fi
    echo "               or .rpm users: yum install boost-devel"
 nogo=true
fi

dnl don't look for the flash compilers when cross compiling.
if test x$cross_compiling = xno; then
  if test x"$MING_LIBS" != x; then
    echo "        MING version code is $MING_VERSION_CODE"
    echo "        MING flags are $MING_CFLAGS"
    echo "        MING libs are $MING_LIBS"
    echo "        MAKESWF is $MAKESWF"
else
    echo "        WARNING: You need to have the Ming development and utilities packages"
    echo "                 installed to run most of the tests in Gnash testsuite."
    echo "                 Install it from http://ming.sourceforge.net"
    echo "                 or .deb users: apt-get install libming-dev"
  fi

  if test x"$SWFDEC_TESTSUITE" != x; then
    echo "        SWFDEC testsuite dir is $SWFDEC_TESTSUITE"
  fi

  if test x"$MTASC" != x; then
    echo "        MTASC is $MTASC"
    echo "        MTASC CLASSPATH is $MTASC_CLASSPATH"
  else
    echo "        WARNING: You need to have the MTASC compiler packages installed"
    echo "                 to run some of the tests in Gnash testsuite."
    echo "                 You can install it from http://mtasc.org"
    echo "                 or .deb users: apt-get install mtasc"
  fi

  if test x"$SWFMILL" != x; then
    echo "        SWFMILL is $SWFMILL"
  else
    echo "        WARNING: You need to have the 'swfmill' tool installed"
    echo "                 to run some of the tests in Gnash testsuite."
    echo "                 You can install it from http://iterative.org/swfmill/"
  fi
fi

if test x"$Z_LIBS" != x; then
  if test x"$Z_CFLAGS" != x; then
    echo "        Z flags are: $Z_CFLAGS"
  else
    echo "        Z flags are: default include path"
  fi
  echo "        Z libs are: $Z_LIBS"
else
  echo "        WARNING: You need to have the zlib development packages installed"
  echo "                 to play compressed SWF (most of them from version 6 up)"
  echo "                 and to display some kinds of JPEG files."
  echo "                 Install it from http://www.zlib.net"
  echo "                 or .deb users: apt-get install zlib1g-dev"
  echo "                 or .rpm users: yum install zlib-dev."
  echo "                 It may still be possible to configure without zlib."
fi

if test x"$FREETYPE2_LIBS" != x; then
  if test x"$FREETYPE2_CFLAGS" != x; then
    echo "        FreeType flags are: $FREETYPE2_CFLAGS"
  else
    echo "        FreeType flags are: default include path"
  fi
  echo "        FreeType libs are: $FREETYPE2_LIBS"
else
  echo "        WARNING: You need to have the freetype development packages installed"
  echo "                 to use device fonts."
  echo "                 Install it from http://www.freetype.org"
  echo "                 or .deb users: apt-get install libfreetype6-dev"
  echo "                 or .rpm users: yum install libfreetype6-dev (??)"
  echo "                 It may still be possible to configure without freetype."
fi

if test x"$FONTCONFIG_LIBS" != x; then
  if test x"$FONTCONFIG_CFLAGS" != x; then
    echo "        Fontconfig flags are: $FONTCONFIG_CFLAGS"
  else
    echo "        Fontconfig flags are: default include path"
  fi
  echo "        Fontconfig libs are: $FONTCONFIG_LIBS"
else
  echo "        WARNING: You need to have the freetype development packages installed"
  echo "                 to use device fonts."
  echo "                 Install it from http://www.fontconfig.org"
  echo "                 or .deb users: apt-get install libfontconfig1-dev"
  echo "                 or .rpm users: yum install ??"
  echo "                 It may still be possible to configure without fontconfig."
fi

if test x$ext_mysql = xyes; then
  if test x$mysql != xno; then
    if test x"$MYSQL_LIBS" != x; then
      echo "        MYSQL flags are: $MYSQL_CFLAGS"
      echo "        MYSQL libs are: $MYSQL_LIBS"
    else
      echo "        ERROR: No MySQL development package is installed."
      echo "               Either reconfigure without --with-extensions=mysql"
      echo "               or install MySQL header files from http://www.mysql.org"
      echo "               or .deb users: apt-get install libmysqlclient-dev"
    fi
  fi
fi

if test x$cross_compiling = xyes; then
  AC_MSG_NOTICE([This build is setup for cross compiling])
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
