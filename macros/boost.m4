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

dnl  
dnl 

dnl $Id: boost.m4,v 1.25 2006/11/04 00:00:30 rsavoye Exp $

dnl Boost modules are:
dnl date-time, filesystem. graph. iostreams, program options, python,
dnl regex, serialization, signals, unit test, thead, and wave.

AC_DEFUN([GNASH_PATH_BOOST],
[
  dnl Lool for the header
  AC_ARG_WITH(boost_incl, AC_HELP_STRING([--with-boost-incl], [directory where boost headers are]), with_boost_incl=${withval})
  AC_CACHE_VAL(ac_cv_path_boost_incl,[
  if test x"${with_boost_incl}" != x ; then
    if test -f ${with_boost_incl}/boost/detail/lightweight_mutex.hpp ; then
      ac_cv_path_boost_incl=-I`(cd ${with_boost_incl}; pwd)`
    else
      AC_MSG_ERROR([${with_boost_incl} directory doesn't contain any headers])
    fi
  fi
  ])

  dnl Attempt to find the top level directory, which unfortunately has a
  dnl version number attached. At least on Debain based systems, this
  dnl doesn't seem to get a directory that is unversioned.
  AC_MSG_CHECKING([for the Boost Version])

  if test x"$PKG_CONFIG" != x; then
  	$PKG_CONFIG --exists boost && gnash_boost_version=`$PKG_CONFIG --modversion boost | cut -d "." -f 1 | awk '{print $1".0"}'`
  fi

  pathlist="${prefix}/${target_alias}/include ${prefix}/include /sw/include /opt/local/include /usr/nekoware/include /usr/freeware/include /usr/local/include /home/latest/include /opt/include /opt/local/include /opt/local/include /usr/include /usr/pkg/include .. ../.."
  gnash_boost_topdir=""
  gnash_boost_version=""
  for i in $pathlist; do
    for j in `ls -dr $i/boost* 2>/dev/null`; do
      if test -f ${j}/boost/detail/lightweight_mutex.hpp; then
        gnash_boost_topdir=`basename $j`
        gnash_boost_version=`echo ${gnash_boost_topdir} | sed -e 's:boost-::'`
        break
      fi
    done
    if test x$gnash_boost_version != x; then
      break;
    fi
  done

  if test x"${gnash_boost_version}" = x; then
    AC_MSG_RESULT([no version needed])
  else
   AC_MSG_RESULT(${gnash_boost_version})
  fi

  AC_LANG_PUSH(C++)
  if test x"${ac_cv_path_boost_incl}" = x ; then
    AC_CHECK_HEADERS(boost/detail/lightweight_mutex.hpp, [ac_cv_path_boost_incl="-I/usr/include"],[
    if test x"${ac_cv_path_boost_incl}" = x; then
      incllist="${prefix}/${target_alias}/include  ${prefix}/include /usr/local/include /sw/include /opt/local/include /usr/local/include /usr/nekoware/include /usr/freeware/include /home/latest/include /opt/include /opt/local/include /opt/local/include /usr/include /usr/pkg/include .. ../.."
      for i in $incllist; do
        if test -f $i/boost/detail/lightweight_mutex.hpp; then
          ac_cv_path_boost_incl="-I$i"
          break
        fi
        for j in `ls -dr $i/boost* 2>/dev/null`; do
          if test -f $j/boost/detail/lightweight_mutex.hpp; then
            ac_cv_path_boost_incl="-I$j"
            break
          fi
        done
      done
   fi])
  fi
  AC_LANG_POP(C++)
  AC_MSG_CHECKING([for boost header])
  AC_MSG_RESULT(${ac_cv_path_boost_incl})
  BOOST_CFLAGS="$ac_cv_path_boost_incl"
  AC_SUBST(BOOST_CFLAGS)

  dnl Look for the library
  AC_ARG_WITH(boost_lib, AC_HELP_STRING([--with-boost-lib], [directory where boost libraries are]), with_boost_lib=${withval})
  AC_CACHE_VAL(ac_cv_path_boost_lib,[
  if test x"${with_boost_lib}" != x ; then
     ac_cv_path_boost_lib=`(cd ${with_boost_lib}; pwd)`
  fi
])

  dnl This is the default list of names to search for. The function needs to be
  dnl a C function, as double colons screw up autoconf. We also force the probable 
  boostnames="boost_thread-gcc-mt boost_thread boost-thread boost_thread-mt boost-thread-gcc-mt"
  version_suffix=`echo ${gnash_boost_version} | tr '_' '.'`
  save_LIBS="$LIBS"
  AC_LANG_PUSH(C++)
  if test x"${ac_cv_path_boost_lib}" = x; then
  AC_SEARCH_LIBS(cleanup_slots, ${boostnames}, [ac_cv_path_boost_lib="${LIBS}"],[
  AC_MSG_CHECKING([for Boost thread library])
      libslist="${prefix}/${target_alias}/lib ${prefix}/lib64 ${prefix}/lib32 ${prefix}/lib /usr/lib64 /usr/lib32 /usr/nekoware/lib /usr/freeware/lib /usr/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib /opt/local/lib /usr/pkg/lib .. ../.."
      for i in $libslist; do
        boostnames=`ls -dr $i/libboost?thread*.so 2>/dev/null`
        for libname in ${boostnames}; do
	  if test -f ${libname}; then
            linkname=`basename ${libname} | sed -e 's/lib//' -e 's/.so//'`
	    if test x"$i" != x"/usr/lib"; then
	      ac_cv_path_boost_lib="-L$i -l${linkname}"
	      break
            else
	      ac_cv_path_boost_lib="-l${linkname}"
	      break
            fi
          fi
        done
        if test x"${ac_cv_path_boost_lib}" != x ; then 
          break; 
        fi        
      done
    AC_MSG_RESULT(${ac_cv_path_boost_lib})
    ])
  else
    if test -f ${ac_cv_path_boost_lib}/lib${j}.a -o -f ${ac_cv_path_boost_lib}/lib${j}.so; then
      linkname=`basename ${libname} | sed -e 's/lib//'`
      if test x"${ac_cv_path_boost_lib}" != x"/usr/lib"; then
	ac_cv_path_boost_lib="-L${ac_cv_path_boost_lib} -l${linkname}"
      else
        ac_cv_path_boost_lib="-l${linkname}"
      fi
    fi
  fi
  AC_LANG_POP(C++)
  
  dnl we don't want any boost libraries in LIBS, we prefer to kep it seperate.
  LIBS="$save_LIBS"
  BOOST_LIBS="$ac_cv_path_boost_lib"
 
  AC_SUBST(BOOST_LIBS)

  AM_CONDITIONAL(HAVE_BOOST, [test x${ac_cv_path_boost_incl} != x]) 
])
