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


AC_DEFUN([GNASH_PATH_PANGO],
[

  dnl Look for the header
  AC_ARG_WITH(pango_incl, AC_HELP_STRING([--with-pango-incl], [directory where libpango header is]), with_pango_incl=${withval})
  AC_CACHE_VAL(ac_cv_path_pango_incl,[
    if test x"${with_pango_incl}" != x ; then
      if test -f ${with_pango_incl}/pango/pango.h ; then
        ac_cv_path_pango_incl="-I`(cd ${with_pango_incl}; pwd)`"
      else
        AC_MSG_ERROR([${with_pango_incl} directory doesn't contain pango/pango.h])
      fi
    fi
  ])

  if test x$windows = xyes; then
    pango_pkg=pangowin32
  else
    pango_pkg=pangox
  fi

  if test x$cross_compiling = xno; then
    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_pango_incl}" = x; then
      $PKG_CONFIG --exists $pango_pkg && ac_cv_path_pango_incl="`$PKG_CONFIG --cflags $pango_pkg`"
    fi
  fi

  dnl Attempt to find the top level directory, which unfortunately has a
  dnl version number attached. At least on Debain based systems, this
  dnl doesn't seem to get a directory that is unversioned.

  AC_MSG_CHECKING([for the Pango Version])

  if test x$cross_compiling = xno; then
    if test x"$PKG_CONFIG" != x; then
      $PKG_CONFIG --exists $pango_pkg && gnash_pango_version=`$PKG_CONFIG --modversion $pango_pkg | cut -d "." -f 1 | awk '{print $'0'".0"}'`
    fi
  fi

  if test x"${gnash_pango_version}" = x; then
    gnash_pango_topdir=""
    gnash_pango_version=""
    for i in $incllist; do
      for j in `ls -dr $i/pango-[[0-9]].[[0-9]] 2>/dev/null`; do
        if test -f $j/pango/pango.h; then
	  gnash_pango_topdir=`basename $j`
	  gnash_pango_version=`echo ${gnash_pango_topdir} | sed -e 's:pango-::'`
 	  ac_cv_path_pango_incl="-I$i/${gnash_pango_topdir}"
	  break
	fi
      done
	if test x$gnash_pango_version != x; then
	  break;
	fi
    done
  fi      

  if test x"${gnash_pango_version}" = x; then
    AC_MSG_RESULT(none)
  else
    AC_MSG_RESULT([${gnash_pango_version}])
  fi
  
  AC_MSG_CHECKING([for libpango header])
  AC_MSG_RESULT(${ac_cv_path_pango_incl}) 

  dnl Look for the library
  AC_ARG_WITH(pango_lib, AC_HELP_STRING([--with-pango-lib], [directory where pango library is]), with_pango_lib=${withval})
  AC_CACHE_VAL(ac_cv_path_pango_lib,[
    if test x"${with_pango_lib}" != x ; then
      if test -f ${with_pango_lib}/libpangox-${gnash_pango_version}.a -o -f ${with_pango_lib}/libpangox-${gnash_pango_version}.${shlibext} -o -f ${with_pango_lib}/libpangox-${gnash_pango_version}.${shlibext}.a; then
	ac_cv_path_pango_lib=-L`(cd ${with_pango_lib}; pwd)`
      else
	AC_MSG_ERROR([${with_pango_lib} directory doesn't contain libpango.])
      fi
    fi
  ])
  
  if test x$cross_compiling = xno; then
    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_pango_lib}" = x; then
      $PKG_CONFIG --exists $pango_pkg && ac_cv_path_pango_lib=`$PKG_CONFIG --libs-only-l $pango_pkg`
    fi
  fi

  if test x"${ac_cv_path_pango_lib}" = x; then
    for i in $libslist; do
      if test -f $i/libpango-${gnash_pango_version}.a -o -f $i/libpango-${gnash_pango_version}.${shlibext} -o -f $i/libpango-${gnash_pango_version}.${shlibext}.a; then
        if test ! x"$i" = x"/usr/lib" -a ! x"$i" = x"/usr/lib64"; then
	  ac_cv_path_pango_lib="-L$i -lpango-${gnash_pango_version}"
	  break
        else
	  ac_cv_path_pango_lib="-lpango-${gnash_pango_version}"
	  break
	fi
      fi
    done
  fi
	
  AC_MSG_CHECKING([for libpango library])
  AC_MSG_RESULT(${ac_cv_path_pango_lib})
  
  if test x"${ac_cv_path_pango_lib}" = x; then
    AC_CHECK_LIB(pango-${gnash_pango_version}, pango_engine_shape_class_init, [ac_cv_path_pango_lib="-lpango-${gnash_pango_version}"])
  fi

  if test x"${ac_cv_path_pango_incl}" != x; then
    PANGO_CFLAGS="${ac_cv_path_pango_incl}"
  else
    PANGO_CFLAGS=""
  fi

  if test x"${ac_cv_path_pango_lib}" != x; then
    PANGO_LIBS="${ac_cv_path_pango_lib}"
  else
    PANGO_LIBS=""
  fi

  AC_SUBST(PANGO_CFLAGS)
  AC_SUBST(PANGO_LIBS)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
