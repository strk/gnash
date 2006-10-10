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

dnl $Id: ogg.m4,v 1.14 2006/10/10 20:17:52 nihilus Exp $

AC_DEFUN([GNASH_PATH_OGG],
[
  AC_ARG_ENABLE(ogg, [  --enable-ogg            Enable support for playing oggs],
  [case "${enableval}" in
    yes) ogg=yes ;;
    no)  ogg=no ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for enable-ogg option]) ;;
  esac], ogg=yes)

  if test x"$ogg" = x"yes"; then
    dnl Look for the header
  AC_ARG_WITH(ogg_incl, [  --with-ogg_incl         directory where libogg header is], with_ogg_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_ogg_incl,[
    if test x"${with_ogg_incl}" != x ; then
      if test -f ${with_ogg_incl}/ogg.h ; then
	ac_cv_path_ogg_incl=`(cd ${with_ogg_incl}; pwd)`
      else
	AC_MSG_ERROR([${with_ogg_incl} directory doesn't contain ogg.h])
      fi
    fi
    ])

    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_ogg_incl}" = x; then
      $PKG_CONFIG --exists ogg && ac_cv_path_ogg_incl=`$PKG_CONFIG --cflags ogg`
    fi

    dnl If the path hasn't been specified, go look for it.
    AC_MSG_CHECKING([for libogg header])
    if test x"${ac_cv_path_ogg_incl}" = x; then
      AC_CHECK_HEADERS(ogg.h, [ac_cv_path_ogg_incl=""],[
      if test x"${ac_cv_path_ogg_incl}" = x; then
        incllist="${prefix}/include /sw/include /usr/local/include /home/latest/include /opt/include /opt/local/include /usr/include/usr/pkg/include .. ../.."

        for i in $incllist; do
	  if test -f $i/ogg/ogg.h; then
	    if test x"$i" != x"/usr/include"; then
	      ac_cv_path_ogg_incl="-I$i"
	      break
            else
	      ac_cv_path_ogg_incl=""
	      break
	    fi
	  fi
        done
      fi])
    else
      if test x"${ac_cv_path_ogg_incl}" != x"/usr/include"; then
	ac_cv_path_ogg_incl="${ac_cv_path_ogg_incl}"
       else
	ac_cv_path_ogg_incl=""
      fi
    fi
    AC_MSG_RESULT(${ac_cv_path_ogg_incl})
 
    if test x"${ac_cv_path_ogg_incl}" != x ; then
      OGG_CFLAGS="${ac_cv_path_ogg_incl}"
      AC_MSG_RESULT(${ac_cv_path_ogg_incl})
    else
      OGG_CFLAGS=""
    fi

      dnl Look for the library
      AC_ARG_WITH(ogg_lib, [  --with-ogg-lib          directory where ogg library is], with_ogg_lib=${withval})
      AC_CACHE_VAL(ac_cv_path_ogg_lib,[
      if test x"${with_ogg_lib}" != x ; then
        if test -f ${with_ogg_lib}/libogg.a -o -f ${with_ogg_lib}/libogg.so; then
	  ac_cv_path_ogg_lib=`(cd ${with_ogg_incl}; pwd)`
        else
	  AC_MSG_ERROR([${with_ogg_lib} directory doesn't contain libogg.])
        fi
      fi
      ])

      dnl If the header doesn't exist, there is no point looking for the library.
      if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_ogg_lib}" = x; then
        $PKG_CONFIG --exists ogg && ac_cv_path_ogg_lib=`$PKG_CONFIG --libs ogg`
      fi
      AC_MSG_CHECKING([for libogg library])
      if test x"${ac_cv_path_ogg_lib}" = x; then
        AC_CHECK_LIB(ogg, ogg_sync_init, [ac_cv_path_ogg_lib="-logg"],[
          libslist="${prefix}/lib64 ${prefix}/lib /usr/lib64 /usr/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib  /opt/local/lib /usr/pkg/lib .. ../.."
          for i in $libslist; do
	    if test -f $i/libogg.a -o -f $i/libogg.so; then
	      if test x"$i" != x"/usr/lib"; then
	        ac_cv_path_ogg_lib="-L$i"
	        break
              else
	        ac_cv_path_ogg_lib=""
	        break
	      fi
	    fi
          done])      
      fi
      
      AC_MSG_RESULT(${ac_cv_path_ogg_lib})
      
      if test x"${ac_cv_path_ogg_lib}" != x ; then
        OGG_LIBS="${ac_cv_path_ogg_lib}"
      else
        OGG_LIBS=""
      fi
    fi

  if test x"${ac_cv_path_ogg_lib}" != x ; then
      OGG_LIBS="${ac_cv_path_ogg_lib}"
  fi

  AM_CONDITIONAL(OGG, [test x$ogg = xyes])

  AC_SUBST(OGG_CFLAGS)
  AC_SUBST(OGG_LIBS)
])
