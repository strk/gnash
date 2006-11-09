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

dnl $Id: cairo.m4,v 1.12 2006/11/09 18:28:15 nihilus Exp $

AC_DEFUN([GNASH_PATH_CAIRO],
[
  dnl Look for the header
  AC_ARG_WITH(cairo_incl, AC_HELP_STRING([--with-cairo_incl], [directory where libcairo header is]), with_cairo_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_cairo_incl,[
    if test x"${with_cairo_incl}" != x ; then
      if test -f ${with_cairo_incl}/cairo.h ; then
	ac_cv_path_cairo_incl=`(cd ${with_cairo_incl}; pwd)`
      else
	AC_MSG_ERROR([${with_cairo_incl} directory doesn't contain cairo.h])
      fi
    fi
  ])

if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_cairo_incl}" = x; then
	$PKG_CONFIG --exists cairo && ac_cv_path_cairo_incl=`$PKG_CONFIG --cflags cairo`
fi
  
  dnl we can use cairo even if no plugin is enabled
  dnl if test x"$plugin" = x"yes"; then

    dnl If the path hasn't been specified, go look for it.
    if test x"${ac_cv_path_cairo_incl}" = x; then
      AC_CHECK_HEADERS(cairo/cairo.h, [ac_cv_path_cairo_incl=""],[
        if test x"${ac_cv_path_cairo_incl}" = x; then
          incllist="/sw/include /usr/local/include /home/latest/include /opt/include /opt/local/include /usr/include /usr/pkg/include .. ../.."

          for i in $incllist; do
	    if test -f $i/cairo/cairo.h; then
	      ac_cv_path_cairo_incl="$i/cairo"
	    fi
          done
        fi
      ])   
	    if test x"${ac_cv_path_cairo_incl}" != x"/usr/include"; then
	      ac_cv_path_cairo_incl="-I${ac_cv_path_cairo_incl}"
	    else
	      ac_cv_path_cairo_incl=""
	    fi
    
    else
	    AC_MSG_CHECKING([for libcairo header])
	    AC_MSG_RESULT(${ac_cv_path_cairo_incl}) 
    fi


    dnl Look for the library
    AC_ARG_WITH(cairo_lib, AC_HELP_STRING([--with-cairo-lib], [directory where cairo library is]), with_cairo_lib=${withval})
      AC_CACHE_VAL(ac_cv_path_cairo_lib,[
      if test x"${with_cairo_lib}" != x ; then
        if test -f ${with_cairo_lib}/libcairo.a -o -f ${with_cairo_lib}/libcairo.so; then
	  ac_cv_path_cairo_lib=`(cd ${with_cairo_incl}; pwd)`
        else
	  AC_MSG_ERROR([${with_cairo_lib} directory doesn't contain libcairo.])
        fi
      fi
    ])

  if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_cairo_lib}" = x; then
    $PKG_CONFIG --exists cairo && ac_cv_path_cairo_lib=`$PKG_CONFIG --libs cairo`
  fi

    dnl If the header doesn't exist, there is no point looking for the library.
    if test x"${ac_cv_path_cairo_lib}" = x; then
      AC_CHECK_LIB(cairo, cairo_status, [ac_cv_path_cairo_lib="-lcairo"],[
        libslist="/usr/lib64 /usr/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib  /opt/local/lib /usr/pkg/lib .. ../.."
        for i in $libslist; do
	  if test -f $i/libcairo.a -o -f $i/libcairo.so; then
	    if test x"$i" != x"/usr/lib"; then
	      ac_cv_path_cairo_lib="-L$i -lcairo"
	      break
            else
              ac_cv_path_cairo_lib=""
	      break
	    fi
	  fi
        done
      ])
    else
	    AC_MSG_CHECKING([for libcairo library])
	    AC_MSG_RESULT(${ac_cv_path_cairo_lib})    
    fi

  dnl we seek cairo even if no plugin is enabled
  dnl fi

  if test x"${ac_cv_path_cairo_incl}" != x ; then
    CAIRO_CFLAGS="${ac_cv_path_cairo_incl}"
  else
    CAIRO_CFLAGS=""
  fi

  if test x"${ac_cv_path_cairo_lib}" != x ; then
    CAIRO_LIBS="${ac_cv_path_cairo_lib}"
  else
    CAIRO_LIBS=""
  fi

  AC_SUBST(CAIRO_CFLAGS)
  AC_SUBST(CAIRO_LIBS)
])
