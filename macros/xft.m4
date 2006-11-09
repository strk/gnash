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

dnl $Id: xft.m4,v 1.5 2006/11/09 18:28:15 nihilus Exp $

AC_DEFUN([GNASH_PATH_XFT],
[
if test x"$fltk" = x"yes"; then

dnl Look for the header
AC_ARG_WITH(xft_incl, AC_HELP_STRING([--with-xft-incl], [directory where libxft header is]), with_xft_incl=${withval})
AC_CACHE_VAL(ac_cv_path_xft_incl, [
AC_MSG_CHECKING([for xft.h header in specified directory])
	if test x"${with_xft_incl}" != x ; then
		if test -f ${with_xft_incl}/Xft/Xft.h; then
			ac_cv_path_xft_incl="-I`(cd ${with_xft_incl}; pwd)`"
			AC_MSG_RESULT([yes])
		else
			AC_MSG_ERROR([${with_xft_incl} directory doesn't contain Xft/Xft.h])
		fi
	else
		AC_MSG_RESULT([no])
	fi
])

	if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_xft_incl}" = x; then
		$PKG_CONFIG --exists xft && ac_cv_path_xft_incl=`$PKG_CONFIG --cflags xft`
	fi

	dnl If the path hasn't been specified, go look for it.
	if test x"${ac_cv_path_xft_incl}" = x; then
	AC_CHECK_HEADERS(Xft/Xft.h, [ac_cv_path_xft_incl=""],[
		if test x"${ac_cv_path_xft_incl}" = x; then
			incllist="${prefix}/include /sw/include /usr/local/include /home/latest/include /opt/include /usr/include /usr/pkg/include .. ../.."
			ac_cv_path_xft_incl=""
			for i in $incllist; do
				if test -f $i/X11/Xft/Xft.h; then
					ac_cv_path_xft_incl="-I$i/X11"
					break
				fi
			done
      		fi
	])
	fi

AC_MSG_CHECKING([for libxft header])
AC_MSG_RESULT({ac_cv_path_xft_incl})

if test x"${ac_cv_path_xft_incl}" != x ; then
	XFT_CFLAGS="${ac_cv_path_xft_incl}"
else
	XFT_CFLAGS=""
fi

dnl Look for the library			
AC_ARG_WITH(xft_lib, AC_HELP_STRING([--with-xft-lib], [directory where xft library is]), with_xft_lib=${withval})
AC_CACHE_VAL(ac_cv_path_xft_lib,[
AC_MSG_CHECKING([for libxft library in specified directory])
	if test x"${with_xft_lib}" != x ; then
		if test -f ${with_xft_lib}/libXft.a -o -f ${with_xft_lib}/libXft.so; then
			tmp=`(cd ${with_xft_lib}; pwd)`
			ac_cv_path_xft_lib="-L${tmp} -lXft"
			AC_MSG_RESULT([yes])
		else
		AC_MSG_ERROR([${with_xft_lib} directory doesn't contain libxft.])
		fi
	else
		AC_MSG_RESULT([no])	
	fi
])

	if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_xft_lib}" = x; then
		$PKG_CONFIG --exists xft && ac_cv_path_xft_lib=`$PKG_CONFIG --libs xft`
	fi

	dnl If the header doesn't exist, there is no point looking for the library.
	if test x"${ac_cv_path_xft_lib}" = x; then
	AC_CHECK_LIB(Xft, XftGlyphRender, [ac_cv_path_xft_lib="-lXft"],[
	libslist="${prefix}/lib64 ${prefix}/lib /usr/lib64 /usr/lib32 /usr/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib /usr/pkg/lib /usr/X11R6/lib .. ../.."
	ac_cv_path_xft_lib=""
	for i in $libslist; do
		if test -f $i/libXft.a -o -f $i/libXft.so; then
			if test x"$i" != x"/usr/lib"; then
				ac_cv_path_xft_lib="-L$i -lXft"
				break
			else
				ac_cv_path_xft_lib="-lXft"
				break
			fi
		fi
          done
	  ])
	fi
	
	AC_MSG_CHECKING([for libxft library])
	AC_MSG_RESULT(${ac_cv_path_xft_lib})	  
      
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
