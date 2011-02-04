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


AC_DEFUN([GNASH_PATH_DBUS],
[

  dnl Look for the header
  AC_ARG_WITH(dbus_incl, AC_HELP_STRING([--with-dbus-incl], [directory where libdbus header is]), with_dbus_incl=${withval})
  AC_CACHE_VAL(ac_cv_path_dbus_incl,[
    if test x"${with_dbus_incl}" != x ; then
      if test -f ${with_dbus_incl}/dbus/dbus.h ; then
        ac_cv_path_dbus_incl="-I`(cd ${with_dbus_incl}; pwd)`"
      else
        AC_MSG_ERROR([${with_dbus_incl} directory doesn't contain dbus/dbus.h])
      fi
    fi
  ])

  if test x$cross_compiling = xno; then
    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_dbus_incl}" = x; then
      $PKG_CONFIG --exists dbus-1 && ac_cv_path_dbus_incl=`$PKG_CONFIG --cflags dbus-1`
    fi
  fi

  dnl Attempt to find the top level directory, which unfortunately has a
  dnl version number attached. At least on Debain based systems, this
  dnl doesn't seem to get a directory that is unversioned.

  AC_MSG_CHECKING([for the Dbus Version])

  if test x$cross_compiling = xno; then
    if test x"$PKG_CONFIG" != x; then
      $PKG_CONFIG --exists dbus-1 && gnash_dbus_version=`$PKG_CONFIG --modversion dbus-1 | cut -d "." -f 1 | awk '{print $'0'".0"}'`
    fi
  fi

  if test x"${gnash_dbus_version}" = x; then
    gnash_dbus_topdir=""
    gnash_dbus_version=""
    for i in $incllist; do
      for j in `ls -dr $i/dbus-[[0-9]].[[0-9]] 2>/dev/null`; do
        if test -f $j/dbus.h; then
	      gnash_dbus_topdir=`basename $j`
	      gnash_dbus_version=`echo ${gnash_dbus_topdir} | sed -e 's:dbus-::' -e 's:.0::'`
 	      ac_cv_path_dbus_incl="-I$i/${gnash_dbus_topdir}"
	      break
	    fi
      done
	  if test x$gnash_dbus_version != x; then
	    break;
	  fi
    done
  fi      

  if test x"${gnash_dbus_version}" = x; then
    AC_MSG_RESULT(none)
  else
    AC_MSG_RESULT([${gnash_dbus_version}])
  fi
  
  AC_MSG_CHECKING([for Dbus headers])
  AC_MSG_RESULT(${ac_cv_path_dbus_incl}) 

  dnl Look for the library
  AC_ARG_WITH(dbus_lib, AC_HELP_STRING([--with-dbus-lib], [directory where dbus library is]), with_dbus_lib=${withval})
  AC_CACHE_VAL(ac_cv_path_dbus_lib,[
    if test x"${with_dbus_lib}" != x ; then
      if test -f ${with_dbus_lib}/libdbus-1.a -o -f ${with_dbus_lib}/libdbus-1.${shlibext}; then
	      ac_cv_path_dbus_lib="-L`(cd ${with_dbus_lib}; pwd)`"
      else
	      AC_MSG_ERROR([${with_dbus_lib} directory doesn't contain libdbus.])
      fi
    fi
  ])
  
  if test x$cross_compiling = xno; then
    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_dbus_lib}" = x; then
      $PKG_CONFIG --exists dbus-1 && ac_cv_path_dbus_lib=`$PKG_CONFIG --libs-only-l dbus-1`
    fi
  fi

  if test x"${ac_cv_path_dbus_lib}" = x; then
    newlist="/lib /lib64 $libslist"
    for i in $newlist; do
      if test -f $i/libdbus-${gnash_dbus_version}.a -o -f $i/libdbus-${gnash_dbus_version}.${shlibext}; then
        if test x"$i" != x"/lib"; then
	        ac_cv_path_dbus_lib="-L$i -ldbus-${gnash_dbus_version}"
	        break
        else
	        ac_cv_path_dbus_lib="-ldbus-${gnash_dbus_version}"
	  break
	fi
      fi
    done
  fi
	
  AC_MSG_CHECKING([for Dbus library])
  AC_MSG_RESULT(${ac_cv_path_dbus_lib})
  
  if test x"${ac_cv_path_dbus_lib}" = x; then
    AC_CHECK_LIB(dbus-${gnash_dbus_version}, dbus_engine_shape_class_init, [ac_cv_path_dbus_lib="-ldbus-${gnash_dbus_version}"])
  fi

  if test x"${ac_cv_path_dbus_incl}" != x; then
    DBUS_CFLAGS="${ac_cv_path_dbus_incl}"
  else
    DBUS_CFLAGS=""
  fi

  if test x"${ac_cv_path_dbus_lib}" != x; then
    DBUS_LIBS="${ac_cv_path_dbus_lib}"
  else
    DBUS_LIBS=""
  fi

  AC_SUBST(DBUS_CFLAGS)
  AC_SUBST(DBUS_LIBS)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
