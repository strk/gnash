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
      if test -f ${with_glib_incl}/glib/glibgl.h ; then
	ac_cv_path_glib_incl=`(cd ${with_glib_incl}; pwd)`
      else
	AC_MSG_ERROR([${with_glib_incl} directory doesn't contain glib/glibgl.h])
      fi
    fi
  ])

  dnl Attempt to find the top level directory, which unfortunately has a
  dnl version number attached. At least on Debain based systems, this
  dnl doesn't seem to get a directory that is unversioned.
  if test x"${ac_cv_path_glib_incl}" = x; then
    AC_MSG_CHECKING([for the Glib Version])
    pathlist="${prefix}/include /sw/include /usr/local/include /home/latest/include /opt/include /usr/include /usr/pkg/include .. ../.."

    topdir=""
    version=""
    for i in $pathlist; do
      for j in `ls -dr $i/glib-[[0-9]].[[0-9]] 2>/dev/null`; do
        if test -f $j/glib.h; then
          topdir=`basename $j`
          version=`echo ${topdir} | sed -e 's:glib-::'`
          break
        fi
      done
    done
  fi

  if test x"${topdir}" = x; then
    AC_MSG_RESULT(none)
  else
    AC_MSG_RESULT([${version}])
  fi

  dnl If the path hasn't been specified, go look for it.
  if test x"${ac_cv_path_glib_incl}" = x; then
    AC_CHECK_HEADERS(glib.h, [ac_cv_path_glib_incl=""],[
      if test x"${ac_cv_path_glib_incl}" = x; then
        AC_MSG_CHECKING([for libglib header])
        incllist="${prefix}/include /sw/include /usr/local/include /home/latest/include /opt/include /usr/include /usr/pkg/include .. ../.."

        for i in $incllist; do
          if test -f $i/glib.h; then
            ac_cv_path_glib_incl="$i"
            break
          else
            if test -f $i/${topdir}/glib.h; then
              ac_cv_path_glib_incl="$i/${topdir}"
              break
            fi
          fi
        done
      fi
    ])
  fi

  if test x"${ac_cv_path_glib_incl}" != x ; then
    libincl=`echo ${ac_cv_path_glib_incl} | sed -e 's/include/lib/'`
    GLIB_CFLAGS="-I${ac_cv_path_glib_incl} -I${libincl}/include"
    AC_MSG_RESULT(yes)
  else
    GLIB_CFLAGS=""
    AC_MSG_RESULT(no)
  fi

  dnl Look for the library
  AC_ARG_WITH(glib_lib, [  --with-glib-lib         directory where glib library is], with_glib_lib=${withval})
    AC_CACHE_VAL(ac_cv_path_glib_lib,[
    if test x"${with_glib_lib}" != x ; then
      if test -f ${with_glib_lib}/libglib-${version}.a -o -f ${with_glib_lib}/libglib-${version}.so; then
        ac_cv_path_glib_lib=`(cd ${with_glib_incl}; pwd)`
      else
        AC_MSG_ERROR([${with_glib_lib} directory doesn't contain libglib.])
      fi
    fi
  ])

  dnl If the header doesn't exist, there is no point looking for
  dnl the library.
  if test x"${ac_cv_path_glib_incl}" != x; then
    AC_CHECK_LIB(glib-${version}, g_io_channel_init, [ac_cv_path_glib_lib="-lglib-${version}"],[
      AC_MSG_CHECKING([for libglib library])
      libslist="${prefix}/lib64 ${prefix}/lib /usr/lib64 /usr/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib /usr/pkg/lib .. ../.."
      for i in $libslist; do
        if test -f $i/libglib-${version}.a -o -f $i/libglib-${version}.so; then
          if test x"$i" != x"/usr/lib"; then
            ac_cv_path_glib_lib="-L$i -lglib-${version}"
            break
          else
            ac_cv_path_glib_lib=""
            break
          fi
        else
          if test -f $i/libglib-${version}.a -o -f $i/libglib-${version}.so; then
            ac_cv_path_glib_lib="$i/${topdir}"
            break
          fi
        fi
      done
    ])
  else
    if test -f $i/libglib-${version}.a -o -f $i/libglib-${version}.so; then
      if test x"${ac_cv_path_glib_lib}" != x"/usr/lib"; then
        ac_cv_path_glib_lib="-L${ac_cv_path_glib_lib}"
      else
        ac_cv_path_glib_lib=""
      fi
    fi
  fi


  if test x"${ac_cv_path_glib_lib}" != x ; then
    GLIB_LIBS="${ac_cv_path_glib_lib}"
  else
    GLIB_LIBS=""
  fi

  AC_SUBST(GLIB_CFLAGS)
  AC_SUBST(GLIB_LIBS)
])
