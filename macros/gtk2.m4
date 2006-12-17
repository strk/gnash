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
dnl 

dnl $Id: gtk2.m4,v 1.34 2006/12/17 19:51:28 nihilus Exp $

AC_DEFUN([GNASH_PATH_GTK2],
[
  dnl Look for the header
  AC_ARG_WITH(gtk2_incl, AC_HELP_STRING([--with-gtk2-incl], [directory where libgtk2 header is]), with_gtk2_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_gtk2_incl,[
    if test x"${with_gtk2_incl}" != x ; then
      if test -f ${with_gtk2_incl}/gtk/gtk.h; then
	ac_cv_path_gtk2_incl=-I`(cd ${with_gtk2_incl}; pwd)`
      else
	AC_MSG_ERROR([${with_gtk2_incl} directory doesn't contain gtk/gtk.h])
      fi
    fi
  ])


if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_gtk2_incl}" = x; then
	$PKG_CONFIG --exists gtk+-2.0 && ac_cv_path_gtk2_incl=`$PKG_CONFIG --cflags gtk+-2.0`
fi

  dnl Attempt to find the top level directory, which unfortunately has a
  dnl version number attached. At least on Debain based systems, this
  dnl doesn't seem to get a directory that is unversioned.

AC_MSG_CHECKING([for the Gtk Version])

if test x"$PKG_CONFIG" != x; then
	$PKG_CONFIG --exists gtk+-2.0 && gnash_gtk2_version=`$PKG_CONFIG --modversion gtk+-2.0`
fi

  if test x"${gnash_gtk2_version}" = x; then
    pathlist="${prefix}/${target_alias}/include ${prefix}/include /sw/include /opt/local/include /usr/local/include /usr/X11R6/include /home/latest/include /opt/include /opt/local/include /usr/include /usr/pkg/include .. ../.."

    gnash_gtk2_topdir=""
    gnash_gtk2_version=""
    for i in $pathlist; do
      for j in `ls -dr $i/gtk-[[2-9]].[[0-9]] 2>/dev/null`; do
        if test -f $j/gtk/gtk.h; then
          gnash_gtk2_topdir=`basename $j`
          gnash_gtk2_version=`echo ${gnash_gtk2_topdir} | sed -e 's:gtk-::'`
          break
        fi
      done
      if test x$gnash_gtk2_version != x; then
 	break;
      fi
    done
  fi
  AC_MSG_RESULT(${gnash_gtk2_version})


  dnl If the path hasn't been specified, go look for it.
  if test x"${ac_cv_path_gtk2_incl}" = x; then
    AC_CHECK_HEADERS(gtk/gtk.h, [ac_cv_path_gtk2_incl=""],[
      if test x"${ac_cv_path_gtk2_incl}" = x; then
        AC_MSG_CHECKING([for libgtk2 header])
        incllist="${prefix}/${target_alias}/include ${prefix}/include /sw/include /opt/local/lib /usr/local/include /usr/X11R6/include /home/latest/include /opt/include /opt/local/include /usr/include /usr/pkg/include /usr/nekoware/include /usr/freeware/include /usr/ .. ../.."

        for i in $incllist; do
          if test -f $i/${gnash_gtk2_topdir}/gtk/gtk.h; then
              ac_cv_path_gtk2_incl="-I$i/${gnash_gtk2_topdir}"
	      break
          else
            if test -f $i/gtk/gtk.h; then
              ac_cv_path_gtk2_incl="-I$i"
              break
            fi
          fi
        done
        if test x"${ac_cv_path_gtk2_incl}" != x ; then
          AC_MSG_RESULT(yes)
        else
          AC_MSG_RESULT(no)
        fi
      fi
    ])
  fi


    dnl Look for the library
  AC_ARG_WITH(gtk2_lib,
  	AC_HELP_STRING([--with-gtk2-lib], [directory where gtk2 library is]),
	with_gtk2_lib=${withval})

  dnl disabled as semantic is not really clear to me:
  dnl when should we set the cache ? what should we set in it ?
  dnl should any piece of code get disabled if a cache exists ?
  dnl AC_CACHE_VAL(ac_cv_path_gtk2_lib, [ ac_cv_path_gtk2_lib=-L${with_gtk2_lib}])

  dnl Use PKG_CONFIG only if no --with-gtk2-lib has been specified

if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_gtk2_lib}" = x; then
	$PKG_CONFIG --exists gtk+-2.0 && ac_cv_path_gtk2_lib=`$PKG_CONFIG --libs gtk+-2.0`
fi

dnl If the header doesn't exist, there is no point looking for
dnl the library. 
  if test x"${ac_cv_path_gtk2_incl}" != x -a x"$ac_cv_path_gtk2_lib" = x; then
    AC_CHECK_LIB(gtk-x11-2.0, gtk_init, [ac_cv_path_gtk2_lib="-lgtk-x11-2.0 -lgdk-x11-2.0"],[
      libslist="${with_gtk2_lib} ${prefix}/${target_alias}/lib ${prefix}/lib64 ${prefix}/lib /usr/X11R6/lib64 /usr/X11R6/lib /usr/lib64 /usr/lib /sw/lib /usr/local/lib /opt/local/lib /home/latest/lib /opt/lib /usr/pkg/lib .. ../.."
      for i in $libslist; do
        if test -f $i/libgtk-x11-2.0.a -o -f $i/libgtk-x11-2.0.so; then
          if test x"$i" != x"/usr/lib"; then
            ac_cv_path_gtk2_lib="-L$i -lgtk-x11-2.0 -lgdk-x11-2.0"
            break
          else
            ac_cv_path_gtk2_lib="-lgtk-x11-2.0 -lgdk-x11-2.0"
            break
          fi
        fi
      done
    ])
  else
    if test -f $i/libgtk-x11-2.0.a -o -f $i/libgtk-x11-2.0.so; then
      if test x"${ac_cv_path_gtk2_lib}" != x"/usr/lib"; then
        ac_cv_path_gtk2_lib="-L${ac_cv_path_gtk2_lib} -lgtk-x11-2.0 -lgdk-x11-2.0"
        else
        ac_cv_path_gtk2_lib="-lgtk-x11-2.0 -lgdk-x11-2.0"
      fi
    fi
  fi
  AC_MSG_CHECKING([for libgtk2 library])
  AC_MSG_RESULT($ac_cv_path_gtk2_lib)
 
  if test x"${ac_cv_path_gtk2_incl}" != x; then
    libslist="${prefix}/${target_alias}/lib ${prefix}/lib64 ${prefix}/lib /usr/X11R6/lib64 /usr/X11R6/lib /usr/lib64 /usr/lib /sw/lib /usr/local/lib /opt/local/lib /usr/nekoware/lib /usr/freeware/lib /home/latest/lib /opt/lib /usr/pkg/lib .. ../.."
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
