dnl  
dnl    Copyright (C) 2005, 2006, 2007, 2009, 2010 Free Software Foundation, Inc.
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

AC_DEFUN([GNASH_PATH_KDE4],
[ 
  has_kde4=no

  dnl setup the various options to custimize paths
  AC_ARG_WITH(kde4_incl, AC_HELP_STRING([--with-kde4-incl],
             [directory where KDE 4.x headers are]),
             with_kde4_incl=${withval})
  dnl make sure the path is a useable one
  if test x"${with_kde4_incl}" != x ; then
    if test ! -f ${with_kde4_incl}/kapplication.h ; then
      AC_MSG_ERROR([${with_kde4_incl} directory doesn't contain any KDE 4.x headers])
    fi
  fi

  AC_ARG_WITH(kde4_prefix, AC_HELP_STRING([--with-kde4-prefix],
             [Prefix for KDE plugin, like /usr]),
             with_kde4_prefix=${withval})

  AC_ARG_WITH(kde4_lib, AC_HELP_STRING([--with-kde4-lib],
             [directory where KDE 4.x libraries are]),
             with_kde4_lib=${withval})
  dnl make sure the path is a useable one
  if test x"${with_kde4_lib}" != x ; then 
    if test ! -f ${with_kde4_lib}/libkdeui.so; then
      AC_MSG_ERROR([${with_kde4_lib} directory doesn't contain any KDE 4.x libraries!])
    fi
  fi

  dnl Specifiy a custom directory for the kparts plugin
  AC_ARG_WITH(kde4_plugindir, AC_HELP_STRING([--with-kde4-plugindir=DIR],
             [Directory to install KDE 4.x plugin in]),
             with_kde4_plugindir=${withval})
  if test x"${with_kde4_plugindir}" != x ; then 
    if test ! -d ${with_kde4_plugindir}/imageformats; then
      AC_MSG_ERROR([${with_kde4_plugindir} directory doesn't contain any KDE 4.x plugins!])
    fi
  fi

  dnl Specifiy a custom directory for the kde services directory
  AC_ARG_WITH(kde4_servicesdir, AC_HELP_STRING([--with-kde4-servicesdir=DIR],
             [Directory to install KDE 4.x plugin in]),
             with_kde4_servicesdir=${withval})
  if test x"${with_kde4_servicesdir}" != x ; then 
    if test ! -d ${with_kde4_servicesdir}; then
      AC_MSG_ERROR([${with_kde4_} directory doesn't contain any KDE 4.x plugins!])
    fi
  fi

  AC_ARG_WITH(kde4-configdir, AC_HELP_STRING([--with-kde4-configdir=DIR],
      [Directory to install KDE KDE 4.x configfile in]),
    [KDE4_CONFIGDIR=${withval}
  ])

  AC_ARG_WITH(kde-appsdatadir, AC_HELP_STRING([--with-kde-appsdatadir=DIR],
      [Directory to install KDE 4.x data in]),
    [KDE4_APPSDATADIR=${withval}
  ])

  dnl Only run these tests if this version was specified by the user, and they
  dnl haven't spcified a custom path.
    kde4_prefix=
    dnl FreeBSD puts kde4-config in /usr/local
    pathlist="${pathlist}:/usr/local/kde4/bin"
    AC_PATH_PROG(KDE4_CONFIG, kde4-config, ,[${pathlist}])
    if test "x$KDE4_CONFIG" != "x" ; then
      kde4_prefix=`$KDE4_CONFIG --prefix`
      AC_MSG_NOTICE([KDE4 prefix from kde4-config is ${kde4_prefix}])
    fi

    AC_CACHE_VAL(ac_cv_path_kde4_incl,[
      dnl if the user specified a path, sanity check it and then use it
      if test x"${with_kde4_incl}" != x ; then
        if test -f ${with_kde4_incl}/kapplication.h ; then
          ac_cv_path_kde4_incl="-I`(cd ${with_kde4_incl}; pwd)`"
        fi
      fi
      AC_MSG_CHECKING([for KDE 4.x header path])
      dnl if the user didn't specify a path, go search a list of
      dnl likely directories for the header files.
      if test x"${ac_cv_path_kde4_incl}" = x ; then
        dnl incllist is inherited from configure.ac, and lives in /macros
        for i in ${kde4_prefix}/include $incllist; do
          if test -f $i/kde4/kapplication.h; then          
            ac_cv_path_kde4_incl="-I$i/kde4"
            kde4_prefix=`dirname $i`
            break
          fi
          if test -f $i/kde/kapplication.h; then
            ac_cv_path_kde4_incl="-I$i/kde"
            kde4_prefix=`dirname $i`
            break
          fi
          if test -f $i/kapplication.h; then
            ac_cv_path_kde4_incl="-I$i"
            kde4_prefix=`dirname $i`
            break
          fi
        done
      fi
      if test x"${ac_cv_path_kde4_incl}" != x ; then
        AC_MSG_RESULT(${ac_cv_path_kde4_incl})
      else
        AC_MSG_RESULT(no)
        if test x${cross_compiling} = xno; then
          dnl if no headers have been found yet, make a last ditch
          dnl attempt to use the compiler to find them.
          AC_CHECK_HEADERS(kde4/kapplication.h, [ac_cv_path_kde4_incl=""])
        fi
      fi
    ])                  dnl end of cache ac_cv_path_kde4_incl

    dnl Look for the libraries
    AC_CACHE_VAL(ac_cv_path_kde4_lib, [
      if test x"${with_kde4_lib}" != x ; then
        if test -f ${with_kde4_lib}/libkdeui.la -o -f ${with_kde4_lib}/libkdeui.${shlibext}; then
	        ac_cv_path_kde4_lib="-L`(cd ${with_kde4_lib}; pwd)`"
        else
	        AC_MSG_ERROR([${with_kde4_lib} directory doesn't contain KDE 4.x libraries.])
        fi
      fi
      if test x"${ac_cv_path_kde4_lib}" = x; then
        AC_MSG_CHECKING([for kdeui library])
        kde4_topdir=
        for i in ${kde4_prefix}/lib64 ${kde4_prefix}/lib $libslist /usr/lib/kde4/devel /usr/lib64/kde4/devel; do
          if test -f $i/libkdeui.${shlibext} -o -f $i/libkdeui.la; then
            kde4_topdir=$i
            AC_MSG_RESULT(${kde4_topdir}/libkdeui)
	          if test ! x"$i" = x"/usr/lib" -a ! x"$i" = x"/usr/lib64"; then
	            ac_cv_path_kde4_lib="-L$i -lkdeui"
       	      break
            else
	            ac_cv_path_kde4_lib="-lkdeui"
	            break
            fi
          fi
        done
      fi
      dnl Look for the kdecore library, which is required
      if test x"${ac_cv_path_kde4_lib}" != x; then
        AC_MSG_CHECKING([for kdecore library])
        if test -f ${kde4_topdir}/libkdecore.so; then
          ac_cv_path_kde4_lib="${ac_cv_path_kde4_lib} -lkdecore"
          AC_MSG_RESULT(${kde4_topdir}/libkdecore)
        else
         AC_MSG_RESULT(no)
        fi
      fi
    ])                  dnl end of cache ac_cv_path_kde4_lib

  KLASH_PLUGIN=
  
  dnl If building the kparts plugin, get the install paths correct.  
    AC_CACHE_VAL(ac_cv_path_kde4_prefix,[
      dnl if the user specified a path, sanity check it and then use it
      if test x"${with_kde4_prefix}" != x; then
        ac_cv_path_kde4_prefix="`(cd ${with_kde4_prefix}; pwd)`"
      else
        ac_cv_path_kde4_prefix=${kde4_prefix}
      fi
    ])                  dnl end of cache ac_cv_path_kde4_prefix

    KDE4_PREFIX="${ac_cv_path_kde4_prefix}"
    if test x"${PLUGINS_INSTALL_POLICY}" != x; then
      case "${PLUGINS_INSTALL_POLICY}" in
        user)
       	  KDE4_PREFIX=${HOME}/.kde
          ;;
        system)
          KDE4_PREFIX="${kde4_prefix}"
          ;;
        prefix)
          KDE4_PREFIX="\${prefix}"
          ;;
      esac
    fi

    if test x"${with_kde4_plugindir}" != x ; then 
      KDE4_PLUGINDIR="${with_kde4_plugindir}"
    else
      if test -d ${KDE4_PREFIX}/lib64 -a ! -h ${KDE4_PREFIX}/lib64; then
        KDE4_PLUGINDIR="${KDE4_PREFIX}/lib64/kde4"
      else
        KDE4_PLUGINDIR="${KDE4_PREFIX}/lib/kde4"
      fi
    fi
    if test x"${with_kde4_servicesdir}" != x ; then 
      KDE4_SERVICESDIR="${with_kde4_servicesdir}"
    else
      KDE4_SERVICESDIR="${KDE4_PREFIX}/share/kde4/services"
    fi
    if test x"${KDE4_CONFIGDIR}" = x ; then 
      KDE4_CONFIGDIR="${KDE4_PREFIX}/share/kde4/config"
    fi
    if test x"${KDE4_APPSDATADIR}" = x ; then 
      KDE4_APPSDATADIR="${KDE4_PREFIX}/share/kde4/apps/klash"
    fi

  if test x"${ac_cv_path_kde4_incl}" != x ; then
    if test x"${ac_cv_path_kde4_incl}" != x"-I/usr/include"; then
      KDE4_CFLAGS="${ac_cv_path_kde4_incl}"
    else
      KDE4_CFLAGS=""
    fi
    if test x"${ac_cv_path_kde4_lib}" != x; then
      KDE4_LIBS="${ac_cv_path_kde4_lib}"
      AC_DEFINE(HAVE_KDE4, 1,[Have KDE 4.x installed])
      has_kde4=yes
    else
      KDE4_LIBS=""
      has_kde4=no
    fi
  fi

  AC_SUBST(KLASH_PLUGIN)
  AC_SUBST(KDE4_CFLAGS)  
  AC_SUBST(KDE4_LIBS)

  AC_SUBST(KDE4_PLUGINDIR)
  AC_SUBST(KDE4_SERVICESDIR)
  AC_SUBST(KDE4_CONFIGDIR)
  AC_SUBST(KDE4_APPSDATADIR)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
