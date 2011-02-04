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

AC_DEFUN([GNASH_PATH_ALP],
[

  dnl See if we're running under scratchbox, specifically ALP
  BARTENDER=
  ALP_CFLAGS=
  ALP_LIBS=
  have_alp=no

  if test x"$sbox" = xyes; then
    dnl Look for Alp
    for i in $incllist; do
      if test -f $i/alp/bundlemgr.h; then
        ac_cv_path_alp_incl="-I$i"
        alp=yes
        break
      fi
    done

    ac_cv_path_alp_lib=""
    newlist="/opt/alp/lib /opt/alp/lib/tmp ${incllist}"
    for i in $newlist; do
      if test -f $i/libalp_bundlemgr.a -o -f $i/libalp_bundlemgr.${shlibext}; then
   	    ac_cv_path_alp_lib="-L$i -lalp_bundlemgr ${ac_cv_path_alp_lib}"
        break
      fi
    done
    for i in $newlist; do
      if test -f $i/libalp_appmgr.a -o -f $i/libalp_appmgr.${shlibext}; then
   	    ac_cv_path_alp_lib="${ac_cv_path_alp_lib} -lalp_appmgr"
        break
      fi
    done
    for i in $newlist; do
      if test -f $i/alp_max.a -o -f $i/alp_max.${shlibext}; then
   	    ac_cv_path_alp_lib="${ac_cv_path_alp_lib} -lalp_max"
        break
      fi
    done
  fi

  if test -f ${_SBOX_DIR}/tools/bin/bartender; then
    BARTENDER=${_SBOX_DIR}/tools/bin/bartender
  fi

  if test x"${ac_cv_path_alp_incl}" != x; then
    ALP_CFLAGS="${ac_cv_path_alp_incl}"
  fi

  if test x"${ac_cv_path_alp_lib}" != x; then
    ALP_LIBS="${ac_cv_path_alp_lib}"
    AC_DEFINE(HAVE_ALP, [1], [has the ALP/Hiker mobile framework])
    have_alp=yes
  fi

  AC_SUBST(BARTENDER)
  AC_SUBST(ALP_CFLAGS)
  AC_SUBST(ALP_LIBS)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
