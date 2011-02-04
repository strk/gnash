dnl  
dnl  Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
dnl  2011 Free Software Foundation, Inc.
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

AC_DEFUN([GNASH_PATH_KDE3],
[ 
  has_kde3=no

  dnl setup the various options to custimize paths
  AC_ARG_WITH(kde3_incl, AC_HELP_STRING([--with-kde3-incl],
             [directory where KDE 3.x headers are]),
             with_kde3_incl=${withval})
  dnl make sure the path is a useable one
  if test x"${with_kde3_incl}" != x ; then
    if test ! -f ${with_kde3_incl}/kapp.h ; then
      AC_MSG_ERROR([${with_kde3_incl} directory doesn't contain any KDE 3.x headers])
    fi
  fi

  AC_ARG_WITH(kde3_prefix, AC_HELP_STRING([--with-kde3-prefix],
             [Prefix for KDE plugin, like /usr]),
             with_kde3_prefix=${withval})

  AC_ARG_WITH(kde3_lib, AC_HELP_STRING([--with-kde3-lib],
             [directory where KDE 3.x libraries are]),
             with_kde3_lib=${withval})
  dnl make sure the path is a useable one
  if test x"${with_kde3_lib}" != x ; then 
    if test ! -f ${with_kde3_lib}/libkdeui.la; then
      AC_MSG_ERROR([${with_kde3_lib} directory doesn't contain any KDE 3.x libraries!])
    fi
  fi

  dnl Specifiy a custom directory for the kparts plugin
  AC_ARG_WITH(kde3_plugindir, AC_HELP_STRING([--with-kde3-plugindir=DIR],
             [Directory to install KDE 3.x plugin in]),
             with_kde3_plugindir=${withval})
  if test x"${with_kde3_plugindir}" != x ; then 
    if test ! -d ${with_kde3_plugindir}/designer; then
      AC_MSG_ERROR([${with_kde3_plugindir} directory doesn't contain any KDE 3.x plugins!])
    fi
  fi

  dnl Specifiy a custom directory for the kde services directory
  AC_ARG_WITH(kde3_servicesdir, AC_HELP_STRING([--with-kde3-servicesdir=DIR],
             [Directory to install KDE 3.x plugin in]),
             with_kde3_servicesdir=${withval})
  if test x"${with_kde3_servicesdir}" != x ; then 
    if test ! -d ${with_kde3_servicesdir}; then
      AC_MSG_ERROR([${with_kde3_} directory doesn't contain any KDE 3.x plugins!])
    fi
  fi

  AC_ARG_WITH(kde3-configdir, AC_HELP_STRING([--with-kde3-configdir=DIR],
      [Directory to install KDE 3.x configfile in]),
    [KDE3_CONFIGDIR=${withval}
  ])

  AC_ARG_WITH(kde3-appsdatadir, AC_HELP_STRING([--with-kde3-appsdatadir=DIR],
      [Directory to install KDE 3.x data in]),
    [KDE3_APPSDATADIR=${withval}
  ])

  dnl Look for the files necessary to build KDE 3.x applications
    kde3_prefix=
    AC_PATH_PROG(KDE3_CONFIG, kde-config, ,[${pathlist}])
    if test "x$KDE3_CONFIG" != "x" ; then
      kde3_prefix=`$KDE3_CONFIG --prefix`
      AC_MSG_NOTICE([KDE3 prefix from kde-config is ${kde3_prefix}])
    fi

    AC_CACHE_VAL(ac_cv_path_kde3_incl,[
      dnl if the user specified a path, sanity check it and then use it
      if test x"${with_kde3_incl}" != x; then
        if test -f ${with_kde3_incl}/kapp.h; then
          ac_cv_path_kde3_incl="-I`(cd ${with_kde3_incl}; pwd)`"
        fi
      fi
      dnl if the user didn't specify a path, go search a list of
      dnl likely directories for the header files.
      if test x"${ac_cv_path_kde3_incl}" = x ; then
        AC_MSG_CHECKING([for KDE 3.x header path])
        dnl incllist is inherited from configure.ac, and lives in /macros
        for i in ${kde3_prefix}/include ${incllist}; do 
         if test -f $i/kde/kapplication.h; then
            ac_cv_path_kde3_incl="-I$i/kde"
            kde3_prefix=`dirname $i`
            break
          fi
          if test -f $i/kapplication.h; then
            ac_cv_path_kde3_incl="-I$i"
            kde3_prefix=`dirname $i`
            break
          fi
        done
      fi
      if test x"${ac_cv_path_kde3_incl}" != x ; then
        AC_MSG_RESULT(${ac_cv_path_kde3_incl})
      else
        AC_MSG_RESULT(no)
        if test x${cross_compiling} = xno; then
          dnl if no headers have been found yet, make a last ditch
          dnl attempt to use the compiler to find them.
          AC_CHECK_HEADERS(kde/kapplication.h, [ac_cv_path_kde3_incl=""])
        fi
      fi
    ])                  dnl end of cache ac_cv_path_kde3_incl

    dnl Look for the libraries
    AC_CACHE_VAL(ac_cv_path_kde3_lib, [
      if test x"${with_kde3_lib}" != x ; then
        if test -f ${with_kde3_lib}/libkdeui.la; then
	        ac_cv_path_kde3_lib="-L`(cd ${with_kde3_lib}; pwd)`"
        else
	        AC_MSG_ERROR([${with_kde3_lib} directory doesn't contain KDE 3.x libraries.])
        fi
      fi
      if test x"${ac_cv_path_kde3_lib}" = x; then
        AC_MSG_CHECKING([for kdeui library])
        kde3_topdir=
        for i in ${kde3_prefix}/lib64 ${kde3_prefix}/lib $libslist ; do
          if test -f $i/libkdeui.${shlibext} -o -f $i/libkdeui.la; then
            kde3_topdir=$i
            AC_MSG_RESULT(${kde3_topdir}/libkdeui)
	          if test ! x"$i" = x"/usr/lib" -a ! x"$i" = x"/usr/lib64"; then
	            ac_cv_path_kde3_lib="-L$i -lkdeui"
       	      break
            else
	            ac_cv_path_kde3_lib="-lkdeui"
	            break
            fi
          fi
        done
      fi
      dnl Look for the kdecore library, which is required
      if test x"${ac_cv_path_kde3_lib}" != x; then
        AC_MSG_CHECKING([for kdecore library])
        if test -f ${kde3_topdir}/libkdecore.la; then
          ac_cv_path_kde3_lib="${ac_cv_path_kde3_lib} -lkdecore"
          AC_MSG_RESULT(${kde3_topdir}/libkdecore)
        else
         AC_MSG_RESULT(no)
        fi
      fi
      dnl Look for the kdeprint library, which is required
      AC_MSG_CHECKING([for kdeprint library])
      if test x"${libkdeprint}" = x; then
        if test -f ${kde3_topdir}/libkdeprint.la -o -f ${kde3_topdir}/libkdeprint.${shlibext}; then
          ac_cv_path_kde3_lib="${ac_cv_path_kde3_lib} -lkdeprint"
          AC_MSG_RESULT(${kde3_topdir}/libkdeprint)
        else
          AC_MSG_RESULT(no)
        fi
      else
        AC_MSG_RESULT(${libkdeprint})
        ac_cv_path_kde3_lib="${ac_cv_path_kde3_lib} ${libkdeprint}"
      fi	
    ])                  dnl end of cache ac_cv_path_kde3_lib

  KLASH_PLUGIN=
  
  dnl If building the kparts plugin, get the install paths correct.  
    AC_CACHE_VAL(ac_cv_path_kde3_prefix,[
      dnl if the user specified a path, sanity check it and then use it
      if test x"${with_kde3_prefix}" != x; then
          ac_cv_path_kde3_prefix="`(cd ${with_kde3_prefix}; pwd)`"
      fi
    ])                  dnl end of cache ac_cv_path_kde3_prefix

    KDE3_PREFIX="${ac_cv_path_kde3_prefix}"
    if test x"${PLUGINS_INSTALL_POLICY}" != x; then
      case "${PLUGINS_INSTALL_POLICY}" in
        user)
       	  KDE3_PREFIX=${HOME}/.kde
          ;;
        system)
          KDE3_PREFIX="${kde3_prefix}"
          ;;
        prefix)
        	KDE3_PREFIX="\${prefix}"
          ;;
      esac
    fi

    KDE3_PLUGINDIR="${KDE3_PREFIX}/lib/kde3"
    KDE3_SERVICESDIR="${KDE3_PREFIX}/share/services"
    KDE3_CONFIGDIR="${KDE3_PREFIX}/share/config"
    KDE3_APPSDATADIR="${KDE3_PREFIX}/share/apps/klash"

  if test x"${ac_cv_path_kde3_incl}" != x ; then
    if test x"${ac_cv_path_kde3_incl}" != x"-I/usr/include"; then
      KDE3_CFLAGS="${ac_cv_path_kde3_incl}"
    else
      KDE3_CFLAGS=""
    fi
  else
    KDE3_CFLAGS=""
  fi

  if test x"${ac_cv_path_kde3_lib}" != x; then
    KDE3_LIBS="${ac_cv_path_kde3_lib}"
    AC_DEFINE(HAVE_KDE3, 1,[Have KDE 3.x installed])
    has_kde3=yes
  else
    KDE3_LIBS=""
    has_kde3=no
  fi

  AC_SUBST(KLASH_PLUGIN)
  AC_SUBST(KDE3_CFLAGS)  
  AC_SUBST(KDE3_LIBS)

  AC_SUBST(KDE3_PLUGINDIR)
  AC_SUBST(KDE3_SERVICESDIR)
  AC_SUBST(KDE3_CONFIGDIR)
  AC_SUBST(KDE3_APPSDATADIR)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
