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


AC_DEFUN([GNASH_PATH_CURL],
[
  dnl Look for the header
  AC_ARG_WITH(curl_incl, AC_HELP_STRING([--with-curl-incl], [directory where libcurl header is (w/out the curl/ prefix)]), with_curl_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_curl_incl,[
    if test x"${with_curl_incl}" != x ; then
      if test -f ${with_curl_incl}/curl/curl.h ; then
	      ac_cv_path_curl_incl="`(cd ${with_curl_incl}; pwd)`"
      else
	      AC_MSG_ERROR([${with_curl_incl} directory doesn't contain curl/curl.h])
      fi
    fi
  ])

  curlconfig=""
  AC_PATH_PROG(curlconfig, curl-config, ,[${pathlist}])

  dnl If the path hasn't been specified, go look for it.
  if test x"${ac_cv_path_curl_incl}" = x; then
    if test x"${curlconfig}" != "x"; then
      ac_cv_path_curl_incl="`${curlconfig} --cflags`"
    else
      for i in $incllist; do
        if test -f $i/curl/curl.h; then
          ac_cv_path_curl_incl="-I$i"
	        break
        fi
      done
    fi

    if test x"${ac_cv_path_curl_incl}" = x ; then
      AC_CHECK_HEADERS(curl/curl.h)
    fi

    AC_MSG_CHECKING([for libcurl header])
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
      if test -f ${with_curl_lib}/libcurl.a -o -f ${with_curl_lib}/libcurl.${shlibext}; then # {
        ac_cv_path_curl_lib="-L`(cd ${with_curl_lib}; pwd)` -lcurl"
      else # }{
        AC_MSG_ERROR([${with_curl_lib} directory doesn't contain libcurl.])
      fi # }
    fi # }
  ])

  dnl If the path hasn't been specified, go look for it.
  if test x"${ac_cv_path_curl_lib}" = x; then # {
    if test x"${curlconfig}" != "x" -a x"${darwin}" = xno; then # {
      dnl curl-config gives us way to many libraries, which create nasty linking
      dnl dependancy issue, so we strip them off here. The real dependencies are
      dnl are taken care of by other config tests.
      ac_cv_path_curl_lib=`${curlconfig} --libs | sed -e 's/lcurl.*/lcurl/' -e 's:-L/usr/lib ::'`
    else # }{
      AC_MSG_CHECKING([for libcurl library])
      for i in $libslist; do # {
        if test -f $i/libcurl.a -o -f $i/libcurl.${shlibext}; then # {
          if test ! x"$i" = x"/usr/lib" -a ! x"$i" = x"/usr/lib64"; then # {
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
      if test x"${ac_cv_path_curl_lib}" = x; then # {
        AC_MSG_RESULT(no)
      fi # }
    fi # }

  fi # }

  if test x"${ac_cv_path_curl_incl}" != x ; then
    CURL_CFLAGS="${ac_cv_path_curl_incl}"
  else
    CURL_CFLAGS=""
  fi

  if ${ENABLE_STATIC}; then
    CURL_CFLAGS="$CURL_CFLAGS -DCURL_STATICLIB"
  fi

  if test x"${ac_cv_path_curl_lib}" != x ; then
    CURL_LIBS="${ac_cv_path_curl_lib}"
    dnl libcurl3-dev on Ubuntu has a dependency on lber, and Gnash won't link
    dnl on most machines without it. (Ubuntu packaging bug.)
    save_LIBS="$LIBS"
    LIBS="$LIBS $CURL_LIBS"
    AC_TRY_LINK_FUNC(curl_easy_init, [], 
      [AC_CHECK_LIB(lber, ber_free, [CURL_LIBS="$CURL_LIBS -llber"])])
    dnl FIXME: complain if that didn't do the trick?
    LIBS="$save_LIBS"
  else
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
