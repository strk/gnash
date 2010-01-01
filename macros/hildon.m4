dnl  
dnl    Copyright (C) 2005, 2006, 2009, 2010 Free Software Foundation, Inc.
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


AC_DEFUN([GNASH_PATH_HILDON],
[

  dnl Look for the header
  AC_ARG_WITH(hildon_incl, AC_HELP_STRING([--with-hildon-incl], [directory where libhildon header is]), with_hildon_incl=${withval})
  AC_CACHE_VAL(ac_cv_path_hildon_incl,[
    if test x"${with_hildon_incl}" != x ; then
      if test -f ${with_hildon_incl}/hildon/hildon-program.h ; then
        ac_cv_path_hildon_incl="-I`(cd ${with_hildon_incl}; pwd)`"
      else
        AC_MSG_ERROR([${with_hildon_incl} directory doesn't contain hildon/hildon.h])
      fi
    fi
  ])

  if test x$cross_compiling = xno; then
    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_hildon_incl}" = x; then
      $PKG_CONFIG --exists hildon-1 && ac_cv_path_hildon_incl="`$PKG_CONFIG --cflags-only-I hildon-1 | cut -d ' ' -f 1`"
    fi
  fi

  dnl Attempt to find the top level directory, which unfortunately has a
  dnl version number attached. At least on Debain based systems, this
  dnl doesn't seem to get a directory that is unversioned.

  AC_MSG_CHECKING([for the Hildon Version])

  if test x$cross_compiling = xno; then
    if test x"$PKG_CONFIG" != x; then
      $PKG_CONFIG --exists hildon-1 && gnash_hildon_version="`$PKG_CONFIG --modversion hildon-1 | cut -d '.' -f 1`"
    fi
  fi

  if test x"${gnash_hildon_version}" = x; then
    gnash_hildon_topdir=""
    gnash_hildon_version=""
    for i in $incllist; do
      for j in `ls -dr $i/hildon-[[0-9]] 2>/dev/null`; do
        if test -f $j/hildon/hildon-program.h; then
	        gnash_hildon_topdir="`basename $j`"
	        gnash_hildon_version="`echo ${gnash_hildon_topdir} | sed -e 's:hildon-::'`"
 	        ac_cv_path_hildon_incl="-I$i/${gnash_hildon_topdir}"
	        break
	      fi
      done
	    if test x$gnash_hildon_version != x; then
	      break;
	    fi
    done
  fi      

  if test x"${gnash_hildon_version}" = x; then
    AC_MSG_RESULT(none)
  else
    AC_MSG_RESULT([${gnash_hildon_version}])
  fi
  
  AC_MSG_CHECKING([for libhildon header])
  AC_MSG_RESULT(${ac_cv_path_hildon_incl}) 

  dnl Look for the library
  AC_ARG_WITH(hildon_lib, AC_HELP_STRING([--with-hildon-lib], [directory where hildon library is]), with_hildon_lib=${withval})
  AC_CACHE_VAL(ac_cv_path_hildon_lib,[
    if test x"${with_hildon_lib}" != x ; then
      if test -f ${with_hildon_lib}/libhildon-${gnash_hildon_version}.a -o -f ${with_hildon_lib}/libhildon-${gnash_hildon_version}.${shlibext}; then
	      ac_cv_path_hildon_lib="-L`(cd ${with_hildon_lib}; pwd)`"
      else
	      AC_MSG_ERROR([${with_hildon_lib} directory doesn't contain libhildonhildon.])
      fi
    fi
  ])
  
  if test x$cross_compiling = xno; then
    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_hildon_lib}" = x; then
      $PKG_CONFIG --exists hildon-1 && ac_cv_path_hildon_lib="`$PKG_CONFIG --libs-only-l hildon-1 | cut -d ' ' -f 1`"
    fi
  fi

  if test x"${ac_cv_path_hildon_lib}" = x; then
    for i in $libslist; do
      if test -f $i/libhildon-${gnash_hildon_version}.a -o -f $i/libhildon-${gnash_hildon_version}.${shlibext}; then
        if test ! x"$i" = x"/usr/lib" -a ! x"$i" = x"/usr/lib64"; then
	        ac_cv_path_hildon_lib="-L$i -lhildon-${gnash_hildon_version}"
	        break
        else
	        ac_cv_path_hildon_lib="-lhildon-${gnash_hildon_version}"
	        break
	      fi
      fi
    done
  else
    hildon_lib_path=`$PKG_CONFIG --libs-only-L hildon-1`
    if test ! $hildon_lib_path = "-L/usr/lib" -o $hildon_lib_path = "-L/usr/lib64"; then
      ac_cv_path_hildon_lib="${hildon_lib_path} ${ac_cv_path_hildon_lib}"
    fi
  fi
	
  AC_MSG_CHECKING([for libhildon library])
  AC_MSG_RESULT(${ac_cv_path_hildon_lib})
  
  if test x"${ac_cv_path_hildon_lib}" = x; then
    AC_CHECK_LIB(hildon-${gnash_hildon_version}, hildon_engine_shape_class_init, [ac_cv_path_hildon_lib="-lhildon-${gnash_hildon_version}"])
  fi

  if test x"${ac_cv_path_hildon_incl}" != x; then
    HILDON_CFLAGS="${ac_cv_path_hildon_incl}"
  else
    HILDON_CFLAGS=""
  fi

  if test x"${ac_cv_path_hildon_lib}" != x; then
    HILDON_LIBS="${ac_cv_path_hildon_lib}"
    AC_DEFINE(HAVE_HILDON, [1], [has the Hildon mobile framework])
  else
    HILDON_LIBS=""
  fi

  AC_SUBST(HILDON_CFLAGS)
  AC_SUBST(HILDON_LIBS)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
