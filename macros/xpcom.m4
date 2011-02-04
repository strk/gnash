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
AC_DEFUN([GNASH_PATH_XPCOM],
[dnl 

  has_xpcom=no
  AC_ARG_ENABLE(xpcom,
    AC_HELP_STRING([--enable-xpcom],[Enable xpcom support in NPAPI plugin]),
  [case "${enableval}" in
    yes) xpcom=yes ;;
    no)  xpcom=no ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for --enable-xpcom option]) ;;
   esac],xpcom=no)

  dnl Look for the header
  AC_ARG_WITH(xpcom-incl, AC_HELP_STRING([--with-xpcom-incl], [directory where XPCOM headers are]), with_xpcom_incl=${withval})
  AC_CACHE_VAL(ac_cv_path_xpcom_incl, [
    if test x"${with_xpcom_incl}" != x ; then
      if test -f ${with_xpcom_incl}/mozilla-config.h ; then
        ac_cv_path_xpcom_incl="-I`(cd ${with_xpcom_incl}; pwd)`"
      else
        AC_MSG_ERROR([${with_xpcom_incl} directory doesn't contain mozilla-config.h!])
      fi
    fi
  ])

    dnl Look for the library
  AC_ARG_WITH(xpcom-sdk-dir, AC_HELP_STRING([--with-xpcom-sdk-dir], [directory where XPCOM SDK is]), with_xpcom_sdk_dir=${withval})
    AC_CACHE_VAL(ac_cv_path_xpcom_sdk_dir, [
      if test x"${with_xpcom_sdk_dir}" != x ; then
        if test -f ${with_xpcom_sdk_dir}/libxpcomglue_s.a; then
          ac_cv_path_xpcom_sdk_dir="-L`(cd ${with_xpcom_sdk_dir}; pwd)` -lxpcomglue_s -lnspr4 -lplds4"
        else
          AC_MSG_ERROR([${with_xpcom_sdk_dir} directory doesn't contain libxpcomglue_s.a!])
        fi
      fi
    ])

  if test x$xpcom = xyes; then
    if test x$cross_compiling = xno; then

      # Look for libxpcomglue_s.a (version 1.8 !!) if not explicitly given
      if test x$ac_cv_path_xpcom_sdk_dir = x; then
        for i in /usr/lib/iceape; do
          # TODO: check version ! we do NOT want to use 1.9 here !!!
          # TODO: check availability of nspr4 and plds4 (may be in /usr/lib too)
          if test -e "${i}/libxpcomglue_s.a"; then
            ac_cv_path_xpcom_sdk_dir="-L${i} -lxpcomglue_s -lnspr4 -lplds4"
            break
          fi
        done
      fi

      # Look for xpcom headers (version 1.8 !!) if not explicitly given
      if test x$ac_cv_path_xpcom_incl = x; then
        # Prefer libxul-unstable for cflags.
        if $PKG_CONFIG --exists libxul-unstable; then
              ac_cv_path_xpcom_incl="`$PKG_CONFIG --cflags-only-I libxul-unstable`"
        else
              if $PKG_CONFIG --exists libxul; then
                ac_cv_path_xpcom_incl="`$PKG_CONFIG --cflags-only-I libxul`"
              fi
        fi
      fi

      # hack, hack, broken .pc files in F9's xulrunner-devel
      # they fail to include the include subdirectories
      if test x"${ac_cv_path_xpcom_incl}" = x"-I/usr/include/xulrunner-sdk-1.9/stable -I/usr/include/nspr4  "; then
        ac_cv_path_xpcom_incl="-I/usr/include/xulrunner-sdk-1.9/stable -I/usr/include/xulrunner-sdk-1.9/extensions -I/usr/include/xulrunner-sdk-1.9/necko -I/usr/include/nspr4"
      fi

      # Look for libxul. TODO: needed at all ?
      if $PKG_CONFIG --exists libxul; then
        if test x$ac_cv_path_xpcom_sdk_dir = x; then
          ac_cv_path_xpcom_sdk_dir="`$PKG_CONFIG --libs libxul`"
        fi
        ac_cv_path_xpidl="`$PKG_CONFIG --libs-only-L libxul`"
      fi

    fi
  fi

  if test x"${ac_cv_path_xpcom_incl}" != x; then
    XPCOM_CFLAGS="${ac_cv_path_xpcom_incl}"
    XPCOM_IDL_CFLAGS=`echo $XPCOM_CFLAGS | sed -e 's:include:share/idl:'`
    XPIDL=`echo ${ac_cv_path_xpidl} | sed -e 's:-L::' -e 's:sdk-::' -e 's:sdk/lib::' -e 's:-devel::' -e 's:lib$::'`
    XPIDL="${XPIDL}xpidl"
    AC_DEFINE([HAVE_XPCOM], [1], [Use XPCOM in the NPAPI plugin])
  else
    XPCOM_CFLAGS=""
    XPIDL=""
  fi

  if test x"${ac_cv_path_xpcom_sdk_dir}" != x ; then
    XPCOM_LIBS="${ac_cv_path_xpcom_sdk_dir}"
    has_xpcom=yes
  else
    XPCOM_LIBS=""
    has_xpcom=no
  fi

  AC_SUBST(XPCOM_CFLAGS)
  AC_SUBST(XPCOM_IDL_CFLAGS)
  AC_SUBST(XPCOM_LIBS)
  AC_SUBST(XPIDL)
])
dnl end of GNASH_PATH_XPCOM
# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
