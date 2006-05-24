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

AC_DEFUN([GNASH_PATH_PANGO],
[
    dnl Look for the header
  AC_ARG_WITH(pango_incl, [  --with-pango-incl        directory where libpango header is], with_pango_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_pango_incl,[
    if test x"${with_pango_incl}" != x ; then
      if test -f ${with_pango_incl}/pango/pango.h ; then
	ac_cv_path_pango_incl=`(cd ${with_pango_incl}; pwd)`
      else
	AC_MSG_ERROR([${with_pango_incl} directory doesn't contain pango/pango.h])
      fi
    fi
  ])

  dnl Attempt to find the top level directory, which unfortunately has a
  dnl version number attached. At least on Debain based systems, this
  dnl doesn't seem to get a directory that is unversioned.
  if test x"${ac_cv_path_pango_incl}" = x; then
    AC_MSG_CHECKING([for the Pango Version])
      pathlist="/sw/include /usr/local/include /usr/X11R6/include /home/latest/include /opt/include /usr/include /usr/pkg/include .. ../.."

      gnash_pango_topdir=""
      gnash_pango_version=""
      for i in $pathlist; do
        for j in `ls -dr $i/pango-[[0-9]].[[0-9]] 2>/dev/null`; do
          if test -f $j/pango/pango.h; then
            gnash_pango_topdir=`basename $j`
            gnash_pango_version=`echo ${gnash_pango_topdir} | sed -e 's:pango-::'`
            break
          fi
        done
      done
  fi			dnl if pango_incl

  if test x"${gnash_pango_topdir}" = x; then
    AC_MSG_RESULT(none)
  else
    AC_MSG_RESULT([${gnash_pango_version}])
  fi

  dnl If the path hasn't been specified, go look for it.
  if test x"${ac_cv_path_pango_incl}" = x; then
    AC_CHECK_HEADERS(pango/pango.h, [ac_cv_path_pango_incl=""],[
      if test x"${ac_cv_path_pango_incl}" = x; then
        AC_MSG_CHECKING([for libpango header])
        incllist="/sw/include /usr/local/include /usr/X11R6/include /home/latest/include /opt/include /usr/include /usr/pkg/include .. ../.."

        for i in $incllist; do
          if test -f $i/pango/pango.h; then
            ac_cv_path_pango_incl="$i"
            break
          else
            if test -f $i/${gnash_pango_topdir}/pango/pango.h; then
              ac_cv_path_pango_incl="$i/${gnash_pango_topdir}"
              break
            fi
          fi
        done
      fi
    ])
  fi

  if test x"${ac_cv_path_pango_incl}" != x; then
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
  fi

  dnl Look for the library
  AC_ARG_WITH(pango_lib, [  --with-pango-lib         directory where pango library is], with_pango_lib=${withval})
    AC_CACHE_VAL(ac_cv_path_pango_lib,[
    if test x"${with_pango_lib}" != x ; then
      if test -f ${with_pango_lib}/libpangopango-x11-${gnash_pango_version}.a -o -f ${with_pango_lib}/libpangopango-x11-${gnash_pango_version}.so; then
        ac_cv_path_pango_lib=`(cd ${with_pango_incl}; pwd)`
      else
        AC_MSG_ERROR([${with_pango_lib} directory doesn't contain libpangopango.])
      fi
    fi
  ])

  dnl If the header doesn't exist, there is no point looking for
  dnl the library.
  if test x"${ac_cv_path_pango_incl}" != x; then
    AC_CHECK_LIB(pango-${gnash_pango_version}, pango_engine_shape_class_init, [ac_cv_path_pango_lib="-lpango-${gnash_pango_version}"],[
      AC_MSG_CHECKING([for libpango library])
      libslist="/usr/lib64 /usr/lib /sw/lib /usr/X11R6/lib64 /usr/X11R6/lib /usr/local/lib /home/latest/lib /opt/lib /usr/pkg/lib .. ../.."
      for i in $libslist; do
        if test -f $i/libpango-${gnash_pango_version}.a -o -f $i/libpango-${gnash_pango_version}.so; then
          if test x"$i" != x"/usr/lib"; then
            ac_cv_path_pango_lib="-L$i -lpango-${gnash_pango_version}"
            break
          else
            ac_cv_path_pango_lib="-lpango-${gnash_pango_version}"
            break
          fi
        else
          if test -f $i/libpango-${gnash_pango_version}.a -o -f $i/libpango-${gnash_pango_version}.so; then
            ac_cv_path_pango_lib="$i/${gnash_pango_topdir}"
            break
          fi
        fi
      done
    ])
  else
    if test -f $i/libpango-${gnash_pango_version}.a -o -f $i/libpango-${gnash_pango_version}.so; then
      if test x"${ac_cv_path_pango_lib}" != x"/usr/lib"; then
        ac_cv_path_pango_lib="-L${ac_cv_path_pango_lib} -lpango-${gnash_pango_version}"
        else
        ac_cv_path_pango_lib="-lpango-${gnash_pango_version}"
      fi
    fi
  fi


  if test x"${ac_cv_path_pango_incl}" != x; then
    PANGO_CFLAGS="-I${ac_cv_path_pango_incl}"
  else
    PANGO_CFLAGS=""
  fi

  if test x"${ac_cv_path_pango_lib}" != x; then
    PANGO_LIBS="${ac_cv_path_pango_lib}"
  else
    PANGO_LIBS=""
  fi

  AC_SUBST(PANGO_CFLAGS)
  AC_SUBST(PANGO_LIBS)
])
