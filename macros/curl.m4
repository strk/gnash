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

AC_DEFUN([GNASH_PATH_CURL],
[
  dnl Look for the header
  AC_ARG_WITH(curl_incl, [  --with-curl_incl         directory where libcurl header is (w/out the curl/ prefix)], with_curl_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_curl_incl,[
    if test x"${with_curl_incl}" != x ; then
      if test -f ${with_curl_incl}/curl/curl.h ; then
	ac_cv_path_curl_incl=`(cd ${with_curl_incl}; pwd)`
      else
	AC_MSG_ERROR([${with_curl_incl} directory doesn't contain curl/curl.h])
      fi
    fi
  ])

  dnl If the path hasn't been specified, go look for it.
  if test x"${ac_cv_path_curl_incl}" = x; then
    AC_CHECK_HEADERS(curl/curl.h, [ac_cv_path_curl_incl=""],[
      if test x"${ac_cv_path_curl_incl}" = x; then
        AC_MSG_CHECKING([for libcurl header])
        incllist="/sw/include /usr/local/include /opt/local/include /home/latest/include /opt/include /usr/include /usr/pkg/include .. ../.."

        for i in $incllist; do
   if test -f $i/curl/curl.h; then
     ac_cv_path_curl_incl="$i"
   fi
        done
      fi
    ])
  fi

  if test x"${ac_cv_path_curl_incl}" != x ; then
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
  fi

  if test x"${ac_cv_path_curl_incl}" != x"/usr/include"; then
    ac_cv_path_curl_incl="${ac_cv_path_curl_incl}"
  else
    ac_cv_path_curl_incl=""
  fi

  dnl Look for the library
  AC_ARG_WITH(curl_lib, [  --with-curl-lib          directory where curl library is], with_curl_lib=${withval})
    AC_CACHE_VAL(ac_cv_path_curl_lib,[
    if test x"${with_curl_lib}" != x ; then
      if test -f ${with_curl_lib}/libcurl.a -o -f ${with_curl_lib}/libcurl.so; then
 ac_cv_path_curl_lib=`(cd ${with_curl_incl}; pwd)`
      else
 AC_MSG_ERROR([${with_curl_lib} directory doesn't contain libcurl.])
      fi
    fi
  ])

  dnl If the header doesn't exist, there is no point looking for the library.
  if test x"${ac_cv_path_curl_lib}" = x; then 
    AC_CHECK_LIB(curl, curl_global_init, [ac_cv_path_curl_lib="-lcurl"],[
      AC_MSG_CHECKING([for libcurl library])
      libslist="/usr/lib64 /usr/lib /sw/lib /opt/local/lib /usr/local/lib /home/latest/lib /opt/lib /usr/pkg/lib .. ../.."
      for i in $libslist; do
 if test -f $i/libcurl.a -o -f $i/libcurl.so; then
   if test x"$i" != x"/usr/lib"; then
     ac_cv_path_curl_lib="-L$i"
            AC_MSG_RESULT(${ac_cv_path_curl_lib})
     break
          else
            ac_cv_path_curl_lib=""
            AC_MSG_RESULT(yes)
     break
   fi
 fi
      done
    ])
  else 
    if test -f ${ac_cv_path_curl_lib}/libcurl.a -o -f ${ac_cv_path_curl_lib}/libcurl.so; then 

      if test x"${ac_cv_path_curl_lib}" != x"/usr/lib"; then
 ac_cv_path_curl_lib="-L${ac_cv_path_curl_lib}"
       else
 ac_cv_path_curl_lib=""
      fi
    fi 
  fi 

  if test x"${ac_cv_path_curl_incl}" != x ; then
    CURL_CFLAGS="-I${ac_cv_path_curl_incl}"
  else
    CURL_CFLAGS=""
  fi

  if test x"${ac_cv_path_curl_lib}" != x ; then
    CURL_LIBS="${ac_cv_path_curl_lib}"
  else
    CURL_LIBS=""
  fi

  AM_CONDITIONAL(CURL, [test -n "$CURL_LIBS"])

  AC_SUBST(CURL_CFLAGS)
  AC_SUBST(CURL_LIBS)
])
