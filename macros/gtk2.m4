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



AC_DEFUN([GNASH_PATH_GTK2],
[

if test x"$windows" = x"yes"; then
  gtklib="libgtk-win32-2.0"
  gdklib="libgdk-win32-2.0"
else
  gtklib="libgtk-x11-2.0"
  gdklib="libgdk-x11-2.0"
fi

  dnl Look for the header
  AC_ARG_WITH(gtk2_incl, AC_HELP_STRING([--with-gtk2-incl], [directory where libgtk2 header is]), with_gtk2_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_gtk2_incl,[
    if test x"${with_gtk2_incl}" != x ; then
      if test -f ${with_gtk2_incl}/gtk/gtk.h; then
      	ac_cv_path_gtk2_incl="-I`(cd ${with_gtk2_incl}; pwd)`"
      else
      	AC_MSG_ERROR([${with_gtk2_incl} directory doesn't contain gtk/gtk.h])
      fi
    fi
  ])


  if test x$cross_compiling = xno; then
    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_gtk2_incl}" = x; then
      $PKG_CONFIG --exists gtk+-2.0 && ac_cv_path_gtk2_incl="`$PKG_CONFIG --cflags gtk+-2.0`"
      $PKG_CONFIG --exists gtk+-2.0 && gnash_gtk2_version="`$PKG_CONFIG --modversion gtk+-2.0`"
    fi
  fi

  dnl Attempt to find the top level directory, which unfortunately has a
  dnl version number attached. At least on Debain based systems, this
  dnl doesn't seem to get a directory that is unversioned.

  AC_MSG_CHECKING([for the gtk2 header])
  if test x"${gnash_gtk2_version}" = x; then
    gnash_gtk2_topdir=""
    gnash_gtk2_version=""
    for i in $incllist; do
      for j in `ls -dr $i/gtk-[[2-9]].[[0-9]] 2>/dev/null`; do
        if test -f $j/gtk/gtk.h; then
          gnash_gtk2_topdir="`basename $j`"
          gnash_gtk2_version="`echo ${gnash_gtk2_topdir} | sed -e 's:gtk-::'`"
      	  ac_cv_path_gtk2_incl="-I$i/${gnash_gtk2_topdir}"
          break
        fi
      done
      if test x$gnash_gtk2_version != x; then
       	break;
      fi
    done
  fi
  if test x"${ac_cv_path_gtk2_incl}" = x; then
    AC_MSG_RESULT([not found, trying again])
    AC_CHECK_HEADERS(gtk/gtk.h)
  else
    AC_MSG_RESULT($ac_cv_path_gtk2_incl)
  fi

  dnl Look for the library
  AC_ARG_WITH(gtk2_lib,
  	AC_HELP_STRING([--with-gtk2-lib], [directory where gtk2 library is]),
	with_gtk2_lib=${withval})

    AC_CACHE_VAL(ac_cv_path_gtk2_lib,[
    if test x"${with_gtk2_lib}" != x ; then
      if test -f ${with_gtk2_lib}/${gtklib}.${shlibext} -o -f ${with_gtk2_lib}/${gtklib}.a; then
        if test -f ${with_gtk2_lib}/${gdklib}.${shlibext} -o -f ${with_gtk2_lib}/${gdklib}.a; then
        	ac_cv_path_gtk2_lib="-I`(cd ${with_gtk2_lib}; pwd)`"
        else
        	AC_MSG_ERROR([${with_gtk2_lib} directory doesn't contain ${gdklib}.${shlibext}])
        fi
      else
      	AC_MSG_ERROR([${with_gtk2_lib} directory doesn't contain ${gtklib}.${shlibext}])
      fi
    fi
  ])

  if test x$cross_compiling = xno; then
    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_gtk2_lib}" = x; then
      $PKG_CONFIG --exists gtk+-2.0 && ac_cv_path_gtk2_lib="`$PKG_CONFIG --libs-only-l gtk+-2.0`"
    fi
  fi

dnl If the header doesn't exist, there is no point looking for
dnl the library. 
  AC_MSG_CHECKING([for libgtk2 library])
  if test x"${ac_cv_path_gtk2_incl}" != x -a x"${ac_cv_path_gtk2_lib}" = x; then
    for i in $libslist; do
      if test -f $i/${gtklib}.a -o -f $i/${gtklib}.${shlibext}; then
        if test ! x"$i" = x"/usr/lib" -a ! x"$i" = x"/usr/lib64"; then
          if test x"$windows" = x"yes"; then
            ac_cv_path_gtk2_lib="-L$i -lgtk-win32-2.0 -lgdk-win32-2.0 -lgobject-2.0 -lgmodule-2.0"
          else
            ac_cv_path_gtk2_lib="-L$i -lgtk-x11-2.0 -lgdk-x11-2.0 -lgobject-2.0 -lgmodule-2.0"
          fi
          break
        else
          if test x"$windows" = x"yes"; then
            ac_cv_path_gtk2_lib="-lgtk-win32-2.0 -lgdk-win32-2.0 -lgobject-2.0 -lgmodule-2.0 "
          else
            ac_cv_path_gtk2_lib="-lgtk-x11-2.0 -lgdk-x11-2.0 -lgobject-2.0 -lgmodule-2.0 "
          fi
          break
        fi
      fi
    done
  fi

  if test x"${ac_cv_path_gtk2_lib}" = x; then
    if test x"$windows" = x"yes"; then
      AC_CHECK_LIB([gtk-win32-2.0], [gtk_init], [ac_cv_path_gtk2_lib="-lgtk-win32-2.0 -lgdk-win32-2.0 -lgobject-2.0 -lgmodule-2.0"])
    else
      AC_CHECK_LIB([gtk-x11-2.0], [gtk_init], [ac_cv_path_gtk2_lib="-lgtk-x11-2.0 -lgdk-x11-2.0 -lgobject-2.0 -lgmodule-2.0"])
    fi
  fi
  AC_MSG_RESULT($ac_cv_path_gtk2_lib)
 
  if test x"${ac_cv_path_gtk2_incl}" != x; then
    for i in $libslist; do
      if test -f $i/gtk-${gnash_gtk2_version}/include/gdkconfig.h; then
    	 GTK2_CFLAGS="-I${i}/gtk-${gnash_gtk2_version}/include"
    	 break
      fi
    done
    if test x"${ac_cv_path_gtk2_incl}" = x"yes"; then
      GTK2_CFLAGS="$GTK2_CFLAGS"
    else
      GTK2_CFLAGS="${ac_cv_path_gtk2_incl} $GTK2_CFLAGS"
    fi
    AC_DEFINE([HAVE_GTK2], [1], [Use GTK2 for windowing])
  else
    GTK2_CFLAGS=""
  fi

  if test x"${ac_cv_path_gtk2_lib}" != x ; then
    GTK2_LIBS="${ac_cv_path_gtk2_lib}"
    has_gtk2=yes
  else
    GTK2_LIBS=""
    has_gtk2=no
  fi

  AC_SUBST(GTK2_CFLAGS)
  AC_SUBST(GTK2_LIBS)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
