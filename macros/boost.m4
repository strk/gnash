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

dnl $Id: boost.m4,v 1.34 2007/03/11 00:15:55 rsavoye Exp $

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
          gnash_boost_version=`echo ${gnash_boost_topdir} | sed -e 's:boost-::'`
          ac_cv_path_boost_incl="-I$j"
          break
        fi
      done
      if test x$gnash_boost_version != x; then
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
  BOOST_CFLAGS="$ac_cv_path_boost_incl"
  AC_SUBST(BOOST_CFLAGS)

  dnl Look for the library
  AC_ARG_WITH(boost_lib, AC_HELP_STRING([--with-boost-lib], [directory where boost libraries are]), with_boost_lib=${withval})
  AC_CACHE_VAL(ac_cv_path_boost_lib, [
    if test x"${with_boost_lib}" != x ; then
      ac_cv_path_boost_lib=`(cd ${with_boost_lib}; pwd)`
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
  boostnames="boost_thread-gcc-mt boost_thread boost-thread-mt boost_thread-gcc"
  version_suffix=`echo ${gnash_boost_version} | tr '_' '.'`
  if test x"${ac_cv_path_boost_lib}" = x; then
    AC_MSG_CHECKING([for Boost's thread libraries])
    for i in $libslist; do
      for libname in ${boostnames}; do
    	  if test -f $i/lib${libname}.a -o -f $i/lib${libname}.so; then
    	    if test x"$i" != x"/usr/lib"; then
            ac_cv_path_boost_lib="${ac_cv_path_boost_lib} -L$i -l${libname}"
            break
          else
    	      ac_cv_path_boost_lib="${ac_cv_path_boost_lib} -l${libname}"
            break
          fi
        fi
      done
      if test x"${ac_cv_path_boost_lib}" != x ; then 
        break; 
      fi        
    done
    AC_MSG_RESULT(${ac_cv_path_boost_lib})
  else
    for k in ${boostnames}; do
      if test -f ${ac_cv_path_boost_lib}/lib${k}.a -o -f ${ac_cv_path_boost_lib}/lib${k}.so; then
        if test x"${ac_cv_path_boost_lib}" != x"/usr/lib"; then
	        ac_cv_path_boost_lib="${ac_cv_path_boost_lib} -L${ac_cv_path_boost_lib} -l${k}"
        else
          ac_cv_path_boost_lib="${ac_cv_path_boost_lib} -l${k}"
        fi
      fi
    done
  fi
  if test x"${ac_cv_path_boost_lib}" = x; then
    AC_SEARCH_LIBS(cleanup_slots, ${boostnames}, [ac_cv_path_boost_lib="${LIBS}"])
  fi

  dnl The naming convention is the same as for threads. -mt is the
  dnl preferance, followed by -gcc-mt, followed by -gcc.
  boostnames="boost_date_time_mt boost_date_time-gcc-mt boost_date_time-gcc boost_date_time-mt boost_date_time"
  if test x"${ac_cv_path_boost_lib}" != x; then
    AC_MSG_CHECKING([for Boost's date time libraries])
    for i in $libslist; do
      for libname in ${boostnames}; do
    	  if test -f $i/lib${libname}.a -o -f $i/lib${libname}.so; then
    	    if test x"$i" != x"/usr/lib"; then
            ac_cv_path_boost_lib="${ac_cv_path_boost_lib} -L$i -l${libname}"
            break;
          else
    	      ac_cv_path_boost_lib="${ac_cv_path_boost_lib} -l${libname}"
            break;
          fi
        fi
      done
    done
    AC_MSG_RESULT(${ac_cv_path_boost_lib})
  else
    for k in ${boostnames}; do
      if test -f ${ac_cv_path_boost_lib}/lib${k}.a -o -f ${ac_cv_path_boost_lib}/lib${k}.so; then
        if test x"${ac_cv_path_boost_lib}" != x"/usr/lib"; then
	        ac_cv_path_boost_lib="${ac_cv_path_boost_lib} -L${ac_cv_path_boost_lib} -l${k}"
          break;
        else
          ac_cv_path_boost_lib="${ac_cv_path_boost_lib} -l${k}"
          break;
        fi
      fi
    done
  fi
  if test x"${ac_cv_path_boost_lib}" = x; then
    AC_SEARCH_LIBS(boost::date_time::nth_as_str, ${boostnames}, [ac_cv_path_boost_lib="${LIBS}"], , -lpthread)
  fi

  AC_LANG_POP(C++)  
  dnl we don't want any boost libraries in LIBS, we prefer to kep it seperate.
  LIBS="$save_LIBS"
  BOOST_LIBS="$ac_cv_path_boost_lib"
 
  AC_SUBST(BOOST_LIBS)

  AM_CONDITIONAL(HAVE_BOOST, [test x${ac_cv_path_boost_incl} != x]) 
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
