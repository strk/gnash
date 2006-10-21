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

dnl $Id: mad.m4,v 1.15 2006/10/21 10:34:15 nihilus Exp $

AC_DEFUN([GNASH_PATH_MAD],
[
  dnl Look for the header
  AC_ARG_WITH(mad_incl, AC_HELP_STRING([--with-mad_incl], [directory where libmad header is]), with_mad_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_mad_incl,[
    if test x"${with_mad_incl}" != x ; then
      if test -f ${with_mad_incl}/mad.h ; then
	ac_cv_path_mad_incl=`(cd ${with_mad_incl}; pwd)`
      else
	AC_MSG_ERROR([${with_mad_incl} directory doesn't contain mad.h])
      fi
    fi
  ])


    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_mad_incl}" = x; then
      $PKG_CONFIG --exists mad && ac_cv_path_mad_incl=`$PKG_CONFIG --cflags mad`
    fi

  dnl If the path hasn't been specified, go look for it.
  if test x"${ac_cv_path_mad_incl}" = x; then
    AC_CHECK_HEADERS(mad.h, [ac_cv_path_mad_incl=""],[
    if test x"${ac_cv_path_mad_incl}" = x; then
      incllist="${prefix}/include /sw/include /opt/local/include /usr/local/include /home/latest/include /opt/include /opt/local/include /usr/include /usr/pkg/include .. ../.."

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
    if test x"${ac_cv_path_mad_incl}" != x"/usr/include"; then
      ac_cv_path_mad_incl="-I${ac_cv_path_mad_incl}"
    else
      ac_cv_path_mad_incl=""
    fi
  fi
  AC_MSG_CHECKING([for libmad header])
  AC_MSG_RESULT(${ac_cv_path_mad_incl})

  if test x"${ac_cv_path_mad_incl}" != x ; then
    MAD_CFLAGS="${ac_cv_path_mad_incl}"
  else
    MAD_CFLAGS=""
  fi

  dnl Look for the library
  AC_ARG_WITH(mad_lib, AC_HELP_STRING([--with-mad-lib], [directory where mad library is]), with_mad_lib=${withval})
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
      if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_mad_lib}" = x; then
        $PKG_CONFIG --exists mad && ac_cv_path_mad_lib=`$PKG_CONFIG --libs mad`
      fi

   if test x"${ac_cv_path_mad_lib}" = x; then
     AC_CHECK_LIB(mad, mad_copyright, [ac_cv_path_mad_lib="-lmad"],[
       libslist="${prefix}/lib64 ${prefix}/lib /usr/lib64 /opt/local/lib /usr/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib /opt/local/lib /usr/pkg/lib .. ../.."
       for i in $libslist; do
	 if test -f $i/libmad.a -o -f $i/libmad.so; then
	   if test x"$i" != x"/usr/lib"; then
	     ac_cv_path_mad_lib="-L$i"             
	     break
           else
	     ac_cv_path_mad_lib=""
	     break
	   fi
	 fi
       done
     ])
   else
    if test -f ${ac_cv_path_mad_lib}/libmad.a -o -f ${ac_cv_path_mad_lib}/libmad.so; then
      if test x"${ac_cv_path_mad_lib}" != x"/usr/lib"; then
	ac_cv_path_mad_lib="-L${ac_cv_path_mad_lib}"
       else
	ac_cv_path_mad_lib=""
      fi
    fi
  fi
  AC_MSG_CHECKING([for libmad library])
  AC_MSG_RESULT(${ac_cv_path_mad_lib})
  
  if test x"${ac_cv_path_mad_lib}" != x ; then
      MAD_LIBS="${ac_cv_path_mad_lib}"
  fi

  AC_SUBST(MAD_CFLAGS)
  AC_SUBST(MAD_LIBS)
])
