dnl  
dnl  Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
dnl  2011, 2012 Free Software Foundation, Inc.
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

dnl Boost modules are:
dnl date-time, filesystem. graph. iostreams, program options, system, chrono
dnl regex, serialization, signals, unit test, thead, and wave.

AC_DEFUN([GNASH_PATH_BOOST],
[
  dnl start variables with a known value
  gnash_boost_topdir=""
  gnash_boost_libdir=""
  missing_headers=""
  missing_libs=""
  missing_plugin_libs=""
  missing_cygnal_libs=""
  gcc_version=""
  dirname=""
  libname=""
  dnl this is a list of *required* headers. If any of these are missing, this
  dnl test will return a failure, and Gnash won't build.
  boost_headers="multi_index_container.hpp multi_index/key_extractors.hpp program_options/options_description.hpp iostreams/stream.hpp"
  dnl this is a list of *required* libraries. If any of these are missing, this
  dnl test will return a failure, and Gnash won't build.
  boost_libs="program_options"
  boost_plugin_libs="iostreams"

  dnl this is a list of *recommended* libraries. If any of these are missing,
  dnl this test will return a warning, and Gnash will build, but testing
  dnl won't work.
  boost_cygnal_libs="serialization date_time"

  dnl this is the default list for paths to search. This gets
  dnl redefined if --with-boost-incl= is specified.
  newlist=$incllist

  dnl Look for the header
  AC_ARG_WITH(boost_incl, AC_HELP_STRING([--with-boost-incl], [directory where boost headers are]), with_boost_incl=${withval})
  if test x"${with_boost_incl}" != x ; then
    gnash_boost_topdir="`(cd ${with_boost_incl}; pwd)`"
    newlist=${gnash_boost_topdir}
  fi

  dnl munge the GCC version number, which Boost uses to label it's libraries.
  if test x"${GXX}" = xyes; then
  	gcc_version="`${CXX} --version | head -1 | cut -d ' ' -f 3 | cut -d '.' -f 1-2 | tr -d '.'`"
  fi

  AC_MSG_CHECKING([for boost header])
  for i in $newlist; do
    gnash_boost_topdir=/usr/include/boost
    dirs="`ls -dr $i/boost* 2>/dev/null | xargs`"
    for u in ${dirs}; do
      if test -n "$u" -a -d "$u" -a x"$u" != x"/usr/include/boost"; then
        gnash_boost_topdir="`(cd $u; pwd)`"
        gnash_boost_subdir="`dirname ${gnash_boost_topdir}`"
        dnl Fix for packaging systems not adding extra fluff to the path-name.
        for k in ${boost_headers}; do
          if test ! -f ${gnash_boost_topdir}/boost/$k; then
            if test ! -f ${gnash_boost_subdir}/boost/$k; then
              missing_headers="${missing_headers} $k"
            else
              gnash_boost_topdir=${gnash_boost_subdir}
            fi
          fi
        done
        if test x"${missing_headers}" = x ; then
          ac_cv_path_boost_incl="-I${gnash_boost_topdir}"
          AC_MSG_RESULT(${ac_cv_path_boost_incl})
          break
        else
          AC_MSG_RESULT([headers missing])
          AC_MSG_WARN([You need to install ${missing_headers}])
        fi
    fi
    done
  done

  dnl this is the default list for paths to search. This gets
  dnl redefined if --with-boost-lib= is specified.
  newlist=$libslist

  dnl Look for the library
  AC_ARG_WITH(boost_lib, AC_HELP_STRING([--with-boost-lib], [directory where boost libraries are]), with_boost_lib=${withval})
  if test x"${with_boost_lib}" != x ; then
    gnash_boost_libdir="`(cd ${with_boost_lib}; pwd)`"
    newlist="${gnash_boost_libdir}"
  fi

  dnl Specify the list of probable names. Boost creates 8 identical
  dnl libraries with different names. The prefered order is to always
  dnl use the one with -mt on it, because it's the thread safe
  dnl version. Then look for the version with -gcc in it, as it's the
  dnl version compiled with GCC instead of the native
  dnl compiler. Finally look for the library without any qualitfying
  dnl attributes.
  if test x${ac_cv_path_boost_lib} = x; then
    AC_MSG_CHECKING([for Boost libraries])
    for i in $newlist; do
      if test x"${ac_cv_path_boost_lib}" != x; then
        break
      else
        missing_libs=""
      fi
      for j in ${boost_libs}; do
        dirs="`ls -dr $i/libboost_${j}*.${shlibext} $i/libboost_${j}*.a 2>/dev/null`"
        if test -n "${dirs}"; then
          libname="`echo ${dirs} | sed -e 's:^.*/lib::' -e "s:\.${shlibext}::" -e "s:\.a::"`"
          if test x$dirname = x; then
            dirname="`echo ${dirs} | sed -e 's:/libboost.*$::'`"
           if test x"${dirname}" != "x/usr/lib"; then
      	      ac_cv_path_boost_lib="-L${dirname}"
            fi
          fi
          ac_cv_path_boost_lib="${ac_cv_path_boost_lib} -l${libname}"
        else
          missing_libs="${missing_libs} $j"
        fi
      done
    done
    for j in ${boost_plugin_libs}; do
      dirs="`ls -dr ${dirname}/libboost_${j}*.${shlibext} ${dirname}/libboost_${j}*.a 2>/dev/null`"
      if test -n "${dirs}"; then
          libname="`echo ${dirs} | sed -e 's:^.*/lib::' -e "s:\.${shlibext}::" -e "s:\.a::"`"
        ac_cv_path_boost_plugin_lib="${ac_cv_path_boost_plugin_lib} -l${libname}"
      else
        missing_plugin_libs="${missing_plugin_libs} $j"
      fi
    done
    for j in ${boost_cygnal_libs}; do
      dirs="`ls -dr ${dirname}/libboost_${j}*.${shlibext} ${dirname}/libboost_${j}*.a 2>/dev/null`"
      if test -n "${dirs}"; then
          libname="`echo ${dirs} | sed -e 's:^.*/lib::' -e "s:\.${shlibext}::" -e "s:\.a::"`"
        ac_cv_path_boost_cygnal_lib="${ac_cv_path_boost_cygnal_lib} -l${libname}"
      else
        missing_cygnal_libs="${missing_cygnal_libs} $j"
      fi
    done
  fi

  if test x"${missing_libs}" != x -o x"${missing_plugin_libs}" != x -o x"${missing_cygnal_libs}" != x; then
    AC_MSG_WARN([Libraries ${missing_libs} ${missing_plugin_libs} ${missing_cygnal_libs} aren't installed ])
  fi
  AC_MSG_RESULT(${ac_cv_path_boost_lib})

  if test x"${ac_cv_path_boost_incl}" != x; then
    BOOST_CFLAGS="$ac_cv_path_boost_incl"
  fi

  if test x"${ac_cv_path_boost_lib}" != x; then
    BOOST_LIBS="$ac_cv_path_boost_lib"
  fi

  if test x"${ac_cv_path_boost_plugin_lib}" != x; then
    BOOST_PLUGIN_LIBS="$ac_cv_path_boost_plugin_lib"
  fi

  if test x"${ac_cv_path_boost_cygnal_lib}" != x; then
    BOOST_CYGNAL_LIBS="$ac_cv_path_boost_cygnal_lib" 
  fi

  dnl ------------------------------------------------------------------
  dnl Set HAVE_BOOST conditional, BOOST_CFLAGS and BOOST_LIBS variables
  dnl ------------------------------------------------------------------

  AC_SUBST(BOOST_CFLAGS)
  AC_SUBST(BOOST_LIBS)
  AC_SUBST(BOOST_PLUGIN_LIBS)
  AC_SUBST(BOOST_CYGNAL_LIBS)

  dnl This isn't right: you don't need boot date-time installed unless u build
  dnl cygnal, and it is sometimes a separate package from Boost core and thread.
  dnl TODO: why is this needed, lack of boost being a fatal error?
  AM_CONDITIONAL(HAVE_BOOST, [test -n "${BOOST_LIBS}"])
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
