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

AC_DEFUN([GNASH_PATH_FLTK],
[
  AC_ARG_ENABLE(fltk, AC_HELP_STRING([--enable-fltk], [Enable support for FLTK]),
  [case "${enableval}" in
    yes) fltk=yes ;;
    no)  fltk=no ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for enable-fltk option]) ;;
  esac], fltk=yes)

  if test x"$fltk" = x"yes"; then
    dnl Look for the header
  AC_ARG_WITH(fltk_incl, AC_HELP_STRING([--with-fltk-incl], [directory where libfltk header is]), with_fltk_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_fltk_incl, [
    AC_MSG_CHECKING([for fltk.h header in specified directory])
    if test x"${with_fltk_incl}" != x ; then
      if test -f ${with_fltk_incl}/fltk/FL_API.h; then
	ac_cv_path_fltk_incl=`(cd ${with_fltk_incl}; pwd)`
	AC_MSG_RESULT([yes])
      else
	AC_MSG_ERROR([${with_fltk_incl} directory doesn't contain fltk/FL_API.h])
      fi
    else
	AC_MSG_RESULT([no])
    fi
    ])

    dnl If the path hasn't been specified, go look for it.
    if test x"${ac_cv_path_fltk_incl}" = x; then
      AC_CHECK_HEADERS(fltk/FL_API.h, [ac_cv_path_fltk_incl=""],[
      if test x"${ac_cv_path_fltk_incl}" = x; then
        AC_MSG_CHECKING([for libfltk header])
        incllist="${prefix}/include /sw/include /usr/local/include /usr/X11R6/include /home/latest/include /opt/include /usr/include /usr/pkg/include .. ../.."

	ac_cv_path_fltk_incl=""
        for i in $incllist; do
	  if test -f $i/fltk/FL_API.h; then
	    if test x"$i" != x"/usr/include"; then
	      ac_cv_path_fltk_incl="$i"
	      break
	    fi
	  fi
        done
      fi])
    else
      if test x"${ac_cv_path_fltk_incl}" != x"/usr/include"; then
	ac_cv_path_fltk_incl="${ac_cv_path_fltk_incl}"
      fi
    fi

    if test x"${ac_cv_path_fltk_incl}" != x ; then
      FLTK_CFLAGS="-I${ac_cv_path_fltk_incl}"
    else
      FLTK_CFLAGS=""
    fi

      dnl Look for the library
      AC_ARG_WITH(fltk_lib, AC_HELP_STRING([--with-fltk-lib], [directory where fltk library is]), with_fltk_lib=${withval})
      AC_CACHE_VAL(ac_cv_path_fltk_lib,[
      if test x"${with_fltk_lib}" != x ; then
        AC_MSG_CHECKING([for libfltk library in specified directory])
        if test -f ${with_fltk_lib}/libfltk.a -o -f ${with_fltk_lib}/libfltk_gl.a; then
	  tmp=`(cd ${with_fltk_lib}; pwd)`
	  ac_cv_path_fltk_lib="-L${tmp} -lfltk"
	  AC_MSG_RESULT([yes])
        else
	  AC_MSG_ERROR([${with_fltk_lib} directory doesn't contain libfltk.])
	  AC_MSG_RESULT([no])
        fi
      fi
      ])

      dnl If the header doesn't exist, there is no point looking for the library.
      if test x"${ac_cv_path_fltk_lib}" = x; then
        AC_CHECK_LIB(fltk, fl_xmap, [ac_cv_path_fltk_lib="-lfltk -lfltk_gl"],[
          AC_MSG_CHECKING([for libfltk library])
          libslist="${prefix}/lib64 ${prefix}/lib /usr/lib64 /usr/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib /usr/pkg/lib /usr/X11R6/lib .. ../.."
          ac_cv_path_fltk_lib=""
          for i in $libslist; do
	    if test -f $i/libfltk.a -o -f $i/libfltk_gl.a; then
	      if test x"$i" != x"/usr/lib"; then
	        ac_cv_path_fltk_lib="-L$i -lfltk -lfltk_gl"
                AC_MSG_RESULT(${ac_cv_path_fltk_lib})
	        break
              else
	        ac_cv_path_fltk_lib="-lfltk -lfltk_g"
                AC_MSG_RESULT(yes)
	        break
	      fi
	    fi
          done])
      fi
    fi

  if test x"${ac_cv_path_fltk_lib}" != x ; then
      FLTK_LIBS="${ac_cv_path_fltk_lib}"
      has_fltk=yes
  else
      has_fltk=no
      FLTK_LIBS=""
  fi

  AC_SUBST(FLTK_CFLAGS)
  AC_SUBST(FLTK_LIBS)
])
