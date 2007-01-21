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

dnl $Id: curl.m4,v 1.13 2007/01/21 22:41:02 rsavoye Exp $

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

  curlconfig=""
  AC_CHECK_PROG(curlconfig, [echo], [curl-config], , ${pathlist})
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
    if test x"${curlconfig}" != "x"  -a x$cross_compiling = xno; then
      ac_cv_path_curl_incl=`${curlconfig} --cflags`
    else
      for i in $incllist; do
        if test -f $i/curl/curl.h; then
          ac_cv_path_curl_incl="-I$i"
	        break
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

    if test x"${curlconfig}" != "x"  -a x$cross_compiling = xno; then # {
      ac_cv_path_curl_lib=`${curlconfig} --libs`
    else # }{
      AC_MSG_CHECKING([for libcurl library])
      for i in $libslist; do # {
        if test -f $i/libcurl.a -o -f $i/libcurl.so; then # {
          if test x"$i" != x"/usr/lib"; then # {
            ac_cv_path_curl_lib="-L$i -lcurl"
            AC_MSG_RESULT(${ac_cv_path_curl_lib})
            break
          else # }{
            ac_cv_path_curl_lib="-lcurl"
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

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
