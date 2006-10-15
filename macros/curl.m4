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

dnl $Id: curl.m4,v 1.8 2006/10/15 14:26:05 bjacques Exp $

AC_DEFUN([GNASH_PATH_CURL],
[
  dnl Look for the header
  AC_ARG_WITH(curl_incl, AC_HELP_STRING([--with-curl-incl], [directory where libcurl header is (w/out the curl/ prefix)]), with_curl_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_curl_incl,[
    if test x"${with_curl_incl}" != x ; then
      if test -f ${with_curl_incl}/curl/curl.h ; then
	ac_cv_path_curl_incl=`(cd ${with_curl_incl}; pwd)`
      else
	AC_MSG_ERROR([${with_curl_incl} directory doesn't contain curl/curl.h])
      fi
    fi
  ])

  AC_CHECK_PROG(curlconfig, [echo], [curl-config])
  if test x"${curlconfig}" != "x" ; then
    AC_MSG_CHECKING([for RTMP support])
    rtmp=`${curlconfig} --protocols|grep -c RTMP`
    if test $rtmp -eq 0; then
	AC_MSG_RESULT([none])
	rtmp=no
    else
	AC_MSG_RESULT([yes])
	rtmp=yes
    fi
  fi

  dnl If the path hasn't been specified, go look for it.
  if test x"${ac_cv_path_curl_incl}" = x; then

    AC_MSG_CHECKING([for libcurl header])
    if test x"${curlconfig}" != "x" ; then

      ac_cv_path_curl_incl=`${curlconfig} --cflags`

    else

          incllist="/sw/include /usr/local/include /opt/local/include /home/latest/include /opt/include /opt/local/include /usr/include /usr/pkg/include .. ../.."

          for i in $incllist; do
            if test -f $i/curl/curl.h; then
              ac_cv_path_curl_incl="-I$i"
            fi
          done

    fi

    if test x"${ac_cv_path_curl_incl}" != x ; then
      AC_MSG_RESULT(yes)
    else
      AC_MSG_RESULT(no)
    fi

  fi


  if test x"${ac_cv_path_curl_incl}" != x"/usr/include"; then
    ac_cv_path_curl_incl="${ac_cv_path_curl_incl}"
  else
    ac_cv_path_curl_incl=""
  fi

  dnl Look for the library
  AC_ARG_WITH(curl_lib, AC_HELP_STRING([--with-curl-lib], [directory where curl library is]), with_curl_lib=${withval})
    AC_CACHE_VAL(ac_cv_path_curl_lib,[
    if test x"${with_curl_lib}" != x ; then # {
      if test -f ${with_curl_lib}/libcurl.a -o -f ${with_curl_lib}/libcurl.so; then # {
 ac_cv_path_curl_lib="-L`(cd ${with_curl_lib}; pwd)`"
      else # }{
 AC_MSG_ERROR([${with_curl_lib} directory doesn't contain libcurl.])
      fi # }
    fi # }
  ])

  dnl If the path hasn't been specified, go look for it.
  if test x"${ac_cv_path_curl_lib}" = x; then # {

    if test x"${curlconfig}" != "x" ; then # {
      ac_cv_path_curl_lib=`${curlconfig} --libs`
    else # }{
      AC_MSG_CHECKING([for libcurl library])

      libslist="/usr/lib64 /usr/lib /sw/lib /opt/local/lib /usr/local/lib /home/latest/lib /opt/lib /opt/local/lib /usr/pkg/lib .. ../.."
      for i in $libslist; do # {
        if test -f $i/libcurl.a -o -f $i/libcurl.so; then # {
          if test x"$i" != x"/usr/lib"; then # {
            ac_cv_path_curl_lib="-L$i"
            AC_MSG_RESULT(${ac_cv_path_curl_lib})
            break
          else # }{
            ac_cv_path_curl_lib=""
            AC_MSG_RESULT(yes)
            break
          fi # }
        fi # }
      done # }
    fi # }

  fi # }

  if test x"${ac_cv_path_curl_incl}" != x ; then
    CURL_CFLAGS="${ac_cv_path_curl_incl}"
  else
    CURL_CFLAGS=""
  fi

  if test x"${ac_cv_path_curl_lib}" != x ; then
    CURL_LIBS="${ac_cv_path_curl_lib}"
  else
    CURL_LIBS=""
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
  CFLAGS="$CFLAGS $CURL_CFLAGS"
  CPPFLAGS="$CPPFLAGS $CURL_CFLAGS"
  AC_CHECK_HEADERS(curl/curl.h)
  CFLAGS="$ac_save_CFLAGS"
  CPPFLAGS="$ac_save_CPPFLAGS"

  dnl
  dnl Call AC_CHECK_LIB here to
  dnl make sure curl actually works
  dnl

  ac_save_LIBS="$LIBS"
  LIBS="$LIBS $CURL_LIBS"
  AC_CHECK_LIB(curl, curl_global_init, [curl_lib_ok="yes"],[curl_lib_ok="no"])
  LIBS="$ac_save_LIBS"

  if test x"${curl_lib_ok}" = xno; then
     CURL_LIBS=""
  fi

  AM_CONDITIONAL(CURL, [test -n "$CURL_LIBS"])

  if test -n "$CURL_LIBS"; then
    AC_DEFINE(USE_CURL, [1], [Define this if you want to enable curl usage])
  fi

  AC_SUBST(CURL_CFLAGS)
  AC_SUBST(CURL_LIBS)
])
