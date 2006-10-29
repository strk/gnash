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

dnl $Id: glib.m4,v 1.18 2006/10/29 18:34:10 rsavoye Exp $

AC_DEFUN([GNASH_PATH_GLIB],
[
    dnl Look for the header
  AC_ARG_WITH(glib_incl, AC_HELP_STRING([--with-glib-incl], [directory where libglib header is]), with_glib_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_glib_incl,[
    if test x"${with_glib_incl}" != x ; then
      if test -f ${with_glib_incl}/glib.h ; then
	ac_cv_path_glib_incl="-I`(cd ${with_glib_incl}; pwd)`"
        gnash_glib_topdir=`basename $ac_cv_path_glib_incl`
        gnash_glib_version=`echo ${gnash_glib_topdir} | sed -e 's:glib-::'`
      else
	AC_MSG_ERROR([${with_glib_incl} directory doesn't contain glib.h])
      fi
    fi
  ])

  dnl Try with pkg-config
  if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_glib_incl}" = x; then
    $PKG_CONFIG --exists glib-2.0 && ac_cv_path_glib_incl=`$PKG_CONFIG --cflags glib-2.0`
    $PKG_CONFIG --exists glib-2.0 && gnash_glib_version=`$PKG_CONFIG --modversion glib-2.0`
  fi

  dnl Attempt to find the top level directory, which unfortunately has a
  dnl version number attached. At least on Debain based systems, this
  dnl doesn't seem to get a directory that is unversioned.
  AC_MSG_CHECKING([for the Glib Version])
  if test x"${gnash_glib_version}" = x; then
    pathlist="${with_glib_incl} ${prefix}/include /sw/include /opt/local/include /usr/local/include /home/latest/include /opt/include /usr/include /usr/pkg/include .. ../.."

    gnash_glib_topdir=""
    gnash_glib_version=""
    for i in $pathlist; do
      for j in `ls -dr $i/glib-[[0-9]].[[0-9]] 2>/dev/null`; do
        if test -f $j/glib.h; then
          gnash_glib_topdir=`basename $j`
          gnash_glib_version=`echo ${gnash_glib_topdir} | sed -e 's:glib-::'`
          break
        fi
      done
    done
  fi

  AC_MSG_RESULT([${gnash_glib_version}])
  AC_MSG_CHECKING([for libglib header])  
  dnl If the path hasn't been specified, go look for it.
  if test x"${ac_cv_path_glib_incl}" = x; then
    AC_CHECK_HEADERS(glib.h, [ac_cv_path_glib_incl=""],[
      if test x"${ac_cv_path_glib_incl}" = x; then
        incllist="${prefix}/include /sw/include /opt/local/include /usr/local/include /home/latest/include /usr/include /opt/include /usr/pkg/include .. ../.."

        for i in $incllist; do
          if test -f $i/glib.h; then
            ac_cv_path_glib_incl="-I$i"
            break
          else
            if test -f $i/${gnash_glib_topdir}/glib.h; then
              ac_cv_path_glib_incl="-I$i/${gnash_glib_topdir}"
              break
            fi
          fi
        done
      fi
    ])
  fi
 AC_MSG_RESULT(${ac_cv_path_glib_incl})
 
  dnl Look for the library
  AC_ARG_WITH(glib_lib, AC_HELP_STRING([--with-glib-lib], [directory where glib library is]), with_glib_lib=${withval})
    AC_CACHE_VAL(ac_cv_path_glib_lib,[
    if test x"${with_glib_lib}" != x ; then
      if test -f ${with_glib_lib}/libglib-${gnash_glib_version}.a -o -f ${with_glib_lib}/libglib-${gnash_glib_version}.so; then
        ac_cv_path_glib_lib="-L`(cd ${with_glib_lib}; pwd)` -lglib-${gnash_glib_version}"
      else
        AC_MSG_ERROR([${with_glib_lib} directory doesn't contain libglib.])
      fi
    fi
  ])

  dnl Try with pkg-config
  if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_glib_lib}" = x; then
    $PKG_CONFIG --exists glib-2.0 && ac_cv_path_glib_lib=`$PKG_CONFIG --libs glib-2.0`
  fi

  AC_MSG_CHECKING([for libglib library])
  if test x"${ac_cv_path_glib_lib}" = x; then
    AC_CHECK_LIB(glib-${gnash_glib_version}, g_io_channel_init, [ac_cv_path_glib_lib="-lglib-${gnash_glib_version}"],[
      libslist="${ac_cv_path_glib_lib} ${prefix}/lib64 ${prefix}/lib /usr/lib /usr/lib64 /sw/lib /usr/local/lib /opt/local/lib /home/latest/lib /opt/lib /usr/pkg/lib .. ../.."
      for i in $libslist; do
        if test -f $i/libglib-${gnash_glib_version}.a -o -f $i/libglib-${gnash_glib_version}.so; then
          if test x"$i" != x"/usr/lib"; then
            ac_cv_path_glib_lib="-L$i -lglib-${gnash_glib_version}"
            break
          else
            ac_cv_path_glib_lib="-lglib-${gnash_glib_version}"
            break
          fi
        else
          if test -f $i/libglib-${gnash_glib_version}.a -o -f $i/libglib-${gnash_glib_version}.so; then
            ac_cv_path_glib_lib="-L$i/${gnash_glib_topdir} -lglib-${gnash_glib_version}"
            break
          fi
        fi
      done
    ])
  fi
 AC_MSG_RESULT(${ac_cv_path_glib_lib})
 
dnl
dnl The problem with these macros is that ac_cv_path_package_lib
dnl is ambiguos. Sometimes it refers to a directory, some other
dnl times it refers to full LDFLAGS
dnl

if false; then
dnl  else
    if test -f $i/libglib-${gnash_glib_version}.a -o -f $i/libglib-${gnash_glib_version}.so; then
      if test x"${ac_cv_path_glib_lib}" != x"/usr/lib"; then
        ac_cv_path_glib_lib="-L${ac_cv_path_glib_lib} -lglib-${gnash_glib_version}"
      else
        ac_cv_path_glib_lib="-lglib-${gnash_glib_version}"
      fi
    fi
dnl  fi
fi

  if test x"${ac_cv_path_glib_incl}" != x; then
    GLIB_CFLAGS="${ac_cv_path_glib_incl}"
  else
    GLIB_CFLAGS=""
  fi



  if test x"${ac_cv_path_glib_lib}" != x ; then
    GLIB_LIBS="${ac_cv_path_glib_lib}"
  else
    GLIB_LIBS=""
  fi

  AC_SUBST(GLIB_CFLAGS)
  AC_SUBST(GLIB_LIBS)
])
