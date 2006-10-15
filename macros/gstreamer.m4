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

dnl $Id: gstreamer.m4,v 1.21 2006/10/15 14:26:05 bjacques Exp $

AC_DEFUN([GNASH_PATH_GSTREAMER],
[
    dnl Look for the header
    AC_ARG_WITH(gst_incl, AC_HELP_STRING([--with-gst-incl], [directory where libgstreamer header is]), with_gstreamer_incl=${withval})
      AC_CACHE_VAL(ac_cv_path_gstreamer_incl,[
      if test x"${with_gstreamer_incl}" != x ; then
        if test -f ${with_gstreamer_incl}/gst/gst.h ; then
  	ac_cv_path_gstreamer_incl=-I`(cd ${with_gstreamer_incl}; pwd)`
        else
  	AC_MSG_ERROR([${with_gstreamer_incl} directory doesn't contain gst/gst.h])
        fi
      fi])

    dnl Try with pkg-config
    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_gstreamer_incl}" = x; then
      $PKG_CONFIG --exists gstreamer-0.10 && ac_cv_path_gstreamer_incl=`$PKG_CONFIG --cflags gstreamer-0.10`
    fi

    dnl If the path hasn't been specified, go look for it.
    if test x"${ac_cv_path_gstreamer_incl}" = x; then

        AC_MSG_CHECKING([for libgstreamer header])
        incllist="/sw/include /usr/local/include /home/latest/include /opt/include /opt/local/include /usr/include /usr/pkg/include .. ../.."

        for i in $incllist; do
	  if test -f $i/gstr/gst.h; then
            ac_cv_path_gstreamer_incl="-I$i"
	    break;
	  else
	    if test -f $i/gstreamer-0.10/gst/gst.h; then
	      ac_cv_path_gstreamer_incl="-I$i/gstreamer-0.10"
	      break
	    fi
	  fi
        done

        if test x"${ac_cv_path_gstreamer_incl}" != x ; then
          AC_MSG_RESULT(yes)
        else
          AC_MSG_RESULT(no)
        fi

    fi

   if test x"${ac_cv_path_gstreamer_incl}" != x ; then
      GSTREAMER_CFLAGS="${ac_cv_path_gstreamer_incl}"
   else
      GSTREAMER_CFLAGS=""
   fi



   dnl Look for the library
   AC_ARG_WITH(gst_lib, AC_HELP_STRING([--with-gst-lib], [directory where gstreamer library is]), with_gstreamer_lib=${withval})
      AC_CACHE_VAL(ac_cv_path_gstreamer_lib,[
      if test x"${with_gstreamer_lib}" != x ; then
        if test -f ${with_gstreamer_lib}/libgstreamer-0.10.so; then
	  ac_cv_path_gstreamer_lib="-L`(cd ${with_gstreamer_lib}; pwd)` -lgstreamer-0.10"
        else
	  AC_MSG_ERROR([${with_gstreamer_lib} directory doesn't contain libgstreamer-0.10.so.])
        fi
      fi
      ])

    dnl Try with pkg-config
    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_gstreamer_lib}" = x; then
	$PKG_CONFIG --exists gstreamer-0.10 && ac_cv_path_gstreamer_lib=`$PKG_CONFIG --libs gstreamer-0.10`
    fi

    dnl If the path hasn't been specified, go look for it.
    if test x"${ac_cv_path_gstreamer_lib}" = x; then

       libslist="${prefix}/lib64 ${prefix}/lib /usr/lib64 /usr/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib /opt/local/lib /usr/pkg/lib .. ../.."
       for i in $libslist; do
   
          if test -f $i/gstreamer-0.10/libgstreamer-0.10.so; then
            if test x"$i" != x"/usr/lib"; then
               ac_cv_path_gstreamer_lib="-L$i -lgstreamer-0.10"
               break
            else
               ac_cv_path_gstreamer_lib="-lgstreamer-0.10"
               break
            fi
          else
              if test -f $i/libgstreamer-0.10.a -o -f $i/gstreamer-0.10.so; then
                 ac_cv_path_gstreamer_lib="-L$i/gstreamer-0.10 -lgstreamer-0.10"
                 break
              fi
          fi
       done

   fi

   if test x"${ac_cv_path_gstreamer_lib}" != x ; then
      GSTREAMER_LIBS="${ac_cv_path_gstreamer_lib}"
   else
      GSTREAMER_LIBS=""
   fi

  dnl
  dnl Call AC_CHECK_HEADERS here to
  dnl get HAVE_* macros automatically defined
  dnl
  dnl NOTE: we need additional CFLAGS for things to work
  dnl       (stuff included by gstreamer header)
  dnl

  ac_save_CFLAGS="$CFLAGS"
  ac_save_CPPFLAGS="$CPPFLAGS"
  CFLAGS="$CFLAGS $GSTREAMER_CFLAGS $GLIB_CFLAGS $LIBXML_CFLAGS"
  CPPFLAGS="$CPPFLAGS $GSTREAMER_CFLAGS $GLIB_CFLAGS $LIBXML_CFLAGS"
  dnl echo "GST: CFLAGS=${CFLAGS} CPPFLAGS=${CPPFLAGS}"
  AC_CHECK_HEADERS(gst/gst.h)
  CFLAGS="$ac_save_CFLAGS"
  CPPFLAGS="$ac_save_CPPFLAGS"

  dnl
  dnl Call AC_CHECK_LIB here to
  dnl make sure gstreamer actually works
  dnl

  ac_save_LIBS="$LIBS"
  LIBS="$LIBS $GSTREAMER_LIBS"
  dnl echo "GSTREAMER LIBS=${LIBS}"
  AC_CHECK_LIB(gstreamer-0.10, gst_init, [gstreamer_lib_ok="yes"], [gstreamer_lib_ok="no"])
  LIBS="$ac_save_LIBS"

  if test x"${gstreamer_lib_ok}" = xno; then
     GSTREAMER_LIBS=""
  fi


  AC_SUBST(GSTREAMER_CFLAGS)
  AC_SUBST(GSTREAMER_LIBS)
])
