dnl
dnl  Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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
dnl
dnl  You should have received a copy of the GNU General Public License
dnl  along with this program; if not, write to the Free Software
dnl  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

AC_DEFUN([AM_PATH_PNG],
[
  AC_ARG_ENABLE(png, [  --enable-png       Enable support for png images],
  [case "${enableval}" in
    yes) png=yes ;;
    no)  png=no ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for enable-png option]) ;;
  esac], png=yes)

  if test x"$png" = x"yes"; then
    dnl Look for the header
  AC_ARG_WITH(png_incl, [  --with-png_incl         directory where libpng header is], with_png_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_png_incl,[
    if test x"${with_png_incl}" != x ; then
      if test -f ${with_png_incl}/png.h ; then
	ac_cv_path_png_incl=`(cd ${with_png_incl}; pwd)`
      else
	AC_MSG_ERROR([${with_png_incl} directory doesn't contain png.h])
      fi
    fi
    ])

    dnl If the path hasn't been specified, go look for it.
    if test x"${ac_cv_path_png_incl}" = x; then
      AC_CHECK_HEADERS(png.h, [ac_cv_path_png_incl=""],[
      if test x"${ac_cv_path_png_incl}" = x; then
        AC_MSG_CHECKING([for libpng header])
        incllist="/sw/include /usr/local/include /home/latest/include /opt/include /usr/include /usr/pkg/include .. ../.."

        for i in $incllist; do
	  if test -f $i/png.h; then
	    if test x"$i" != x"/usr/include"; then
	      ac_cv_path_png_incl="-I$i"
	      break
            else
	      ac_cv_path_png_incl=""
	      break
	    fi
	  fi
        done
      fi])
    else
      AC_MSG_RESULT(-I${ac_cv_path_png_incl})
      if test x"${ac_cv_path_png_incl}" != x"/usr/include"; then
	ac_cv_path_png_incl="-I${ac_cv_path_png_incl}"
       else
	ac_cv_path_png_incl=""
      fi
    fi

    if test x"${ac_cv_path_png_incl}" != x ; then
      PNG_CFLAGS="${ac_cv_path_png_incl}"
      AC_MSG_RESULT(${ac_cv_path_png_incl})
    else
      PNG_CFLAGS=""
    fi

      dnl Look for the library
      AC_ARG_WITH(png_lib, [  --with-png-lib          directory where png library is], with_png_lib=${withval})
      AC_CACHE_VAL(ac_cv_path_png_lib,[
      if test x"${with_png_lib}" != x ; then
        if test -f ${with_png_lib}/libpng.a -o -f ${with_png_lib}/libpng.so; then
	  ac_cv_path_png_lib=`(cd ${with_png_incl}; pwd)`
        else
	  AC_MSG_ERROR([${with_png_lib} directory doesn't contain libpng.])
        fi
      fi
      ])

      dnl If the header doesn't exist, there is no point looking for the library.
      if test x"${ac_cv_path_png_lib}" = x; then
        AC_CHECK_LIB(png, png_crc_read, [ac_cv_path_png_lib="-lpng"],[
          AC_MSG_CHECKING([for libpng library])
          libslist="/sw/lib /usr/local/lib /home/latest/lib /opt/lib /usr/lib /usr/pkg/lib .. ../.."
          for i in $libslist; do
	    if test -f $i/libpng.a -o -f $i/libpng.so; then
	      if test x"$i" != x"/usr/lib"; then
	        ac_cv_path_png_lib="-L$i"
                AC_MSG_RESULT(${ac_cv_path_png_lib})
	        break
              else
	        ac_cv_path_png_lib=""
                AC_MSG_RESULT(yes)
	        break
	      fi
	    fi
          done])
      else
        if test -f ${ac_cv_path_png_lib}/libpng.a -o -f ${ac_cv_path_png_lib}/libpng.so; then

          if test x"${ac_cv_path_png_lib}" != x"/usr/lib"; then
	    ac_cv_path_png_lib="-L${ac_cv_path_png_lib}"
           else
	    ac_cv_path_png_lib=""
          fi
        fi
      fi
    fi

  if test x"${ac_cv_path_png_lib}" != x ; then
      PNG_LIBS="${ac_cv_path_png_lib}"
  fi

  AM_CONDITIONAL(HAVE_PNG, [test x$png = xyes])

  AC_SUBST(PNG_CFLAGS)
  AC_SUBST(PNG_LIBS)
])
