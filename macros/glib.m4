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

dnl Linking Gnash statically or dynamically with other modules is making a
dnl combined work based on Gnash. Thus, the terms and conditions of the GNU
dnl General Public License cover the whole combination.
dnl
dnl As a special exception, the copyright holders of Gnash give you
dnl permission to combine Gnash with free software programs or libraries
dnl that are released under the GNU LGPL and with code included in any
dnl release of Talkback distributed by the Mozilla Foundation. You may
dnl copy and distribute such a system following the terms of the GNU GPL
dnl for all but the LGPL-covered parts and Talkback, and following the
dnl LGPL for the LGPL-covered parts.
dnl
dnl Note that people who make modified versions of Gnash are not obligated
dnl to grant this special exception for their modified versions; it is their
dnl choice whether to do so. The GNU General Public License gives permission
dnl to release a modified version without this exception; this exception
dnl also makes it possible to release a modified version which carries
dnl forward this exception.
dnl  
dnl 

AC_DEFUN([GNASH_PATH_GLIB],
[
    dnl Look for the header
  AC_ARG_WITH(glib_incl, [  --with-glib-incl        directory where libglib header is], with_glib_incl=${withval})
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
  pkg=no
  if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_glib_incl}" = x; then
    ac_cv_path_glib_incl=`$PKG_CONFIG --cflags glib-2.0`
    pkg=yes
  fi

  dnl Attempt to find the top level directory, which unfortunately has a
  dnl version number attached. At least on Debain based systems, this
  dnl doesn't seem to get a directory that is unversioned.
  if test x"${gnash_glib_version}" = x; then
    AC_MSG_CHECKING([for the Glib Version])
    pathlist="${with_glib_incl} ${prefix}/include /sw/include /usr/local/include /home/latest/include /opt/include /usr/include /usr/pkg/include .. ../.."

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

    if test x"${gnash_glib_topdir}" = x; then
      AC_MSG_RESULT(none)
    else
      AC_MSG_RESULT([${gnash_glib_version}])
    fi

  fi

  dnl If the path hasn't been specified, go look for it.
  if test x"${ac_cv_path_glib_incl}" = x; then
    AC_CHECK_HEADERS(glib.h, [ac_cv_path_glib_incl=""],[
      if test x"${ac_cv_path_glib_incl}" = x; then
        AC_MSG_CHECKING([for libglib header])
        incllist="${prefix}/include /sw/include /usr/local/include /home/latest/include /usr/include /opt/include /usr/pkg/include .. ../.."

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

  dnl Look for the library
  AC_ARG_WITH(glib_lib, [  --with-glib-lib         directory where glib library is], with_glib_lib=${withval})
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
    ac_cv_path_glib_lib=`$PKG_CONFIG --libs glib-2.0`
  fi

  if test x"${ac_cv_path_glib_lib}" = x; then
    AC_CHECK_LIB(glib-${gnash_glib_version}, g_io_channel_init, [ac_cv_path_glib_lib="-lglib-${gnash_glib_version}"],[
      AC_MSG_CHECKING([for libglib library])
      libslist="${ac_cv_path_glib_lib} ${prefix}/lib64 ${prefix}/lib /usr/lib /usr/lib64 /sw/lib /usr/local/lib /home/latest/lib /opt/lib /usr/pkg/lib .. ../.."
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
    if test x"$pkg" = x"no"; then
      libslist="${with_glib_lib} ${ac_cv_path_glib_incl}/../../lib ${prefix}/lib64 ${prefix}/lib /usr/lib64 /usr/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib /usr/pkg/lib .. ../.."
      for i in $libslist; do
        if test -f $i/glib-${gnash_glib_version}/include/glibconfig.h; then
	  ac_cv_path_glib_incl="${ac_cv_path_glib_incl} -I${i}/glib-${gnash_glib_version}/include"
        break
        fi
      done
    fi
    GLIB_CFLAGS="${ac_cv_path_glib_incl}"
  else
    GLIB_CFLAGS=""
    AC_MSG_RESULT(no)
  fi



  if test x"${ac_cv_path_glib_lib}" != x ; then
    GLIB_LIBS="${ac_cv_path_glib_lib}"
  else
    GLIB_LIBS=""
  fi

  AC_SUBST(GLIB_CFLAGS)
  AC_SUBST(GLIB_LIBS)
])
