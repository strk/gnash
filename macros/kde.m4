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

AC_DEFUN([GNASH_PATH_KDE],
[
  kde_version=0
  dnl Lool for the header
  AC_ARG_WITH(kde_incl, AC_HELP_STRING([--with-kde-incl], [directory where kde headers are]), with_kde_incl=${withval})
  AC_CACHE_VAL(ac_cv_path_kde_incl,[
    if test x"${with_kde_incl}" != x ; then
      if test -f ${with_kde_incl}/kapp.h ; then
        ac_cv_path_kde_incl="-I`(cd ${with_kde_incl}; pwd)`"
      else
        AC_MSG_ERROR([${with_kde_incl} directory doesn't contain any headers])
      fi
    fi
  ])

  AC_PATH_PROG(KDE_CONFIG, kde-config, ,[${pathlist}])

  AC_ARG_WITH(kparts-install,
    AC_HELP_STRING([--with-kparts-install=system|user|prefix], [Policy for KPARTS plugin install. Default: user.]),
	[case "${withval}" in
	  user) KPARTS_INSTALL_POLICY=user ;;
	  system) KPARTS_INSTALL_POLICY=system ;;
	  prefix) KPARTS_INSTALL_POLICY=prefix ;;
	  *)  AC_MSG_ERROR([bad value ${withval} for --with-kparts-install]) ;;
	 esac 
	], KPARTS_INSTALL_POLICY=${PLUGINS_INSTALL_POLICY}) dnl Inherit a generic PLUGINS_INSTALL_POLICY when available

  AC_ARG_WITH(kde-pluginprefix, AC_HELP_STRING([--with-kde-pluginprefix=DIR],
      [Prefix for KDE plugin, like /usr]),
    [KDE_PLUGINPREFIX=${withval}
  ])

  kde_prefix="/usr"
  has_kde=no
  if test x"${ac_cv_path_kde_incl}" = x; then
    if test "x$KDE_CONFIG" != "x" ; then
      if test "x$KDE_CFLAGS" = "x" ; then
        kde_prefix=`$KDE_CONFIG --prefix`
        dnl KDE4
        if test -f ${kde_prefix}/include/kde4/kapplication.h; then
          ac_cv_path_kde_incl="-I${kde_prefix}/include/kde4 -I${kde_prefix}/include/kde4/kio"
          kde_version=4
        fi
        dnl KDE3. If both exist, we prefer KDE3 over KDE4 for now.
        if test -f ${kde_prefix}/include/kde/kapp.h; then
          ac_cv_path_kde_incl="-I${kde_prefix}/include/kde -I${kde_prefix}/include/kde/kio"
          kde_version=3
        fi
      fi
    else
      AC_MSG_RESULT(no)
    fi
  fi
  AC_MSG_CHECKING([for kde header])

  dnl incllist is inherited from configure.ac.
  if test x"${ac_cv_path_kde_incl}" = x ; then
    for i in $incllist; do
      dnl KDE4
      if test -f $i/kde4/kapplication.h; then
        ac_cv_path_kde_incl="-I$i/kde4"
        kde_prefix=`dirname $i`
        kde_version=4
        break
      fi
      dnl KDE3. If both exist, we prefer KDE3 over KDE4 for now.
      if test -f $i/kde/kapp.h; then
        ac_cv_path_kde_incl="-I$i/kde"
        kde_prefix=`dirname $i`
        kde_version=3
        break
      fi
    done
  fi

  if test x"${ac_cv_path_kde_incl}" = x; then
    AC_MSG_RESULT(no)
    if test x${cross_compiling} = xno; then
      if test $kde_version -eq 4; then
        AC_CHECK_HEADERS(kde/kapplication.h, [ac_cv_path_kde_incl=""])
      fi
      if test $kde_version -eq 3; then
        AC_CHECK_HEADERS(kde/kapp.h, [ac_cv_path_kde_incl=""])
      fi
    fi
  else
    AC_MSG_RESULT(${ac_cv_path_kde_incl})
  fi

  if test x"${ac_cv_path_kde_incl}" != x ; then
    KDE_CFLAGS="${ac_cv_path_kde_incl}"
  else
    KDE_CFLAGS=""
  fi

  dnl Look for the libraries
  AC_ARG_WITH(kde_lib, AC_HELP_STRING([--with-kde-lib], [directory where kde libraries are]), with_kde_lib=${withval})
  AC_CACHE_VAL(ac_cv_path_kde_lib, [
    if test x"${with_kde_lib}" != x ; then
      if test `ls -C1 ${with_kde_lib}/libkdeui.* | wc -l` -gt 0; then
	      ac_cv_path_kde_lib="-L`(cd ${with_kde_lib}; pwd)`"
      else
	      AC_MSG_ERROR([${with_kde_lib} directory doesn't contain kde libraries.])
      fi
    fi
  ])

  if test x"${ac_cv_path_kde_lib}" = x; then
    AC_MSG_CHECKING([for kdeui library])
    topdir=""
    newlist="${kde_prefix}/lib ${libslist}"
    for i in $newlist ; do
      if test -f $i/libkdeui.${shlibext} -o -f $i/libkdeui.la; then
        topdir=$i
        AC_MSG_RESULT(${topdir}/libkdeui)
	      if test ! x"$i" = x"/usr/lib" -a ! x"$i" = x"/usr/lib64"; then
	        ac_cv_path_kde_lib="-L$i -lkdeui"
       	  break
        else
	        ac_cv_path_kde_lib="-lkdeui"
	        break
        fi
      fi
    done
  fi

  dnl Look for the kdecore library, which is required
  if test x"${ac_cv_path_kde_lib}" != x; then
    AC_MSG_CHECKING([for kdecore library])
    if test $kde_version -eq 3; then
      if test `ls -C1 ${topdir}/libkdecore.* | wc -l` -gt 0; then
        ac_cv_path_kde_lib="${ac_cv_path_kde_lib} -lkdecore"
        AC_MSG_RESULT(${topdir}/libkdecore)
      else
        AC_MSG_RESULT(no)
      fi
    fi

    dnl Look for the kdeprint library, which is required
    AC_MSG_CHECKING([for kdeprint library])
    if test x"${libkdeprint}" = x; then
      if test -f ${topdir}/libkdeprint.la -o -f ${topdir}/libkdeprint.${shlibext}; then
        ac_cv_path_kde_lib="${ac_cv_path_kde_lib} -lkdeprint"
        AC_MSG_RESULT(${topdir}/libkdeprint)
      else
        AC_MSG_RESULT(no)
      fi
    else
      AC_MSG_RESULT(${libkdeprint})
      ac_cv_path_kde_lib="${ac_cv_path_kde_lib} ${libkdeprint}"
    fi	
  fi                            dnl end of all optional library tests

  if test x"${ac_cv_path_kde_lib}" != x; then
    KDE_LIBS="${ac_cv_path_kde_lib}"
    AC_DEFINE(HAVE_KDE,1,[Have KDE installed])
    has_kde=yes
  else
    KDE_LIBS=""
    has_kde=no
  fi

  KLASH_PLUGIN=
  
  AC_SUBST(KLASH_PLUGIN)
  AC_SUBST(KDE_CFLAGS)  
  AC_SUBST(KDE_LIBS)

  dnl If building the kparts plugin, get the install paths correct.  
  if test x$kparts = xyes; then
    if test x$KDE_PLUGINPREFIX = x; then
      if test "x$KPARTS_INSTALL_POLICY" = "xuser"; then
      	KDE_PLUGINPREFIX=${HOME}/.kde
      elif test "x$KPARTS_INSTALL_POLICY" = "xsystem"; then
        if test "x$KDE_CONFIG" != "x" ; then
          KDE_PLUGINPREFIX=`$KDE_CONFIG --prefix`
        else
          AC_MSG_ERROR([Dunno how to make a 'system' kde plugin install not having found kde-config]);
        fi
      elif test "x$KPARTS_INSTALL_POLICY" = "xprefix"; then
      	KDE_PLUGINPREFIX="\${prefix}/kparts"
      fi
    fi
    KDE_PLUGINDIR="${KDE_PLUGINPREFIX}/lib/kde3"
    KDE_SERVICESDIR="${KDE_PLUGINPREFIX}/share/services"
    KDE_CONFIGDIR="${KDE_PLUGINPREFIX}/share/config"
    KDE_APPSDATADIR="${KDE_PLUGINPREFIX}/share/apps/klash"
  fi

  AC_ARG_WITH(kde-plugindir,
    AC_HELP_STRING([--with-kde-plugindir=DIR], [Directory to install KDE plugin in]),
    [KDE_PLUGINDIR=${withval}
  ])

  AC_ARG_WITH(kde-servicesdir, AC_HELP_STRING([--with-kde-servicesdir=DIR],
      [Directory to install KDE service in]),
    [KDE_SERVICESDIR=${withval}
  ])

  AC_ARG_WITH(kde-configdir, AC_HELP_STRING([--with-kde-configdir=DIR],
      [Directory to install KDE configfile in]),
    [KDE_CONFIGDIR=${withval}
  ])

  AC_ARG_WITH(kde-appsdatadir, AC_HELP_STRING([--with-kde-appsdatadir=DIR],
      [Directory to install KDE data in]),
    [KDE_APPSDATADIR=${withval}
  ])


  AC_SUBST(KDE_PLUGINDIR)
  AC_SUBST(KDE_SERVICESDIR)
  AC_SUBST(KDE_CONFIGDIR)
  AC_SUBST(KDE_APPSDATADIR)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
