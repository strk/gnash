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

dnl agg_rasterizer_compound_aa.h is a new file included in AGG 2.4,
dnl but not in AGG 2.3. As we need AGG 2.4, we use this as 
AC_DEFUN([GNASH_PATH_AGG],
[
  dnl Lool for the header
  AC_ARG_WITH(agg_incl, [  --with-agg-incl        directory where AGG headers are], with_agg_incl=${withval})
  AC_CACHE_VAL(ac_cv_path_agg_incl,[
  if test x"${with_agg_incl}" != x ; then
    if test -f ${with_agg_incl}/agg_rasterizer_compound_aa.h ; then
      ac_cv_path_agg_incl="-I`(cd ${with_agg_incl}; pwd)`"
      agg24=yes
    else
      AC_MSG_ERROR([${with_agg_incl} directory doesn't contain any headers])
      agg24=no
    fi
  fi
  ])

  if test x"${ac_cv_path_agg_incl}" = x ; then
    AC_MSG_CHECKING([for AGG headers])
    incllist="${prefix}/include /sw/include /usr/local/include /home/latest/include /opt/include /usr/include .. ../.."

    for i in $incllist; do
      if test -f $i/agg2/agg_rasterizer_compound_aa.h; then
        ac_cv_path_agg_incl="-I$i/agg2"
	agg24=yes
        break
      fi
    done
    if test x"${ac_cv_path_agg_incl}" = x ; then
      for i in $incllist; do
        if test -f $i/agg2/agg_math.h; then
	  AC_MSG_WARN([You appear to have AGG 2.3 installed, version 2.4 or greater is required])
	  agg24=no
          break
        fi
      done
    fi
  fi

  if test x"${ac_cv_path_agg_incl}" != x ; then
    AGG_CFLAGS="${ac_cv_path_agg_incl}"
  fi
  AC_SUBST(AGG_CFLAGS)


  dnl Look for the library
  AC_ARG_WITH(agg_lib, [  --with-agg-lib         directory where AGG libraries are], with_agg_lib=${withval})
    AC_CACHE_VAL(ac_cv_path_agg_lib,[
    if test x"${with_agg_lib}" != x ; then
      if test -f ${with_agg_lib}/libagg.a -o -f ${with_agg_lib}/libagg.so; then
	ac_cv_path_agg_lib="-L`(cd ${with_agg_lib}; pwd)`"
      else
	AC_MSG_ERROR([${with_agg_lib} directory doesn't contain AGG libraries.])
      fi
    fi
  ])

  if test x"${ac_cv_path_agg_lib}" = x; then
    AC_CHECK_LIB(agg, agg::gamma_ctrl_impl::calc_points, [ac_cv_path_agg_lib=""],[
      AC_MSG_CHECKING([for libagg library])
      libslist="${prefix}/lib64 ${prefix}/lib /usr/lib64 /usr/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib /usr/pkg/lib .. ../.."
      for i in $libslist; do
	if test -f $i/libagg.a -o -f $i/libagg.so; then
	  if test x"$i" != x"/usr/lib"; then
	    ac_cv_path_agg_lib="-L$i"
            AC_MSG_RESULT(${ac_cv_path_agg_lib})
	    break
          else
	    ac_cv_path_agg_lib=""
            AC_MSG_RESULT(yes)
	    break
          fi
        fi
      done])
  else
    if test -f ${ac_cv_path_agg_lib}/libagg.a -o -f ${ac_cv_path_agg_lib}/libagg.so; then

      if test x"${ac_cv_path_agg_lib}" != x"/usr/lib"; then
	ac_cv_path_agg_lib="-L${ac_cv_path_agg_lib}"
      else
        ac_cv_path_agg_lib=""
      fi
    fi
  fi

  if test x"${ac_cv_path_agg_lib}" != x ; then
      AGG_LIBS="${ac_cv_path_agg_lib} -lagg -laggplatformX11"
  else
      AGG_LIBS="-lagg -laggplatformX11"
  fi

  AC_SUBST(AGG_LIBS)

])
