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

AC_DEFUN([AM_PATH_MP3],
[
  AC_ARG_ENABLE(mp3, [  --enable-mp3       Enable support for playing mp3s],
  [case "${enableval}" in
    yes) mp3=yes ;;
    no)  mp3=no ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for enable-mp3 option]) ;;
  esac], mp3=no)

  if test x"$mp3" = x"yes"; then
    dnl Look for the header
  AC_ARG_WITH(mad_incl, [  --with-mad_incl         directory where libmad header is], with_mad_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_mad_incl,[
    if test x"${with_mad_incl}" != x ; then
      if test -f ${with_mad_incl}/mad.h ; then
	ac_cv_path_mad_incl=`(cd ${with_mad_incl}; pwd)`
      else
	AC_MSG_ERROR([${with_mad_incl} directory doesn't contain mad.h])
      fi
    fi
    ])

    dnl If the path hasn't been specified, go look for it.
    if test x"${ac_cv_path_mad_incl}" = x; then
      AC_CHECK_HEADERS(mad.h, [ac_cv_path_mad_incl=""],[
      if test x"${ac_cv_path_mad_incl}" = x; then
        AC_MSG_CHECKING([for libmad header])
        incllist="/sw/include /usr/local/include /home/latest/include /opt/include /usr/include .. ../.."

        for i in $incllist; do
	  if test -f $i/mad.h; then
	    if test x"$i" != x"/usr/include"; then
	      ac_cv_path_mad_incl="-I$i"
	      break
            else
	      ac_cv_path_mad_incl=""
	      break
	    fi
	  fi
        done
      fi])
    else
      AC_MSG_RESULT(-I${ac_cv_path_mad_incl})
      if test x"${ac_cv_path_mad_incl}" != x"/usr/include"; then
	ac_cv_path_mad_incl="-I${ac_cv_path_mad_incl}"
       else
	ac_cv_path_mad_incl=""
      fi
    fi

    if test x"${ac_cv_path_mad_incl}" != x ; then
      MAD_CFLAGS="${ac_cv_path_mad_incl}"
      AC_MSG_RESULT(${ac_cv_path_mad_incl})
    else
      MAD_CFLAGS=""
    fi

      dnl Look for the library
      AC_ARG_WITH(mad_lib, [  --with-mad-lib          directory where mad library is], with_mad_lib=${withval})
      AC_CACHE_VAL(ac_cv_path_mad_lib,[
      if test x"${with_mad_lib}" != x ; then
        if test -f ${with_mad_lib}/libmad.a -o -f ${with_mad_lib}/libmad.so; then
	  ac_cv_path_mad_lib=`(cd ${with_mad_incl}; pwd)`
        else
	  AC_MSG_ERROR([${with_mad_lib} directory doesn't contain libmad.])
        fi
      fi
      ])

      dnl If the header doesn't exist, there is no point looking for the library.
      if test x"${ac_cv_path_mad_lib}" = x; then
        AC_CHECK_LIB(mad, mad_copyright, [ac_cv_path_mad_lib="-lmad"],[
          AC_MSG_CHECKING([for libmad library])
          libslist="/sw/lib /usr/local/lib /home/latest/lib /opt/lib /usr/lib .. ../.."
          for i in $libslist; do
	    if test -f $i/libmad.a -o -f $i/libmad.so; then
	      if test x"$i" != x"/usr/lib"; then
	        ac_cv_path_mad_lib="-L$i"
                AC_MSG_RESULT(${ac_cv_path_mad_lib})
	        break
              else
	        ac_cv_path_mad_lib=""
                AC_MSG_RESULT(yes)
	        break
	      fi
	    fi
          done])
      else
        if test -f ${ac_cv_path_mad_lib}/libmad.a -o -f ${ac_cv_path_mad_lib}/libmad.so; then

          if test x"${ac_cv_path_mad_lib}" != x"/usr/lib"; then
	    ac_cv_path_mad_lib="-I${ac_cv_path_mad_lib}"
           else
	    ac_cv_path_mad_lib=""
          fi
        fi
      fi

      if test x"${ac_cv_path_mad_lib}" != x ; then
        MAD_LIBS="${ac_cv_path_mad_lib}"
      else
        MAD_LIBS=""
      fi
    fi

  if test x"${ac_cv_path_mad_lib}" != x ; then
      MAD_LIBS="${ac_cv_path_mad_lib}"
  fi

  AM_CONDITIONAL(MP3, [test x$mp3 = xyes])

  AC_SUBST(MAD_CFLAGS)
  AC_SUBST(MAD_LIBS)
])
