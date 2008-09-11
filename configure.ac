dnl  
dnl  Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

AC_PREREQ(2.50)
AC_INIT(gnash, trunk)
AC_CONFIG_SRCDIR(libcore/gnash.h)
AM_CONFIG_HEADER(gnashconfig.h)

dnl AC_CANONICAL_TARGET
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

dnl --------------------------------------------------------
dnl Figure out development tool stuff
dnl --------------------------------------------------------

AC_PROG_CC
AC_PROG_CXX
AC_EXEEXT
AC_PROG_INSTALL
AM_COMPILER_LIB

dnl Set the default values for Flash Version. These are converted into
dnl various strings to make JavaScript or ActionScript detectors
dnl recognize Gnash as a Flash Player.
DEFAULT_FLASH_MAJOR_VERSION="9"
DEFAULT_FLASH_MINOR_VERSION="0"
DEFAULT_FLASH_REV_NUMBER="999"
AC_SUBST(DEFAULT_FLASH_MAJOR_VERSION)
AC_SUBST(DEFAULT_FLASH_MINOR_VERSION)
AC_SUBST(DEFAULT_FLASH_REV_NUMBER)

AC_DEFINE_UNQUOTED([DEFAULT_FLASH_MAJOR_VERSION], ["${DEFAULT_FLASH_MAJOR_VERSION}"], [Default Flash major version])
AC_DEFINE_UNQUOTED([DEFAULT_FLASH_MINOR_VERSION], ["${DEFAULT_FLASH_MINOR_VERSION}"], [Default Flash minor version])
AC_DEFINE_UNQUOTED([DEFAULT_FLASH_REV_NUMBER], ["${DEFAULT_FLASH_REV_NUMBER}"], [Default Flash revision number])

dnl TODO: use host/build/target -- whatever is more appropriate
case "${host}" in
  *-apple-*)
    DEFAULT_FLASH_PLATFORM_ID="MAC"
    DEFAULT_FLASH_SYSTEM_OS="MacOS"
    ;;
  *-openbsd*)
    DEFAULT_FLASH_PLATFORM_ID="BSD"
    DEFAULT_FLASH_SYSTEM_OS="OpenBSD"
    ;;
  *-freebsd*)
    DEFAULT_FLASH_PLATFORM_ID="BSD"
    DEFAULT_FLASH_SYSTEM_OS="FreeBSD"
    ;;
  *-netbsd*)
    DEFAULT_FLASH_PLATFORM_ID="BSD"
    DEFAULT_FLASH_SYSTEM_OS="NetBSD"
    ;;
  *-linux-gnu)
    DEFAULT_FLASH_PLATFORM_ID="LNX"
    DEFAULT_FLASH_SYSTEM_OS="GNU/Linux"
    ;;
  *-linux*)
    DEFAULT_FLASH_PLATFORM_ID="LNX"
    DEFAULT_FLASH_SYSTEM_OS="Linux"
    ;;
  *-cygwin* | *-mingw* | *-pw32*)
    DEFAULT_FLASH_PLATFORM_ID="WIN"
    DEFAULT_FLASH_SYSTEM_OS="Windows"
    ;;
  *-*solaris*)
    DEFAULT_FLASH_PLATFORM_ID="SUN"
    DEFAULT_FLASH_SYSTEM_OS="Solaris"
    ;;
  *-os2*)
    DEFAULT_FLASH_PLATFORM_ID="OS2"
    DEFAULT_FLASH_SYSTEM_OS="OS/2"
    ;;
  *-sco*)
    DEFAULT_FLASH_PLATFORM_ID="SCO"
    DEFAULT_FLASH_SYSTEM_OS="SCO/Unix"
    ;;
  *-irix*)
    DEFAULT_FLASH_PLATFORM_ID="IRX"
    DEFAULT_FLASH_SYSTEM_OS="IRIX"
    ;;
  *-hpux*)
    DEFAULT_FLASH_PLATFORM_ID="HPX"
    DEFAULT_FLASH_SYSTEM_OS="HPUX"
    ;;    
  *)
    DEFAULT_FLASH_PLATFORM_ID="UNK"
    DEFAULT_FLASH_SYSTEM_OS="Unknown"
    ;;
esac

AC_DEFINE_UNQUOTED([DEFAULT_FLASH_PLATFORM_ID], ["${DEFAULT_FLASH_PLATFORM_ID}"], [Default 3-letter platform identifier for version string])
AC_DEFINE_UNQUOTED([DEFAULT_FLASH_SYSTEM_OS], ["${DEFAULT_FLASH_SYSTEM_OS}"], [Default value for System.capabilities.os])

AC_SUBST(DEFAULT_FLASH_PLATFORM_ID)
AC_SUBST(DEFAULT_FLASH_SYSTEM_OS)

DEFAULT_STREAMS_TIMEOUT=10
AC_SUBST(DEFAULT_STREAMS_TIMEOUT)
AC_DEFINE_UNQUOTED([DEFAULT_STREAMS_TIMEOUT], [${DEFAULT_STREAMS_TIMEOUT}], [Default streams timeout in seconds])


dnl Some things you can only do by looking at the platform name.
case "${host}" in
  powerpc-apple-darwin*)
    AC_DEFINE([__powerpc64__], [1], [this is a 64 bit powerpc])
    darwin=yes
    ;;
  *-apple-darwin*)
    darwin=yes
    AC_DEFINE([DARWIN], [1], [this is a Darwin platform])
    ;;
    dnl Unfortunately, all BSD distributions are not identical, so as tacky as it is
    dnl to look for the distribution name, we don't have much choice. The use of these
    dnl should be avoid as much as possible.
  *-openbsd*)
    bsd=yes
    openbsd=yes
    AC_DEFINE([OPENBSD], [1], [this is an OpenBSD platform])
    ;;
  *-freebsd*)
    bsd=yes
    freebsd=yes
    AC_DEFINE([FREEBSD], [1], [this is a FreeBSD platform])
    ;;
  *-netbsd*)
    bsd=yes
    netbsd=yes
    AC_DEFINE([NETBSD], [1], [this is a NetBSD platform])
    ;;
  *-*solaris*)
    solaris=yes
    AC_DEFINE([SOLARIS], [1], [this is a Solaris platform])
    ;;
  *-*linux*)
    linux=yes
    AC_DEFINE([LINUX], [1], [this is a Linux platform])
    ;;
  *-cygwin* | *-mingw* | *-pw32*)
    windows=yes
    AC_DEFINE([WIN32], [1], [this is a Win32 platform])
    ;;
  *64-*-*bsd*)
    bsd_os=bsd 
    AC_DEFINE([WORDSIZE], [64], [this is a 64 platform])
    ;;
esac

AM_CONDITIONAL(PLUGIN_LINK_UNDEFINED, test x$openbsd_os = xopenbsd)
AM_CONDITIONAL(WIN32, test x$windows = xyes)

dnl Get build date for helping us debugging
BUILDDATE="`date +%Y%m%d`"
AC_SUBST(BUILDDATE)

dnl These are required by automake
dnl AM_INIT_AUTOMAKE(gnash, "trunk$BUILDDATE")
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE
AC_PROG_MAKE_SET

dnl AC_DISABLE_STATIC dnl Disable building static libs.

AM_GNU_GETTEXT([external])
AM_CONDITIONAL(HAS_GETTEXT, test x$ac_cv_path_XGETTEXT != x)
dnl AM_GNU_GETTEXT_VERSION(0.15)

dnl We shouldn't need SIZET_FMT anywhere anymore, so all of this
dnl is useless.

dnl AC_CHECK_SIZEOF([size_t], 4, AC_INCLUDES_DEFAULT)

dnl AC_DEFINE(SIZET_FMT, ["%z"], [size_t printf modifier is z])

dnl NOTE: Using AC_TRY_COMPILE is bogus when cross-compiling !
dnl       We need to know the size of size_t in the *target* host, 
dnl       not the build host (right?)
dnl
dnl AC_LANG_PUSH(C++)
dnl if test "$GXX" = yes; then
dnl save_CXXFLAGS="$CXXFLAGS"
dnl CXXFLAGS="$CXXFLAGS -Werror -Wformat"
dnl fi

dnl AC_TRY_COMPILE([#include <sys/types.h>
dnl   #include <stdio.h>], [
dnl   size_t num = 0;
dnl   printf("%d", num); ],
dnl   AC_DEFINE(SIZET_FMT, ["%d"], [size_t printf modifier is d]), [

dnl AC_TRY_COMPILE([#include <sys/types.h>
dnl   #include <stdio.h>], [
dnl   size_t num = 0;
dnl   printf("%u", num); ],
dnl   AC_DEFINE(SIZET_FMT, ["%u"], [size_t printf modifier is u]), [

dnl AC_TRY_COMPILE([#include <sys/types.h>
dnl   #include <stdio.h>], [
dnl   size_t num = 0;
dnl   printf("%lu", num); ],
dnl   AC_DEFINE(SIZET_FMT, ["%lu"], [size_t printf modifier is lu]), [

dnl AC_TRY_COMPILE([#include <sys/types.h> 
dnl   #include <stdio.h>], [
dnl   size_t num = 0;
dnl   printf("%ld", num); ],
dnl   AC_DEFINE(SIZET_FMT, ["%ld"], [size_t printf modifier is ld])
dnl )])])])

dnl if test "$GXX" = yes; then
dnl CXXFLAGS="$save_CXXFLAGS"
dnl fi
dnl AC_LANG_POP(C++)

dnl This is primarily used when compiling for a similar architecture,
dnl like pentium->geode, which can use the same compiler, but have
dnl different development libraries.

AC_ARG_WITH(top_level,
  AC_HELP_STRING([--with-top-level],
  [top level directory for cross compiling files]),
  with_top_level=${withval} ;
  cross_compiling=yes)

soldir=/tmp
AC_ARG_WITH(soldir,
 AC_HELP_STRING([--with-soldir],
 [directory for .sol files]),
 with_soldir=${withval})
if test x${with_soldir} != x; then
  soldir=${with_soldir}
fi
SOLDIR=${soldir}
AC_SUBST(SOLDIR)

dnl lckey=0xdd3adabd
AC_ARG_WITH(lckey,
 AC_HELP_STRING([--with-lckey],
 [shared memory key for your system]),
 with_lckey=${withval})

if test x${with_lckey} != x; then
  lckey=${with_lckey}
else
  lckey=0xcbc384f8
fi
LC_KEY=${lckey}
AC_SUBST(LC_KEY)

# Maybe use jemalloc, which handles memory fragmentation for
# ECAMscript languages better than the regular system malloc.
# This seems like a good idea, as both the other player and
# Mozilla/Firefox both recently switched to using jemalloc.
AC_ARG_ENABLE(jemalloc,
  AC_HELP_STRING([--enable-jemalloc],[Enable jemalloc instead of system malloc]),
[case "${enableval}" in
  yes) jemalloc=yes ;;
  no)  jemalloc=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for --enable-jemalloc option]) ;;
esac],jemalloc=no)

dnl There is some weird stuff going on with NetBSD and jemalloc, so don't built it for now.
if test x"${netbsd}" = x"yes"; then
  jemalloc=no
fi
dnl If the compiler doesn't have local thread storage enabled, don't try to
dnl use jemalloc.
if test x"${jemalloc}" = x"yes"; then
  AC_TRY_COMPILE([], [
    extern __thread int global_i; ],
    has_local_thread_storage=yes
  )
  if test x"${has_local_thread_storage}" = x"yes"; then
    AC_DEFINE([HAVE_LOCAL_THREAD_STORAGE], [1], [Has __thread (local thread storage) support])
    AC_DEFINE([USE_JEMALLOC], [], [Use jemalloc instead of system malloc])
  else
    jemalloc=no
  fi
fi
AC_TRY_COMPILE([#include <malloc.h>], [
  struct mallinfo x = mallinfo(); ],
  mallinfo=yes, mallinfo=no
)
if test x"${mallinfo}" = x"yes"; then
  AC_DEFINE(HAVE_MALLINFO, [1], [Has mallinfo()])
fi
AM_CONDITIONAL(JEMALLOC, test x$jemalloc = xyes)
AM_CONDITIONAL(HAVE_MALLINFO, [test x$mallinfo = xyes])

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

AC_ARG_ENABLE(fps-debug,
  AC_HELP_STRING([--enable-fps-debug],[Enable FPS debugging code]),
[case "${enableval}" in
  yes) AC_DEFINE([GNASH_FPS_DEBUG], [1], [Enable FPS debugging code])
esac])

dnl When we're making binary releases, it's often nice to just statically link
dnl the final executables so we don't worry about what's installed, or which
dnl version it is.
AC_ARG_ENABLE(allstatic,
  AC_HELP_STRING([--enable-allstatic],[Enable using static libraries when possible for dependencies]),
[case "${enableval}" in
  yes) allstatic=yes ;;
  no)  allstatic=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for --enable-allstatic option]) ;;
esac],allstatic=no)
AM_CONDITIONAL(ALLSTATIC, test x$allstatic = xyes)

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

AC_ARG_WITH(shm,
  AC_HELP_STRING([--with-shm],[specify a shared memory type.]),
  [case "${withval}" in
    sysv) with_shm=sysv
          ;;
    posix) with_shm=posix
          ;;    
    *)   AC_MSG_ERROR([bad value ${enableval} for --with-shm option (try sysv or posix])
          ;;
   esac],with_shm=sysv)

if test x"${with_shm}" = x"sysv"; then
  AC_DEFINE([USE_SYSV_SHM], [1], [Use SYSV shared memory for compatability])
  dnl IPC_INFO isn't portable, and doesn't exist on BSD
  AC_TRY_COMPILE([#include <sys/ipc.h> #include <sys/shm.h>], [
    int flag = IPC_INFO; ],
    AC_DEFINE([HAVE_IPC_INFO], [1], [Use shm_info])
  )
else
  AC_DEFINE([USE_POSIX_SHM], [1], [Use POSIX shared memory])
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

dnl Don't gather SWF information in tree form. This takes
dnl resources and memory that can be saved if there's no
dnl need to examine SWF internals.
AC_ARG_ENABLE(swftree,
  AC_HELP_STRING([--disable-swftree],[Disable showing SWF properties]),
[case "${enableval}" in
  yes) swftree=yes ;;
  no)  swftree=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for --disable-swf-properties option]) ;;
esac],swftree=yes)

if test x"$swftree" = x"yes"; then
  AC_DEFINE([USE_SWFTREE], [], [View SWF information])
fi
AM_CONDITIONAL(SWFTREE, test x$swftree = xyes)

dnl Disable running any tests for "make check". This may sound stupid, but
dnl this option is designed to soley be used by maintainers in the 
dnl DISTCHECK_CONFIGURE_FLAGS when building packages. Gnash's testing infrastructure
dnl is complex, and often the the testsuites will work, but due to some obscure reason,
dnl make distcheck fails.
AC_ARG_ENABLE(testsuite,
  AC_HELP_STRING([--disable-testsuite],[Disable the testsuite, maintainers option only]),
[case "${enableval}" in
  yes) testsuite=yes ;;
  no)  testsuite=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for --disable-testsuite option]) ;;
esac],testsuite=yes)

if test x"$testsuite" = x"yes"; then
  AC_DEFINE([USE_TESTSUITE], [], [Testsuite support, maintainers option only])
fi
AM_CONDITIONAL(TESTSUITE, test x$testsuite = xyes)

dnl -- dnl Enable building the gui support even when statically
dnl -- dnl linking. Normally this is only disabled when building a statically
dnl -- dnl linked gnash executable.
dnl -- AC_ARG_ENABLE(dynamic-gui,
dnl --   AC_HELP_STRING([--enable-dynamic-gui],[Enable building dynamically loadable GUIs and renderers ]),
dnl -- [case "${enableval}" in
dnl --   yes) dynamic_gui=yes ;;
dnl --   no)  dynamic_gui=no ;;
dnl --   *)   AC_MSG_ERROR([bad value ${enableval} for --enable-dynamic-gui option]) ;;
dnl -- esac],dynamic_gui=no)
dnl -- if test x"${dynamic_gui}" = x"yes"; then
dnl --   AC_DEFINE([USE_DYNAMIC_GUI], [], [Dynamically loadable GUI and renderer support])
dnl -- fi
dnl -- AM_CONDITIONAL(DYNAMIC_GUI, test x${dynamic_gui} = xyes)

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

dnl Build the cygnal server if specified.
AC_ARG_ENABLE(cygnal,
  AC_HELP_STRING([--enable-cygnal], [Enable building of the Cygnal server]),
[case "${enableval}" in
  yes) cygnal=yes ;;
  no)  cygnal=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for enable-cygnal option]) ;;
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

has_kde=no
has_gtk2=no
build_fb=no
build_fltk=no
build_kde=no
build_gtk=no
build_sdl=no
build_riscos=no
build_aqua=no
build_hildon=no
build_alp=no
build_qtopia=no
build_dump=no
AC_ARG_ENABLE(gui,
  AC_HELP_STRING([--enable-gui=], [Use the specified GUI toolkit (default=gtk,kde)]),
  if test -n ${enableval}; then
    enableval=`echo ${enableval} | tr '\054' ' ' `
  fi
  while test -n "${enableval}" ; do
    val=`echo ${enableval} | cut -d ' ' -f 1`
    [case "${val}" in
      hildon|HILDON|hildon2|HILDON2)
        build_hildon=yes
        AC_DEFINE(USE_HILDON, [1], [Use the Hildon mobile framework])
        build_gtk=yes
dnl        AC_DEFINE(USE_GTK, [1], [Use the GTK GUI])
        ;;
      alp|ALP|alp|ALP)
        build_alp=yes
dnl        AC_DEFINE(USE_ALP, [1], [Use the ALP framework])
        build_gtk=yes
        ;;
      gtk|GTK|gtk2|GTK2)
dnl        AC_DEFINE(USE_GTK, [1], [Use the GTK GUI])
        build_gtk=yes
        ;;
      kde|KDE)
        build_kde=yes
        ;;
      qtopia|QTOPIA)
        build_qtopia=yes
        build_qt=yes
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
      dump|DUMP)
        build_dump=yes
        ;;
      all|ALL)
        build_dump=yes
        build_fb=yes
        build_fltk=yes
        build_kde=yes
        build_gtk=yes
        build_sdl=yes
dnl        build_qtopia=yes
dnl        build_hildon=yes
dnl        build_alp=yes
dnl        build_riscos=yes
        ;;
      *) AC_MSG_ERROR([invalid gui ${enableval} given (accept: gtk|kde|fltk|sdl|riscos|aqua|fb|hildon|alp|qtopia|dump)])
         ;;
      esac]
    enableval=`echo ${enableval} | cut -d ' ' -f 2-6`
    if test "x$val" = "x$enableval"; then
      break;
    fi
  done,build_gtk=yes;build_kde=yes;
)

add_renderer=
build_ogl=no
build_agg=no
build_cairo=no
AC_ARG_ENABLE(renderer,
  AC_HELP_STRING([--enable-renderer=], [Use the specified GUI toolkit (default=ogl,agg)]),
dnl  if test -n ${enableval}; then
dnl    add_renderer="${enableval}"
dnl    enableval=`echo ${enableval} | tr '\054' ' ' `
dnl  fi
dnl  while test -n "${enableval}" ; do
dnl    val=`echo ${enableval} | cut -d ' ' -f 1`
    [case "${enableval}" in
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
dnl       all|ALL|All)
dnl         build_ogl=yes
dnl         build_agg=yes
dnl         build_cairo=yes
dnl 	add_renderer="ogl,agg,cairo"
dnl         ;;
      *) AC_MSG_ERROR([invalid renderer ${enableval} given (accept: ogl,agg,cairo)])
         ;;
      esac],
	build_agg=yes
	add_renderer=agg
dnl    enableval=`echo ${enableval} | cut -d ' ' -f 2-6`
dnl    if test x$val = x$enableval; then
dnl      break;
dnl    fi
dnl  done
)

dnl if test x$add_renderer = x; then
dnl    build_ogl=yes
dnl    build_agg=yes
dnl    add_renderer="ogl,agg"
dnl fi

if test x$build_ogl = xyes; then
  if test x$build_fb = xyes; then
    AC_MSG_ERROR([OpenGL renderer is not supported by FB gui. Use --enable-renderer=AGG or --enable-gui=kde,gtk,sdl]);
  fi
  if test x$build_fltk = xyes; then
    AC_MSG_ERROR([OpenGL renderer is not supported by FLTK gui. Use --enable-renderer=AGG or --enable-gui=kde,gtk,sdl]);
  fi
  if test x$build_dump = xyes; then
    AC_MSG_ERROR([OpenGL renderer is not supported by Dump gui. Use --enable-renderer=AGG or --enable-gui=kde,gtk,sdl]);
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
      all)
        # allow special value "all" set by user (handled below)
        ;; 
      argb32|ARGB32)
        AC_DEFINE(PIXELFORMAT_ARGB32, [1], [ARGB32])
        ;;
      abgr32|ABGR32)
        AC_DEFINE(PIXELFORMAT_ABGR32, [1], [ABGR32])
        ;;
      bgra32|BGRA32)
        AC_DEFINE(PIXELFORMAT_BGRA32, [1], [BGRA32])
        ;;
      bgr24|BGR24)
        AC_DEFINE(PIXELFORMAT_BGR24, [1], [BGR24])
        ;;
      rgba32|RGBA32)
        AC_DEFINE(PIXELFORMAT_RGBA32, [1], [RGBA32])
        ;;
      rgb24|RGB24)
        AC_DEFINE(PIXELFORMAT_RGB24, [1], [RGB24])
        ;;
      rgb555|RGB555)
        AC_DEFINE(PIXELFORMAT_RGB555, [1], [RGB555])
        ;;
      rgb565|RGB565)
        AC_DEFINE(PIXELFORMAT_RGB565, [1], [RGB565])
        ;;
      *) AC_MSG_ERROR([invalid pixel format ${withval} given (accept: all|RGB555|RGB565|RGB24|BGR24|BGRA32|RGBA32|ARGB32|ABGR32)])
         ;;
      esac]
    withval=`echo ${withval} | cut -d ' ' -f 2-6`
    if test "x$val" = "x$withval"; then
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
      AC_DEFINE(PIXELFORMAT_ARGB32, [1], [ARGB32 pixel format])
      AC_DEFINE(PIXELFORMAT_ABGR32, [1], [ABGR32 pixel format])
    #fi
  fi
fi

dnl --------------------------------------------------------
dnl  Toggle support for MIT-SHM extension to the X server
dnl --------------------------------------------------------

AC_ARG_ENABLE(mit-shm,
  AC_HELP_STRING([--disable-mit-shm], [Disable support for the MIT-SHM X extension]),
[case "${enableval}" in
  yes) xmitshm=yes ;;
  no)  xmitshm=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for --enable-mit-shm option]) ;;
esac],xmitshm=yes)
dnl as windows doesn't run X11, don't enable this extension there.
if test "x${xmitshm}" = "xyes" -a "x${windows}" = "xno"; then
	AC_DEFINE(ENABLE_MIT_SHM, [1], [Use MIT-SHM X extension if available])
fi

dnl --------------------------------------------------------
dnl  Select media handler
dnl --------------------------------------------------------

media_handler_specified=false
AC_ARG_ENABLE(media,
 AC_HELP_STRING([--enable-media=handler], [Enable media handling support using the specified handler: gst, ffmpeg or none (no sound) [[gst]] ]),
 [case "${enableval}" in
   GST|gst)
     media_handler=gst
     media_handler_specified=true
     ;;
   ffmpeg|FFMPEG)
     media_handler=ffmpeg
     media_handler_specified=true
     ;;
   no|NO|none)
     media_handler=none
     media_handler_specified=true
     ;;
   *) AC_MSG_ERROR([bad value ${enableval} for --enable-media option]) ;;
  esac],
 [media_handler=gst; media_handler_specified=true]
)

AC_ARG_ENABLE(lirc, AC_HELP_STRING([--enable-lirc], [Disable support for Lirc]),
[case "${enableval}" in
  yes) lirc=yes ;;
  no)  lirc=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for enable-lirc option]) ;;
esac], lirc=no)

if test x"$lirc" = x"yes"; then
  AC_DEFINE([USE_LIRC], [], [LIRC daemon support])
fi
AM_CONDITIONAL(USE_LIRC, test x$lirc = xyes)

dnl --------------------------------------------------------
dnl Figure out which extensions to build.
dnl --------------------------------------------------------

extensions_list=
ext_dejagnu=no
ext_mysql=no
ext_fileio=no
ext_gtk=no
ext_lirc=no
ext_dbus=no
ext_metome=no
ext_all=no
ext_launcher=no
AC_ARG_ENABLE(extensions,
  AC_HELP_STRING([--enable-extensions=], [Specify which extensions to build]),
  if test -n ${enableval}; then
    if test "x${enableval}" != "xno"; then
      extlist="${enableval}"
      enableval=`echo ${enableval} | tr '\054' ' ' `
    else
      extlist=""
      enableval=""
    fi
  fi
  nextensions=0
  while test -n "${enableval}" ; do
    val=`echo ${enableval} | cut -d ' ' -f 1`
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
      launcher|LAUNCHER)
        AC_DEFINE(USE_LAUNCHER_EXT, [1], [Build the Launcher extension])
        ext_launcher=yes
        nextensions=$((nextensions+1))
        ;;
      lirc|LIRC)
        AC_DEFINE(USE_LIRC_EXT, [1], [Build the LIRC extension])
        ext_lirc=yes
        nextensions=$((nextensions+1))
        ;;
      dbus|DBUS)
        AC_DEFINE(USE_DBUS_EXT, [1], [Build the DBUS extension])
        ext_dbus=yes
        nextensions=$((nextensions+1))
        ;;
      metome|METOME)
        AC_DEFINE(USE_METOME_EXT, [1], [Build the METOME extension])
        ext_metome=yes
        nextensions=$((nextensions+1))
        ;;
      all|ALL)
        AC_DEFINE(USE_GTK_EXT, [1], [Build all the extensions])
        ext_dejagnu=yes
        ext_mysql=yes
        ext_fileio=yes
        ext_gtk=yes
        ext_lirc=yes
        ext_dbus=yes
        ext_metome=yes
        ext_launcher=yes
        ext_all=yes
        nextensions=9
        ;;
      *) AC_MSG_ERROR([invalid extension specified: ${enableval} given (accept: MYSQL|DEJAGNU|FILEIO|GTK|LIRC|DBUS|METOME|ALL)])
         ;;
      esac]
    enableval=`echo ${enableval} | cut -d ' ' -f 2-6`
    if test "x$val" = "x$enableval"; then
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
AM_CONDITIONAL(BUILD_LAUNCHER_EXT, [ test x$ext_launcher = xyes ])
AM_CONDITIONAL(BUILD_GTK_EXT, [ test x$ext_gtk = xyes ])
AM_CONDITIONAL(BUILD_LIRC_EXT, [ test x$ext_lirc = xyes ])
AM_CONDITIONAL(BUILD_DBUS_EXT, [ test x$ext_dbus = xyes ])
AM_CONDITIONAL(BUILD_METOME_EXT, [ test x$ext_metome = xyes ])
AM_CONDITIONAL(BUILD_EXTENSIONS, [ test -n "$extensions_list"])

AC_MSG_CHECKING([For the version of libtool])
ltver=`${LIBTOOLIZE:-libtoolize} --version | head -1 | cut -d ' ' -f 4`
ltmajor=`echo $ltver | cut -d '.' -f 1`
AC_MSG_RESULT([$ltver])
if test $ltmajor -eq 1; then
AC_LIBLTDL_INSTALLABLE
AC_LIBTOOL_DLOPEN
AC_LIBTOOL_WIN32_DLL
else
LT_INIT([dlopen win32-dll])
LTDL_INIT([nonrecursive])
fi
AC_PROG_LIBTOOL
AM_CONDITIONAL(LIBLTDL2, [test $ltmajor -eq 2])
AM_CONDITIONAL(INSTALL_LTDL, test x"${enable_ltdl_install-no}" = xyes)
AM_CONDITIONAL(CONVENIENCE_LTDL, test x"${enable_ltdl_convenience-no}" = xyes)

AC_C_CONST
AC_C_INLINE
AC_SUBST([LIBTOOL_DEPS])
AC_LIB_LTDL

AM_CONDITIONAL(ENABLE_SHARED, [test x"${enable_shared}" = xyes])
AM_CONDITIONAL(ENABLE_STATIC, [test x"${enable_shared}" = xno])
dnl -- AM_CONDITIONAL(STATIC_GUI, test x"${dynamic_gui}" = xno -o x"${enable_shared}" = xno)

AC_PATH_PROG(DEJAGNU, runtest)

AM_CONDITIONAL(BUILD_OGL_RENDERER, [ test x$build_ogl = xyes ])
AM_CONDITIONAL(BUILD_AGG_RENDERER, [ test x$build_agg = xyes ])
AM_CONDITIONAL(BUILD_CAIRO_RENDERER, [ test x$build_cairo = xyes ])

dnl AC_CHECK_LIB(ltdl, lt_dlmutex_register, AC_DEFINE(LT_DLMUTEX, 1, [Has lt_dlmutex_register]),
dnl                                         AC_DEFINE(LT_DLMUTEX, 0, [doesn't have lt_dlmutex_register]) )

dnl These options are for Cygnal, which collects optional statistics on all the network traffic
dnl By default, we turn on statistics collecting for the incoming and outgoing queues, since
dnl those are required to be compatiable with FMS 3, and the ActionScript server management API.
dnl buffers are the lowest level data storage, this data is the time spent in the queue, and is
dnl primarily only used for tuning the queueing API in Gnash. Memoryis the same, it's only used
dnl by developers for tuning performance of memory allocations in Gnash.
buffers=no
que=no
memory=no
AC_ARG_WITH(statistics,
  AC_HELP_STRING([--with-statistics=], [Specify which statistics features to enable]),
  if test -n ${withval}; then
    if test "x${withval}" != "xno"; then
      extlist="${withval}"
      withval=`echo ${withval} | tr '\054' ' ' `
    else
      extlist=""
      withval=""
    fi
  fi
  statistics_list=""
  nstatistics=0
  while test -n "${withval}" ; do
    val=`echo ${withval} | cut -d ' ' -f 1`
    [case "${val}" in
      buffers)
        buffers=yes
        nstatistics=$((nstatistics+1))
        ;;
      que)
        que=yes
        nstatistics=$((nstatistics+1))
        ;;
      memory)
        memory=yes
        nstatistics=$((nstatistics+1))
        ;;
      all|ALL)
        buffers=yes
        memory=yes
        queu=yes
        nstatistics=3
        ;;
      *) AC_MSG_ERROR([invalid statistics feature specified: ${withval} given (accept: buffers|que|memory|all)])
        ;;
      esac]
    withval=`echo ${withval} | cut -d ' ' -f 2-6`
    if test "x$val" = "x$withval"; then
      break;
    fi
  done
)
if test x${buffers} = xyes; then
  statistics_list="${statistics_list} buffers"
  AC_DEFINE(USE_STATS_BUFFERS, [1], [Support statistics collecting for the queue buffers])
  AC_MSG_WARN([This option will effect your performance])
fi

if test x${memory} = xyes; then
  statistics_list="${statistics_list} memory"
  AC_DEFINE(USE_STATS_MEMORY, [1], [Support statistics collecting for all memory profiling])
  AC_MSG_WARN([This option will effect your performance])
fi

if test x${que} = xyes; then
  statistics_list="${statistics_list} queues"
  AC_DEFINE(USE_STATS_QUEUE, [1], [Support statistics collecting for the queues])
fi
dnl this is just so Makefile can print the same list
STATISTICS_LIST="$statistics_list"
AC_SUBST(STATISTICS_LIST)

#
# These settings are compile time options for the security
# setting for anything that lets gnash exchange data with
# other applications. Currently this only supports Shared
# Objects and Local Connections. Shared Objects are like
# your web browsers cookies, and Local Connections use
# shared memory to execute methods remotely, and to
# exchange data.
#
# The default is to enable everything, and these can
# also be controlled dynamically by the $HOME/.gnashrc
# file.
#
solreadonly=no
localconnection=yes
lctrace=yes

AC_ARG_WITH(security,
  AC_HELP_STRING([--with-security=], [Specify which security features to enable]),
  if test -n ${withval}; then
    if test "x${withval}" != "xno"; then
      extlist="${withval}"
      withval=`echo ${withval} | tr '\054' ' ' `
    else
      extlist=""
      withval=""
    fi
  fi
  security_list=""
  nsecurity=0
  while test -n "${withval}" ; do
    val=`echo ${withval} | cut -d ' ' -f 1`
dnl    security_list="${security_list} ${val}"
    [case "${val}" in
      solreadonly)
        solreadonly=yes
        nsecurity=$((nsecurity+1))
        ;;
      lc)
        localconnection=yes
        nsecurity=$((nsecurity+1))
        ;;
      lctrace)
        lctrace=yes
        nsecurity=$((nsecurity+1))
        ;;
      all|ALL)
        solreadonly=yes
        lc=yes
        lctrace=yes
        nsecurity=3
        ;;
      *) AC_MSG_ERROR([invalid security feature specified: ${withval} given (accept: solreadonly|lc|lctrace)])
        ;;
      esac]
    withval=`echo ${withval} | cut -d ' ' -f 2-6`
    if test "x$val" = "x$withval"; then
      break;
    fi
  done
)
if test xlctrace = xyes; then
  security_list="${security_list} lctrace"
  AC_DEFINE(USE_LC_TRACE, [1],
           [Support LocalConnection])
  AC_MSG_NOTICE([This build supports LocalConnection tracing])
  AC_MSG_WARN([This option will effect your performance])
fi

if test x$localconnection = xyes; then
  security_list="${security_list} localconnection"
  AC_DEFINE(USE_LC, [1],
                 [Support LocalConnection])
  AC_MSG_NOTICE([This build supports LocalConnection])
fi
if test x$solreadonly = xyes; then
  security_list="${security_list} solreadonly"
  AC_DEFINE(USE_SOL_READONLY, [1],
                 [Shared Objects are read-only])
  AC_MSG_NOTICE([Shared Objects are read-only])
fi
SECURITY_LIST="$security_list"
AC_SUBST(SECURITY_LIST)


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
if test x"${archiver}" != x && test "${archiver}" -eq 1; then
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
  dnl pkgroot only works correctly with builds of cross tools not in
  dnl /usr, ie... installed from the distribution packages, or just
  dnl plain installed in the system tools. This contaminates configure
  dnl when building for variations of the same basic architecture,
  dnl like i686-linux -> i586-mingw32.
  if test x${pkgroot} = "/usr"; then
    export pkgroot=""
  fi
  export incllist="`eval echo ${with_top_level}/include ${pkgroot}/${host_alias}/include ${pkgroot}/include`"
  export libslist="`eval echo ${with_top_level}/lib ${pkgroot}/${host_alias}/lib ${pkgroot}/lib64 ${pkgroot}/lib32 ${pkgroot}/lib`"
  export pathlist="`eval echo ${pkgroot}/${host_alias}/bin:${pkgroot}/bin`"
  npapi=no
else
  AC_MSG_NOTICE([Configuring Gnash for native compilation])
  export incllist="`eval cat ${srcdir}/macros/incllist`"
  export libslist="`eval cat ${srcdir}/macros/libslist`"
  export pathlist=$PATH
fi

AM_CONDITIONAL(CROSS_COMPILING, [ test x$cross_compiling = xyes ])

dnl Check for PKG_CONFIG before any GNASH_PATH call
PKG_PROG_PKG_CONFIG

AC_PATH_PROG(PYTHON, python, ,[${pathlist}])
AM_CONDITIONAL([ENABLE_GNASH_PYTHON_MODULE], false)

dnl Look for scratchbox
sbox=no
if test x"${SBOX_REDIRECT_FROM_DIRS}" != x; then
  sbox=yes
fi
if test x"${build_alp}" = xyes; then
  GNASH_PATH_ALP
  if test x"${have_alp}" = x"yes"; then
    AC_DEFINE([HAVE_ALP], 1, [Access Linux Platform framework])
  fi
  cross_compiling=yes
dnl  build_gtk=yes
  build_kde=no
  build_ogl=no
  build_agg=yes
fi

dnl !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
dnl !! 
dnl !! IMPORTANT NOTICE 
dnl !!
dnl !!  Any call to GNASH_PATH_XXX must be be given *after* this snippet.
dnl !!  This is to ensure that PKG_CONFIG, cross_compiling and incllist are
dnl !!  properly set at the time of call.
dnl !! 
dnl !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

if test x$ext_dbus = xyes; then
  GNASH_PATH_DBUS
fi

if test x$ext_mysql = xyes; then
  GNASH_PATH_MYSQL
fi

dnl -----------------------------------------------------------
dnl   Verify dependencies for requested GUIs are met, and
dnl   disable build of the GUIS for which deps are NOT met
dnl ------------------------------------------------------------

if test x$build_gtk = xyes -o $build_alp = xyes -o x$build_hildon = xyes; then
   GNASH_PATH_GTK2
   GNASH_PATH_PANGO
   GNASH_PKG_FIND(atk, [atk/atk.h], [atk library], atk_focus_tracker_init, [1.0])
   if test x$build_ogl = xyes; then
      GNASH_PATH_GLEXT
   fi

  GNASH_PKG_FIND(cairo, [cairo.h], [cairo render library], cairo_status)
fi
if test x$build_hildon = xyes; then
  GNASH_PATH_HILDON
  build_gtk=yes
  build_kde=no
dnl   build_ogl=no
dnl   build_agg=yes
fi
dnl TODO: add checks for all other GUIs


dnl -----------------------------------------------------------
dnl Try to ignore stupid dependencies
dnl -----------------------------------------------------------

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

AC_PATH_TOOL([AUTOTRACE], [autotrace])
AC_PATH_TOOL([NETCAT], [netcat])
AC_HEADER_DIRENT

dnl -----------------------------------------------------------------
dnl PLUGIN RELATED STUFF
dnl -----------------------------------------------------------------

dnl Zip is used insted of tar when building an xpi package
dnl for Mozilla/Firefox.
AC_PATH_PROG(ZIP, zip, ,[${pathlist}])

dnl !! This has been moved here to make --enable-npapi work
dnl !! All of plugin-related macro calls could be moved into
dnl !! a specialized macros/plugin.m4

dnl ----------------------------------------------------
dnl Add KPARTS support, if specified or KDE gui is built
dnl ----------------------------------------------------

AC_ARG_ENABLE(kparts,
  AC_HELP_STRING([--disable-kparts], [Disble support for Konqueror plugin (default: enabled if kde gui is)]),
[case "${enableval}" in
  yes) kparts=yes ;;
  no)  kparts=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for --disable-kpart option]) ;;
esac],kparts=$build_kde)

dnl ----------------------------------------------------
dnl Add NPAPI support, if specified or GTK gui is built
dnl ----------------------------------------------------

AC_ARG_ENABLE(npapi,
  AC_HELP_STRING([--disable-npapi], [Disable NPAPI plugin build (default: enabled if gtk gui is)]),
  [case "${enableval}" in
    yes) npapi=yes ;;
    no)  npapi=no ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for disable-npapi option]) ;;
  esac],
  npapi=$build_gtk
)

dnl -----------------------------------------------------------------
dnl Enable us to disable building all browser plugins in one command.
dnl -----------------------------------------------------------------

AC_ARG_ENABLE(plugins,
  AC_HELP_STRING([--disable-plugins], [Disable all browser plugins from building (default=no)]),
  [case "${enableval}" in
    yes) plugins=yes ;;
    no)  plugins=no ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for disable-plugins option]) ;;
  esac],
  plugins=yes
)
if test x$plugins = xno; then
  npapi=no
  kparts=no
fi

dnl
dnl Set the general plugins install policy here
dnl
AC_ARG_WITH(plugins-install,
  AC_HELP_STRING([--with-plugins-install=system|user|prefix], [Policy for plugins install. Default: user.]),
	[case "${withval}" in
	  user) PLUGINS_INSTALL_POLICY=user ;;
	  system) PLUGINS_INSTALL_POLICY=system ;;
	  prefix) PLUGINS_INSTALL_POLICY=prefix ;;
	  *)  AC_MSG_ERROR([bad value ${withval} for --with-plugins-install]) ;;
	 esac 
	], PLUGINS_INSTALL_POLICY=user) 


GNASH_PATH_FIREFOX
dnl XPCOM and NSPR can be used in the plugin for better integration
dnl within Firefox.
GNASH_PATH_XPCOM
if test x$has_xpcom = xyes; then nspr4=yes; fi
GNASH_PATH_NSPR
dnl If we don't have both XPCOM and NSPR, we can't build either.
AM_CONDITIONAL(XPCOM, test x$has_xpcom = xyes -a x$has_nspr = xyes)

dnl -----------------------------------------------------------------
dnl END OF PLUGIN RELATED STUFF
dnl -----------------------------------------------------------------

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

AM_CONDITIONAL(LIRC, [test x$lirc_ext = xyes])

AC_CHECK_HEADERS(endian.h)
AC_CHECK_HEADERS(malloc.h)
AC_CHECK_HEADERS(getopt.h)
AC_CHECK_HEADERS(libgen.h)
AC_CHECK_HEADERS(pwd.h)
AC_CHECK_HEADERS(sys/utsname.h)
AC_CHECK_HEADERS(signal.h)
AC_CHECK_HEADERS(unistd.h)
AC_CHECK_HEADERS(sys/time.h)
AC_CHECK_LIB(lber, ber_free)
dnl AC_CHECK_LIB(m, sqrt)
AC_CHECK_LIB(c, getpwnam, AC_DEFINE(HAVE_GETPWNAM, 1, [Has getpwnam] ))

dnl AC_CHECK_LIB(m, exp2, AC_DEFINE(HAVE_EXP2, 1, [Has exp2()]))
dnl AC_CHECK_LIB(m, log2, AC_DEFINE(HAVE_LOG2, 1, [Has log2()]))
dnl AC_CHECK_LIB(m, trunc, AC_DEFINE(HAVE_TRUNC, 1, [Has trunc()]))

dnl X11 is needed for fltk (at least),
dnl and probably for many other guis too ...
dnl if ! test x$build_fb = xyes; then # <--- this is wrong as build_x is non-exclusive
dnl AC_PATH_XTRA
dnl AC_CHECK_LIB(Xmu, XmuCvtStringToOrientation)
dnl AC_CHECK_LIB(gmp, _gmp_get_memory_functions)
  GNASH_PATH_X11
  AC_CHECK_LIB(Xi, XInput_find_display)
  AC_CHECK_LIB(X11, XDisableAccessControl)
dnl fi
AM_CONDITIONAL(HAVE_X11, [test x$x11 = xyes])
AC_CHECK_LIB(rt, shm_unlink)
AC_CHECK_FUNCS(shm_open shm_unlink)
AC_TRY_COMPILE([#include <strings.h>], [
  char *p1 = (char *)"Hello";
  char *p2 = (char *)"World";
  strcasecmp(p1, p2); ],
  AC_DEFINE(HAVE_STRINGCASECMP, [1], [Has strcasecmp])
)

dnl See if ipc_perm structure has the ipc_perm.key field, and if so,
dnl which variant of the name is used.
ipc_key=no
AC_TRY_COMPILE([
  #include <sys/ipc.h>
  #include <sys/shm.h>], [
 struct shmid_ds shmseg;      
 key_t x = shmseg.shm_perm.key;],
 ipc_key=yes
 AC_DEFINE(IPC_PERM_KEY, [key], [Has the key field in ipc_perm])
)

if test x$ipc_key = xno; then
  AC_TRY_COMPILE([
    #include <sys/ipc.h>
    #include <sys/shm.h>], [
    struct shmid_ds shmseg;      
    key_t x = shmseg.shm_perm.__key;],
    AC_DEFINE(IPC_PERM_KEY, [__key], [Has the key field in ipc_perm])
    ipc_key=yes
  )
fi

dnl AC_CHECK_FUNCS(strcasecmp stricmp)
dnl AC_CHECK_FUNCS(vsnprintf)

dnl These two tests need the math library or they won't link
dnl on OpenBSD, even if the functions exist.
save_LIBS=$LIBS
LIBS="$LIBS -lm"
AC_CACHE_CHECK([for finite], ac_cv_finite,
 [AC_TRY_LINK([#include <math.h>],
 [double x; int y; y = finite(x);],
 ac_cv_finite=yes,
 ac_cv_finite=no
)])
if test x"$ac_cv_finite" = x"yes"; then
  AC_DEFINE(HAVE_FINITE, [1], [Has finite])
fi

AC_CACHE_CHECK([for isfinite], ac_cv_isfinite,
 [AC_TRY_LINK([#include <math.h>],
 [double x; int y; y = isfinite(x);],
 ac_cv_isfinite=yes,
 ac_cv_isfinite=no
)])
if test x"$ac_cv_isfinite" = x"yes"; then
  AC_DEFINE(HAVE_ISFINITE, [1], [Has isfinite])
fi
LIBS=$save_LIBS

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
AC_CHECK_FUNC(strndup, AC_DEFINE(HAVE_STRNDUP, 1, [Has strndup()] ))
AC_CHECK_FUNC(clock_gettime, AC_DEFINE(HAVE_CLOCK_GETTIME, 1, [Has clock_gettime()] ))
AC_CHECK_FUNCS(strlcpy, AC_DEFINE(HAVE_STRLCPY_PROTO, [1],[Define if you have the strlcpy prototype]))
AC_CHECK_FUNCS(strlcat, AC_DEFINE(HAVE_STRLCAT_PROTO, [1],[Define if you have the strlcat prototype]))
dnl Look for Win32 networking stuff
AC_CHECK_HEADERS(winsock.h)
AC_CHECK_HEADERS(winsock2.h)
AC_CHECK_FUNCS(socket)
AC_CHECK_FUNCS(CreateFileMappingA)

dnl GNASH_HASHMAP
dnl Let's get rid of this small file and inline the function.

dnl AC_LANG_PUSH(C++)
dnl 	AC_CHECK_HEADER(ext/hash_map, [
dnl 		AC_DEFINE([GNU_HASH_MAP], [1], [Using GNU ext/hash_map.h])
dnl 		AC_DEFINE([HASH_MAP_NS], [__gnu_cxx], [Using GNU __gnu_cxx::])
dnl 	], [
dnl 		AC_CHECK_HEADER(hash_map, [
dnl 			AC_DEFINE([WIN32_HASH_MAP], [1], [Using Win32 hash_map.h])
dnl 			AC_DEFINE([HASH_MAP_NS], [stdext], [Using win32 stdext::])
dnl 		])
dnl 	])
dnl AC_LANG_POP(C++)

dnl Shm::resize() uses this if it exists
AC_CHECK_LIB(c, mremap)

GNASH_PKG_FIND(nspr, [nspr.h], [Netscape Portable Runtime (NSPR)], PR_Init)

AC_PATH_TOOL(WINDRES, [windres])
AC_SUBST(WINDRES)

GNASH_PKG_FIND(z, [zlib.h], [zlib compression library], compress)
GNASH_PKG_FIND(jpeg, [jpeglib.h], [jpeg images], jpeg_mem_init)
GNASH_PKG_FIND(libpng, [png.h], [png images], png_info_init)
GNASH_PKG_FIND(gif, [gif_lib.h], [gif images], DGifOpen)
GNASH_PKG_INCLUDES([dejagnu], [dejagnu.h])

dnl Find freetype and fontconfig
dnl GNASH_PKG_FIND(freetype2, [freetype/freetype.h], [freetype2 font library], FT_Load_Char)
dnl rob might be working on this or not ;)
GNASH_PATH_FREETYPE2
GNASH_PKG_FIND(fontconfig, [fontconfig/fontconfig.h], [fontconfig library], FcFontMatch)

if test x$cross_compiling = xno; then
  AC_PATH_MING
fi
AM_CONDITIONAL(ENABLE_MING, [ test x"$MAKESWF" != x ])

AM_CONDITIONAL(MING_VERSION_0_4,
        [ test x"$MAKESWF" != x && test $MING_VERSION_CODE -ge 00040000  ])
AM_CONDITIONAL(MAKESWF_SUPPORTS_PREBUILT_CLIPS,
        [ test x"$MAKESWF" != x && test $MING_VERSION_CODE -ge 00040002  ])
AM_CONDITIONAL(MING_SUPPORTS_INIT_ACTIONS,
        [ test x"$MAKESWF" != x && test $MING_VERSION_CODE -ge 00040004  ])
AM_CONDITIONAL(MING_SUPPORTS_REPLACE_TAG,
        [ test x"$MAKESWF" != x && test $MING_VERSION_CODE -ge 00040005 ])
AM_CONDITIONAL(MING_SUPPORTS_SWFBUTTON_ADD_CHARACTER,
        [ test x"$MAKESWF" != x && test $MING_VERSION_CODE -ge 00040005 ])
AM_CONDITIONAL(MING_SUPPORTS_STREAMING_SOUND,
        [ test x"$MAKESWF" != x && test $MING_VERSION_CODE -ge 00040006 ])
AM_CONDITIONAL(MING_SUPPORTS_SWFBUTTON_SET_DEPTH,
        [ test x"$MAKESWF" != x && test $MING_VERSION_CODE -ge 00040006 ])

if test x$cross_compiling = xno; then
  AC_ARG_WITH([swfdec_testsuite],
	  AC_HELP_STRING([--with-swfdec-testsuite],
		  [directory where swfdec testsuite (the 'test' dir) is]),
	SWFDEC_TESTSUITE=${withval})
  AC_SUBST(SWFDEC_TESTSUITE)
fi
AM_CONDITIONAL(ENABLE_SWFDEC_TESTSUITE, [ test x"$SWFDEC_TESTSUITE" != x ])

dnl
dnl See if we can use the swfmill, mtasc, swfc and haxe based testsuites 
dnl
if test x$cross_compiling = xno; then
  AC_PATH_PROG(SWFMILL, swfmill)
  AC_PATH_PROG(SWFC, swfc)
  AC_PATH_MTASC
  AC_PATH_HAXE
fi
AM_CONDITIONAL(ENABLE_SWFMILL, [ test x"$SWFMILL" != x ])
AM_CONDITIONAL(ENABLE_MTASC, [ test x"$MTASC" != x ])
AM_CONDITIONAL(ENABLE_HAXE, [ test x"$HAXE" != x ])
AM_CONDITIONAL(ENABLE_SWFC, [ test x"$SWFC" != x ])

AC_PATH_PROG(DOXYGEN, doxygen)
AM_CONDITIONAL(ENABLE_DOXYGEN, [ test x"$DOXYGEN" != x ])

dnl currently unused
dnl GNASH_PKG_FIND(ogg, [ogg.h], [decode ogg streams], ogg_stream_init)

if test x$build_sdl = xyes -o x"$media_handler" = xffmpeg -o x"$media_handler" = xmad; then
  GNASH_PATH_SDL
fi

AM_CONDITIONAL(HAVE_SDL, [ test x$has_SDL = xyes ])

GNASH_PATH_LIBXML

dnl
dnl NOTE: make sure to check for GLIB and LIBXML2 
dnl before checking for gstreamer
dnl

AC_CHECK_SIZEOF(size_t) dnl set SIZEOF_SIZE_T.
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(short)

dnl # Check KDE if kde gui or kparts were requested
if test x$build_kde = xyes -o x$kparts = xyes; then
  GNASH_PATH_KDE
fi
if test x$build_qt = xyes -o x$build_qtopia = xyes -o x$build_kde = xyes; then
  GNASH_PATH_QT
  if test x$build_qtopia = xyes; then
    GNASH_PATH_QTOPIA
  fi
fi
AM_CONDITIONAL(HAVE_QTOPIA, [test x$has_qtopia = xyes])
AM_CONDITIONAL(HAVE_KDE, [test x$has_kde = xyes])
AM_CONDITIONAL(HAVE_QT, [test x$has_qt = xyes])

AM_CONDITIONAL(HAVE_QTOPIA_2, [test x"$gnash_qtopia_version" = x2])
AM_CONDITIONAL(HAVE_QTOPIA_3, [test x"$gnash_qtopia_version" = x3])
AM_CONDITIONAL(HAVE_QTOPIA_4, [test x"$gnash_qtopia_version" = x4])

AM_CONDITIONAL(HAVE_QT_2, [test x"${gnash_qt_version}" != x && test "${gnash_qt_version}" -eq 2])
AM_CONDITIONAL(HAVE_QT_3, [test x"${gnash_qt_version}" != x && test "${gnash_qt_version}" -eq 3])
AM_CONDITIONAL(HAVE_QT_4, [test x"${gnash_qt_version}" != x && test "${gnash_qt_version}" -eq 4])

dnl Need GLIB for both GTK and GST
if test x$build_gtk = xyes -o x${media_handler} = "xgst"; then
  GNASH_PATH_GLIB
fi

AM_CONDITIONAL(HAVE_GLIB, [ test x$has_glib = xyes ])

if test x$build_gtk = xno -a x$npapi = xyes; then
  AC_MSG_WARN(["Enabled NPAPI plugin, but you aren't building a GTK based GUI!"])
fi

if test x$windows = xyes -a x$npapi = xyes; then
  if test "x$NSPR_CFLAGS" = x -a "x$NSPR_LIBS" = x; then
    AC_MSG_ERROR(["On Win32, NPAPI plugin requires NSPR."])
  fi
  if test "x$WINDRES" = x; then
    AC_MSG_ERROR(["On Win32, NPAPI plugin requires windres."])
  fi
fi

dnl if kde isn't installed, even if it's specified, don't try to build
dnl the KPARTS plugin, which is KDE based.
if test x$has_kde = xno -a x$kparts = xyes; then
  kparts=no
dnl  build_kde=no
  AC_MSG_WARN(["Disabling KPARTS plugin, no KDE development found"])
fi

if test x$build_kde = xno -a x$kparts = xyes; then
  AC_MSG_WARN(["Enabled KPARTS plugin, but you aren't building a KDE based GUI!"])
fi

AM_CONDITIONAL(BUILD_QTOPIA_GUI, [ test x$build_qtopia = xyes ])
AM_CONDITIONAL(BUILD_ALP_GUI, [ test x$build_alp = xyes ])
AM_CONDITIONAL(BUILD_QT_GUI, [ test x$build_qt = xyes ])
AM_CONDITIONAL(BUILD_HILDON_GUI, [ test x$build_hildon = xyes ])
AM_CONDITIONAL(BUILD_GTK_GUI, [ test x$build_gtk = xyes ])
AM_CONDITIONAL(BUILD_KDE_GUI, [ test x$build_kde = xyes ])
AM_CONDITIONAL(BUILD_FLTK_GUI, [ test x$build_fltk = xyes ])
AM_CONDITIONAL(BUILD_SDL_GUI, [ test x$build_sdl = xyes ])
AM_CONDITIONAL(BUILD_FB_GUI, [ test x$build_fb = xyes ])
AM_CONDITIONAL(BUILD_AQUA_GUI, [ test x$build_aqua = xyes ])
AM_CONDITIONAL(BUILD_RISCOS_GUI, [ test x$build_riscos = xyes ])
AM_CONDITIONAL(BUILD_DUMP_GUI, [ test x$build_dump = xyes ])

# plugin building flags
AM_CONDITIONAL(NPAPI, [test x$npapi = xyes])
AM_CONDITIONAL(KPARTS, [test x$kparts = xyes])

AC_SUBST(KDE_CFLAGS)
AC_SUBST(KDE_LIBS)
if test x$build_ogl = xyes; then
  GNASH_PATH_OPENGL
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
AM_CONDITIONAL(HAS_KDE, [test x$has_kde = xyes])
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
  npapi=no
  AC_MSG_WARN([GTK2 specified for the GUI, but GtkGlExt is not present. Trying SDL instead.])
fi

missing_codecs=""
if test "$media_handler" = "gst"; then
  AC_PATH_PROG(GST_INSPECT, gst-inspect, ,[${pathlist}])
  if test "x$GST_INSPECT" != "x" -a x"${darwin}" = xno ; then
    AC_PATH_PROG(GST_INSPECT, gst-inspect-0.10, ,[${pathlist}])
  fi
  if test "x$GST_INSPECT" != "x" -a x"${darwin}" = xno ; then
    dnl FIXME: there may be multiple acceptable plugins that are acceptable for
    dnl our use. For example, mad or ffmpeg will play mp3.
    codecs="ffdec_flv ffdec_flashsv ffdec_vp6f ffdec_flashsv mad vorbisdec ffdec_vp6"
    for i in $codecs; do
       hits="`$GST_INSPECT $i | grep -c 'Long name'`"
       if test $hits -eq 0; then
         missing_codecs="$missing_codecs $i"
         AC_MSG_WARN([Missing codec: $i])
       fi
    done
  fi
  GNASH_PKG_FIND(gstreamer, [gst/gst.h], [gstreamer library], gst_init, [0.10])
  if test x"${media_handler_specified}" = xfalse; then
    # If the library is not found, or its version is not ok, we'll try mad
    if test x"$GSTREAMER_LIBS" = x; then
       AC_MSG_WARN([No appropriate gstreamer library found, will try using ffmpeg.])
       media_handler=ffmpeg
    fi
  fi
fi

if test x"$media_handler" = x"ffmpeg"; then
  GNASH_PATH_FFMPEG
  if test x"${media_handler_specified}" = xfalse; then
     # If the library is not found, or its version is not ok, we'll try gst
     if test x"${ac_cv_path_ffmpeg_lib}" = x -o x"${ffmpeg_version}" != xok; then
       AC_MSG_WARN([No appropriate ffmpeg library found, disabling media handling.])
       media_handler=none
     fi
  fi
fi

#if test x"$media_handler" = x"mad"; then
#  dnl Will set MAD_LIBS and MAD_CFLAGS.
#  dnl Handles --with-mad-incl and --with-mad-lib
#  GNASH_PKG_FIND(mad, [mad.h], [mad library], mad_copyright)
#  if test x"${media_handler_specified}" = xfalse; then
#    # If the library is not found, we'll disable media handling
#    if test x"${ac_cv_path_mad_lib}" = x ; then
#      AC_MSG_WARN([No libmad found, disabling media handling.])
#      media_handler=none
#    fi
#  fi
#fi

if test x"$media_handler" = x"gst"; then
  AC_MSG_CHECKING([for modern pbutils])
  save_cflags=$CFLAGS
  CFLAGS=$GSTREAMER_CFLAGS
  AC_TRY_COMPILE([#include <gst/pbutils/install-plugins.h>],[void* pointer = &gst_install_plugins_async],
    GSTREAMER_LIBS="$GSTREAMER_LIBS -lgstpbutils-0.10",
    has_modern_gstpbutils='no')
  AC_TRY_COMPILE([#include <gst/pbutils/install-plugins.h>],[void* pointer = &gst_install_plugins_supported],
    AC_DEFINE([HAVE_GST_INSTALL_PLUGINS_SUPPORTED], [], [GStreamer provides the gst_install_plugins_supported function]),
    has_gst_install_plugins_supported='no')
  CFLAGS=$save_cflags
  if test x$has_modern_gstpbutils = xno; then
    AC_MSG_RESULT([no {GSTREAMER_CFLAGS=$GSTREAMER_CFLAGS}])
  else
    AC_DEFINE([GST_HAS_MODERN_PBUTILS], [], [GStreamer has pbutils with missing codec install routines installed])
    AC_MSG_RESULT([ok])
  fi
fi



if test x$build_cairo = xyes; then
  GNASH_PKG_FIND(cairo, [cairo.h], [cairo render library], cairo_status)
dnl   if test x"${CAIRO_CFLAGS}" != x; then
dnl      CAIRO_CFLAGS=`echo ${CAIRO_CFLAGS} | sed -e 's/ *//'`
dnl      CAIRO_CFLAGS="${CAIRO_CFLAGS} ${CAIRO_CFLAGS}/cairo"
dnl   fi
  if test x$build_fb = xyes; then
    AC_MSG_ERROR([Cairo renderer is not supported by FB gui. Use --enable-renderer=AGG or --enable-gui=gtk,sdl]);
  fi
  if test x$build_sdl = xyes; then
    AC_MSG_ERROR([Cairo renderer is not supported by SDL gui. Use --enable-renderer=AGG or --enable-gui=gtk,sdl]);
  fi
  if test x$build_fltk = xyes; then
    AC_MSG_ERROR([Cairo renderer is not supported by FLTK gui. Use --enable-renderer=AGG or --enable-gui=gtk,sdl]);
  fi
  if test x$build_kde = xyes; then
    AC_MSG_ERROR([Cairo renderer is not supported by KDE gui. Use --enable-renderer=AGG or --enable-gui=gtk,sdl]);
  fi
  if test x$build_dump = xyes; then
    AC_MSG_ERROR([Cairo renderer is not supported by Dump gui. Use --enable-renderer=AGG or --enable-gui=gtk,sdl]);
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
  ffmpeg) AC_DEFINE([SOUND_SDL],  [1], [Use SDL for sound handling]) ;;
  *)
esac

case "${media_handler}" in
  ffmpeg)  AC_DEFINE([USE_FFMPEG],  [1], [Use FFMPEG for media decoding]) ;;
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
AM_CONDITIONAL(ENABLE_INFO, test x${INSTALL_INFO} != x)
AM_CONDITIONAL(DOCBOOK, test x$docbook = xyes)
AM_CONDITIONAL(ENABLE_TEXI, [ test x"$DB2X_TEXI" != x -o x"$DB2X_TEXIXML" != x ])
AM_CONDITIONAL(ENABLE_PDF, [ test x"$DB2X_PDF" ])
AM_CONDITIONAL(ENABLE_HTML, [ test x"$XSLTPROC" != x -a x"$docbook_styles" != x ])
AM_CONDITIONAL(ENABLE_FOP, [ test x"$FOP" != x -a x"$docbook_styles" != x ])
AM_CONDITIONAL(ENABLE_XMLTEX, [ test x"$PDFXMLTEX" != x -a x"$XSLTPROC" != x -a x"$docbook_styles" != x ])
dnl  AM_CONDITIONAL(ENABLE_DBLATEX, [ test x"$DBLATEX" != x ])
AM_CONDITIONAL(ENABLE_MAN, [ test x"$DB2X_MAN" != x -o x"$DB2X_MANXML" != x ])
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

GNASH_PATH_BOOST
dnl AX_GCC_ARCHFLAG(no)

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
# It should be possible to build with -ansi, not with
# -pedantic because of agg.
if test x"$GCC" = x"yes"; then
  CXXFLAGS="$CXXFLAGS \
    -W \
    -Wall \
    -Wcast-align \
    -Wcast-qual \
    -Wpointer-arith \
    -Wreturn-type \
    -Wnon-virtual-dtor \
    -Wunused \
    #-ansi \
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
      -Wextra \   
      -pedantic \
      -Wno-long-long \
      "

    CFLAGS="$CFLAGS \
      -pedantic \
      -Wno-long-long \
      -ansi \
      "
  fi
fi

AC_ARG_ENABLE([cassert],
  AC_HELP_STRING([--disable-cassert],[Disable assertion checking]),
  [case "${enableval}" in
    yes) cassert=yes ;;
    no) cassert=no ;;
    *) AC_MSG_ERROR([bad value ${enableval} for --enable-cassert option]) ;;
  esac],
  [cassert=yes]
)

if test x"$cassert" = x"no"; then
    CXXFLAGS="$CXXFLAGS \
      -DNDEBUG \
      "
    CFLAGS="$CFLAGS \
      -DNDEBUG \
      "
fi

dnl /* http://gcc.gnu.org/bugzilla/show_bug.cgi?id=19664 */
AC_DEFUN([CHECK_VISIBILITY_GCC_BUG],
  [
    AC_CACHE_CHECK([if -fvisibility-inlines-hidden is broken], gcc_visibility_bug, [
        AC_LANG_PUSH(C++)
        save_CXXFLAGS=$CXXFLAGS
        save_LDFLAGS=$LDFLAGS
        CXXFLAGS="-fPIC -fvisibility-inlines-hidden -O0"
        LDFLAGS="$LDFLAGS -shared -fPIC"

        AC_TRY_LINK(
        [          
          template<typename CharT>
          struct VisTest
          {
            inline VisTest ();
          };
          template<typename CharT>
          inline VisTest<CharT>::VisTest()
        {}
        extern template class VisTest<char>;  // It works if we drop that line
        int some_function( int do_something ) __attribute__((visibility("default")));
	int some_function( int )
          {
            VisTest<char> a;
            return 0;
          }
        ], [],
        gcc_visibility_bug=no, gcc_visibility_bug=yes)

        CXXFLAGS=$save_CXXFLAGS
        LDFLAGS=$save_LDFLAGS
        AC_LANG_POP(C++)
      ]
    )
    if test x$gcc_visibility_bug = xno; then
      CXXFLAGS="$CXXFLAGS -fvisibility-inlines-hidden"
      if test x"$enable_visibility" != x"no"; then
        CXXFLAGS="$CXXFLAGS -fvisibility=hidden"
      fi
    fi
  ]
)

CHECK_VISIBILITY_GCC_BUG

dnl Define convienience constants so Gnash can print out the
dnl default configuration of the build.
RENDERER_CONFIG=${add_renderer}
RENDERER_LIBS=
for rend in `echo "${add_renderer}" | tr ',' ' '`; do
	RENDERER_LIBS="${RENDERER_LIBS} \$(top_builddir)/backend/libgnash${rend}.la"
done
dnl echo "RENDERER_LIBS=$RENDERER_LIBS"
AC_SUBST(RENDERER_LIBS)
AC_SUBST(RENDERER_CONFIG)


dnl check for missing libraries and disable them.
if test x"$BOOST_LIBS" != x; then
  if test x"${extra_missing_libs}" != x; then
    for i in ${extra_missing_libs}; do
      if test $i = serialization; then
        AC_DEFINE([BOOST_MULTI_INDEX_DISABLE_SERIALIZATION], ["1"], [if the library is missing, don't use it.])
      fi
    done
  fi
fi


SUPPORTED_GUIS=
if test x$build_qtopia = xyes; then
  SUPPORTED_GUIS="${SUPPORTED_GUIS},qtopia"
fi
if test x$build_hildon = xyes; then
  SUPPORTED_GUIS="${SUPPORTED_GUIS},hildon"
fi
if test x$build_alp = xyes; then
  SUPPORTED_GUIS="${SUPPORTED_GUIS},alp"
fi
if test x$build_fb = xyes; then
  SUPPORTED_GUIS="${SUPPORTED_GUIS},fb"
fi
if test x$build_fltk = xyes; then
  SUPPORTED_GUIS="${SUPPORTED_GUIS},fltk"
fi
if test x$build_kde = xyes; then
  SUPPORTED_GUIS="${SUPPORTED_GUIS},kde"
fi
if test x$build_gtk = xyes; then
  SUPPORTED_GUIS="${SUPPORTED_GUIS},gtk"
fi
if test x$build_sdl = xyes; then
  SUPPORTED_GUIS="${SUPPORTED_GUIS},sdl"
fi
if test x$build_riscos = xyes; then
  SUPPORTED_GUIS="${SUPPORTED_GUIS},riscos"
fi
if test x$build_aqua = xyes; then
  SUPPORTED_GUIS="${SUPPORTED_GUIS},aqua"
fi
if test x$build_dump = xyes; then
  SUPPORTED_GUIS="${SUPPORTED_GUIS},dump"
fi
SUPPORTED_GUIS="`echo ${SUPPORTED_GUIS} | sed 's/,//'`" # Strip leading comma
AC_SUBST(SUPPORTED_GUIS)

MEDIA_CONFIG=${media_handler}
AC_SUBST(MEDIA_CONFIG)

dnl AC_CONFIG_LINKS(doc/C/images)
dnl AC_CONFIG_LINKS(gnashconfig.h,libltdl/config.h)
AC_CONFIG_LINKS(cygnal/testsuite/cygnal.all/cygnalrc:cygnal/testsuite/cygnal.all/cygnalrc.in)
AC_CONFIG_LINKS(testsuite/libbase/gnashrc:testsuite/libbase/gnashrc.in)
AC_CONFIG_LINKS(testsuite/libbase/gnashrc-local:testsuite/libbase/gnashrc-local.in)

AC_OUTPUT(Makefile
po/Makefile
libmedia/Makefile
libbase/Makefile
libcore/Makefile
libcore/asobj/Makefile
libcore/vm/Makefile
libcore/parser/Makefile
libnet/Makefile
libamf/Makefile
backend/Makefile
utilities/Makefile
doc/Makefile
doc/C/Makefile
doc/Doxyfile
pythonmodule/Makefile
testsuite/Makefile
testsuite/libbase/Makefile
testsuite/actionscript.all/Makefile
testsuite/samples/Makefile
testsuite/swfdec/Makefile
testsuite/misc-ming.all/Makefile
testsuite/misc-mtasc.all/Makefile
testsuite/misc-haxe.all/Makefile
testsuite/misc-swfmill.all/Makefile
testsuite/misc-swfc.all/Makefile
testsuite/movies.all/Makefile
testsuite/libcore.all/Makefile
testsuite/libamf.all/Makefile
testsuite/libnet.all/Makefile
gui/Makefile
gui/Info.plist
extensions/Makefile
extensions/dejagnu/Makefile
extensions/mysql/Makefile
extensions/fileio/Makefile
extensions/gtk2/Makefile
extensions/lirc/Makefile
extensions/dbus/Makefile
extensions/metome/Makefile
plugin/Makefile
plugin/xpcom/Makefile
plugin/klash/Makefile
plugin/mozilla-sdk/Makefile
plugin/win32/Makefile
cygnal/Makefile
cygnal/testsuite/Makefile
cygnal/testsuite/cygnal.all/Makefile
)

###
########################## Final report begins... ############################
###

cerr=/tmp/gnash-configure-errors.$$
cwarn=/tmp/gnash-configure-warnings.$$
echo ""

#trap 'rm cerr' 0 # trap isn't a good idea, might override other traps
exec 3> $cerr 
exec 4> $cwarn

echo "Configurable options are:"

if test x"$i810lodbias" = x"yes"; then
  echo "        Intel 810 LOD bias hack enabled"
else
  echo "        Intel 810 LOD bias hack disabled (default). Use --enable-i810-lod-bias to enable."
fi

if test x"$xmitshm" = x"yes"; then
  echo "        MIT-SHM X extension support enabled"
else
  echo "        MIT-SHM X extension support disabled (default). Use --enable-mit-shm to enable."
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

if test x"$npapi" = x"yes"; then
  echo "        NPAPI plugin enabled (default). Use --disable-npapi to disable."
  echo "            NPAPI plugin will be installed in ${FIREFOX_PLUGINS}"
else
  echo "        NPAPI plugin disabled."
fi

if test x"$kparts" = x"yes"; then
  echo "        KPARTS plugin enabled (default). Use --disable-kparts to disable"
  echo "            KPARTS plugin will be installed in ${KDE_PLUGINDIR}"
  echo "            KPARTS service will be installed in ${KDE_SERVICESDIR}"
  echo "            KPARTS config dir will be in ${KDE_CONFIGDIR}"
  echo "            KPARTS appsdata will be installed in ${KDE_APPSDATADIR}"
else
  echo "        KPARTS plugin disabled."
fi

dnl -- if test x"${dynamic_gui}" = x"yes"; then
dnl --   echo "        Loadable GUI & renderer enabled."
dnl -- else
dnl --   echo "        Loadable GUI & renderer disabled (default). Use --enable-dynamic-gui to enable."
dnl -- fi

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
  echo "        WARNING: Pixel format RGB565 selected in combination with the" >&4
  echo "                 GTK GUI. Only a hacked GTK will work (e.g. on the OLPC)." >&4
fi

if test x"$extensions_list" != x; then
  echo "        Building extensions: ${extensions_list}"
fi

if test x"$security_list" != x; then
  echo "        Enabling security features: ${security_list}"
fi

if test x"$statistics_list" != x; then
  echo "        Enabling statistics collecting for: ${statistics_list}"
fi

if test x"${SUPPORTED_GUIS}" = x; then
  AC_MSG_ERROR(no supported GUIs found);
fi

echo "        GUI toolkits supported: ${SUPPORTED_GUIS}"
echo "        Renderers supported: ${add_renderer}"
echo "        Media handler: "$media_handler
if test x"$media_handler" = xffmpeg -o x"$media_handler" = xmad; then
echo "        Using SDL for sound handling"
fi
echo "        Using $with_shm mode for shared memory"
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

if test x$xml != xno; then
  if test x"$LIBXML_CFLAGS" != x; then
    echo "        XML flags are: $LIBXML_CFLAGS"
    echo "        XML libs are: $LIBXML_LIBS"
  else
    echo "        ERROR: No libxml2 development package installed!" >&3
    echo "               To compile this project install" >&3
    echo "               or .deb users: apt-get install libxml2-dev" >&3
    echo "               or .rpm users: yum install libxml2-devel" >&3
  fi
fi

if test x$build_kde = xyes -o x$kparts = xyes; then
  if test x"$has_kde" = xyes; then
    echo "        KDE flags are: $KDE_CFLAGS"
    echo "        KDE libs are: $KDE_LIBS"
  else
      echo "        ERROR: No KDE development package installed!" >&3
      echo "               To disable the KDE gui," >&3
      echo "               reconfigure using --enable-gui=<list-of-guis>" >&3
      echo "               and omit "kde" from the list." >&3
      echo "               When the option --enable-gui=... is omitted," >&3
      echo "               the default is the same of --enable-gui=kde,gtk" >&3
      echo "               To be able to build the kde gui," >&3
      echo "               install the KDE development environment from http://kde.org" >&3
      echo "               or .deb users: apt-get install kdelibs-dev" >&3
      echo "               or .rpm users: yum install <something-else>." >&3
  fi
  if test x"$has_qt" = xyes; then
    echo "        QT flags are: $QT_CFLAGS"
    echo "        QT libs are: $QT_LIBS"
  else
    if test x$build_kde = xyes -o x$kparts = xyes ; then
      if test x"$has_kde" != xyes; then
	# We have already moaned about KDE absence, so presumably they'll get
	# QT-dev when they install KDE-dev
	:
      else
	# This should never happen unless they have a very odd version of KDE
        echo "        ERROR: No QT development package installed!" >&3
        echo "               Install a QT development environment from http://trolltech.com" >&3
        echo "               or .deb users: apt-get install libqt3-mt-dev" >&3
	if test x$kparts = xyes; then
          echo "               or reconfigure with --disable-kparts" >&3
	fi
	if test x$build_kde = xyes; then
          echo "               or change to a different gui with --enable-gui=..." >&3
	fi
      fi
    fi
  fi
fi

if test x"$build_qtopia" = xyes; then
  if test x"${QTOPIA_LIBS}" != x ; then
    if test x"${QTOPIA_CFLAGS}" != x ; then
      echo "        QTOPIA flags are: $QTOPIA_CFLAGS"
    else
      echo "        QTOPIA flags are: default include path"
    fi
    echo "        QTOPIA libs are: $QTOPIA_LIBS"
  else
    echo "        ERROR: No QTOPIA library development package installed!" >&3
    echo "               Install it from http://trolltech.com/downloads/" >&3
    echo "               binary packages are not available." >&3
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
  echo "        ERROR: No JPEG library development package installed!" >&3
  echo "               Install it from http://ijg.org" >&3
  echo "               or .deb users: apt-get install libjpeg-dev" >&3
  echo "               or .rpm users: yum install libjpeg-devel" >&3
fi

if test x"${GIF_LIBS}" != x ; then
  if test x"${GIF_CFLAGS}" != x ; then
    echo "        GIF flags are: $GIF_CFLAGS"
  else
    echo "        GIF flags are: default include path"
  fi
  echo "        GIF libs are: $GIF_LIBS"
else
  echo "        ERROR: No GIF library development package installed!" >&3
  echo "               Install it from http://sourceforge.net/projects/giflib/" >&3
  echo "               or .deb users: apt-get install libungif-dev" >&3
  echo "               or maybe     : apt-get install libgif-dev" >&3
  echo "               or .rpm users: yum install libungif-devel" >&3
fi

if test x"${LIBPNG_LIBS}" != x ; then
  if test x"${LIBPNG_CFLAGS}" != x ; then
    echo "        PNG flags are: $LIBPNG_CFLAGS"
  else
    echo "        PNG flags are: default include path"
  fi
  echo "        PNG libs are: $LIBPNG_LIBS"
else
  echo "        WARNING: No PNG library development package installed!" >&4
  echo "                 Gnash will be built without support for dynamic loading of PNG files." >&4
  echo "                 Install it from http://www.libpng.org" >&4
  echo "                 or .deb users: apt-get install libpng12-dev" >&4
  echo "                 or .rpm users: yum install libpng-devel" >&4
fi

dnl if test x"$OGG_LIBS" != x; then
dnl   if test x"$OGG_CFLAGS" != x; then
dnl     echo "        Ogg flags are: $OGG_CFLAGS"
dnl   else
dnl     echo "        Ogg flags are: default include path"
dnl   fi
dnl   echo "        Ogg libs are: $OGG_LIBS"
dnl else
dnl   echo "        ERROR: No Ogg Vorbis development package installed!" >&3
dnl fi

dnl if test x"$VORBIS_LIBS" != x; then
dnl   if test x"$VORBIS_CFLAGS" != x; then
dnl     echo "        VORBIS flags are: $VORBIS_CFLAGS"
dnl   else
dnl     echo "        VORBIS flags are: default include path"
dnl   fi
dnl   echo "        VORBIS libs are: $VORBIS_LIBS"
dnl else
dnl   echo "        ERROR: No VORBIS Vorbis development package installed!" >&3
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
      echo "        ERROR: No OpenGL development package installed!" >&3
      echo "               You need to install the libmesa development package" >&3
      echo "               or .deb users: apt-get install libgl1-mesa-dev" >&3
      echo "               or .rpm users: yum install xorg-x11-Mesa-libGL" >&3
      echo "               or use a different renderer with --enable-renderer=" >&3
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
        echo "        ERROR: No GtkGLExt development package installed!" >&3
	echo "               It is needed to build the GTK/OpenGL GUI/renderer combination." >&3
	echo "               Either install it from http://gtkglext.sourceforge.net" >&3
        echo "               or .deb users: apt-get install libgtkglext1-dev" >&3
        echo "               or .rpm users: yum install gtkglext-devel" >&3
        echo "               or --enable-gui=sdl or --enable-renderer=agg" >&3
      fi
    fi
  fi
fi


if test x$build_hildon = xyes; then #{
  if test x"$HILDON_LIBS" != x; then
    if test x"$HILDON_CFLAGS" != x; then
      echo "        HILDON flags are: $HILDON_CFLAGS"
    else
      echo "        HILDON flags are: default include path"
    fi
      echo "        HILDON libs are: $HILDON_LIBS"
  else
    echo "        ERROR: No HILDON development package installed!" >&3
    echo "               Install it from http://gtk.org" >&3
    #echo "               or .deb users: apt-get install libhildon.0-dev" >&3
    echo "               or .deb users: apt-get install libhildon-1-dev" >&3
    echo "               or .rpm users: yum install hildon-devel" >&3
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
    echo "        ERROR: No GTK2 development package installed!" >&3
    echo "               Install it from http://gtk.org" >&3
    echo "               or .deb users: apt-get install libgtk2.0-dev" >&3
    echo "               or .rpm users: yum install gtk2-devel" >&3
  fi

  if test x"$PANGO_LIBS" != x; then
    if test x"$PANGO_CFLAGS" != x; then
      echo "        Pango flags are: $PANGO_CFLAGS"
    else
      echo "        Pango flags are: default include path"
    fi
    echo "        Pango libs are: $PANGO_LIBS"
  else
    echo "        ERROR: No Pango development package installed!" >&3
    echo "               Install it from http://pango.org" >&3
    echo "               or .deb users: apt-get install libpango1.0-dev" >&3
    echo "               or .rpm users: yum install pango-devel" >&3
  fi

  if test x"$GLIB_LIBS" != x; then
    if test x"$GLIB_CFLAGS" != x; then
      echo "        GLib flags are: $GLIB_CFLAGS"
    else
      echo "        GLib flags are: default include path"
    fi
    echo "        GLib libs are: $GLIB_LIBS"
  else
    echo "        ERROR: No GLib development package installed!" >&3
    echo "               Install it from http://gtk.org" >&3
    echo "               or .deb users: apt-get install libglib2.0-dev" >&3
    echo "               or .rpm users: yum install glib2-devel" >&3
  fi

  if test x"$ATK_LIBS" != x; then
    if test x"$ATK_CFLAGS" != x; then
      echo "        ATK flags are: $ATK_CFLAGS"
    else
      echo "        ATK flags are: default include path"
    fi
      echo "        ATK libs are: $ATK_LIBS"
  else
    echo "        ERROR: No ATK development package installed!" >&3
    echo "               Install it from http://gtk.org" >&3
    echo "               or .deb users: apt-get install libatk1.0-dev" >&3
    echo "               or .rpm users: yum install atk-devel" >&3
  fi

fi

if test "$media_handler" = "gst"; then
  if test x"$missing_codecs" != x; then   
      echo "        Your Gstreamer installation is missing these codecs: $missing_codecs"
      echo "        Please install the gstreamer-ffmpeg for Gstreamer"
  fi  
  if test x"$GSTREAMER_LIBS" != x; then
    if test x"$GSTREAMER_CFLAGS" != x; then
      echo "        Gstreamer flags are: $GSTREAMER_CFLAGS"
    else
      echo "        Gstreamer flags are: default include path"
    fi
      echo "        Gstreamer libs are: $GSTREAMER_LIBS"
  else
    echo "        ERROR: GST media handling requested but gstreamer-0.10+ not found" >&3
    echo "               Install it from http://www.gstreamer.net" >&3
    echo "               or .deb users: apt-get install libgstreamer0.10-dev" >&3
    echo "               or .rpm users: yum install gstreamer-devel" >&3
    echo "               or             yast install gstreamer010-devel" >&3
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
      echo "        ERROR: No libmad development package installed!" >&3
      echo "               You can install it from http://mad.sourceforge.net" >&3
      echo "               or .deb users: apt-get install libmad0-dev" >&3
      echo "               or .rpm users: yum install libmad" >&3
      echo "               or reconfigure using --enable-media=ffmpeg" >&3
    fi
  fi

  if test x"$media_handler" = x"ffmpeg"; then
    if test x"$FFMPEG_LIBS" != x; then
      echo "        MP3 and video support enabled through ffmpeg"
      if test x"${ffmpeg_version}" != xok; then
        echo "        ERROR: You have version ${ffmpeg_version} of ffmpeg installed." >&3
        echo "               This version of ffmpeg may not work!" >&3
        echo "               Version 51.27.0 or newer is highly recommended." >&3
      else
	if test x"${avformat_h}" = x; then
          echo "        ERROR: FFMPEG's libavcodec header is installed but not libavformat." >&3
          echo "               You can install FFMPEG from http://ffmpeg.mplayerhq.hu" >&3
          echo "               or .deb users: apt-get install libavformat-dev" >&3
          echo "               or YaST users: yast -i libavformat-api (but currently installs into /usr/lib64)" >&3
          echo "               or explicitly set the path using --with-ffmpeg-incl=" >&5
          echo "               or reconfigure with --enable-media=gst" >&3
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
      echo "        ERROR: No FFMPEG development package installed!" >&3
      echo "               You can install FFMPEG from http://ffmpeg.mplayerhq.hu" >&3
      echo "               or .deb users: apt-get install libavformat-dev" >&3
      echo "               or .rpm users: yum install ffmpeg-devel" >&3
      echo "               or reconfigure with --enable-media=gst" >&3
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
    echo "        ERROR: No Cairo development package installed!" >&3
    echo "               You need to have the Cairo development package installed" >&3
    echo "               if you have used --enable-render=cairo to configure." >&3
    echo "               Install it from http://cairographics.org" >&3
    echo "               or .deb users: apt-get install libcairo-dev" >&3
    echo "               or .rpm users: yum install cairo-devel" >&3
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
    echo "        ERROR: No FLTK2 development package installed!" >&3
    echo "               There are currently no Debian or RPM packages for FLTK2;" >&3
    echo "		 see http://www.fltk.org to install it from source." >&3
dnl What it was for FLTK 1:
dnl    echo "               or .deb users: apt-get install fltk-1.1-dev"
dnl    echo "               or .rpm users: yum install fltk-devel"
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
dnl       echo "        ERROR: No Xft development package installed!" >&3
dnl       echo "               Install the xft development package" >&3
dnl       echo "               or .deb users: apt-get install libxft-dev" >&3
dnl       echo "               or .rpm users: yum install xft-devel" >&3
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
    echo "        ERROR: No SDL development package installed!" >&3
    echo "               Install it from http://www.libsdl.org/download-1.2.php" >&3
    echo "               or .deb users: apt-get install libsdl1.2-dev" >&3
    echo "               or .rpm users: yum install SDL-devel" >&3
    test x$build_sdl = xyes &&
	echo "               or select a different GUI with --enable-gui= " >&3
    test x"$media_handler" = x"ffmpeg" -o x"$media_handler" = x"mad" &&
	echo "               or use --enable-media=gst" >&3
  fi
fi
unset need_sdl

if test x"$nsapi" = x"yes"; then
dnl  if test x"$FIREFOX_CFLAGS" != x; then
dnl    echo "        Firefox flags are: $FIREFOX_CFLAGS"
dnl    echo "        Firefox libs are: $FIREFOX_LIBS"
    echo "        Plugin will be installed in ${FIREFOX_PLUGINS}"
dnl  else
dnl    echo "        ERROR: No Firefox or Mozilla development package installed!" >&3
dnl  fi
fi

if test x"$pthreads" = x"yes"; then
  if test x"$PTHREAD_CFLAGS" != x; then
    echo "        POSIX Threads flags are: $PTHREAD_CFLAGS"
  fi
  if test x"$PTHREAD_LIBS" != x; then
    echo "        POSIX Threads lib is: $PTHREAD_LIBS"
  else
    echo "ERROR: No pthread development package installed!" >&3
  fi
fi

if test x"$docbook" = x"yes"; then
  if test x"$MAKEINFO" = x; then
    echo "        ERROR: no makeinfo tools installed!" >&3
    echo "               Either install it from http://www.gnu.org/software/texinfo/" >&3
    echo "               or .deb users: apt-get install texinfo" >&3
    echo "               or configure without --enable-docbook" >&3
  fi
  dnl low-level tools
  if test x"$DB2X_TEXIXML" = x -o x"$DB2X_MANXML" = x -o x"$DB2X_XSLTPROC" = x; then
    dnl high-level tools
    if test x"${DB2X_TEXI}" = x -o x"${DB2X_MAN}" = x; then
      echo "        ERROR: No DocBook2X tools installed!" >&3
      echo "               Either install it from http://docbook2x.sourceforge.net" >&3
      echo "               or .deb users: apt-get install docbook docbook2x docbook-utils" >&3
      echo "                              docbook-xml docbook-xsl texinfo xsltproc" >&3
      echo "               or configure without --enable-docbook" >&3
    fi
  else
    echo "        You have version $db2x_version of the DocBook2X tools."
  fi
fi

if test x"$CURL_LIBS" != x; then
  if test x"$CURL_CFLAGS" != x; then
    echo "        CURL flags are: $CURL_CFLAGS"
  else
    echo "        CURL flags are: default include path"
  fi
    echo "        CURL libs are: $CURL_LIBS"
else
  echo "        WARNING: CURL library not found." >&4
  echo "                 Gnash will be built without support for streaming from URLs." >&4
  echo "                 Why not install libcurl from http://curl.haxx.se/libcurl" >&4
  echo "                 or .deb users: apt-get install libcurl3-dev" >&4
  echo "                 or .rpm users: yum install curl-devel" >&4
fi

if test x"$ext_dbus" = xyes; then
  if test x"$DBUS_LIBS" != x; then
    if test x"$DBUS_CFLAGS" != x; then
      echo "        DBUS flags are: $DBUS_CFLAGS"
    else
      echo "        DBUS flags are: default include path"
    fi
      echo "        DBUS libs are: $DBUS_LIBS"
  else
    echo "        WARNING: DBUS library not found." >&4
    echo "                 Gnash will be built without support for remote controls." >&4
    echo "                 Why not install libdbus from http://www.dbus.org" >&4
    echo "                 or .deb users: apt-get install dbus-dev" >&4
    echo "                 or .rpm users: yum install dbus-devel" >&4
  fi
fi

if test x$build_agg = xyes; then # {
  echo "        AGG Pixel format is: $pixelformat"
    if test x"$AGG_LIBS" != x; then # {
      if test x"${agg25}" != xyes; then # {
        echo "        ERROR: Your installation of AGG appears to be version 2.4 or older." >&3
        echo "               Please upgrade to AGG 2.5 or greater." >&3
        echo "               Install it from http://www.antigrain.com" >&3
        echo "               or .deb users: apt-get install libagg-dev" >&3
        echo "               or .rpm users: yum install agg-devel" >&3
      else # }{
        if test x"$AGG_CFLAGS" != x; then # {
          echo "        AGG flags are: $AGG_CFLAGS"
        else # }{
          echo "        AGG flags are: default include path"
        fi # }
        echo "        AGG libs are: $AGG_LIBS"
      fi # }
    else # }{
      echo "        ERROR: No AGG development package installed!" >&3
      echo "               Install it from http://www.antigrain.com" >&3
      echo "               or .deb users: apt-get install libagg-dev" >&3
      echo "               or .rpm users: yum install agg-devel" >&3
    fi # }
fi # }

if test x"$BOOST_LIBS" != x; then
    dnl Only cygnal requires date_time at present, so it's OK if either
    dnl you don't want cygnal or do have date_time installed.
    dnl if test x"$cygnal" = xno; then
	echo "        BOOST flags are: $BOOST_CFLAGS"
	echo "        BOOST libs are: $BOOST_LIBS"
	echo "        BOOST Extra libs are: $BOOST_EXTRA_LIBS"
    dnl fi
    if test x"${missing_headers}" != x; then
      for i in ${missing_headers}; do
	# They have some boost libs but no date_time and want to compile cygnal.
	echo "        ERROR: The BOOST $i header file is needed!" >&3
	echo "               Install it from http://boost.org" >&3
	echo "               or from a Boost development package" >&3
      done
    fi
    if test x"${extra_missing_libs}" != x; then
      for i in ${extra_missing_libs}; do
	      echo "        WARNING: The BOOST $i library is recommended!" >&4
        echo "               Install it from http://www.boost.org" >&4
        echo "               or .deb users: apt-get install libboost-"`echo ${i} | sed 's/_/-/g'`"-dev" >&4
        echo "               Gnash will compile anyway, but not all tests will work." >&4
      done
    fi
    if test x"${missing_libs}" != x; then
      for i in ${missing_libs}; do
	# They have some boost libs but no date_time and want to compile cygnal.
	echo "        ERROR: The BOOST $i library is needed!" >&3
	echo "               Install it from http://boost.org" >&3
	echo "               or .deb users: apt-get install libboost-"`echo ${i} | sed 's/_/-/g'`"-dev" >&3
      done
    fi
else
    echo "        ERROR: No BOOST development package installed!" >&3
    echo "               Install it from http://www.boost.org" >&3
    echo "               or .deb users: apt-get install libboost-dev libboost-thread-dev" >&3
    if test x"$cygnal" = xyes; then
    echo "                              and libboost-date-time-dev (for cygnal)" >&3
    fi
    echo "               or .rpm users: yum install boost-devel" >&3
fi

dnl don't look for the flash compilers when cross compiling.
if test x$cross_compiling = xno; then
  if test x"$MING_LIBS" != x; then
    echo "        MING flags are $MING_CFLAGS"
    echo "        MING libs are $MING_LIBS"
  else
    echo "        WARNING: You need to have the Ming development package" >&4
    echo "                 installed to run most of the tests in Gnash testsuite." >&4
    echo "                 Install it from http://www.libming.org/" >&4
    echo "                 or .deb users: apt-get install libming-dev" >&4
  fi

  if test x"$MAKESWF" != x; then
    echo "        MING version code is $MING_VERSION_CODE"
    echo "        MAKESWF is $MAKESWF"
  else
    echo "        WARNING: You need to have the Ming utilities package" >&4
    echo "                 installed to run most of the tests in Gnash testsuite." >&4
    echo "                 Install it from http://www.libming.org" >&4
    echo "                 or .deb users: apt-get install libming-util" >&4
  fi

  if test x"$MAKESWF" != x && test $MING_VERSION_CODE -lt 00040006; then
    echo "        WARNING: You have an older version of Ming installed and will not" >&4
    echo "                 be able to run all of the tests in Gnash testsuite." >&4
    echo "                 Install the latest version from http://www.libming.org" >&4
  fi

  if test x"$SWFDEC_TESTSUITE" != x; then
    echo "        SWFDEC testsuite dir is $SWFDEC_TESTSUITE"
  fi

  if test x"$MTASC" != x; then
    echo "        MTASC is $MTASC"
    echo "        MTASC CLASSPATH is $MTASC_CLASSPATH"
  else
    echo "        WARNING: You need to have the MTASC compiler packages installed" >&4
    echo "                 to run some of the tests in Gnash testsuite." >&4
    echo "                 You can install it from http://mtasc.org" >&4
    echo "                 or .deb users: apt-get install mtasc" >&4
  fi

  if test x"$HAXE" != x; then
    echo "        HAXE is $HAXE"
    echo "        HAXE CLASSPATH is $HAXE_CLASSPATH"
  else
    echo "        WARNING: You need to have the HAXE compiler packages installed" >&4
    echo "                 to run some of the tests in Gnash testsuite." >&4
    echo "                 You can install it from http://haxe.org" >&4
    echo "                 or .deb users: apt-get install haxe (version 2 or later required)" >&4
  fi

  if test x"$SWFMILL" != x; then
    echo "        SWFMILL is $SWFMILL"
  else
    echo "        WARNING: You need to have the 'swfmill' tool installed" >&4
    echo "                 to run some of the tests in Gnash testsuite." >&4
    echo "                 You can install it from http://swfmill.org/" >&4
  fi

  if test x"$SWFC" != x; then
    echo "        SWFC is $SWFC"
  else
    echo "        WARNING: You need to have 'swfc' from SWFTools installed" >&4
    echo "                 to run some of the tests in Gnash testsuite." >&4
    echo "                 You can install it from http://www.swftools.org/" >&4
    echo "                 or .deb users: apt-get install swftools" >&4
  fi

  if test x"$PYTHON" != x; then
    echo "        PYTHON is $PYTHON"
  else
    echo "        WARNING: You need to have python installed" >&4
    echo "                 to run some of the tests in Gnash testsuite." >&4
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
  echo "        WARNING: You need to have the zlib development packages installed" >&4
  echo "                 to play compressed SWF (most of them from version 6 up)" >&4
  echo "                 and to display some kinds of JPEG files." >&4
  echo "                 Install it from http://www.zlib.net" >&4
  echo "                 or .deb users: apt-get install zlib1g-dev" >&4
  echo "                 or .rpm users: yum install zlib-dev." >&4
  echo "                 It may still be possible to configure without zlib." >&4
fi

if test x"$FREETYPE2_LIBS" != x; then
  if test x"$FREETYPE2_CFLAGS" != x; then
    echo "        FreeType flags are: $FREETYPE2_CFLAGS"
  else
    echo "        FreeType flags are: default include path"
  fi
  echo "        FreeType libs are: $FREETYPE2_LIBS"
else
  echo "        WARNING: You need to have the freetype development packages installed" >&4
  echo "                 to use device fonts." >&4
  echo "                 Install it from http://www.freetype.org" >&4
  echo "                 or .deb users: apt-get install libfreetype6-dev" >&4
  echo "                 or .rpm users: yum install libfreetype6-dev (??)" >&4
  echo "                 It may still be possible to configure without freetype." >&4
fi

if test x"$FONTCONFIG_LIBS" != x; then
  if test x"$FONTCONFIG_CFLAGS" != x; then
    echo "        Fontconfig flags are: $FONTCONFIG_CFLAGS"
  else
    echo "        Fontconfig flags are: default include path"
  fi
  echo "        Fontconfig libs are: $FONTCONFIG_LIBS"
else
  echo "        WARNING: You need to have the freetype development packages installed" >&4
  echo "                 to use device fonts." >&4
  echo "                 Install it from http://www.fontconfig.org" >&4
  echo "                 or .deb users: apt-get install libfontconfig1-dev" >&4
  echo "                 or .rpm users: yum install ??" >&4
  echo "                 It may still be possible to configure without fontconfig." >&4
fi

if test x$ext_mysql = xyes; then
  if test x$mysql != xno; then
    if test x"$MYSQL_LIBS" != x; then
      echo "        MYSQL flags are: $MYSQL_CFLAGS"
      echo "        MYSQL libs are: $MYSQL_LIBS"
    else
      echo "        ERROR: No MySQL development package is installed." >&3
      echo "               Either reconfigure without --with-extensions=mysql" >&3
      echo "               or install MySQL header files from http://www.mysql.org" >&3
      echo "               or .deb users: apt-get install libmysqlclient-dev" >&3
    fi
  fi
fi

if test "$GMSGFMT" = ":"; then
  echo "        WARNING: You need the gettext package installed to use translations." >&4
  echo "                 Required for building a package or 'make distcheck'" >&4
  echo "                 Install it from http://www.gnu.org/software/gettext/" >&4
  echo "                 or .deb users: apt-get install gettext" >&4
  #echo "                or .rpm users: yum install gettext" >&4
fi

if test x"${DEJAGNU}" != x""; then
  echo "        DEJAGNU's runtest is $DEJAGNU"
else
  echo "        WARNING: You need the dejagnu package installed to get a summary" >&4
  echo "                 report after running ``make check''" >&4
  echo "                 Install it from http://www.gnu.org/software/dejagnu/" >&4
  echo "                 or .deb users: apt-get install dejagnu" >&4
  #echo "                or .rpm users: yum install dejagnu" >&4
fi


dnl Access Linux Platform
if test x"${build_alp}" = xyes; then
  if test x"${ALP_CFLAGS}" != x; then
    echo "        ALP flags are: $ALP_CFLAGS"
    echo "        ALP libs are: $ALP_LIBS"
  else
    echo "        ERROR: No ALP development package is installed." >&3
  fi
fi

dnl XPCOM support for Mozilla/Firefox
if test x"$xpcom" = x"yes"; then
  if test x"$has_xpcom" = x"yes"; then
    echo "        XPCOM flags are: $XPCOM_CFLAGS"
    echo "        XPCOM libs are: $XPCOM_LIBS"
    echo "        XPIDL path is: $XPIDL"
  else
    echo "        ERROR: No XPCOM development package is installed." >&3
    echo "                Install it from http://www.mozilla.org/projects/xpcom/" >&3
    echo "                or .deb users: apt-get install xulrunner-1.9-dev" >&3
  fi
fi
if test x"$nspr4" = x"yes"; then
  if test x"$has_nspr" = x"yes"; then
    echo "        NSPR flags are: $NSPR_CFLAGS"
    echo "        NSPR libs are: $NSPR_LIBS"
  else
    echo "        ERROR: No NSPR development package is installed." >&3
  fi
fi

if test x$cross_compiling = xyes; then
  AC_MSG_NOTICE([This build is setup for cross compiling])
fi

if test -s $cwarn; then
  echo ""
  cat $cwarn
  rm $cwarn
  echo ""
  echo "Gnash should still compile even with these warnings.  If it doesn't,"
  echo "report the warnings as a bug."
fi

dnl If anything critical is missing, don't bother to continue
if test -s $cerr; then
  echo ""
  cat $cerr
  rm $cerr
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
