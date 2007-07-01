dnl  
dnl    Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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

dnl $Id: boost.m4,v 1.47 2007/07/01 10:54:11 bjacques Exp $

dnl Boost modules are:
dnl date-time, filesystem. graph. iostreams, program options, python,
dnl regex, serialization, signals, unit test, thead, and wave.

AC_DEFUN([GNASH_PATH_BOOST],
[
  dnl Lool for the header
  AC_ARG_WITH(boost_incl, AC_HELP_STRING([--with-boost-incl], [directory where boost headers are]), with_boost_incl=${withval})
  AC_CACHE_VAL(ac_cv_path_boost_incl, [
    if test x"${with_boost_incl}" != x ; then
      if test -f ${with_boost_incl}/boost/detail/lightweight_mutex.hpp ; then
        ac_cv_path_boost_incl=-I`(cd ${with_boost_incl}; pwd)`
      else
        AC_MSG_ERROR([${with_boost_incl} directory doesn't contain any headers])
      fi
    fi
  ])

  dnl Attempt to find the top level directory, which unfortunately has a
  dnl version number attached. At least on Debian based systems, this
  dnl doesn't seem to get a directory that is unversioned.
  AC_MSG_CHECKING([for the Boost Version])
  if test x$cross_compiling = xno; then
    if test x"$PKG_CONFIG" != x; then
      $PKG_CONFIG --exists boost && gnash_boost_version=`$PKG_CONFIG --modversion boost | cut -d "." -f 1 | awk '{print $'0'".0"}'`
    fi
  fi

  if test x"${gnash_boost_version}" = x; then
    gnash_boost_topdir=""
    gnash_boost_version=""
    for i in $incllist; do
      for j in `ls -dr $i/boost* 2>/dev/null`; do
        if test -f ${j}/boost/detail/lightweight_mutex.hpp; then
          gnash_boost_topdir=`basename $j`
          ac_cv_path_boost_incl="-I${j}"
          break
        fi
        if test -f ${j}/detail/lightweight_mutex.hpp; then
          gnash_boost_topdir=`basename $j`
          ac_cv_path_boost_incl="-I${i}"
          break
        fi
      done
      if test x$gnash_boost_topdir != x; then
        gnash_boost_version=`echo ${gnash_boost_topdir} | sed -e 's:boost-::'`
        break;
      fi
    done
  fi

  if test x"${gnash_boost_version}" = x; then
    AC_MSG_RESULT([no version needed])
  else
   AC_MSG_RESULT(${gnash_boost_version})
  fi

  if test x"${ac_cv_path_boost_incl}" = x ; then
    AC_CHECK_HEADERS(boost/detail/lightweight_mutex.hpp, [ac_cv_path_boost_incl="-I/usr/include"])
  fi

  AC_MSG_CHECKING([for boost header])
  AC_MSG_RESULT(${ac_cv_path_boost_incl})

  dnl Look for the library
  AC_ARG_WITH(boost_lib, AC_HELP_STRING([--with-boost-lib], [directory where boost libraries are]), with_boost_lib=${withval})
  AC_CACHE_VAL(ac_cv_path_boost_lib, [
    if test x"${with_boost_lib}" != x ; then
      ac_cv_path_boost_lib="-L`(cd ${with_boost_lib}; pwd)`"
    fi
  ])

  AC_LANG_PUSH(C++)
  save_LIBS="$LIBS"

  dnl Specify the list of probable names. Boost creates 8 identical
  dnl libraries with different names. The prefered order is to always
  dnl use the one with -mt on it, because it's the thread safe
  dnl version. Then look for the version with -gcc in it, as it's the
  dnl version compiled with GCC instead of the native
  dnl compiler. Finally look for the library without any qualitfying
  dnl attributes.
  boost_date_time="no"
  boost_thread="no"
  AC_MSG_CHECKING([for Boost libraries])
  for i in $libslist; do
    if test x${boost_date_time} = xyes && test x${boost_thread} = xyes; then
      break;
    fi
    dirs=`ls -dr $i/libboost_date_time*.${shlibext} $i/libboost_date_time*.${shlibext}.* $i/libboost_date_time*.a 2>/dev/null`
    for libname in $dirs; do
      if test x"${boost_date_time}" = xno; then
        lfile=`basename ${libname} | eval sed -e 's:^lib::' -e 's:.a$::' -e 's:\.${shlibext}.*::'`
        ldir=`dirname ${libname}`
        if test -f ${ldir}/lib${lfile}-mt.${shlibext}; then
          lfile="${lfile}-mt"
        fi
        boost_date_time=yes
	if test x"${ldir}" != "x/usr/lib"; then
        	ac_cv_path_boost_lib="-L${ldir} -l${lfile}"
	else
        	ac_cv_path_boost_lib="-l${lfile}"
	fi
        break
      else
        break
      fi
    done

    dnl now look for the Boost Thread library
    dirs=`ls -dr $i/libboost_thread*.${shlibext} $i/libboost_thread*.${shlibext}.* $i/libboost_thread*.a 2>/dev/null`
    for libname in $dirs; do
      if test x"${boost_thread}" = xno; then
        lfile=`basename ${libname} | eval sed -e 's:^lib::'  -e 's:.a$::' -e 's:\.${shlibext}.*::'`
        ldir=`dirname ${libname}`
        if test -f ${ldir}/lib${lfile}-mt.${shlibext}; then
          lfile=${lfile}-mt
        fi
        boost_thread=yes
        ac_cv_path_boost_lib="${ac_cv_path_boost_lib} -l${lfile}"
        break
      else
        break
      fi
    done
  done
  AC_MSG_RESULT(${ac_cv_path_boost_lib})

  dnl we don't want any boost libraries in LIBS, we prefer to keep it seperate.
  AC_LANG_POP(C++)  
  LIBS="$save_LIBS"

  BOOST_CFLAGS="$ac_cv_path_boost_incl"
  AC_SUBST(BOOST_CFLAGS)

  BOOST_LIBS="$ac_cv_path_boost_lib" 
  AC_SUBST(BOOST_LIBS)

  # This isn't right: you don't need boot date-time installed unless u build
  # cygnal, and it is sometimes a separate package from Boost core and thread.
  AM_CONDITIONAL(HAVE_BOOST, [test x${boost_date_time} = xyes && test x${boost_thread} = xyes])
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
