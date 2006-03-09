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
dnl  Linking Gnash statically or dynamically with other modules is making
dnl  a combined work based on Gnash. Thus, the terms and conditions of
dnl  the GNU General Public License cover the whole combination.
dnl  
dnl  In addition, as a special exception, the copyright holders of Gnash give
dnl  you permission to combine Gnash with free software programs or
dnl  libraries that are released under the GNU LGPL and/or with Mozilla, 
dnl  so long as the linking with Mozilla, or any variant of Mozilla, is
dnl  through its standard plug-in interface. You may copy and distribute
dnl  such a system following the terms of the GNU GPL for Gnash and the
dnl  licenses of the other code concerned, provided that you include the
dnl  source code of that other code when and as the GNU GPL requires
dnl  distribution of source code. 
dnl  
dnl  Note that people who make modified versions of Gnash are not obligated
dnl  to grant this special exception for their modified versions; it is
dnl  their choice whether to do so.  The GNU General Public License gives
dnl  permission to release a modified version without this exception; this
dnl  exception also makes it possible to release a modified version which
dnl  carries forward this exception.
dnl 

AC_DEFUN([GNASH_PATH_CAIRO],
[
  dnl Look for the header
  AC_ARG_WITH(cairo_incl, [  --with-cairo_incl         directory where libcairo header is], with_cairo_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_cairo_incl,[
    if test x"${with_cairo_incl}" != x ; then
      if test -f ${with_cairo_incl}/cairo.h ; then
	ac_cv_path_cairo_incl=`(cd ${with_cairo_incl}; pwd)`
      else
	AC_MSG_ERROR([${with_cairo_incl} directory doesn't contain cairo.h])
      fi
    fi
  ])

  if test x"$glext" = x"yes"; then
    dnl If the path hasn't been specified, go look for it.
    if test x"${ac_cv_path_cairo_incl}" = x; then
      AC_CHECK_HEADERS(cairio/cairo.h, [ac_cv_path_cairo_incl=""],[
        if test x"${ac_cv_path_cairo_incl}" = x; then
          AC_MSG_CHECKING([for Cairo header])
          incllist="/sw/include /usr/local/include /home/latest/include /opt/include /usr/include /usr/pkg/include .. ../.."

          for i in $incllist; do
	    if test -f $i/cairo/cairo.h; then
	      ac_cv_path_cairo_incl="$i/cairo"
	    fi
          done
        fi
      ])
    fi

    if test x"${ac_cv_path_cairo_incl}" != x"/usr/include"; then
      ac_cv_path_cairo_incl="${ac_cv_path_cairo_incl}"
    else
      ac_cv_path_cairo_incl=""
    fi

    if test x"${ac_cv_path_cairo_incl}" != x ; then
      AC_MSG_RESULT(yes)
    else
      AC_MSG_RESULT(no)
    fi

    dnl Look for the library
    AC_ARG_WITH(cairo_lib, [  --with-cairo-lib          directory where cairo library is], with_cairo_lib=${withval})
      AC_CACHE_VAL(ac_cv_path_cairo_lib,[
      if test x"${with_cairo_lib}" != x ; then
        if test -f ${with_cairo_lib}/libcairo.a -o -f ${with_cairo_lib}/libcairo.so; then
	  ac_cv_path_cairo_lib=`(cd ${with_cairo_incl}; pwd)`
        else
	  AC_MSG_ERROR([${with_cairo_lib} directory doesn't contain libcairo.])
        fi
      fi
    ])

    dnl If the header doesn't exist, there is no point looking for the library.
    if test x"${ac_cv_path_cairo_lib}" = x; then
      AC_CHECK_LIB(cairo, cairo_status, [ac_cv_path_cairo_lib="-lcairo"],[
        AC_MSG_CHECKING([for libcairo library])
        libslist="/usr/lib64 /usr/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib /usr/pkg/lib .. ../.."
        for i in $libslist; do
	  if test -f $i/libcairo.a -o -f $i/libcairo.so; then
	    if test x"$i" != x"/usr/lib"; then
	      ac_cv_path_cairo_lib="-L$i"
              AC_MSG_RESULT(${ac_cv_path_cairo_lib})
	      break
            else
              ac_cv_path_cairo_lib=""
              AC_MSG_RESULT(yes)
	      break
	    fi
	  fi
        done
      ])
    else
      if test -f ${ac_cv_path_cairo_lib}/libcairo.a -o -f ${ac_cv_path_cairo_lib}/libcairo.so; then

        if test x"${ac_cv_path_cairo_lib}" != x"/usr/lib"; then
	  ac_cv_path_cairo_lib="-L${ac_cv_path_cairo_lib}"
         else
	  ac_cv_path_cairo_lib=""
        fi
      fi
    fi
  fi

  if test x"${ac_cv_path_cairo_incl}" != x ; then
    CAIRO_CFLAGS="-I${ac_cv_path_cairo_incl}"
  else
    CAIRO_CFLAGS=""
  fi

  if test x"${ac_cv_path_cairo_lib}" != x ; then
    CAIRO_LIBS="${ac_cv_path_cairo_lib}"
  else
    CAIRO_LIBS=""
  fi

  AM_CONDITIONAL(CAIRO, [test x$cairo = xyes])

  AC_SUBST(CAIRO_CFLAGS)
  AC_SUBST(CAIRO_LIBS)
])
