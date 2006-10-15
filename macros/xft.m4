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

AC_DEFUN([GNASH_PATH_XFT],
[
  if test x"$fltk" = x"yes"; then
    dnl Look for the header
  AC_ARG_WITH(xft_incl, AC_HELP_STRING([--with-xft-incl], [directory where libxft header is]), with_xft_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_xft_incl, [
    AC_MSG_CHECKING([for xft.h header in specified directory])
    if test x"${with_xft_incl}" != x ; then
      if test -f ${with_xft_incl}/Xft/Xft.h; then
	ac_cv_path_xft_incl=`(cd ${with_xft_incl}; pwd)`
	AC_MSG_RESULT([yes])
      else
	AC_MSG_ERROR([${with_xft_incl} directory doesn't contain Xft/Xft.h])
      fi
    else
	AC_MSG_RESULT([no])
    fi
    ])

    dnl If the path hasn't been specified, go look for it.
    if test x"${ac_cv_path_xft_incl}" = x; then
      AC_CHECK_HEADERS(Xft/Xft.h, [ac_cv_path_xft_incl=""],[
      if test x"${ac_cv_path_xft_incl}" = x; then
        AC_MSG_CHECKING([for libxft header])
        incllist="${prefix}/include /sw/include /usr/local/include /home/latest/include /opt/include /usr/include /usr/pkg/include .. ../.."

	ac_cv_path_xft_incl=""
        for i in $incllist; do
	  if test -f $i/X11/Xft/Xft.h; then
            ac_cv_path_xft_incl="$i/X11"
	    break
	  fi
        done
      fi])
    else
      if test x"${ac_cv_path_xft_incl}" != x"/usr/include"; then
	ac_cv_path_xft_incl="${ac_cv_path_xft_incl}"
      fi
    fi

    if test x"${ac_cv_path_xft_incl}" != x ; then
      XFT_CFLAGS="-I${ac_cv_path_xft_incl}"
    else
      XFT_CFLAGS=""
    fi

      dnl Look for the library
      AC_ARG_WITH(xft_lib, AC_HELP_STRING([--with-xft-lib], [directory where xft library is]), with_xft_lib=${withval})
      AC_CACHE_VAL(ac_cv_path_xft_lib,[
      if test x"${with_xft_lib}" != x ; then
        AC_MSG_CHECKING([for libxft library in specified directory])
        if test -f ${with_xft_lib}/libXft.a -o -f ${with_xft_lib}/libXft.so; then
	  tmp=`(cd ${with_xft_lib}; pwd)`
	  ac_cv_path_xft_lib="-L${tmp} -lXft"
	  AC_MSG_RESULT([yes])
        else
	  AC_MSG_ERROR([${with_xft_lib} directory doesn't contain libxft.])
	  AC_MSG_RESULT([no])
        fi
      fi
      ])

      dnl If the header doesn't exist, there is no point looking for the library.
      if test x"${ac_cv_path_xft_lib}" = x; then
        AC_CHECK_LIB(Xft, XftGlyphRender, [ac_cv_path_xft_lib="-lXft"],[
          AC_MSG_CHECKING([for libxft library])
          libslist="${prefix}/lib64 ${prefix}/lib /usr/lib64 /usr/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib /usr/pkg/lib /usr/X11R6/lib .. ../.."
          ac_cv_path_xft_lib=""
          for i in $libslist; do
	    if test -f $i/libXft.a -o -f $i/libXft.so; then
	      if test x"$i" != x"/usr/lib"; then
	        ac_cv_path_xft_lib="-L$i -lXft"
                AC_MSG_RESULT(${ac_cv_path_xft_lib})
	        break
              else
	        ac_cv_path_xft_lib="-lXft"
                AC_MSG_RESULT(yes)
	        break
	      fi
	    fi
          done])
      fi
    fi

  if test x"${ac_cv_path_xft_lib}" != x ; then
      XFT_LIBS="${ac_cv_path_xft_lib}"
      has_xft=yes
  else
      has_xft=no
      XFT_LIBS=""
  fi

  AC_SUBST(XFT_CFLAGS)
  AC_SUBST(XFT_LIBS)
])
