dnl  
dnl  Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

AC_PREREQ(2.59c)
AC_INIT(gnash, 0.8.10dev)
AC_CONFIG_SRCDIR([libcore/as_object.h])
AC_CONFIG_HEADERS([gnashconfig.h])
AC_CONFIG_MACRO_DIR([macros])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

dnl --------------------------------------------------------
dnl Figure out development tool stuff
dnl --------------------------------------------------------

AC_PROG_CXX
AC_PROG_CC
AC_EXEEXT
AC_PROG_INSTALL

dnl Set the default values for Flash Version. These are converted into
dnl various strings to make JavaScript or ActionScript detectors
dnl recognize Gnash as a SWF Player.
DEFAULT_FLASH_MAJOR_VERSION="10"
DEFAULT_FLASH_MINOR_VERSION="1"
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
  *-freebsd* | *-kfreebsd*)
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
  *-amigaos*)
    DEFAULT_FLASH_PLATFORM_ID="OS4"
    DEFAULT_FLASH_SYSTEM_OS="AmigaOS4"
    ;;    
  *-haiku*)
    DEFAULT_FLASH_PLATFORM_ID="HAIKU"
    DEFAULT_FLASH_SYSTEM_OS="Haiku"
    ;;    
  *-gnu*)
    DEFAULT_FLASH_PLATFORM_ID="GNU"
    DEFAULT_FLASH_SYSTEM_OS="GNU/HURD"
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

DEFAULT_STREAMS_TIMEOUT=60
AC_SUBST(DEFAULT_STREAMS_TIMEOUT)
AC_DEFINE_UNQUOTED([DEFAULT_STREAMS_TIMEOUT], [${DEFAULT_STREAMS_TIMEOUT}], [Default streams timeout in seconds])

DEFAULT_SOL_SAFEDIR="~/.gnash/SharedObjects"
AC_SUBST(DEFAULT_SOL_SAFEDIR)
AC_DEFINE_UNQUOTED([DEFAULT_SOL_SAFEDIR], ["${DEFAULT_SOL_SAFEDIR}"], [Default SharedObject base directory])


dnl Some things you can only do by looking at the platform name.
case "${host}" in
  powerpc-apple-darwin*)
    AC_DEFINE([__powerpc64__], [1], [this is a 64 bit powerpc])
    darwin=yes
    AC_DEFINE([DARWIN_HOST], [1], [this is a Darwin platform])
    ;;
  *-apple-darwin*)
    darwin=yes
    AC_DEFINE([DARWIN_HOST], [1], [this is a Darwin platform])
    ;;
    dnl Unfortunately, all BSD distributions are not identical, so 
    dnl as tacky as it is to look for the distribution name, we don't
    dnl have much choice. The use of these should be avoid as much as possible.
  *-openbsd*)
    bsd=yes
    openbsd=yes
    AC_DEFINE([OPENBSD_HOST], [1], [this is an OpenBSD platform])
    ;;
  *-freebsd* | *-kfreebsd*)
    bsd=yes
    freebsd=yes
    AC_DEFINE([FREEBSD_HOST], [1], [this is a FreeBSD platform])
    ;;
  *-netbsd*)
    bsd=yes
    netbsd=yes
    AC_DEFINE([NETBSD_HOST], [1], [this is a NetBSD platform])
    ;;
  *-*solaris*)
    solaris=yes
    AC_DEFINE([SOLARIS_HOST], [1], [this is a Solaris platform])
    ;;
  *-*linux*)
    linux=yes
    AC_DEFINE([LINUX_HOST], [1], [this is a Linux platform])
    ;;
  *-cygwin* | *-mingw* | *-pw32*)
    windows=yes
    AC_DEFINE([WIN32_HOST], [1], [this is a Win32 platform])
    ;;
  *64-*-*bsd*)
    bsd_os=bsd 
    AC_DEFINE([WORDSIZE], [64], [this is a 64 platform])
    ;;
  *-*amigaos*)
    amigaos4=yes
    AC_DEFINE([AMIGAOS4_HOST], [1], [this is an AmigaOS4 platform])
    ;;
  *-*haiku*)
    haiku=yes
    AC_DEFINE([HAIKU_HOST], [1], [this is a Haiku platform])
    ;;
  *-gnu*)
    gnu=yes
    AC_DEFINE([GNU_HOST], [1], [this is a GNU platform])
    ;;
  *-*wince)
    wince=yes
    AC_DEFINE([WINCE_HOST], [1], [this is a WINCE platform])
    ;;
  *-*winmo)
    wince=yes
    AC_DEFINE([WINCE_HOST], [1], [this is a WINCE platform])
    AC_DEFINE([WINCE6_HOST], [1], [this is a WINCE6 platform])
    ;;
  *-android*)
    android=yes
    AC_DEFINE([ANDROID_HOST], [1], [this is an Android platform])
    ;;
esac


AM_CONDITIONAL(WIN32, test x$windows = xyes)
AM_CONDITIONAL(HAIKU, test x$haiku = xyes)
AM_CONDITIONAL(AMIGAOS4, test x$amigaos4 = xyes)

dnl Get build date for helping us debugging
BUILDDATE="`date +%Y%m%d`"
AC_SUBST(BUILDDATE)

dnl These are required by automake
AM_INIT_AUTOMAKE
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AM_MAINTAINER_MODE
AC_PROG_MAKE_SET

AM_GNU_GETTEXT([external])
AM_CONDITIONAL(HAS_GETTEXT, test x$ac_cv_path_XGETTEXT != x)

dnl Many of the Gnash macros depend on gettext macros defining shlibext; recent
dnl gettext however does not.
if test x"${shlibext}" = x; then
  if test x"${acl_cv_shlibext}" = x; then
    echo "Gettext macros were supposed to define shared library extensions"
    exit 1;
  else
    shlibext="${acl_cv_shlibext}"
  fi
fi

dnl This is primarily used when compiling for a similar architecture,
dnl like pentium->geode, which can use the same compiler, but have
dnl different development libraries.

AC_ARG_WITH(sysroot,
  AC_HELP_STRING([--with-sysroot],
  [system root directory for cross compiling]),
  with_top_level=${withval} ;
  cross_compiling=yes)

dnl Android is a little different when using a standard cross toolchain,
dnl so we have to configure especially for it for now. Usually it's
dnl something like this:
dnl
dnl arm-linux-eabi-gcc -Wl,--dynamic-linker -Wl,/system/bin/linker
dnl -nostdlib -Wl,-rpath -Wl,/system/lib -Wl,-rpath
dnl -Wl,/opt/android-ndk-r3/build/platforms/android-5/arch-arm/usr/lib
dnl -L/opt/android-ndk-r3/build/platforms/android-5/arch-arm/usr/lib
dnl -lc -o hello
dnl  /opt/android-ndk-r3/build/platforms/android-3/arch-arm/usr/lib/crtbegin_dynamic.o
dnl Recent versions of G++ (I'm using 4.5 from source
android_ndk=no
AC_ARG_WITH([android],
  AC_HELP_STRING([--with-android], [directory where android NDK is installed]),
        android_ndk=${withval}
        if test x"${withval}" != x; then
          android_ndk=${withval}
        else
          android_ndk=/opt/android-ndk-r3/build/platforms/android-5/arch-arm
        fi
)
CROSS_CXXFLAGS=
if test x"${android_ndk}" != xno; then
  CROSS_CXXFLAGS=-mandroid -fexceptions
  if test x"${with_top_level}" = x; then
    with_top_level=/usr/local/android-arm/sysroot/usr
    cross_compiling=yes
  fi
  ANDROID_NDK=${android_ndk}
  AC_DEFINE(ANDROID, [1], [This is an Android build])
else
  ANDROID_NDK=
fi
AC_SUBST(ANDROID_NDK)
AM_CONDITIONAL(ANDROID, [ test x"${android_ndk}" != xno ])

AC_C_BIGENDIAN
AC_C_CONST
AC_C_INLINE

AM_CONDITIONAL(ENABLE_SHARED, [test x"${enable_shared}" = xyes])
AM_CONDITIONAL(ENABLE_STATIC, [test x"${enable_shared}" = xno])

AC_PATH_PROG(DEJAGNU, runtest)

dnl These options are for Cygnal, which collects optional statistics
dnl on all the network traffic By default, we turn on statistics
dnl collecting for the incoming and outgoing queues, since those are
dnl required to be compatiable with FMS 3, and the ActionScript server
dnl management API. buffers are the lowest level data storage, this
dnl data is the time spent in the queue, and is primarily only used
dnl for tuning the queueing API in Gnash. Memoryis the same, it's only
dnl used by developers for tuning performance of memory allocations in
dnl Gnash.
buffers=no
que=no
memory=no
cache=yes
stat_proplookup=no
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
      cache)
        cache=yes
        nstatistics=$((nstatistics+1))
        ;;
      proplookup)
        stat_proplookup=yes
        nstatistics=$((nstatistics+1))
        ;;
      all|ALL)
        buffers=yes
        memory=yes
        queu=yes
        cache=yes
        stat_proplookup=yes
        nstatistics=5           dnl this must be incremented if you add anything
        ;;
      *) AC_MSG_ERROR([invalid statistics feature specified: ${withval} given (accept: buffers|que|memory|cache|proplookup|all)])
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

if test x${cache} = xyes; then
  statistics_list="${statistics_list} cache"
  AC_DEFINE(USE_STATS_CACHE, [1], [Support statistics collecting for the cache])
fi

if test x${stat_proplookup} = xyes; then
  statistics_list="${statistics_list} proplookup"
  AC_DEFINE(GNASH_STATS_OBJECT_URI_NOCASE, [1], [Collecting and report stats about ObjectURI case lookups])
  AC_DEFINE(GNASH_STATS_PROPERTY_LOOKUPS, [1], [Collecting and report stats about property lookups])
  AC_DEFINE(GNASH_STATS_STRING_TABLE_NOCASE, [1], [Collecting and report stats about string_table::key case lookups])
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
    [case "${val}" in
      solreadonly)
        solreadonly=yes
        nsecurity=$((nsecurity+1))
        ;;
      lc)
        localconnection=yes
        nsecurity=$((nsecurity+1))
        ;;
      all|ALL)
        solreadonly=yes
        lc=yes
        nsecurity=3
        ;;
      *) AC_MSG_ERROR([invalid security feature specified: ${withval} given (accept: solreadonly|lc)])
        ;;
      esac]
    withval=`echo ${withval} | cut -d ' ' -f 2-6`
    if test "x$val" = "x$withval"; then
      break;
    fi
  done
)

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

dnl For Haiku, we know the sysroot is in a non-standard place
dnl it is important that -lagg comes before -lbe
if test x"${haiku}" = xyes; then
  HAIKU_LIBS=-lbe
  AC_SUBST(HAIKU_LIBS)
fi

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
   export MACOSX_DEPLOYMENT_TARGET="10.3"
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

if test x"${android_ndk}" != xno; then
   incllist="${android_ndk}/include ${incllist}"
fi

AM_CONDITIONAL(CROSS_COMPILING, [ test x$cross_compiling = xyes ])

for dir in ${incllist}; do
  test -d ${dir} && pruned_incllist="${pruned_incllist} ${dir}";
done

for dir in ${libslist}; do
  test -d ${dir} && pruned_libslist="${pruned_libslist} ${dir}";
done

libslist="${pruned_libslist}";
incllist="${pruned_incllist}";

dnl !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
dnl !! 
dnl !! IMPORTANT NOTICE 
dnl !!
dnl !!  Any call to GNASH_PATH_XXX must be be given *after* this snippet.
dnl !!  This is to ensure that PKG_CONFIG, cross_compiling and incllist are
dnl !!  properly set at the time of call.
dnl !!
dnl !!  Also GNASH_PKG_FIND has to be called later. Not sure
dnl !!  why but calling before breaks detection of CPP (see
dnl !!  gnash-dev mailing archives for June 12 2009
dnl !!  http://lists.gnu.org/archive/html/gnash-dev/2009-06/index.html
dnl !! 
dnl !! 
dnl !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

dnl Check for PKG_CONFIG before any GNASH_PATH call
PKG_PROG_PKG_CONFIG

dnl --------------------------------------------------------
dnl  GUI selection
dnl --------------------------------------------------------

build_haiku=no
build_aos4=no      dnl AmigaOS4 GUI
build_kde3=no      dnl WARNING: doesn't work (see https://savannah.gnu.org/bugs/index.php?31782)
build_qt4=no
build_qtopia3=no
build_qtopia4=no
build_gtk=no
build_fb=no                     dnl Raw framebuffer
build_fltk=no
build_sdl=no
build_aqua=no                   dnl Native MacOSX
build_dump=no
AC_ARG_ENABLE(gui,
  AC_HELP_STRING([--enable-gui=], [Enable support for the specified GUI toolkits (default=gtk,qt4)]),
  [if test -n ${enableval}; then
    enableval=`echo ${enableval} | tr '\054' ' ' `
  fi
  while test -n "${enableval}" ; do
    val=`echo ${enableval} | cut -d ' ' -f 1`
    case "${val}" in
      gtk|GTK|gtk2|GTK2)
        build_gtk=yes
        ;;
      kde3|KDE3)
        build_kde3=yes
        ;;
      qt4|QT4|kde4|KDE4)
        build_qt4=yes
        ;;
      qtopia3|QTOPIA3)
        build_qtopia3=yes
        ;;
      qtopia4|QTOPIA4)
        build_qtopia4=yes
        ;;
      sdl|SDL)
        build_sdl=yes
        ;;
      aqua|AQUA|Aqua)
        build_aqua=yes
        ;;
      fltk|FLTK|fltk2|FLTK2)
        build_fltk=yes
        ;;
      fb|FB)
        build_fb=yes
        ;;
      aos4|AOS4)
        build_aos4=yes
        ;;
      haiku|HAIKU)
        build_haiku=yes
        ;;
      dump|DUMP)
        build_dump=yes
        ;;
      all|ALL)
        build_dump=yes
        dnl BSD doesn't have a framebuffer interface
        if test x${linux} = xyes; then
          build_fb=yes
        fi
        if test x${openbsd} != xyes; then
          build_qt4=yes
        fi
        build_sdl=yes
        build_gtk=yes
        ;;
      *) AC_MSG_ERROR([invalid gui ${enableval} given (accept: gtk|kde3|qt4|fltk|sdl|aqua|fb|qtopia3|qtopia4|dump|aos4|haiku)])
         ;;
      esac
    enableval=`echo ${enableval} | cut -d ' ' -f 2-6`
    if test "x$val" = "x$enableval"; then
      break;
    fi
  done],
  [ dnl Run the following code if no --enable-gui is given
  build_fb=auto
  build_sdl=auto
  if test x"${openbsd}" = xyes; then
    build_gtk=yes
    build_dump=auto
  else if test x"${haiku}" = xyes; then
    build_haiku=yes;
  else
    build_qt4=auto
    build_gtk=yes
    build_dump=auto
  fi
  fi
  ]
)

if test x"${build_haiku}" = xyes; then
  if test x"$haiku" != xyes; then
    echo "        ERROR: Can not build Haiku gui outside of Haiku
operating system." >&3
  fi
fi

dnl We can use Xephyr or fbe to create a fake framebuffer instead of real
dnl video memory. This lets us test on a desktop machine.
AC_ARG_WITH(fakefb,
AC_HELP_STRING([--with-fakefb],
 [specify a file to be mapped instead of a real framebuffer]),
 with_fakefb=${withval})

fakefb=
if test x"${with_fakefb}" = xyes; then
  dnl This is the default file name fbe uses.
  fakefb=/tmp/fbe_buffer
else
  if test x"${with_fakefb}" != x; then
    fakefb=${with_fakefb}
  fi
fi
FAKEFB=${fakefb}
AC_SUBST(FAKEFB)

if test x"${fakefb}" != x; then
    AC_DEFINE(ENABLE_FAKE_FRAMEBUFFER, [1], [Enable using a file instead of a real framebuffer])
fi


dnl --------------------------------------------------------
dnl  Sound handler selection
dnl --------------------------------------------------------
add_sound=
build_sound_none=no
build_sound_sdl=no
build_sound_ahi=no
build_sound_mkit=no
AC_ARG_ENABLE(sound,
  AC_HELP_STRING([--enable-sound=[[sdl|ahi|mkit]]], [Use the specified sound handler (default=sdl)]),
     [case "${enableval}" in
      sdl|SDL|Sdl)
        build_sound_sdl=yes
        add_sound="sdl"
        ;;
      none|NONE|None)
        build_sound_none=yes
        add_sound="none"
        ;;
      ahi|AHI|Ahi)
        build_sound_ahi=yes
        add_sound="ahi"
        ;;
      mkit|MKIT|Mkit)
        build_sound_mkit=yes
        add_sound="mkit"
        ;;
     esac],
  [if test x"${haiku}" = xyes; then
     build_sound_mkit=yes
     add_sound="mkit"
   else
     if test x${build_sound_none} = xno; then
       build_sound_sdl=yes
       add_sound=sdl
     fi
   fi]
)

dnl --------------------------------------------------------
dnl  Media handler selection
dnl --------------------------------------------------------
build_media_gst=no
build_media_ffmpeg=auto
build_media_none=no
AC_ARG_ENABLE(media,
  AC_HELP_STRING([--enable-media=handler],
    [Enable media handling support using the specified handler: gst, ffmpeg or none (no sound) [[gst]] ]),
  
    if test -n ${enableval}; then
      enableval=`echo ${enableval} | tr '\054' ' ' `
    fi
    dnl When --enable-media is given, all media defaults to off
    dnl except the explicitly enabled ones
    build_media_ffmpeg=no
    build_media_gst=no
    while test -n "${enableval}"; do
      val=`echo ${enableval} | cut -d ' ' -f 1`
      [case "${val}" in
        GST|gst)
          build_media_gst=yes
          media_list="${media_list}gst "
          ;;
        FFMPEG|ffmpeg)
          build_media_ffmpeg=yes
          media_list="${media_list}ffmpeg "
          ;;
        no|NO|none)
          build_media_none=yes
          media_list="none"
          ;;
        *)
          AC_MSG_ERROR([bad value ${enableval} for --enable-media option])
          ;;
     
      esac]
      enableval=`echo ${enableval} | cut -d ' ' -f 2-6`
      if test "x$val" = "x$enableval"; then
        break;
      fi
    done,
    [if test x"${build_haiku}" = xyes; then
        build_media_ffmpeg=yes
        build_media_haiku=yes
        media_list="ffmpeg haiku"
     else
        build_media_gst=yes
        media_list="gst"
    fi]
)

dnl If no render is selected, and media handling isn't disabled, them enable gst
if test x${build_media_none} = xno -a x${build_media_gst} = xno -a x${build_media_ffmpeg} = xno; then
  build_media_gst=yes
fi

if test x"$build_media_ffmpeg" != x"no"; then # yes or auto
  GNASH_PATH_FFMPEG
  if test x"${build_media_ffmpeg}" = xauto; then
    dnl TODO: have GNASH_PATH_FFMPEG set ${has_ffmpeg}
    if test x"$FFMPEG_LIBS" != x; then
      build_media_ffmpeg=yes
      media_list="${media_list} ffmpeg"
    else
      build_ogl=no
    fi
  fi
fi

MEDIA_CONFIG="${media_list}"
AC_SUBST(MEDIA_CONFIG)

dnl -----------------------------------------------------
dnl Check for SDL and decide about auto gui accordingly.
dnl Need be done after build_sound_sdl and build_sdl are
dnl initialized and before they are checked for yes/no
dnl -----------------------------------------------------

if test x$build_sdl != xno -o x$build_sound_sdl = xyes; then
  GNASH_PATH_SDL
fi

if test x"${build_sdl}" = xauto; then
  if test xyes = x"${has_sdl}"; then
    build_sdl=yes
  else
    AC_MSG_NOTICE([sdl GUI will not be built (no sdl development files found)])
    build_sdl=no
  fi
fi


dnl -------------------------------
dnl Renderer Selection
dnl -------------------------------
input_events=
dnl By default, we want to to build all renderers
build_ovg=no
build_gles=no
build_ogl=auto
build_agg=yes
build_cairo=yes
renderer_list="agg cairo"
AC_ARG_ENABLE(renderer,
  AC_HELP_STRING([--enable-renderer=], [Enable support for the specified renderers (agg|cairo|opengl|all, default=all)]),
  if test -n ${enableval}; then
    if test "x${enableval}" != "xno" -o "x${enableval}" != "xnone" ; then
      renderer_list="none"
      enableval=`echo ${enableval} | tr '\054' ' ' `
    else
      renderer_list=""
      enableval=""
    fi
  fi
  renderer_list=""
  build_ovg=no
  build_ogl=no
  build_gles=no
  build_agg=no
  build_cairo=no
  while test -n "${enableval}" ; do
    val=`echo ${enableval} | cut -d ' ' -f 1`
    [case "${val}" in
      no*|NO*)
        renderer_list="none"
        build_ovg=no
        build_ogl=no
        build_gles=no
        build_agg=no
        build_cairo=no
        ;;
      all|ALL)
        renderer_list="agg cairo opengl"
        build_ogl=yes
        build_agg=yes
        build_cairo=yes
        build_ovg=yes
        ;;
      ogl|OGL|OpenGL|opengl)
        renderer_list="${renderer_list} opengl"
        build_gles=no
        build_ogl=yes
        ;;
      gles|GLES|GL-ES|gl-es)
        renderer_list="${renderer_list} opengl-es"
        build_gles=yes
        build_ogl=no
        ;;
      ovg|OVG|OpenVG|openvg)
        renderer_list="${renderer_list} openvg"
        build_ogv=yes
        ;;
      agg|AGG)
        renderer_list="${renderer_list} agg"
        build_agg=yes
        ;;
      cairo|CAIRO|Cairo*)
        renderer_list="${renderer_list} cairo"
        build_cairo=yes
        ;;
      *) AC_MSG_ERROR([invalid renderer specified: ${enableval} given (accept:  (agg|cairo|opengl|all)])
        ;;
      esac]
    enableval=`echo ${enableval} | cut -d ' ' -f 2-`
    if test "x$val" = "x$enableval"; then
      break;
    fi
  done
)
dnl 16 bit: RGB555, RGB565
dnl 24 bit: RGB24, BGR24
dnl 32 bit: RGBA32, BGRA32
pixelformat=all
AC_ARG_WITH(pixelformat,
  AC_HELP_STRING([--with-pixelformat=], [Use the specified pixel format for AGG (default=all)]),
  [if test -n ${withval}; then
    pixelformat="${withval}"
    withval=`echo ${withval} | tr '\054' ' ' `
  fi
  while test -n "${withval}" ; do
    val=`echo ${withval} | cut -d ' ' -f 1`
    case "${val}" in
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
      esac
    withval=`echo ${withval} | cut -d ' ' -f 2-6`
    if test "x$val" = "x$withval"; then
      break;
    fi
  done],
  [if test x$build_haiku = xyes -a x$build_sdl != xyes; then
	AC_DEFINE(PIXELFORMAT_BGRA32, [1], [BGRA32])
        pixelformat=BGRA32
   fi]
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

if test x"${build_ogl}" != xno; then # yes or auto
  GNASH_PATH_OPENGL
  if test x"${build_ogl}" = xauto; then
    if test xyes = x"${has_opengl}"; then
      build_ogl=yes
      renderer_list="${renderer_list} OpenGL"
    else
      build_ogl=no
    fi
  fi
fi

AM_CONDITIONAL(BUILD_OVG_RENDERER, [ test x$build_ovg = xyes ])
AM_CONDITIONAL(BUILD_GLES_RENDERER, [ test x$build_gles = xyes ])
AM_CONDITIONAL(BUILD_OGL_RENDERER, [ test x$build_ogl = xyes])
AM_CONDITIONAL(BUILD_AGG_RENDERER, [ test x$build_agg = xyes ])
AM_CONDITIONAL(BUILD_CAIRO_RENDERER, [ test x$build_cairo = xyes ])

if test x"${build_ogl}" = xyes; then
   AC_DEFINE([RENDERER_OPENGL], [], [Use OpenGL renderer])
fi

if test x"$build_agg" = xyes; then
  AC_DEFINE([RENDERER_AGG], [], [Use AntiGrain renderer])
fi

if test x"$build_cairo" = xyes; then
  GNASH_PKG_FIND(cairo, [cairo.h], [cairo render library], cairo_status)
  AC_DEFINE([RENDERER_CAIRO], [], [Use cairo renderer])
fi

if test x"${build_agg}" = xyes; then
   GNASH_PATH_AGG
fi

dnl VA API is used by default for all H.264 videos. HW requirements:
dnl     * AMD GPUs with UVD2 and xvba-video VA driver
dnl     * NVIDIA GPUs with vdpau-video VA driver
dnl     * All HW with a VA driver supporting the VA/GLX extensions or
dnl     vaPutSurface(Pixmap,...). This may include the Intel
dnl     Moorestown platform and future G45 VA driver.
dnl NOTE: it is possible to use Gnash/VAAPI on platforms with an Intel
dnl GMA500 but you currently will have to build the AGG renderer
dnl instead of the OGL (OpenGL) one. 

build_vaapi=yes
hwaccel_list="VAAPI"
nhwaccel=1
AC_ARG_ENABLE(hwaccel,
  AC_HELP_STRING([--enable-hwaccel], [Enable hardware accelerated video decoding (none,vaapi)]),
  if test -z ${enableval}; then
      hwaccel_list="none"
      nhwaccel=0
      enableval=""
  fi
  while test -n "${enableval}" ; do
    val=`echo ${enableval} | cut -d ' ' -f 1`
    [case "${val}" in
      no*|NO*)
        hwaccel_list="none"
        build_vaapi=no
        build_omap=no
        nhwaccel=0
        ;;
      va*|VA*)
        hwaccel_list="${hwaccel_list} vaapi"
        build_vaapi=yes
        nhwaccel=$((nhwaccel+1))
        ;;
      all|ALL)
        hwaccel_list="VAAPI OMAP"
        nhwaccel=2
        build_omap=yes
        build_vaapi=yes
        ;;
      omap*|OMAP*)
        hwaccel_list="${hwaccel_list} omap"
        build_omap=yes
        nhwaccel=$((nhwaccel+1))
        ;;
      *) AC_MSG_ERROR([invalid hwaccel feature specified: ${enableval} given (accept: none,vaapi,omap)])
        ;;
      esac]
    enableval=`echo ${enableval} | cut -d ' ' -f 2-6`
    if test "x$val" = "x$enableval"; then
      break;
    fi
  done
)

if test x"${have_ffmpeg_vaapi}" = x"yes" -a x"${build_vaapi}" = x"yes"; then
  use_libva=no
  use_libva_x11=no
  use_libva_glx=no
  dnl if the version of FFmpeg is recent enough, (r20957, 52.45.0), then
  dnl look for VAAPI support so we can use use the VAAPI enabled FFmpeg.
  if test x"${have_ffmpeg_vaapi}" = xyes; then
    use_libva=yes
    GNASH_PKG_FIND([va],
      [va/va.h],
      [Video Acceleration API],
      vaInitialize
    )

    use_libva_x11=yes
    GNASH_PKG_FIND([va_x11],
      [va/va_x11.h],
      [VA API (X11 display)],
      vaGetDisplay,
      [], [-lva-x11]
    )

    if test x$build_ogl = xyes; then
      use_libva_glx=yes
      GNASH_PKG_FIND([va_glx],
        [va/va_glx.h],
        [VA API (GLX display)],
        vaGetDisplayGLX,
        [], [-lva-glx]
      )
    fi
  fi
fi

dnl libVA drivers. We declare conditional for both the option being
dnl selected, as well as whether or not it's found. This we can
dnl generate better error handling if it's not found.
AM_CONDITIONAL(BUILD_OMAP, test x"${build_omap}" = xyes)
AM_CONDITIONAL(USE_VAAPI, test x"${use_libva}" = xyes)

dnl Only build the vaapi support if we have a version of FFmpeg that
dnl supports it.
if test x"${have_ffmpeg_vaapi}" = x"yes" -a x"${found_libva_incl}" = xyes; then
  build_vaapi=yes
  hwaccel_list="VAAPI"
  nhwaccel=1
else
  build_vaapi=no
  hwaccel_list=""
  nhwaccel=0
fi

dnl Until the hwaccel patches in FFmpeg wind up in the ffmpeg-plugin,
dnl restrict using HW Accel to using FFmpeg directly.
dnl test xyes = xyes -a ( x != xyes -o x != xyes )
if test x"${build_vaapi}" = x"yes" -a x"${have_ffmpeg}" != x"yes"; then
  AC_MSG_ERROR(["Hardware acceleration currently not supported unless using FFmpeg."])
fi

AM_CONDITIONAL(HAVE_VAAPI, test x"${found_va_incl}" = xyes)
AM_CONDITIONAL(HAVE_VAAPI_GLX, test x"${found_libva_glx_incl}" = xyes)
AM_CONDITIONAL(HAVE_VAAPI_X11, test x"${found_libva_x11_incl}" = xyes)


AC_PATH_PROG(PERL, perl)
AM_CONDITIONAL(HAVE_PERL, test x"$PERL" != x)

AC_PATH_PROG(CSOUND, csound)
AM_CONDITIONAL(HAVE_CSOUND, test x"$CSOUND" != x)

AC_PATH_PROG(GIT, git)
AC_SUBST(GIT)

dnl --------------------------------------------------------
dnl Extension selection
dnl --------------------------------------------------------
ext_dejagnu=no
ext_mysql=no
ext_fileio=no
ext_gtk=no
ext_lirc=no
ext_dbus=no
ext_all=no
extensions_list=
extensions_support=yes
nextensions=0
AC_ARG_ENABLE(extensions,
  AC_HELP_STRING([--enable-extensions=], [Specify which extensions to build (default: none)])
  AC_HELP_STRING([--disable-extensions], [Disable support for extensions entirely]),
  if test -n ${enableval}; then
    if test "x${enableval}" != "xno"; then
      extlist="${enableval}"
      enableval=`echo ${enableval} | tr '\054' ' ' `
      AC_DEFINE([USE_EXTENSIONS], [1], [Specify that extension support is enabled.])
    else
      extlist=""
      enableval=""
      extensions_support=no
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
      all|ALL)
        AC_DEFINE(USE_GTK_EXT, [1], [Build all the extensions])
        ext_dejagnu=yes
        ext_mysql=yes
        ext_fileio=yes
        ext_gtk=yes
        ext_lirc=yes
        ext_dbus=yes
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
  EXTENSIONS_LIST="$extensions_list"
  AC_SUBST(EXTENSIONS_LIST)
)

if test x$ext_dbus = xyes; then
  GNASH_PATH_DBUS
fi

if test x$ext_mysql = xyes; then
  GNASH_PATH_MYSQL
fi

AM_CONDITIONAL(BUILD_DEJAGNU_EXT, [ test x$ext_dejagnu = xyes ])
AM_CONDITIONAL(BUILD_FILEIO_EXT, [ test x$ext_fileio = xyes ])
AM_CONDITIONAL(BUILD_MYSQL_EXT, [ test x$ext_mysql = xyes ])
AM_CONDITIONAL(BUILD_GTK_EXT, [ test x$ext_gtk = xyes ])
AM_CONDITIONAL(BUILD_LIRC_EXT, [ test x$ext_lirc = xyes ])
AM_CONDITIONAL(BUILD_DBUS_EXT, [ test x$ext_dbus = xyes ])
AM_CONDITIONAL(BUILD_EXTENSIONS, [ test -n "$extensions_list"])

AM_CONDITIONAL(ENABLE_EXTENSIONS, [ test "x${extensions_support}" != "xno" ])

dnl --------------------------------------------------------
dnl Libtool
dnl --------------------------------------------------------

AC_LIBTOOL_DLOPEN
AC_LIBTOOL_WIN32_DLL
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)
DLOPEN="-dlopen"
DLPREOPEN="-dlpreopen"
AC_SUBST(DLOPEN)
AC_SUBST(DLPREOPEN)

GNASH_PKG_FIND([ltdl], [ltdl.h], [libltdl library], [lt_dlinit])

if test x"$has_ltdl" = x"yes";then
  AC_DEFINE(HAVE_LTDL, [1], [Libtool 2.x defines this, but libtool 1.5 does not])
fi

dnl --------------------------------------------------------
dnl SOL dir
dnl --------------------------------------------------------
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

dnl --------------------------------------------------------
dnl AVM2
dnl --------------------------------------------------------
dnl AC_ARG_ENABLE(avm2,
dnl   AC_HELP_STRING([--enable-avm2], [Enable support for AS3]),
dnl   [case "${enableval}" in
dnl     yes) avm2=yes ;;
dnl     no)  avm2=no ;;
dnl     *)   AC_MSG_ERROR([bad value ${enableval} for enable-avm2 option]) ;;
dnl   esac], avm2=no
dnl )
dnl AM_CONDITIONAL(ENABLE_AVM2, [test x"$avm2" = xyes])
dnl if test x$avm2 = xyes; then
dnl     AC_DEFINE(ENABLE_AVM2, [1], [Enable AVM2 code])
dnl fi
AM_CONDITIONAL(ENABLE_AVM2, false)

dnl This option is only used if you want Gnash to interwork with 
dnl the Adobe player using the LocalConnection class.
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

AC_ARG_ENABLE(python,
  AC_HELP_STRING([--enable-python],[Enable python for the python wrapper]),
[case "${enableval}" in
  yes) python=yes ;;
  no)  python=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for --enable-python option]) ;;
esac],python=no)

dnl Look for python, which is optional. If enabled, a python loadable
dnl module of Gnash is created.
GNASH_PATH_PYTHON
AM_CONDITIONAL([USE_PYTHON], test x"$python" = xyes)
AM_CONDITIONAL([HAS_PYTHON], test x"$has_python" = xyes)

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
esac],jemalloc=yes)

AC_CHECK_SIZEOF([void *], [4])

if test "x${ac_cv_sizeof_void_p}" != x4 -a x"${ac_cv_sizeof_void_p}" != x8; then
  dnl jemalloc doesn't support unusual pointer sizes
  jemalloc=no
fi

dnl There is some weird stuff going on with NetBSD and jemalloc, so don't 
dnl build it for now.
if test x"${netbsd}" = x"yes" -o x"${windows}" = x"yes" -o x"${freebsd}" = x"yes" -o x"${haiku}" = x"yes" -o x"${gnu}" = x"yes"; then
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

dnl We can search libs for mallinfo to decide whether we have it or not.
dnl This is added to the linker flags when it's found. Usually it's -lc, but
dnl on OpenSolaris it's -lmalloc, so this fixes the build.
AC_SEARCH_LIBS([mallinfo], [c malloc],
               AC_DEFINE(HAVE_MALLINFO, [1], [Has mallinfo()])
               mallinfo=yes
               )

AM_CONDITIONAL([HAVE_MALLINFO], test x$mallinfo = xyes)
AM_CONDITIONAL(JEMALLOC, test x$jemalloc = xyes)

AC_ARG_ENABLE(fps-debug,
  AC_HELP_STRING([--enable-fps-debug],[Enable FPS debugging code]),
[case "${enableval}" in
  yes) AC_DEFINE([GNASH_FPS_DEBUG], [1], [Enable FPS debugging code])
esac])

dnl IPC_INFO isn't portable, and doesn't exist on BSD
AC_TRY_COMPILE([#include <sys/ipc.h> #include <sys/shm.h>], [
  int flag = IPC_INFO; ],
  AC_DEFINE([HAVE_IPC_INFO], [1], [Use ipc_info])
)

dnl --------------------------------------------------------
dnl Disable menus
dnl --------------------------------------------------------
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

dnl --------------------------------------------------------
dnl Disable SWF information
dnl --------------------------------------------------------
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

dnl --------------------------------------------------------
dnl Disable testsuite
dnl --------------------------------------------------------
dnl Disable running any tests for "make check". This may sound stupid, but
dnl this option is designed to solely be used by maintainers in the 
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

dnl --------------------------------------------------------
dnl Write the file to disk in the plugin
dnl --------------------------------------------------------
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

dnl --------------------------------------------------------
dnl Write a standalone gnash launcher to disk from the plugin
dnl --------------------------------------------------------
AC_ARG_ENABLE(sa-launcher,
  AC_HELP_STRING([--disable-sa-launcher], [Drops support for the NPAPI plugin writing of standalone executable launcher scripts for the currently playing SWF movie to /tmp.]),
[case "${enableval}" in
  yes) sa_launcher=yes ;;
  no)  sa_launcher=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for --enable-sa-launcher option]) ;;
esac],sa_launcher=yes)

if test x"$sa_launcher" = x"yes"; then
  AC_DEFINE([CREATE_STANDALONE_GNASH_LAUNCHER], [], [Add support for writing a standalone executable launcher for movies embedded in web pages])
fi

dnl --------------------------------------------------------
dnl Build the cygnal server if specified.
dnl --------------------------------------------------------
AC_ARG_ENABLE(cygnal,
  AC_HELP_STRING([--enable-cygnal], [Enable building of the Cygnal server]),
[case "${enableval}" in
  yes) cygnal=yes ;;
  no)  cygnal=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for enable-cygnal option]) ;;
esac],cygnal=no)
AM_CONDITIONAL(CYGNAL, test x$cygnal = xyes)

dnl --------------------------------------------------------
dnl Build the cgibins server if specified.
dnl --------------------------------------------------------
AC_ARG_ENABLE(cgibins,
  AC_HELP_STRING([--enable-cgibins], [Enable building of the CGIs for Cygnal]),
[case "${enableval}" in
  yes) cgibin=yes ;;
  no)  cgibin=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for enable-cgibins option]) ;;
esac],cgibin=yes)
AM_CONDITIONAL(USE_CGI, test x$cgibin = xyes)
if test x"${cgibin}" = x"yes"; then
  AC_DEFINE([USE_CGIBIN], [1], [Enable cgi-bin processes for Cygnal])
fi

dnl --------------------------------------------------------
dnl  Double buffer
dnl --------------------------------------------------------
dnl Add an option for double buffering when rendering, currently only used by
dnl the frmaebuffer GUI.
dnl If defined, an internal software-buffer is used for rendering and is then
dnl copied to the video RAM. This avoids flicker and is faster for complex 
dnl graphics, as video RAM access is usually slower. (strongly suggested)
AC_ARG_ENABLE(doublebuf,
  AC_HELP_STRING([--enable-doublebuf], [Enable support for double buffering when rendering]),
  [case "${enableval}" in
    yes) doublebuf=yes ;;
    no)  doublebuf=no ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for enable-doublebuf option]) ;;
  esac], doublebuf=no
)
AM_CONDITIONAL(ENABLE_DBUF, [test x"$doublebuf" = xyes])
if test x$doublebuf = xyes; then
    AC_DEFINE(ENABLE_DOUBLE_BUFFERING, [1], [Enable double buffering])
fi

dnl --------------------------------------------------------
dnl  Offscreen buffer
dnl --------------------------------------------------------
dnl This enables rendering to an offscreen buffer, instead of directly to window
AC_ARG_ENABLE(offscreen,
  AC_HELP_STRING([--enable-offscreen], [Enable support for rendering offscreen]),
  [case "${enableval}" in
    yes) offscreen=yes ;;
    no)  offscreen=no ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for enable-offscreen option]) ;;
  esac], offscreen=no
)


dnl --------------------------------------------------------
dnl  SSH support selection
dnl --------------------------------------------------------
dnl Enable using libssh with libnet
AC_ARG_ENABLE(ssh,
  AC_HELP_STRING([--enable-ssh], [Enable using SSH for network authentication]),
[case "${enableval}" in
  yes) build_ssh=yes ;;
  no)  build_ssh=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for --enable-ssh option]) ;;
esac], build_ssh=no)

AM_CONDITIONAL(BUILD_SSH, test x"${build_ssh}" = xyes)
if test x"${build_ssh}" = xyes; then
  GNASH_PKG_FIND(ssh, [libssh/libssh.h], [libssh library], ssh_socket_init)
dnl  GNASH_PKG_FIND(poppler, [popt.h], [Poppler library], poppler_init)
fi
if test x"${has_ssh}" = x"yes"; then
  AC_DEFINE([USE_SSH], [1], [Use SSH for authentication])
fi

dnl --------------------------------------------------------
dnl  SSL support selection
dnl --------------------------------------------------------

dnl Enable using OpenSSL with libnet.
AC_ARG_ENABLE(ssl,
  AC_HELP_STRING([--enable-ssl], [Enable using OpenSSL directly]),
[case "${enableval}" in
  yes) build_ssl=yes ;;
  no)  build_ssl=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for --enable-ssl option]) ;;
esac], build_ssl=no)

with_cert=
with_pem=
AM_CONDITIONAL(BUILD_SSL, test x"${build_ssl}" = xyes)
AC_ARG_WITH(cert,
  AC_HELP_STRING([--with-cert],
  [cert file for SSL]),
  with_cert=${withval})
AC_ARG_WITH(pem,
  AC_HELP_STRING([--with-pe],
  [pem file for SSL]),
  with_pem=${withval})

if test x"${build_ssl}" = xyes; then
  GNASH_PKG_FIND(ssl, [openssl/ssl.h], [OpenSSL library], SSL_library_init)
fi
if test x"${has_ssl}" = x"yes"; then
  AC_DEFINE([USE_SSL], [1], [Use SSL for authentication])
fi

dnl -----------------------------------------------------------
dnl Set the general plugins install policy here
dnl (NOTE: before GNASH_PATH_FIREFOX and GNASH_PATH_KDE*)
dnl -----------------------------------------------------------

AC_ARG_WITH(plugins-install,
  AC_HELP_STRING([--with-plugins-install=system|user|prefix], [Policy for plugins install. Default: user.]),
	[case "${withval}" in
	  user) PLUGINS_INSTALL_POLICY=user ;;
	  system) PLUGINS_INSTALL_POLICY=system ;;
	  prefix) PLUGINS_INSTALL_POLICY=prefix ;;
	  *)  AC_MSG_ERROR([bad value ${withval} for --with-plugins-install]) ;;
	 esac 
	], PLUGINS_INSTALL_POLICY=user) 

dnl -----------------------------------------------------------
dnl   Verify dependencies for requested GUIs are met, and
dnl   disable build of the GUIS for which deps are NOT met
dnl ------------------------------------------------------------

if test x$build_gtk = xyes; then
   GNASH_PATH_GTK2
   GNASH_PATH_PANGO
   GNASH_PKG_FIND(atk, [atk/atk.h], [atk library], atk_focus_tracker_init, [1.0])
   if test x"${build_ogl}" = xyes; then
      GNASH_PATH_GLEXT
   fi
   if test x"${build_cairo}" = xyes; then
      GNASH_PKG_FIND(cairo, [cairo.h], [cairo render library], cairo_status)
   fi
fi

if test x"${build_qt4}" != xno; then
 GNASH_PATH_QT4
fi

if test x"${build_qt4}" = xauto; then
  if test x"${has_qt4}" = xyes; then
     build_qt4=yes
  else
     build_qt4=no
  fi
fi

if test x"${build_kde3}" != xno -o x"${build_qtopia3}" != xno; then
 GNASH_PATH_QT3
fi

if test x"${build_kde3}" != xno; then
  GNASH_PATH_KDE3
fi


if test x"${build_kde3}" = xauto; then
  if test x"${has_qt3}" = xyes -a x"${has_kde3}" = xyes; then
     build_kde3=yes
  else
     build_kde3=no
  fi
fi

dnl Check possibility to build DUMP gui, if requested
if test x"${build_dump}" != xno; then
  if test x"${build_agg}" = xyes; then
    build_dump=yes
  else
    if test x"${build_dump}" = xyes; then
      dnl SHOULD we just check at the end of file instead ?
      AC_MSG_ERROR(dump GUI cannot be built without AGG renderer);
    fi
    build_dump=no
  fi
fi

dnl Check possibility to build FB gui, if requested
dnl FB dependencies are a linux system (linux/fb.h) 
dnl and AGG renderer
if test x"${build_fb}" != xno; then
  if test x"${linux}" = xyes; then
    if test x"${build_agg}" = xyes; then
      build_fb=yes
    else
      if test x"${build_fb}" = xyes; then
        dnl SHOULD we just check at the end of file instead ?
        AC_MSG_ERROR([fb GUI cannot be built without AGG renderer])
      fi
      AC_MSG_NOTICE([fb GUI won't be built (needs AGG renderer)])
      build_fb=no
    fi
  else
    if test x"${build_fb}" = xyes; then
      dnl SHOULD we just check at the end of file instead ?
      AC_MSG_ERROR(fb GUI cannot be built on non-linux platforms);
    fi
    AC_MSG_NOTICE([fb GUI won't be built (non-linux platform)])
    build_fb=no
  fi
fi

dnl TODO: add checks for all other GUIs

dnl -------------------------------
dnl Input Device selection
dnl -------------------------------

dnl Multiple input devices are supported. These can all work in
dnl varying combinations, so several may be listed. These are only
dnl required when using the Framebuffer, as without the X11 desktop,
dnl Gnash has to handle all these internally. This can get
dnl messy, as one might want to use a touchscreen with a normal mouse
dnl or keyboard attached. 
dnl By default, don't build any of these, as they are only for the Framebuffer
dnl running without X11.
if test x"${build_fb}" = xyes; then
build_ps2mouse=yes
build_ps2keyboard=no
build_input_events=yes
build_tslib=yes
input_events="PS/2 Mouse, Input Devices, Touchscreen via Tslib"
else
build_ps2mouse=no
build_ps2keyboard=no
build_input_events=no
build_tslib=no
input_events=
fi
AC_ARG_ENABLE(input,
  AC_HELP_STRING([--enable-input=], [Enable support for the specified input devices for the framebuffer GUI (default=ps2mouse|ps2keyboard|events|touchscreen)]),
  [if test -n ${enableval}; then
    enableval=`echo ${enableval} | tr '\054' ' ' `
  fi
  while test -n "${enableval}" ; do
    val=`echo ${enableval} | cut -d ' ' -f 1`
    case "${val}" in
      ps2m*|PS2m*|m*|M*)        dnl a PS/2 style mouse
        build_ps2mouse=yes
        input_events="${input_events}, PS/2 Mouse"
        ;;
      ps2k*|PS2K*|k*|K*)        dnl a PS/2 style keyboard
        build_ps2keyboard=yes
        input_events="${input_events}, PS/2 Keyboard"
        ;;
      i*|I*|ev*|Ev*)    dnl use the new Input Event, which supports both
        input_events="${input_events}, Input Event Device"
        build_input_events=yes
        ;;
      t*|T*) dnl use a touchscreen with tslib, which works like a mouse
        build_tslib=yes
        input_events="${input_events}, Touchscreen"
        ;;
      *) AC_MSG_ERROR([invalid input device list! ${enableval} given (accept: ps2mouse|keyboard|events,touchscreen)])
         ;;
      esac
    enableval=`echo ${enableval} | cut -d ' ' -f 2-6`
    if test "x$val" = "x$enableval"; then
      break;
    fi
  done],
)

if test x"${build_tslib}" = xyes; then
  AC_DEFINE(USE_TSLIB, [1], [Use a tslib supported touchscreen])
  GNASH_PKG_FIND(ts, [tslib.h], [Touchscreen library], ts_config)
fi
AM_CONDITIONAL(ENABLE_TSLIB, [test x${ac_cv_header_tslib_h} != xno])

if test x"${build_ps2mouse}" = xyes; then
  AC_DEFINE(USE_MOUSE_PS2, [1], [Add support for a directly using a PS/2 Mouse])
fi
AM_CONDITIONAL(ENABLE_MOUSE, [test x"${build_ps2mouse}" = xyes])

if test x"${build_ps2keyboard}" = xyes; then
  AC_DEFINE(USE_KEYBOARD_PS2, [1], [Add support for directly using a PS/2 Keyboard])
fi
if test x"${build_input_events}" = xyes; then
  AC_DEFINE(USE_INPUT_EVENTS, [1], [Add support for a directly using Linux Input Event Devices])
fi
AM_CONDITIONAL(ENABLE_INPUT_EVENTS, [test x"${build_input_events}" = xyes])
dnl this is enabled if we have any input devices at all
AM_CONDITIONAL(ENABLE_INPUT_DEVICES, [test x"${input_events}" != x])


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
  LDFLAGS="$LDFLAGS -Wl,--as-needed"
fi
AC_MSG_RESULT($gcc_cv_ld_as_needed)

AC_DEFINE(USE_GIF, [1], [Use the GIF library])
AC_DEFINE(USE_PNG, [1], [Use the PNG library])
AM_CONDITIONAL(USE_GIF, true)
AM_CONDITIONAL(USE_PNG, true)

AC_PATH_TOOL([AUTOTRACE], [autotrace])
AC_HEADER_DIRENT

dnl -----------------------------------------------------------------
dnl PLUGIN RELATED STUFF
dnl -----------------------------------------------------------------

dnl !! This has been moved here to make --enable-npapi work
dnl !! All of plugin-related macro calls could be moved into
dnl !! a specialized macros/plugin.m4

dnl ----------------------------------------------------
dnl Add KPARTS support, if specified or KDE gui is built
dnl ----------------------------------------------------

AC_ARG_ENABLE(kparts3,
  AC_HELP_STRING([--disable-kparts3], [Disable support for Konqueror 3.x plugin (default: enabled if kde3 gui is)]),
[case "${enableval}" in
  yes) build_kparts3=yes ;;
  no)  build_kparts3=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for --disable-kparts3 option]) ;;
esac],build_kparts3=$build_kde3)

dnl --------------------------------------------------------
dnl Add KPARTS 4.x support, if specified or KDE gui is built
dnl --------------------------------------------------------

AC_ARG_ENABLE(kparts4,
  AC_HELP_STRING([--disable-kparts4], [Disable support for Konqueror 4.x plugin (default: enabled if Qt4 gui is)]),
[case "${enableval}" in
  yes) build_kparts4=yes ;;
  no)  build_kparts4=no ;;
  *)   AC_MSG_ERROR([bad value ${enableval} for --disable-kparts4 option]) ;;
esac],build_kparts4=$build_qt4)

if test x"${build_kparts4}" != xno; then
  GNASH_PATH_KDE4
fi

dnl -----------------------------------------------------------
dnl Add NPAPI support, if specified or GTK or Qt4 gui is built
dnl -----------------------------------------------------------

AC_ARG_ENABLE(npapi,
  AC_HELP_STRING([--disable-npapi], [Disable NPAPI plugin build (default: enabled if gtk or Qt4 gui is)]),
  [case "${enableval}" in
    yes) npapi=yes ;;
    no)  npapi=no ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for disable-npapi option]) ;;
  esac],
  if test x$build_gtk = xyes -o x$build_qt4 = xyes; then
    npapi=yes
  fi
)

if test x"$npapi" = x"yes"; then
  GNASH_PATH_NPAPI
fi

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
  build_kparts3=no
  build_kparts4=no
fi


GNASH_PATH_FIREFOX

dnl -----------------------------------------------------------------
dnl END OF PLUGIN RELATED STUFF
dnl -----------------------------------------------------------------

AM_CONDITIONAL(LIRC, [test x$lirc_ext = xyes])

AC_CHECK_HEADERS(getopt.h)
AC_CHECK_HEADERS(libgen.h)
AC_CHECK_HEADERS(pwd.h)
AC_CHECK_HEADERS(sys/utsname.h)
AC_CHECK_HEADERS(signal.h)
AC_CHECK_HEADERS(unistd.h)
AC_CHECK_HEADERS(sys/time.h)
AC_CHECK_LIB(bz2, BZ2_bzopen, [AC_SUBST(BZ2_LIBS, -lbz2)])
AC_CHECK_LIB(c, getpwnam, AC_DEFINE(HAVE_GETPWNAM, 1, [Has getpwnam] ))

dnl X11 is needed for fltk (at least),
dnl and probably for many other guis too ...
GNASH_PATH_X11
AM_CONDITIONAL(HAVE_X11, [test x$x11 = xyes])

AC_CHECK_LIB(rt, shm_unlink)
AC_CHECK_FUNCS(shm_open shm_unlink)

dnl if test x$cross_compiling = xno; then
AC_LANG_PUSH(C++)
AC_MSG_CHECKING([to see if float formatting is broken])
AC_RUN_IFELSE([
  AC_LANG_PROGRAM([#include <cmath>
    void testFloat(double d, double& s)
    {
        d /= 1000.0;
        s = std::fmod(d, 86400.0);
    }], [
     double d = 3.0935415006117e+23;
     double s;
     testFloat(d, s);
     if (static_cast<int>(s) != 61440) {
         return 1;
     }])],                         dnl end of LANG_PROGRAM
     broken_float=no,              dnl returns 0, works
     broken_float=yes,             dnl returns 1, broken
     broken_float=no               dnl cross compiling
)                                  dnl end of RUN_IFELSE
AC_LANG_POP(C++)
dnl fi

if test x${broken_float} = xyes; then
  AC_MSG_RESULT([yes])
  AC_DEFINE(HAVE_BROKEN_FLOAT, [1], [Has broken float support])
else
  AC_MSG_RESULT([no])
fi
AM_CONDITIONAL(BROKEN_FLOAT, [ test x$broken_float = xyes ])

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

AC_CACHE_CHECK([for finite], ac_cv_finite,
 [AC_TRY_COMPILE([
   #include <math.h>
   #ifdef HAVE_IEEEFP_H
   #include <ieeefp.h>
   #endif],
 [double x; int y; y = finite(x);],
 ac_cv_finite=yes,
 ac_cv_finite=no
)])
if test x"$ac_cv_finite" = x"yes"; then
  AC_SEARCH_LIBS(finite, m,
    [AC_DEFINE(HAVE_FINITE, [1], [Has finite])]
  )
fi

AC_LANG_PUSH(C++)
AC_CACHE_CHECK([for isfinite], ac_cv_isfinite,
 [AC_TRY_COMPILE([#include <cmath>],
 [using namespace std; double x; int y; y = isfinite(x);],
 ac_cv_isfinite=yes,
 ac_cv_isfinite=no
)])
AC_LANG_POP(C++)
if test x"$ac_cv_isfinite" = x"yes"; then
  dnl Don't give up if isfinite is not found in -lm
  dnl isfinite is defined as a macro in C99.
  AC_SEARCH_LIBS(isfinite, m)
  AC_DEFINE(HAVE_ISFINITE, [1], [Has isfinite])
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
 * In practice, you do not need to define it yourself (tested on glibc-2.2.1 
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
AC_CHECK_FUNCS(scandir)         dnl supported by BSD and Linux, but you never know...
AC_CHECK_FUNC(clock_gettime, AC_DEFINE(HAVE_CLOCK_GETTIME, 1, [Has clock_gettime()] ))
dnl Look for Win32 networking stuff
AC_CHECK_HEADERS(winsock.h)
AC_CHECK_HEADERS(winsock2.h)

GNASH_PKG_FIND(nspr, [nspr.h], [Netscape Portable Runtime (NSPR)], PR_Init)

AC_PATH_TOOL(WINDRES, [windres])
AC_SUBST(WINDRES)

GNASH_PKG_FIND(z, [zlib.h], [zlib compression library], compress)
GNASH_PKG_FIND(jpeg, [jpeglib.h], [jpeg images], jpeg_mem_init)
GNASH_PKG_FIND(png, [png.h], [png images], png_info_init)
GNASH_PKG_FIND(gif, [gif_lib.h], [gif images], DGifOpen)
if test x"${GIF_LIBS}" = x ; then
 GNASH_PKG_FIND(ungif, [gif_lib.h], [gif images], DGifOpen)
 GIF_LIBS=${UNGIF_LIBS}
 GIF_CFLAGS=${UNGIF_CFLAGS}
fi
if test x"$testsuite" = x"yes"; then
  GNASH_PKG_INCLUDES([dejagnu], [dejagnu.h])
fi

GNASH_PKG_FIND(speex, [speex.h], [speex audio codec], speex_decode_int)
AM_CONDITIONAL(HAVE_SPEEX, [ test x$has_speex = xyes ])
if test x$has_speex = xyes ; then
  AC_DEFINE([DECODING_SPEEX], [1], [Speex codec available])
fi

GNASH_PKG_FIND(speexdsp, [speex_resampler.h], [speex DSP utilities], speex_resampler_process_int)
if test x$has_speexdsp = xyes ; then
  AC_DEFINE([RESAMPLING_SPEEX], [1], [Speex resampler available])
fi

dnl Find freetype and fontconfig
GNASH_PATH_FREETYPE2
GNASH_PKG_FIND(fontconfig, [fontconfig/fontconfig.h], [fontconfig library], FcFontMatch)

if test x$cross_compiling = xno; then
  AC_PATH_MING
fi

if test x"$MING_VERSION_CODE" = x; then
 MING_VERSION_CODE=00000000
fi

AM_CONDITIONAL(ENABLE_MING,
	[ test x"$MAKESWF" != x -a $MING_VERSION_CODE -gt 00040000 ])

AM_CONDITIONAL(MING_VERSION_0_4,
        [ test $MING_VERSION_CODE -ge 00040000  ])
AM_CONDITIONAL(MAKESWF_SUPPORTS_PREBUILT_CLIPS,
        [ test $MING_VERSION_CODE -ge 00040002  ])
AM_CONDITIONAL(MING_SUPPORTS_INIT_ACTIONS,
        [ test $MING_VERSION_CODE -ge 00040004  ])
AM_CONDITIONAL(MING_SUPPORTS_REPLACE_TAG,
        [ test $MING_VERSION_CODE -ge 00040005 ])
AM_CONDITIONAL(MING_SUPPORTS_SWFBUTTON_ADD_CHARACTER,
        [ test $MING_VERSION_CODE -ge 00040005 ])
AM_CONDITIONAL(MING_SUPPORTS_STREAMING_SOUND,
        [ test $MING_VERSION_CODE -ge 00040006 ])
AM_CONDITIONAL(MING_SUPPORTS_SWFBUTTON_SET_DEPTH,
        [ test $MING_VERSION_CODE -ge 00040006 ])
AM_CONDITIONAL(MING_VERSION_0_4_3,
        [ test $MING_VERSION_CODE -ge 00040300 ])
AM_CONDITIONAL(MING_VERSION_0_4_4,
        [ test $MING_VERSION_CODE -ge 00040400 ])

if test x$cross_compiling = xno; then
    AC_ARG_WITH([swfdec_testsuite],
        AC_HELP_STRING([--with-swfdec-testsuite],
            [directory where swfdec testsuite (the 'test' dir) is]),
        if test x"${withval}" = xyes; then
            dir=`dirname $0`
            dir=`cd ${dir} && pwd`
            withval="${dir}/testsuite/swfdec/src/test"
        fi
        SWFDEC_TESTSUITE=${withval}
    )
    AC_SUBST(SWFDEC_TESTSUITE)
fi
AM_CONDITIONAL(ENABLE_SWFDEC_TESTSUITE, [ test x"$SWFDEC_TESTSUITE" != x ])

if test x$cross_compiling = xno; then
  AC_ARG_ENABLE([http_testsuite],
      dnl # TODO: find out how to add [quotes] around '=<baseurl>'
	  AC_HELP_STRING([--enable-http-testsuite=<baseurl>],
		  [Enable http based testsuite (default url is http://www.gnashdev.org/testcases)]),
	      [case "${enableval}" in
             no) HTTP_TESTSUITE="" ;;
            yes) HTTP_TESTSUITE="http://www.gnashdev.org/testcases" ;;
              *) HTTP_TESTSUITE="${enableval}";;
           esac])
  AC_SUBST(HTTP_TESTSUITE)
fi
AM_CONDITIONAL(ENABLE_HTTP_TESTSUITE, [ test x"$HTTP_TESTSUITE" != x ])

if test x$cross_compiling = xno; then
  AC_ARG_ENABLE([red5_testing],
      dnl # TODO: find out how to add [quotes] around '=<baseurl>'
	  AC_HELP_STRING([--enable-red5-testing=<host>],
		  [Enable red5 based testing (default host is www.gnashdev.org)]),
	      [case "${enableval}" in
             no) RED5_HOST="" ;;
            yes) RED5_HOST="www.gnashdev.org" ;;
              *) RED5_HOST="${enableval}";;
           esac])
  AC_SUBST(RED5_HOST)
fi
AM_CONDITIONAL(ENABLE_RED5_TESTING, [ test x"$RED5_HOST" != x ])

if test x$cross_compiling = xno; then
  AC_ARG_ENABLE([rtmpy_testing],
      dnl # TODO: find out how to add [quotes] around '=<baseurl>'
	  AC_HELP_STRING([--enable-rtmpy-testing=<host>],
		  [Enable rtmpy based testing (default host is www.gnashdev.org)]),
	      [case "${enableval}" in
             no) RTMPY_HOST="" ;;
            yes) RTMPY_HOST="www.gnashdev.org" ;;
              *) RTMPY_HOST="${enableval}";;
           esac])
  AC_SUBST(RTMPY_HOST)
fi
AM_CONDITIONAL(ENABLE_RTMPY_TESTING, [ test x"$RTMPY_HOST" != x ])

dnl The name might differ between systems.
if test x"$testsuite" = x"yes"; then
AC_PATH_PROGS(NETCAT, [nc netcat])
AC_PATH_PROG(WGET, wget)
fi
AM_CONDITIONAL(HAS_NETCAT, [ test x"$NETCAT" != x ])
AM_CONDITIONAL(HAS_WGET, [ test x"$WGET" != x ])

dnl
dnl See if we can use the swfmill, mtasc, swfc and haxe based testsuites 
dnl
if test x"$testsuite" = x"yes" -a x$cross_compiling = xno; then
  AC_PATH_PROG(AS3COMPILE, as3compile)
  AC_PATH_PROG(SWFC, swfc)
  AC_PATH_SWFMILL
  AC_PATH_MTASC
  AC_PATH_HAXE
fi

AM_CONDITIONAL(ENABLE_SWFMILL, [ test x"$SWFMILL" != x ])
AM_CONDITIONAL(SWFMILL_AS3_SUPPORT,
        [ test x"$SWFMILL" != x && test $SWFMILL_VERSION -ge 00021206 ])

dnl SWFMILL versions older than 0.3 didn't get function2 flags order correctly
AM_CONDITIONAL(SWFMILL_FUNCTION2_FLAGS_ORDER_CORRECT,
        [ test x"$SWFMILL" != x && test $SWFMILL_VERSION -ge 00021206 ])

AM_CONDITIONAL(ENABLE_AS3COMPILE, [ test x"$AS3COMPILE" != x ])
AM_CONDITIONAL(ENABLE_MTASC, [ test x"$MTASC" != x ])
AM_CONDITIONAL(ENABLE_HAXE, [ test x"$HAXE" != x ])
AM_CONDITIONAL(ENABLE_SWFC, [ test x"$SWFC" != x ])
AM_CONDITIONAL(ENABLE_HAXE, [ test x"$HAXE" != x ])

AC_PATH_PROG(DOXYGEN, doxygen)
AM_CONDITIONAL(ENABLE_DOXYGEN, [ test x"$DOXYGEN" != x ])

AM_CONDITIONAL(HAVE_KDE3,    [test x$has_kde3 = xyes])
AM_CONDITIONAL(HAVE_KDE4,    [test x$has_kde4 = xyes])
AM_CONDITIONAL(HAVE_QT3,     [test x$has_qt3 = xyes])
AM_CONDITIONAL(HAVE_QT4,     [test x$has_qt4 = xyes])

AM_CONDITIONAL([QT_X11],     [test "$platform" = "X11"])
AM_CONDITIONAL([QTOPIA],     [test "$platform" = "Qtopia"])
AM_CONDITIONAL([QT_OSX],     [test "$platform" = "OSX"])
AM_CONDITIONAL([QT_OS9],     [test "$platform" = "OS9"])
AM_CONDITIONAL([QT_WIN32],   [test "$platform" = "Win32"])

dnl Need GLIB for both GTK and GST
if test x"${build_gtk}" = xyes -o x"${build_media_gst}" = xyes; then
  GNASH_PATH_GLIB
fi

AM_CONDITIONAL(HAVE_GLIB, [ test x"${has_glib}" = xyes ])

if test x$npapi = xyes; then
  if ! test x$build_gtk = xyes -o x$build_qt4 = xyes; then
    AC_MSG_WARN(["Enabled NPAPI plugin, but it's not supported by the selected GUI"])
  fi
fi

if test x$windows = xyes -a x$npapi = xyes; then
  if test "x$NSPR_CFLAGS" = x -a "x$NSPR_LIBS" = x; then
    AC_MSG_ERROR(["On Win32, NPAPI plugin requires NSPR."])
  fi
  if test "x$WINDRES" = x; then
    AC_MSG_ERROR(["On Win32, NPAPI plugin requires windres."])
  fi
fi

dnl Need GLIB for NPAPI plugin
if test x$npapi = xyes; then
  GNASH_PATH_GLIB
fi

dnl if kde isn't installed, even if it's specified, don't try to build
dnl the KPARTS plugin, which is KDE based.
if test x$has_kde3 = xno -a x$build_kparts3 = xyes; then
  build_kparts3=no
  AC_MSG_WARN(["Disabling KPARTS 3.x plugin, no KDE development found"])
fi

if test x$build_kde3 = xno -a x$build_kparts3 = xyes; then
  AC_MSG_WARN(["Enabled KPARTS 3.x plugin, but you aren't building a KDE based GUI!"])
fi
if test x$has_kde4 = xno -a x$build_kparts4 = xyes; then
  build_kparts4=no
  AC_MSG_WARN(["Disabling KPARTS 4.x plugin, no KDE 4.x development found"])
fi

if test x$build_qt4 = xno -a x$kparts4 = xyes; then
  AC_MSG_WARN(["Enabled KPARTS 4.x plugin, but you aren't building a KDE 4.x based GUI!"])
fi

AM_CONDITIONAL(BUILD_QTOPIA3_GUI,  [ test x$build_qtopia3 = xyes ])
AM_CONDITIONAL(BUILD_QTOPIA4_GUI,  [ test x$build_qtopia4 = xyes ])

AM_CONDITIONAL(BUILD_KDE3_GUI,     [ test x$build_kde3 = xyes ])
AM_CONDITIONAL(BUILD_QT4_GUI,     [ test x$build_qt4 = xyes ])

AM_CONDITIONAL(BUILD_GTK_GUI,      [ test x$build_gtk = xyes ])
AM_CONDITIONAL(BUILD_FLTK_GUI,     [ test x$build_fltk = xyes ])
AM_CONDITIONAL(BUILD_SDL_GUI,      [ test x$build_sdl = xyes ])
AM_CONDITIONAL(BUILD_FB_GUI,       [ test x$build_fb = xyes ])
AM_CONDITIONAL(BUILD_AQUA_GUI,     [ test x$build_aqua = xyes ])
AM_CONDITIONAL(BUILD_DUMP_GUI,     [ test x$build_dump = xyes ])
AM_CONDITIONAL(BUILD_AMIGAOS4_GUI, [ test x$build_aos4 = xyes ])
AM_CONDITIONAL(BUILD_HAIKU_GUI,    [ test x$build_haiku = xyes ])

# plugin building flags
AM_CONDITIONAL(NPAPI,   [test x"${npapi}" = xyes])
AM_CONDITIONAL(KPARTS3, [test x"${build_kparts3}" = xyes])
AM_CONDITIONAL(KPARTS4, [test x"${build_kparts4}" = xyes])

if test x"${build_gles}" = xyes; then
  GNASH_PKG_FIND(EGL, [EGL/egl.h], [EGL library], eglGetDisplay)
  GNASH_PATH_GLES
fi

if test x"${build_ogv}" = xyes; then
  GNASH_PKG_FIND(EGL, [EGL/egl.h], [EGL library], eglGetDisplay)
  GNASH_PKG_FIND(OpenVG, [vg/openvg.h], [OpenVG library], vgDrawImage)
fi

if test x"${build_gtk}" = xyes; then
  AC_ARG_ENABLE(ghelp,
    AC_HELP_STRING([--enable-ghelp], [Enable support for the GNOME help system]),
    [case "${enableval}" in
      yes) ghelp=yes ;;
      no)  ghelp=no ;;
      *)   AC_MSG_ERROR([bad value ${enableval} for enable-ghelp option]) ;;
    esac], ghelp=no
  )

  if test x"${ghelp}" = x"yes" ; then
    AC_PATH_PROG(SCROLLKEEPER, scrollkeeper-config, [], [$PATH:/usr/bin/X11:/usr/local/bin/X11:/opt/X11])
    AC_PATH_PROG(SCROLLUPDATE, scrollkeeper-update, [],	[$PATH:/usr/bin/X11:/usr/local/bin/X11:/opt/X11])
    AC_PATH_PROG(SCROLLINSTALL, scrollkeeper-preinstall, [], [$PATH:/usr/bin/X11:/usr/local/bin/X11:/opt/X11])

    if test x"$SCROLLKEEPER" = x -o x"$SCROLLUPDATE" = x -o x"$SCROLLINSTALL" = x ; then
      ghelp=no
      AC_MSG_WARN([You need to install scrollkeeper for gnome help])
    fi
  fi
fi
AM_CONDITIONAL(GHELP, [test x${ghelp} = xyes])

if test x${build_fltk} = xyes; then
  GNASH_PKG_FIND(Xft, [Xft.h], [xft library], XftGlyphRender)
fi

dnl Some systems have a pervered set of dependencies.
dnl Fedora Core 6 appears to have a dependency on expat for fontconfig.
dnl We only need the library, but this is the easy wind to find it.
GNASH_PKG_FIND(expat, [expat.h], [Expat library], XML_ErrorString)

dnl these conditionals were moved out of kde.m4
AM_CONDITIONAL(HAS_KDE3, [test x$has_kde3 = xyes])
# used to disable x11-specific stuff on special platforms
AM_CONDITIONAL(include_x11, test "$kde_use_qt_emb" = "no" && test "$kde_use_qt_mac" = "no")
AM_CONDITIONAL(include_ARTS, test "$build_arts" '!=' "no")
AM_CONDITIONAL(unsermake_enable_pch, test "$kde_use_pch" = "yes" && test "$kde_gcc_supports_pch" = "yes")

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
if test x"$build_media_gst" = "xyes"; then
  AC_PATH_PROG(GST_INSPECT, gst-inspect, ,[${pathlist}])
  if test "x$GST_INSPECT" != "x" -a x"${darwin}" = xno ; then
    AC_PATH_PROG(GST_INSPECT, gst-inspect-0.10, ,[${pathlist}])
  fi
  if test "x$GST_INSPECT" != "x" -a x"${darwin}" = xno ; then
    dnl FIXME: there may be multiple acceptable plugins that are acceptable for
    dnl our use. For example, mad or FFmpeg will play mp3.
    codecs="ffdec_flv ffdec_flashsv ffdec_vp6f ffdec_flashsv mad vorbisdec ffdec_vp6"
    for i in $codecs; do
       hits="`$GST_INSPECT $i | grep -c 'Long name'`"
       if test $hits -eq 0; then
         missing_codecs="$missing_codecs $i"
         AC_MSG_WARN([Missing codec: $i])
       fi
    done
  fi
  GNASH_PKG_FIND(gstreamer_plugins_base, [gst/interfaces/probeprobe.h], [gstreamer interfaces library], gst_property_probe_probe_and_get_values_name, [0.10])
  GNASH_PKG_FIND(gstreamer_app, [gst/app/gstappsink.h], [gstreamer app library], call_gmon_start, [0.10])
  GNASH_PKG_FIND(gstreamer, [gst/gst.h], [gstreamer library], gst_init, [0.10])
  dnl if cross compiling, we're usually not going to be able to pop up
  dnl the codec installer to download the package to install, so disable
  dnl it if cross compiling with gstreamer support.
  if test x$cross_compiling = xno; then
    GNASH_PKG_FIND(gstreamer_pbutils, [gst/pbutils/install-plugins.h], [gstreamer PB Utils library], gst_install_plugins_supported, [0.10])
  fi
  dnl when cross compiling Gstreamer, not all supplied SDKs include all the
  dnl development libraries since most devices don't need to support plugin
  dnl development, only the runtime. In these caes we often have the header
  dnl files but not the libraries.
  if test x"${has_gstreamer_plugins_base}" = xyes; then
    GSTREAMER_LIBS="-lgstinterfaces-0.10 $GSTREAMER_LIBS"
    AC_DEFINE(HAS_GSTREAMER_PLUGINS_BASE, [1], [Has the Gstreamer Plugin Dev package installed])
  fi
fi

AM_CONDITIONAL(HAVE_CAIRO, [true])
AM_CONDITIONAL(HAVE_OPENGL, [true])

AM_CONDITIONAL(USE_SOUND_SDL, test x$build_sound_sdl = xyes)
AM_CONDITIONAL(USE_SOUND_AHI, test x$build_sound_ahi = xyes)
AM_CONDITIONAL(USE_SOUND_MKIT, test x$build_sound_mkit = xyes)
AM_CONDITIONAL(USE_FFMPEG_ENGINE, test x"${build_media_ffmpeg}" = x"yes")
AM_CONDITIONAL(USE_GST_ENGINE, test x"${build_media_gst}" = x"yes")
AM_CONDITIONAL(HAVE_OPENGL, test x"${OPENGL_LIBS}" != x)
dnl for now the Haiku media handler is experimental
AM_CONDITIONAL(USE_HAIKU_ENGINE, test x"$build_media_haiku" = xyes)

if test x"${build_media_ffmpeg}" = x"yes"; then
  AC_DEFINE([ENABLE_FFMPEG_MEDIA],  [1], [Use FFmpeg for media decoding])
fi

if test x"${build_media_gst}" = x"yes"; then
  AC_DEFINE([ENABLE_GST_MEDIA],  [1], [Use gstreamer for media decoding])
fi

if test x"${build_media_haiku}" = x"yes"; then
  AC_DEFINE([ENABLE_HAIKU_MEDIA],  [1], [Use haiku for media decoding])
fi

if test x$build_sound_mkit = xyes; then
  if test x"${haiku}" != xyes; then
    AC_MSG_ERROR([Media Kit sound handling is supported only under Haiku]);
  else
    AC_DEFINE([SOUND_MKIT],  [1], [Use Haiku Media Kit for sound handling])
  fi
fi

if test x"${build_sound_sdl}" = xyes; then
  AC_DEFINE([SOUND_SDL],  [1], [Use SDL for sound handling])
fi

if test x"${build_sound_ahi}" = xyes; then
  if test x"$amigaos4" != xyes; then
    AC_MSG_ERROR([AHI sound handling is supported only under AmigaOS]);
  else
    AC_DEFINE([SOUND_AHI],  [1], [Use AmigaOS AHI for sound handling])
  fi
fi

if test x$build_fltk = xyes; then
  GNASH_PKG_FIND(fltk2, [fltk/FL_API.h], [Fast Light Toolkit], fl_window_flush)
  if test x"${has_fltk2}" = xyes; then
    AC_CHECK_LIB(Xi, XInput_find_display,
      [AC_SUBST(FLTK2_LIBS, "$FLTK2_LIBS -lXi")])
  fi
fi

AM_CONDITIONAL(HAVE_FLTK2, [ test x$has_fltk2 = xyes ])
AM_CONDITIONAL(HAS_XFT, [ test x$has_xft = xyes ])

GNASH_DOCBOOK
AM_CONDITIONAL(ENABLE_INFO, test x${INSTALL_INFO} != x)
AM_CONDITIONAL(DOCBOOK, test x$docbook = xyes)
AM_CONDITIONAL(ENABLE_TEXI, [ test x"$DB2X_TEXI" != x -o x"$DB2X_TEXIXML" != x ])
AM_CONDITIONAL(ENABLE_PDF, [ test x"$DB2X_PDF" ])
AM_CONDITIONAL(ENABLE_HTML, [ test x"$XSLTPROC" != x -a x"$docbook_styles" != x ])
AM_CONDITIONAL(ENABLE_FOP, [ test x"$FOP" != x -a x"$docbook_styles" != x ])
AM_CONDITIONAL(ENABLE_XMLTEX, [ test x"$PDFXMLTEX" != x -a x"$XSLTPROC" != x -a x"$docbook_styles" != x ])
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
  AC_COMPILE_IFELSE([AC_LANG_SOURCE(
    [
      void __attribute__ ((visibility("default"))) test_default (void) {}
      void __attribute__ ((visibility("hidden"))) test_hidden (void) {}
      int main (int argc, char **argv) { test_default (); test_hidden (); return 0; }
    ])],
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

AC_ARG_ENABLE([pch],
  AC_HELP_STRING([--enable-pch], [Enable precompiled header support]), [], [enable_pch=no])

AM_CONDITIONAL([ENABLE_PCH], [test x"$enable_pch" != x"no"])

PCH_FLAGS="-include all-includes.h -Winvalid-pch"
AC_SUBST(PCH_FLAGS)

GNASH_PATH_PTHREADS

GNASH_PATH_BOOST

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
#
# -ansi was actually dropped because it hides 'fileno', which
# is used in a few places
#
if test x"$GCC" = x"yes"; then
  CXXFLAGS="$CXXFLAGS \
    $CROSS_CXXFLAGS \
    -W \
    -Wall \
    -Wcast-align \
    -Wcast-qual \
    -Wpointer-arith \
    -Wreturn-type \
    -Wnon-virtual-dtor \
    -Wunused \
    "
  CFLAGS="$CFLAGS \
    $CROSS_CXXFLAGS \
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
    AC_CACHE_CHECK([if -fvisibility-inlines-hidden is broken], ac_cv_gcc_visibility_bug, [
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
        ac_cv_gcc_visibility_bug=no, ac_cv_gcc_visibility_bug=yes)

        CXXFLAGS=$save_CXXFLAGS
        LDFLAGS=$save_LDFLAGS
        AC_LANG_POP(C++)
      ]
    )
    if test x$ac_cv_gcc_visibility_bug = xno; then
      CXXFLAGS="$CXXFLAGS -fvisibility-inlines-hidden"
    fi
  ]
)

CHECK_VISIBILITY_GCC_BUG

if test x$ac_cv_gcc_visibility_bug = xno; then
  AC_LANG_PUSH(C++)
  AC_MSG_CHECKING([whether $CXX supports -fvisibility=hidden])
  save_CXXFLAGS=$CXXFLAGS
  CXXFLAGS="$CXXFLAGS -fvisibility=hidden"
  AC_LINK_IFELSE([AC_LANG_PROGRAM()], 
                 [ac_cv_gcc_visibility=yes;
                  AC_MSG_RESULT([yes])],
                 [ac_cv_gcc_visibility=no;
                  AC_MSG_RESULT([no])]);
  CXXFLAGS="$save_CXXFLAGS"
  AC_LANG_POP(C++)
fi


AM_CONDITIONAL(VISIBILITY_WORKS, test x"$ac_cv_gcc_visibility" = xyes)

if test x"$ac_cv_gcc_visibility" = xyes -a x"$enable_visibility" != xno; then
  CXXFLAGS="$CXXFLAGS -fvisibility=hidden"
fi

dnl Define convenience constants so Gnash can print out the
dnl default configuration of the build.
RENDERER_CONFIG="${renderer_list}"
RENDERER_LIBS=
for rend in `echo "${add_renderer}" | tr ',' ' '`; do
	RENDERER_LIBS="${RENDERER_LIBS} \$(top_builddir)/librender/libgnash${rend}.la"
done
AC_SUBST(RENDERER_LIBS)
AC_SUBST(RENDERER_CONFIG)

HWACCEL_CONFIG="${hwaccel_list}"
AC_SUBST(HWACCEL_CONFIG)

dnl check for missing libraries and disable them.
if test x"$BOOST_LIBS" != x; then
  if test x"${cygnal_missing_libs}" != x; then
    for i in ${cygnal_missing_libs}; do
      if test $i = serialization; then
        AC_DEFINE([BOOST_MULTI_INDEX_DISABLE_SERIALIZATION], ["1"], [if the library is missing, don't use it.])
      fi
    done
  fi
fi

CYGNAL_PATHS

SUPPORTED_GUIS=
if test x$build_qtopia3 = xyes; then
  SUPPORTED_GUIS="${SUPPORTED_GUIS},qtopia3"
fi
if test x$build_qtopia4 = xyes; then
  SUPPORTED_GUIS="${SUPPORTED_GUIS},qtopia4"
fi
if test x$build_fb = xyes; then
  SUPPORTED_GUIS="${SUPPORTED_GUIS},fb"
fi
if test x$build_fltk = xyes; then
  SUPPORTED_GUIS="${SUPPORTED_GUIS},fltk"
fi
if test x$build_kde3 = xyes; then
  SUPPORTED_GUIS="${SUPPORTED_GUIS},kde3"
fi
if test x$build_qt4 = xyes; then
  SUPPORTED_GUIS="${SUPPORTED_GUIS},qt4"
fi
if test x$build_gtk = xyes; then
  SUPPORTED_GUIS="${SUPPORTED_GUIS},gtk"
fi
if test x$build_sdl = xyes; then
  SUPPORTED_GUIS="${SUPPORTED_GUIS},sdl"
fi
if test x$build_aqua = xyes; then
  SUPPORTED_GUIS="${SUPPORTED_GUIS},aqua"
fi
if test x$build_dump = xyes; then
  SUPPORTED_GUIS="${SUPPORTED_GUIS},dump"
fi
if test x$build_aos4 = xyes; then
  SUPPORTED_GUIS="${SUPPORTED_GUIS},aos4"
fi
if test x$build_haiku = xyes; then
  SUPPORTED_GUIS="${SUPPORTED_GUIS},haiku"
fi
SUPPORTED_GUIS="`echo ${SUPPORTED_GUIS} | sed 's/,//'`" # Strip leading comma
AC_SUBST(SUPPORTED_GUIS)

AC_CONFIG_LINKS(cygnal/testsuite/cygnal.all/cygnalrc:cygnal/testsuite/cygnal.all/cygnalrc.in)
AC_CONFIG_LINKS(testsuite/libbase.all/gnashrc:testsuite/libbase.all/gnashrc.in)
AC_CONFIG_LINKS(testsuite/libbase.all/gnashrc-local:testsuite/libbase.all/gnashrc-local.in)

AC_CONFIG_FILES(
gnash.pc:gnash.pc.in
desktop/gnash-gtk-launcher:desktop/gnash-gtk-launcher.in
desktop/gnash-qt-launcher:desktop/gnash-qt-launcher.in
)

AC_OUTPUT(Makefile
desktop/Makefile
po/Makefile
libmedia/Makefile
libsound/Makefile
libbase/Makefile
libcore/Makefile
libcore/vm/Makefile
libcore/parser/Makefile
libvaapi/Makefile
librender/Makefile
utilities/Makefile
doc/Makefile
doc/C/Makefile
doc/Doxyfile
testsuite/Makefile
testsuite/media/Makefile
testsuite/libbase.all/Makefile
testsuite/as3compile.all/Makefile
testsuite/actionscript.all/Makefile
testsuite/samples/Makefile
testsuite/swfdec/Makefile
testsuite/misc-ming.all/Makefile
testsuite/misc-ming.all/action_order/Makefile
testsuite/misc-ming.all/displaylist_depths/Makefile
testsuite/misc-ming.all/loop/Makefile
testsuite/misc-ming.all/loading/Makefile
testsuite/misc-mtasc.all/Makefile
testsuite/misc-haxe.all/Makefile
testsuite/misc-haxe.all/classes.all/Makefile
testsuite/misc-swfmill.all/Makefile
testsuite/misc-swfmill.all/trace-as2/Makefile
testsuite/misc-swfmill.all/trace-as3/Makefile
testsuite/misc-swfc.all/Makefile
testsuite/network.all/Makefile
testsuite/movies.all/Makefile
testsuite/libcore.all/Makefile
testsuite/libmedia.all/Makefile
gui/Makefile
gui/fb/Makefile
gui/dump/Makefile
gui/Info.plist
gui/pythonmod/Makefile
extensions/Makefile
extensions/dejagnu/Makefile
extensions/mysql/Makefile
extensions/fileio/Makefile
extensions/gtk2/Makefile
extensions/lirc/Makefile
extensions/dbus/Makefile
plugin/Makefile
plugin/npapi/Makefile
plugin/klash/Makefile
plugin/klash4/Makefile
plugin/win32/Makefile
plugin/aos4/Makefile
cygnal/Makefile
cygnal/libnet/Makefile
cygnal/libamf/Makefile
cygnal/cgi-bin/Makefile
cygnal/cgi-bin/echo/Makefile
cygnal/cgi-bin/oflaDemo/Makefile
cygnal/cgi-bin/fitcDemo/Makefile
cygnal/testsuite/Makefile
cygnal/testsuite/libamf.all/Makefile
cygnal/testsuite/libnet.all/Makefile
cygnal/testsuite/cygnal.all/Makefile
)

###
########################## Final report begins... ############################
###

dnl Create temporary directory in a secure way
tmp=`mktemp -d ${TMPDIR=/tmp}/gnash-configure-XXXXXX`
if test \! -n "$tmp" || test \! -d "$tmp"; then
  tmp=`(umask 077 && mkdir -d ${TMPDIR=/tmp}/gnash-configure-${RANDOM}-$$) 2>/dev/null`
fi
cerr=${tmp}/errors
cwarn=${tmp}/warnings
crec=${tmp}/recommended
echo ""

#trap 'rm cerr' 0 # trap isn't a good idea, might override other traps
exec 3> $cerr 
exec 4> $cwarn
exec 5> $crec

echo "Configurable options are:"

if test x"${pthreads}" = x"yes"; then
  echo "        POSIX Threads support enabled (default)"
else
  if test x"${build_haiku}" = x"yes"; then
     echo "        POSIX Thread support built into C library."
  els
     echo "        POSIX Thread support disabled."
  fi
fi

if test x"${npapi}" = x"yes"; then
  echo "        NPAPI plugin enabled (default). Use --disable-npapi to disable."
  echo "            NPAPI plugin will be installed in ${FIREFOX_PLUGINS}"
else
  echo "        NPAPI plugin disabled."
fi

if test x"${build_kparts3}" = x"yes"; then
  echo "        KPARTS 3.x plugin enabled (default). Use --disable-kparts3 to disable"
  echo "            KPARTS 3.x plugin will be installed in ${KDE3_PLUGINDIR}"
  echo "            KPARTS 3.x service will be installed in ${KDE3_SERVICESDIR}"
  echo "            KPARTS 3.x config dir will be in ${KDE3_CONFIGDIR}"
  echo "            KPARTS 3.x appsdata will be installed in ${KDE3_APPSDATADIR}"
else
  echo "        KPARTS 3.x plugin disabled."
fi

if test x"${build_kparts4}" = x"yes"; then
  echo "        KPARTS 4.x plugin enabled (default). Use --disable-kparts4 to disable"
  echo "            KPARTS 4.x plugin will be installed in ${KDE4_PLUGINDIR}"
  echo "            KPARTS 4.x service will be installed in ${KDE4_SERVICESDIR}"
  echo "            KPARTS 4.x config dir will be in ${KDE4_CONFIGDIR}"
  echo "            KPARTS 4.x appsdata will be installed in ${KDE4_APPSDATADIR}"
else
  echo "        KPARTS 4.x plugin disabled."
fi

# set a variable if we shouldn't continue. This way we can print
# out everything that is missing in one pass, hopefully making it
# easy for new developers to get everything they need installed.

echo "Configured paths for ${build} are:"

dnl Dump QT3 options is the user specified a QTOPIA3 or KDE3 GUI
if test x"${build_kde3}" = xyes -o x"${build_qtopia3}" = xyes; then
  if test x"${has_qt3}" = xyes; then
    echo "        QT3 flags are: ${QT3_CFLAGS}"
    echo "        QT3 libs are: ${QT3_LIBS}"
  else
    echo "        ERROR: No QT 3.x development package installed!" >&3
    echo "               Install a QT 3.x development environment from http://trolltech.com" >&3
    echo "               or .deb users: apt-get install libqt3-mt-dev" >&3
    echo "               or change to a different gui with --enable-gui=..." >&3
  fi
fi

dnl Dump QT4 options is the user specified a QTOPIA4 or Qt4 GUI
if test x"${build_qt4}" = xyes -o x"${build_qtopia4}" = xyes; then
  if test x"${has_qt4}" = xyes; then
    echo "        QT4 flags are: ${QT4_CFLAGS}"
    echo "        QT4 libs are: ${QT4_LIBS}"
  else
    echo "        ERROR: No QT 4.x development package installed!" >&3
    echo "               Install a QT 4.x development environment from http://trolltech.com" >&3
    echo "               or .deb users: apt-get install qt4-dev-tools" >&3
    echo "               or change to a different gui with --enable-gui=..." >&3
  fi
fi

if test x"${build_kparts4}" = xyes; then
  if test x"${has_kde4}" = xyes; then
    echo "        KDE4 flags are: ${KDE4_CFLAGS}"
    echo "        KDE4 libs are: ${KDE4_LIBS}"
  else
    echo "        kparts4 plugin is disabled"
    echo "               To enable the KDE 4.x gui," >&3
    echo "               install version 4.x of the KDE development environment" >&3
    echo "               from http://kde.org" >&3
    echo "               or .deb users: apt-get install kdelibs5-dev" >&3
    echo "               or .rpm users: yum install kdelibs-devel." >&3
  fi
fi

if test x"$build_qtopia3" = xyes; then
  if test x"${QTOPIA3_LIBS}" != x ; then
    if test x"${QTOPIA3_CFLAGS}" != x ; then
      echo "        QTOPIA 3.x flags are: $QTOPIA3_CFLAGS"
    else
      echo "        QTOPIA 3.x flags are: default include path"
    fi
    echo "        QTOPIA 3.x libs are: $QTOPIA3_LIBS"
  else
    echo "        ERROR: No QTOPIA 3.x library development package installed!" >&3
    echo "               Install it from http://trolltech.com/downloads/" >&3
    echo "               binary packages are not available." >&3
  fi
fi

if test x"$build_qtopia4" = xyes; then
  if test x"${QTOPIA4_LIBS}" != x ; then
    if test x"${QTOPIA4_CFLAGS}" != x ; then
      echo "        QTOPIA 4.x flags are: $QTOPIA4_CFLAGS"
    else
      echo "        QTOPIA 4.x flags are: default include path"
    fi
    echo "        QTOPIA 4.x libs are: $QTOPIA4_LIBS"
  else
    echo "        ERROR: No QTOPIA 4.x library development package installed!" >&3
    echo "               Install it from http://trolltech.com/downloads/" >&3
    echo "               binary packages are not available." >&3
  fi
fi


# -o x$build_kparts3 = xyes
if test x$build_kde3 = xyes; then
  if test x"$has_kde3" = xyes; then
    echo "        KDE 3.x flags are: $KDE3_CFLAGS"
    echo "        KDE 3.x libs are: $KDE3_LIBS"
  else
      echo "        ERROR: No KDE 3.x development package installed!" >&3
      echo "               To disable the KDE 3.x gui," >&3
      echo "               reconfigure using --enable-gui=<list-of-guis>" >&3
      echo "               and omit "kde" from the list." >&3
      echo "               or avoid --enable-gui=... as a whole." >&3
      echo "               To be able to build the kde 3.x gui," >&3
      echo "               install version 3.x of the KDE development environment" >&3
      echo "               from http://kde.org" >&3
      echo "               or .deb users: apt-get install kdelibs4-dev" >&3
      echo "               or .rpm users: yum install kdelibs3-devel." >&3
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

if test x"${PNG_LIBS}" != x ; then
  if test x"${PNG_CFLAGS}" != x ; then
    echo "        PNG flags are: $PNG_CFLAGS"
  else
    echo "        PNG flags are: default include path"
  fi
  echo "        PNG libs are: $PNG_LIBS"
else
  echo "        RECOMMENDED: No PNG library development package installed!" >&5
  echo "                     Gnash will be built without support for dynamic loading of PNG files." >&5
  echo "                     Install it from http://www.libpng.org" >&5
  echo "                     or .deb users: apt-get install libpng12-dev" >&5
  echo "                     or .rpm users: yum install libpng-devel" >&5
fi

if test x"${build_ogl}" = x"yes"; then
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

if test x"${build_gles}" = x"yes"; then
  if test x"${has_gles}" != x; then
    if test x"$GLES_CFLAGS" != x; then
      echo "        OpenGL-ES flags are: $GLES_CFLAGS"
    else
      echo "        OpenGL-ES flags are: default include path"
    fi
    echo "        OpenGL-ES libs are: $GLES_LIBS"
  else
      echo "        ERROR: No OpenGL-ES development package installed!" >&3
      echo "               You need to install the this from source proobably" >&3
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

if test x"$build_media_gst" = x"yes"; then
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
    if test x"$has_gstreamer_pbutils" != "xyes"; then
      echo "        RECOMMENDED: If the user has not installed the necessary Gstreamer plugins," >&5
      echo "                     Gstreamer can pop up a message prompting them to." >&5
      echo "                     Install gstpbutils (>= 0.10.15) from http://www.gstreamer.net for that to be enabled" >&5
      echo "                     or .deb users: apt-get install libgstreamer-plugins-base0.10-dev" >&5
      echo "                     Also see --with-gstpbutils-incl and --with-gstpbutils-lib" >&5
    fi
    if test x"$has_gstreamer_plugins_base" = "xno"; then
      dnl check if this is really a mandatory asset !
      echo "        ERROR: base plugins are required for gstreamer media" >&3
      echo "                        Install gstreamer-plugins-base from http://www.gstreamer.net" >&3
      echo "                        or .rpm users: yum install gstreamer-plugins-base-devel" >&3
      echo "                        or .deb users: apt-get install libgstreamer-plugins-base0.10-dev" >&3
    fi
  else
    echo "        ERROR: GST media handling requested but gstreamer-0.10+ not found" >&3
    echo "               Install it from http://www.gstreamer.net" >&3
    echo "               or .deb users: apt-get install libgstreamer0.10-dev" >&3
    echo "               or .rpm users: yum install gstreamer-devel" >&3
    echo "               or             yast install gstreamer010-devel" >&3
  fi
fi

  if test x"${build_media_ffmpeg}" = x"yes"; then
    if test x"$FFMPEG_LIBS" != x; then
      echo "        MP3 and video support enabled through FFmpeg"
      if test x"${ffmpeg_version_check}" != xok; then
        echo "        ERROR: You have version ${ffmpeg_version} of FFmpeg installed," >&3
        if test x"${have_ffmpeg_swscale}" = "xno"; then
          echo "               with no swscale enabled." >&3
        else
          echo "               with swscale enabled." >&3
        fi
        echo "               This setup isn't supported!" >&3
        echo "               Version 51.11.0 or newer is required, enabling swscale if >= 52.0.0." >&3
        echo "               You can install libswscale from http://ffmpeg.org" >&3
        echo "               or .deb users: apt-get install libswscale-dev" >&3
      else
	if test x"${avformat_h}" = x; then
          echo "        ERROR: FFmpeg's libavcodec header is installed but not libavformat." >&3
          echo "               You can install FFmpeg from http://ffmpeg.org" >&3
          echo "               or .deb users: apt-get install libavformat-dev" >&3
          echo "               or YaST users: yast -i libavformat-api (but currently installs into /usr/lib64)" >&3
          echo "               or explicitly set the path using --with-ffmpeg-incl=" >&5
          echo "               or reconfigure with --enable-media=gst" >&3
	else
          if test x"$FFMPEG_CFLAGS" != x; then
            echo "        FFmpeg flags are: $FFMPEG_CFLAGS"
          else
            echo "        FFmpeg flags are: default include path"
          fi
          echo "        FFmpeg libs are: $FFMPEG_LIBS"
	fi
      fi
    else
      echo "        ERROR: No FFmpeg development package installed!" >&3
      echo "               You can install FFmpeg from http://ffmpeg.org" >&3
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
    echo "               see http://www.fltk.org to install it from source." >&3
    echo "               or change to a different gui with --enable-gui=..." >&3
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
  fi
fi

# See whether SDL is required
need_sdl=false
test x$build_sdl = xyes			&& need_sdl=true
test x$build_sound_sdl = xyes	&& need_sdl=true

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
  fi
fi
unset need_sdl

if test x"$nsapi" = x"yes"; then
    echo "        Plugin will be installed in ${FIREFOX_PLUGINS}"
fi

if test x"${pthreads}" = x"yes"; then
  if test x"$PTHREAD_CFLAGS" != x; then
    echo "        POSIX Threads flags are: $PTHREAD_CFLAGS"
  fi
  if test x"${PTHREAD_LIBS}" != x; then
    echo "        POSIX Threads lib is: $PTHREAD_LIBS"
  else
    if test x"${cross_compiling}" = xno; then
      echo "ERROR: No pthread development package installed!" >&3
    fi
  fi
fi

if test x"${docbook}" = x"yes"; then
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
else
  echo "        WARNING: without --enable-docbook we will use the cached" >&4
  echo "                 documentation files included in the gnash distribution." >&4
  echo "                 If you change files in doc/C, you should --enable-docbook." >&4
fi

if test x"$CURL_LIBS" != x; then
  if test x"$CURL_CFLAGS" != x; then
    echo "        CURL flags are: $CURL_CFLAGS"
  else
    echo "        CURL flags are: default include path"
  fi
    echo "        CURL libs are: $CURL_LIBS"
else
  echo "        RECOMMENDED: If you install the CURL library, Gnash will be able to" >&5
  echo "                     display remote content (streaming from URLs) using CURL." >&5
  echo "                     Install libcurl from http://curl.haxx.se/libcurl" >&5
  echo "                     or .deb users: apt-get install libcurl-dev" >&5
  echo "                     or .rpm users: yum install curl-devel" >&5
fi

if test x"$SPEEX_LIBS" != x; then
  if test x"$SPEEX_CFLAGS" != x; then
    echo "        Speex flags are: $SPEEX_CFLAGS"
  else
    echo "        Speex flags are: default include path"
  fi
    echo "        Speex libs are: $SPEEX_LIBS"
else
  echo "        RECOMMENDED: If you install the Speex library, Gnash will be able to" >&5
  echo "                     decoded Speex encoded audio in FLV files." >&5
  echo "                     Install libspeex from http://speex.org" >&5
  echo "                     or .deb users: apt-get install libspeex-dev" >&5
  echo "                     or .rpm users: yum install speex-devel" >&5
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
	echo "        BOOST flags are: $BOOST_CFLAGS"
	echo "        BOOST libs are: $BOOST_LIBS"
	echo "        BOOST libs for cygnal are: $BOOST_CYGNAL_LIBS"
    dnl fi
    if test x"${missing_headers}" != x; then
      for i in ${missing_headers}; do
	echo "        ERROR: The BOOST $i header file is needed!" >&3
	echo "               Install it from http://boost.org" >&3
	echo "               or from a Boost development package" >&3
      done
    fi

    if test x"${cygnal}" = x"yes"; then
        if test x"${cygnal_missing_libs}" != x; then
          for i in ${cygnal_missing_libs}; do
            echo "          ERROR: The BOOST $i library is required for cygnal!" >&4
            echo "                 Either configure with --disable-cygnal or" >&4
            echo "                 install it from http://www.boost.org" >&4
            echo "                 or .deb users: apt-get install libboost-"`echo ${i} | sed 's/_/-/g'`"-dev" >&4
          done
        fi
    fi

    if test x"${missing_libs}" != x; then
      for i in ${missing_libs}; do
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
if test x"$testsuite" = x"yes"; then
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
      echo "                 version 0.4 or higher installed to run" >&4
      echo "                 many of the tests in Gnash testsuite." >&4
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
      echo "        WARNING: You need to have the HAXE compiler package " >&4
      echo "                 version 2.00 or higher installed" >&4
      echo "                 to run some of the tests in Gnash testsuite." >&4
      echo "                 You can install it from http://haxe.org" >&4
      echo "                 or .deb users: apt-get install haxe" >&4
    fi

    if test x"$SWFMILL" != x; then
      echo "        SWFMILL is $SWFMILL"
      if test x"$ENABLE_AVM2" != x -a "$SWFMILL_VERSION" -lt 00021206; then
        echo "        WARNING: You are building Gnash with AVM2 support but" >&4
        echo "                 your swfmill version is too old to run AS3" >&4
        echo "                 tests." >&4
      fi
    else
      echo "        WARNING: You need to have the 'swfmill' tool installed" >&4
      echo "                 to run some of the tests in Gnash testsuite." >&4
      echo "                 You can install it from http://swfmill.org/" >&4
      echo "                 or .deb users: apt-get install swfmill" >&4
    fi

    if test x"$SWFC" != x; then
      echo "        SWFC is $SWFC"
    else
      echo "        WARNING: You need to have 'swfc' from SWFTools installed" >&4
      echo "                 to run some of the tests in Gnash testsuite." >&4
      echo "                 You can install it from http://www.swftools.org/" >&4
      echo "                 or .deb users: apt-get install swftools" >&4
    fi

    if test x"$AS3COMPILE" != x; then
      echo "        AS3COMPILE is $AS3COMPILE"
    else
      echo "        WARNING: you need as3compile from SWFTools" >&4
      echo "                 to run some of the tests in Gnash testsuite." >&4
      echo "                 You can install it from http://www.swftools.org/" >&4
    fi

    if test x"$HTTP_TESTSUITE" != x; then
      echo "        HTTP testsuite dir is $HTTP_TESTSUITE"
    fi

    if test x"$RED5_HOST" != x; then
      echo "        RED5 testing host is $RED5_HOST"
    fi
  fi
fi

  if test x"$PERL" != x; then
    echo "        PERL is $PERL"
  else
    echo "        WARNING: You need to have perl installed" >&4
    echo "                 to run some of the tests in Gnash testsuite." >&4
  fi

if test x"$testsuite" = x"yes"; then
  if test x"$CSOUND" != x; then
    echo "        CSOUND is $CSOUND"
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
  echo "        RECOMMENDED: You need to have the zlib development packages installed" >&5
  echo "                     to play compressed SWF (most of them from version 6 up)" >&5
  echo "                     and to display some kinds of JPEG files." >&5
  echo "                     Install it from http://www.zlib.net" >&5
  echo "                     or .deb users: apt-get install zlib1g-dev" >&5
  echo "                     or .rpm users: yum install zlib-devel." >&5
  echo "                     It may still be possible to configure without zlib." >&5
fi

if test x"$FREETYPE2_LIBS" != x; then
  if test x"$FREETYPE2_CFLAGS" != x; then
    echo "        FreeType flags are: $FREETYPE2_CFLAGS"
  else
    echo "        FreeType flags are: default include path"
  fi
  echo "        FreeType libs are: $FREETYPE2_LIBS"
else
  echo "        RECOMMENDED: You need to have the freetype development packages installed" >&5
  echo "                     to use device fonts." >&5
  echo "                     Install it from http://www.freetype.org" >&5
  echo "                     or .deb users: apt-get install libfreetype6-dev" >&5
  echo "                     or .rpm users: yum install freetype-devel" >&5
  echo "                     It may still be possible to configure without freetype." >&5
fi

if test x"$FONTCONFIG_LIBS" != x; then
  if test x"$FONTCONFIG_CFLAGS" != x; then
    echo "        Fontconfig flags are: $FONTCONFIG_CFLAGS"
  else
    echo "        Fontconfig flags are: default include path"
  fi
  echo "        Fontconfig libs are: $FONTCONFIG_LIBS"
else
  echo "        RECOMMENDED: You need to have the fontconfig development packages installed" >&5
  echo "                     to use device fonts." >&5
  echo "                     Install it from http://www.fontconfig.org" >&5
  echo "                     or .deb users: apt-get install libfontconfig1-dev" >&5
  echo "                     or .rpm users: yum install fontconfig-devel" >&5
  echo "                     It may still be possible to configure without fontconfig." >&5
fi

if test x$ext_mysql = xyes; then
  if test x$mysql != xno; then
    if test x"$MYSQL_LIBS" != x; then
      echo "        MYSQL flags are: $MYSQL_CFLAGS"
      echo "        MYSQL libs are: $MYSQL_LIBS"
    else
      echo "        ERROR: No MySQL development package is installed." >&3
      echo "               Either reconfigure without --enable-extensions=mysql" >&3
      echo "               or install MySQL header files from http://www.mysql.org" >&3
      echo "               or .deb users: apt-get install libmysqlclient-dev" >&3
      echo "                 or .rpm users: yum install mysql-devel" >&4
    fi
  fi
fi

if test "$GMSGFMT" = ":"; then
  echo "        WARNING: You need the gettext package installed to use translations." >&4
  echo "                 Required for building a package or 'make distcheck'" >&4
  echo "                 Install it from http://www.gnu.org/software/gettext/" >&4
  echo "                 or .deb users: apt-get install gettext" >&4
  echo "                 or .rpm users: yum install gettext" >&4
fi

if test x"${build_vaapi}" = x"yes"; then
  if test x"${LIBVA_CFLAGS}" = xyes; then
      echo "        LIBVA flags are: default"
    else
      echo "        LIBVA flags are: $LIBVA_CFLAGS"
      echo "        LIBVA libraries are: $LIBVA_LIBS"
  fi
  if test x$use_libva_x11 = xyes; then
    if test x"${LIBVA_X11_CFLAGS}" = xyes; then
      echo "        LIBVA X11 flags are: default"
    else
      echo "        LIBVA X11 flags are: $LIBVA_X11_CFLAGS"
      echo "        LIBVA X11 libraries are: $LIBVA_X11_LIBS"
    fi
  fi
  if test x$use_libva_glx = xyes; then
    if test x"${LIBVA_GLX_CFLAGS}" = xyes; then
      echo "        LIBVA GLXflags are: default"
    else
      echo "        LIBVA GLX flags are: $LIBVA_GLX_CFLAGS"
    fi
    echo "        LIBVA GLX libraries are: $LIBVA_GLX_LIBS"
  fi
fi

if test x"$ac_cv_gcc_visibility" != xyes; then
  if test x"$npapi" = xyes; then
    echo "        WARNING: NPAPI (mozilla) plugin is enabled, but your compiler"
    echo "                 does not support symbol visibility. This may cause "
    echo "                 the plugin to malfunction and may result in small "
    echo "                 children being eaten. You have been warned!"
  fi
fi

if test x"$npapi" = xyes; then
  if test x"$has_npapi" = xyes; then
    echo "        NPAPI flags are: $NPAPI_CFLAGS"
  else
    echo "        ERROR : No xulrunner development package is installed" >&3
    echo "                Install it from http://releases.mozilla.org/pub/mozilla.org/xulrunner" >&3
    echo "                or .deb users: apt-get install xulrunner-dev" >&3
    echo "                or .rpm users: yum install xulrunner-devel" >&3
  fi
else
  echo "        NPAPI plugin is disabled"
fi

if test x"${DEJAGNU}" != x""; then
  echo "        DEJAGNU's runtest is $DEJAGNU"
else
  echo "        WARNING: You need the dejagnu package installed to get a summary" >&4
  echo "                 report after running ''make check''" >&4
  echo "                 Install it from http://www.gnu.org/software/dejagnu/" >&4
  echo "                 or .deb users: apt-get install dejagnu" >&4
  echo "                 or .rpm users: yum install dejagnu" >&4
fi

dnl Haiku
if test x"${build_haiku}" = xyes -o x"${build_sound_mkit}" = xyes -o x"${build_media_haiku}" = x"yes"; then
    echo "        Haiku libs are: $HAIKU_LIBS"
fi

if test x"$has_ltdl" = x"yes";then
  echo "        LIBLTDL flags are: $LTDL_CFLAGS"
  echo "        LIBLTDL libs are: $LTDL_LIBS"
else
  if test x"${extensions_support}" != xno; then
    echo "        ERROR: No libltdl development package is installed," >&3
    echo "               yet --disable-extensions was not specified." >&3
    echo "               Install it from ftp://ftp.gnu.org/gnu/libtool/" >&3
    echo "               or .deb users: apt-get install libltdl-dev" >&3
    echo "               or .rpm users: yum install libtool-ltdl-devel" >&3
  fi
fi
 
if test x"$python" = x"yes"; then
  if test x"$has_python" = x"yes"; then
    echo "        PYTHON flags are: $PYTHON_CFLAGS"
    echo "        PYTHON libs are: $PYTHON_LIBS"
    echo "        PYTHON executable is: $PYTHON"
  else
    echo "        ERROR: No Python development package is installed, but it's enabled." >&3
  fi
fi
if test x${build_ssl} = xyes; then
  if test x"${has_ssl}" = xyes; then
    if test x"${SSL_CFLAGS}" = xyes; then
      echo "        SSL flags are: default"
    else
      echo "        SSL flags are: $SSL_CFLAGS"
    fi
    echo "        SSL libs are: $SSL_LIBS"
  else
    echo "        ERROR: No SSL development package is installed, but it's enabled." >&3
  fi
fi

if test x${build_ssh} = xyes; then
  if test x"${has_ssh}" = xyes; then
    if test x"${SSH_CFLAGS}" = xyes; then
      echo "        SSH flags are: default"
    else
      echo "        SSH flags are: $SSH_CFLAGS"
    fi
    echo "        SSH libs are: $SSH_LIBS"
  else
    echo "        ERROR: No SSH development package is installed, but it's enabled." >&3
  fi
fi

if test x"${build_all_as3}" = x"yes"; then
  echo "        Building the entire ActionScript class libary"
else
  echo "        Only building these ActionScript classes into the library:"
  echo "     ${classlist}"
fi

if test x"$testsuite" = x"yes"; then
  if test x"$NETCAT" != x; then
    echo "        You have netcat installed, which is only used for testing"
  else
    echo "        Install netcat for networking test support"
  fi
  if test x"$WGET" != x; then
    echo "        You have wget installed, which is only used for testing"
  else
    echo "        Install wget for networking test support"
  fi
fi

if test x$cross_compiling = xyes; then
   if test x"${android_ndk}" != xno; then
      AC_MSG_NOTICE([This build is setup for cross compiling for Android])
   else
      AC_MSG_NOTICE([This build is setup for cross compiling])
   fi
fi

echo "--------"

if test x"${cygnal}" = x"yes"; then
  echo "        Building Cygnal media server enabled (default). Use --disable-cygnal to disable."
else
  echo "        Building Cygnal media server disabled."
fi

if test x"${with-top_level}" != x; then
  echo "        Top level for cross compiling support files is: $with_top_level"
fi

if test x"${build_gtk}" = xyes -a x"${pixelformat}" = xrgb565; then
  echo "        WARNING: Pixel format RGB565 selected in combination with the" >&4
  echo "                 GTK GUI. Only a hacked GTK will work (e.g. on the OLPC)." >&4
fi

if test x"${extensions_list}" != x; then
  echo "        Building extensions: ${extensions_list}"
fi

if test x"${extensions_support}" = xno; then
  echo "        Extension support disabled."
fi

if test x"${security_list}" != x; then
  echo "        Enabling security features: ${security_list}"
fi

if test x"${hwaccel_list}" != xnone; then
  echo "        Enabling hardware acceleration features: ${hwaccel_list}"
fi

if test x"${statistics_list}" != x; then
  echo "        Enabling statistics collecting for: ${statistics_list}"
fi

if test x"${SUPPORTED_GUIS}" = x; then
  AC_MSG_ERROR(no supported GUIs found);
fi

echo "        GUI toolkits supported: ${SUPPORTED_GUIS}"
echo "        Renderers supported: ${renderer_list}"
echo "        Hardware Acceleration: ${hwaccel_list}"
echo "        Media handlers: ${media_list}"
echo "        Using ${add_sound} for sound handling"
echo "        Using $with_shm mode for shared memory"

if test x"$docbook" = x"yes"; then
  echo '        DocBook document processing enabled (for "make html" and "make pdf")'
  if test x"$docbook_styles" != x; then
    echo "        Docbook styles sheets in $docbook_styles"
  fi
else
  echo "        DocBook document processing disabled (default)"
fi

dnl The Framebuffer GUI has several other settings. As it doesn't have X11,
dnl we have to handle input devices ourselves.
if test x"${build_fb}" = xyes; then
echo "        Using ${input_events} for Input"
  if test x"${fakefb}" != x; then
    echo "        Using shared memory as a fake framebuffer"
  fi
  if test x"${offscreen}" = xyes; then
    echo "        Using offscreen rendering"
  fi
  if test x"${doublebuf}" = xyes; then
    echo "        Using double buffering when rendering"
  fi
fi

if test -s $cwarn; then
  echo ""
  cat $cwarn
  rm $cwarn
  echo ""
  echo "Gnash should still compile even with these warnings."
  echo "If it doesn't, report the warnings as a bug."
else
  rm $cwarn
fi

if test -s $crec; then
  echo ""
  cat $crec
  rm $crec
  echo ""
  echo "Gnash should still compile, but you'll miss important support"
else
  rm $crec
fi

dnl If anything critical is missing, don't bother to continue
if test -s $cerr; then
  echo ""
  cat $cerr >&2
  rm $cerr
  AC_MSG_ERROR([Please install required packages])
else
  rm $cerr
fi

rmdir $tmp

if test x"$fork" = x"no"; then
  AC_MSG_ERROR([Currently only forking the standalone player works!])
fi
echo ""

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
