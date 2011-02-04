dnl  
dnl  Copyright (C) 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

dnl Configure paths for Firefox. We used to run firefox-config, but that
dnl got too messy. Then with a little experimentation we determined
dnl most of the options weren't actually needed... so now the handful
dnl of critical headers are part of the plugin/mozilla-sdk sources
dnl copied out of a current Firefox release. This greatly simplified
dnl both the configuration and compilation processes.
AC_DEFUN([GNASH_PATH_NSPR],
[dnl 

  has_nspr=no

  dnl Look for the header
  AC_ARG_WITH(nspr-incl, AC_HELP_STRING([--with-nspr-incl], [directory where NSPR headers are]), with_nspr_incl=${withval})
  AC_CACHE_VAL(ac_cv_path_nspr_incl, [
    if test x"${with_nspr_incl}" != x ; then
      if test -f ${with_nspr_incl}/nspr.h ; then
        ac_cv_path_nspr_incl="-I`(cd ${with_nspr_incl}; pwd)`"
      else
        AC_MSG_ERROR([${with_nspr_incl} directory doesn't contain mozilla-config.h!])
      fi
    fi
  ])

  dnl Look for the library
  AC_ARG_WITH(nspr_lib, AC_HELP_STRING([--with-nspr-lib], [directory where NSPR libraries are]), with_nspr_lib=${withval})
  AC_CACHE_VAL(ac_cv_path_nspr_lib, [
    if test x"${with_nspr_lib}" != x ; then
      if test -f ${with_nspr_libs}/libnspr.so; then
        ac_cv_path_nspr_lib="-L`(cd ${with_nspr_lib}; pwd)` -lplds4 -lplc4 -lnspr4"
      fi
    fi
  ])

  #always check to see if we have it; we only use the results if XPCOM is set.
  #if test x$nspr4 = xyes; then
    if test x$cross_compiling = xno; then
      if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_nspr_incl}" = x; then
        $PKG_CONFIG --exists nspr && ac_cv_path_nspr_incl="`$PKG_CONFIG --cflags-only-I nspr`"
        $PKG_CONFIG --exists nspr && ac_cv_path_nspr_lib="`$PKG_CONFIG --libs nspr`"
      fi
    fi
  #fi

  if test x"${ac_cv_path_nspr_incl}" != x; then
    NSPR_CFLAGS="${ac_cv_path_nspr_incl}"
  else
    NSPR_CFLAGS=""
  fi

  if test x"${ac_cv_path_nspr_lib}" != x ; then
    NSPR_LIBS="${ac_cv_path_nspr_lib}"
    has_nspr=yes
  else
    NSPR_LIBS=""
    has_nspr=no
  fi

  AC_SUBST(NSPR_CFLAGS)
  AC_SUBST(NSPR_LIBS)
])
dnl end of GNASH_PATH_NSPR
# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
