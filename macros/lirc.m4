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

AC_DEFUN([GNASH_PATH_LIRC],
[
  dnl Look for the header
  AC_ARG_WITH(lirc_incl, AC_HELP_STRING([--with-lirc-incl], [directory where Lirc header is (w/out the lirc/ prefix)]), with_lirc_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_lirc_incl,[
    if test x"${with_lirc_incl}" != x ; then
      if test -f ${with_lirc_incl}/lirc_client.h ; then
	      ac_cv_path_lirc_incl="-I`(cd ${with_lirc_incl}; pwd)`"
      else
	      AC_MSG_ERROR([${with_lirc_incl} directory doesn't contain lirc/lirc.h])
      fi
    fi
  ])

  AC_ARG_WITH(lirc_lib, AC_HELP_STRING([--with-lirc-lib], [directory where lirc library is]), with_lirc_lib=${withval})
    AC_CACHE_VAL(ac_cv_path_lirc_lib,[
    if test x"${with_lirc_lib}" != x ; then
      if test -f ${with_lirc_lib}/liblirc_client.a -o -f ${with_lirc_lib}/liblirc_client.${shlibext}; then
        ac_cv_path_lirc_lib="-L`(cd ${with_lirc_lib}; pwd)` -llirc_client"
      else
        AC_MSG_ERROR([${with_lirc_lib} directory doesn't contain liblirc.])
      fi
    fi
  ])

  if test x"${ext_lirc}" = x"yes"; then
    dnl If the path hasn't been specified, go look for it.
    if test x"${ac_cv_path_lirc_incl}" = x; then
      for i in $incllist; do
        if test -f $i/lirc/lirc_client.h; then
          ac_cv_path_lirc_incl="-I$i"
	        break
        fi
      done

      if test x"${ac_cv_path_lirc_incl}" = x ; then
        AC_CHECK_HEADERS(lirc/lirc_client.h)
      fi

      AC_MSG_CHECKING([for Lirc header])
      if test x"${ac_cv_path_lirc_incl}" != x ; then
        AC_MSG_RESULT(yes)
      else
        AC_MSG_RESULT(no)
      fi
    fi

    dnl If the path hasn't been specified, go look for it.
    if test x"${ac_cv_path_lirc_lib}" = x; then
      dnl lirc-config gives us way to many libraries, which create nasty linking
      dnl dependancy issue, so we strip them off here. The real dependencies are
      dnl are taken care of by other config tests.
      AC_MSG_CHECKING([for lirc_client library])
      for i in $libslist; do
        if test -f $i/liblirc_client.a -o -f $i/liblirc_client.${shlibext}; then
          if test x"$i" != x"/usr/lib" -o x"$i" != x"/usr/lib64"; then
            ac_cv_path_lirc_lib="-L$i -llirc_client"
            AC_MSG_RESULT(${ac_cv_path_lirc_lib})
            break
          else
            ac_cv_path_lirc_lib="-llirc_client"
            AC_MSG_RESULT(yes)
            break
          fi
        fi
      done
    fi

    if test x"${ac_cv_path_lirc_incl}" != x ; then
      if test x"${ac_cv_path_lirc_incl}" != x"/usr/include"; then
        LIRC_CFLAGS="${ac_cv_path_lirc_incl}"
      fi
    else
      LIRC_CFLAGS=""
    fi

    if test x"${ac_cv_path_lirc_lib}" != x ; then
      if test ! x"${ac_cv_path_lirc_lib}" = x"/usr/lib" -a ! x"$i" = x"/usr/lib64"; then
        LIRC_LIBS="${ac_cv_path_lirc_lib}"
        AC_DEFINE(HAVE_LIRC, [1], [Has Lirc])
      fi
    else
      LIRC_LIBS=""
    fi
  fi

  AC_SUBST(LIRC_CFLAGS)
  AC_SUBST(LIRC_LIBS)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
