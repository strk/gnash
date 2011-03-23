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


dnl agg_rasterizer_compound_aa.h is a new file included in AGG 2.4,
dnl but not in AGG 2.3. As we need AGG 2.4, we use this as 
AC_DEFUN([GNASH_PATH_AGG],
[
  dnl Lool for the header
  AC_ARG_WITH(agg_incl, AC_HELP_STRING([--with-agg-incl], [directory where AGG headers are]), with_agg_incl=${withval})
  AC_MSG_CHECKING([for AGG headers])
  AC_CACHE_VAL(ac_cv_path_agg_incl, [
    if test x"${with_agg_incl}" != x ; then
      if test -f ${with_agg_incl}/agg_rasterizer_compound_aa.h ; then
        agg_include_dir="`(cd ${with_agg_incl}; pwd)`"
        ac_cv_path_agg_incl="-I${agg_include_dir}"
        agg25=yes
      else
        AC_MSG_ERROR([${with_agg_incl} directory doesn't contain any headers])
        agg25=no
      fi
    fi
  ])

  if test x$cross_compiling = xno; then
    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_agg_incl}" = x; then
      if $PKG_CONFIG --exists libagg ; then
        ac_cv_path_agg_incl="`$PKG_CONFIG --cflags libagg`"
        $PKG_CONFIG --atleast-version 2.5.0 libagg && agg25=yes
 
        dnl I think this setting of agg_include_dir is too error prone!
        agg_include_dir="`$PKG_CONFIG --cflags-only-I libagg | cut -d ' ' -f 1 | sed -e 's/-I//g'`"
        if test -f $agg_include_dir/agg_gradient_lut.h ; then
          agg25=yes
        fi
      fi
    fi
  fi

  if test x"${ac_cv_path_agg_incl}" = x; then
    for i in $incllist; do
      if test -f $i/agg2/agg_gradient_lut.h; then
        ac_cv_path_agg_incl="-I$i/agg2"
        agg_include_dir="$i/agg2"
	      agg25=yes
        break
      fi
      dnl Haiku uses only agg as the directory, not agg2
      if test -f $i/agg/agg_gradient_lut.h; then
        ac_cv_path_agg_incl="-I$i/agg"
        agg_include_dir="$i/agg"
	      agg25=yes
        break
      fi
    done
  fi

  AC_MSG_RESULT(${ac_cv_path_agg_incl})

  if test x"${ac_cv_path_agg_incl}" != x ; then
    AGG_CFLAGS="${ac_cv_path_agg_incl}"
  fi

  AC_SUBST(AGG_CFLAGS)


  dnl Look for the library
  AC_ARG_WITH(agg_lib, AC_HELP_STRING([--with-agg-lib], [directory where AGG libraries are]), with_agg_lib=${withval})
  AC_CACHE_VAL(ac_cv_path_agg_lib,[
    if test x"${with_agg_lib}" != x ; then
      if test -f ${with_agg_lib}/libagg.a -o -f ${with_agg_lib}/libagg.${shlibext}; then
      	ac_cv_path_agg_lib="-L`(cd ${with_agg_lib}; pwd)`"
      else
      	AC_MSG_ERROR([${with_agg_lib} directory doesn't contain AGG libraries.])
      fi
    fi
  ])

  pkg=no
  if test x$cross_compiling = xno; then
    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_agg_lib}" = x; then
      $PKG_CONFIG --exists libagg && ac_cv_path_agg_lib="`$PKG_CONFIG --libs libagg`"
      $PKG_CONFIG --exists libagg && pkg=yes
    fi
  fi

  AC_LANG_PUSH(C++)
  if test x"${ac_cv_path_agg_lib}" = x; then
    for i in $libslist; do
      if test -f $i/libagg.a -o -f $i/libagg.${shlibext}; then
      	if test ! x"$i" = x"/usr/lib" -a ! x"$i" = x"/usr/lib64"; then
      	  ac_cv_path_agg_lib="-L$i"
      	  break
        else
      	  ac_cv_path_agg_lib=""
      	  break
        fi
      fi
    done
  fi
  
  if test x"${ac_cv_path_agg_lib}" = x; then
    AC_CHECK_LIB(agg, agg::gamma_ctrl_impl::calc_points, [ac_cv_path_agg_lib=""])
  fi
  AC_MSG_CHECKING([for libagg library])
  AC_MSG_RESULT(${ac_cv_path_agg_lib})
  AC_LANG_POP(C++)

  if test x"${ac_cv_path_agg_lib}" != x -a x"$pkg" = x"yes"; then
    AGG_LIBS="${ac_cv_path_agg_lib}"
  else
    if test x"$agg25" = x"yes"; then
      AGG_LIBS="${ac_cv_path_agg_lib} -lagg"
    else
      AGG_LIBS=""
    fi     
  fi

dnl   AC_EGREP_HEADER(render_scanlines_compound_layered, 
dnl 	${agg_include_dir}/agg_renderer_scanline.h,
dnl 	[ agg_need_compatibility_layer="no" ],
dnl 	[ agg_need_compatibility_layer="yes" ] )

dnl   AC_SUBST(agg_need_compatibility_layer)

dnl   if test x"${agg_need_compatibility_layer}" = xyes; then
dnl 	AC_DEFINE(HAVE_AGG_SCANLINES_COMPOUND_LAYERED, [0], [AGG headers include the render_scanlines_compound_layered templated function])
dnl   else
dnl 	AC_DEFINE(HAVE_AGG_SCANLINES_COMPOUND_LAYERED, [1], [AGG headers include the render_scanlines_compound_layered templated function])
dnl   fi

  AC_SUBST(AGG_LIBS)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
