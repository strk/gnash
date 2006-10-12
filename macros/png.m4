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

dnl $Id: png.m4,v 1.14 2006/10/13 00:36:43 nihilus Exp $

AC_DEFUN([GNASH_PATH_PNG],
[
  AC_ARG_ENABLE(png, [  --enable-png            Enable support for png images],
  [case "${enableval}" in
    yes) png=yes ;;
    no)  png=no ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for enable-png option]) ;;
  esac], png=yes)

  if test x"$png" = x"yes"; then
    dnl Look for the header
  AC_ARG_WITH(png_incl, [  --with-png-incl         directory where libpng header is], with_png_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_png_incl, [
    AC_MSG_CHECKING([for png.h header in specified directory])
    if test x"${with_png_incl}" != x ; then
      if test -f ${with_png_incl}/png.h ; then
	ac_cv_path_png_incl=`(cd ${with_png_incl}; pwd)`
	AC_MSG_RESULT([yes])
      else
	AC_MSG_ERROR([${with_png_incl} directory doesn't contain png.h])
      fi
    else
	AC_MSG_RESULT([no])
    fi
    ])

    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_png_incl}" = x; then
      $PKG_CONFIG --exists libpng && ac_cv_path_png_incl=`$PKG_CONFIG --cflags libpng`
    fi

    dnl If the path hasn't been specified, go look for it.
    AC_MSG_CHECKING([for libpng header])
    if test x"${ac_cv_path_png_incl}" = x; then
      AC_CHECK_HEADERS(png.h, [ac_cv_path_png_incl=""],[
      if test x"${ac_cv_path_png_incl}" = x; then
        incllist="${prefix}/include /sw/include /usr/local/include /home/latest/include /opt/include /opt/local/include /usr/include /usr/pkg/include .. ../.."

        for i in $incllist; do
	  if test -f $i/png.h; then
	    if test x"$i" != x"/usr/include"; then
	      ac_cv_path_png_incl="-I$i"
	      break
            else
	      ac_cv_path_png_incl=""
	      break
	    fi
          else
dnl 
	    if test -f $i/libpng/png.h; then
	      ac_cv_path_png_incl="-I$i/libpng"
	      break
	    fi
	  fi
        done
      fi])
    else
      if test x"${ac_cv_path_png_incl}" != x"/usr/include"; then
	ac_cv_path_png_incl="${ac_cv_path_png_incl}"
       else
	ac_cv_path_png_incl=""
      fi
    fi
    AC_MSG_RESULT(${ac_cv_path_png_incl})

    if test x"${ac_cv_path_png_incl}" != x ; then
      PNG_CFLAGS="${ac_cv_path_png_incl}"
    else
      PNG_CFLAGS=""
    fi

      dnl Look for the library
      AC_ARG_WITH(png_lib, [  --with-png-lib          directory where png library is], with_png_lib=${withval})
      AC_CACHE_VAL(ac_cv_path_png_lib,[
      if test x"${with_png_lib}" != x ; then
        AC_MSG_CHECKING([for libpng library in specified directory])
        if test -f ${with_png_lib}/libpng.a -o -f ${with_png_lib}/libpng.so; then
	  tmp=`(cd ${with_png_lib}; pwd)`
	  ac_cv_path_png_lib="-L${tmp} -lpng"
	  AC_MSG_RESULT([yes])
        else
	  AC_MSG_ERROR([${with_png_lib} directory doesn't contain libpng.])
	  AC_MSG_RESULT([no])
        fi
      fi
      ])

      dnl If the header doesn't exist, there is no point looking for the library.
      if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_png_lib}" = x; then
        $PKG_CONFIG --exists libpng && ac_cv_path_png_lib=`$PKG_CONFIG --libs libpng`
      fi
      AC_MSG_CHECKING([for libpng library])
      if test x"${ac_cv_path_png_lib}" = x; then
        AC_CHECK_LIB(png, png_check_sig, [ac_cv_path_png_lib=""],[

          libslist="${prefix}/lib64 ${prefix}/lib32 ${prefix}/lib /usr/lib64 /usr/lib32 /usr/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib /opt/local/lib /usr/pkg/lib /usr/X11R6/lib .. ../.."
          for i in $libslist; do
	    if test -f $i/libpng.a -o -f $i/libpng.so; then
	      if test x"$i" != x"/usr/lib"; then
	        ac_cv_path_png_lib="-L$i -lpng"
	        break
              else
	        ac_cv_path_png_lib=""
	        break
	      fi
	    fi
          done])
      fi
    fi
 
  AC_MSG_RESULT(${ac_cv_path_png_lib})
  if test x"${ac_cv_path_png_lib}" != x ; then
      PNG_LIBS="${ac_cv_path_png_lib}"
  else
      PNG_LIBS=""
  fi

  AM_CONDITIONAL(HAVE_PNG, [test x$png = xyes])

  AC_SUBST(PNG_CFLAGS)
  AC_SUBST(PNG_LIBS)
])
