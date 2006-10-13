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

dnl $Id: boost.m4,v 1.16 2006/10/13 23:07:35 nihilus Exp $

dnl Boost modules are:
dnl date-time, filesystem. graph. iostreams, program options, python,
dnl regex, serialization, signals, unit test, thead, and wave.

AC_DEFUN([GNASH_PATH_BOOST],
[
  dnl Lool for the header
  AC_ARG_WITH(boost_incl, [  --with-boost-incl        directory where boost headers are], with_boost_incl=${withval})
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

  pathlist="/usr/local/include /sw/include /opt/local/include /usr/local/include /home/latest/include /opt/include /opt/local/include /opt/local/include /usr/include /usr/pkg/include .. ../.."
  gnash_boost_topdir=""
  gnash_boost_version=""
  for i in $pathlist; do
    for libdir in `ls -dr $i/boost* 2>/dev/null`; do
      if test -f ${libdir}/boost/detail/lightweight_mutex.hpp; then
	ac_cv_path_boost_incl="-I${libdir}"
        gnash_boost_topdir=`echo ${libdir} | sed -e 's:/include/.*::'`
        gnash_boost_version=`basename ${libdir} | sed -e 's:boost-::'`
        break
      fi
    done
    if test x"${gnash_boost_topdir}" != x ; then
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
      incllist="/usr/local/include /sw/include /opt/local/include /usr/local/include /home/latest/include /opt/include /opt/local/include /opt/local/include /usr/include /usr/pkg/include .. ../.."
      for i in $incllist; do
        if test -f $i/boost/detail/lightweight_mutex.hpp; then
          ac_cv_path_boost_incl="-I$i"
          break
        fi
      done
   fi])
  fi
  AC_LANG_POP(C++)
  AC_MSG_CHECKING([for boost header])
  AC_MSG_RESULT(${ac_cv_path_boost_incl})
  BOOST_CFLAGS="$ac_cv_path_boost_incl"
  AC_SUBST(BOOST_CFLAGS)

  dnl Look for the library
  AC_ARG_WITH(boost_lib, [  --with-boost-lib         directory where boost libraries are], with_boost_lib=${withval})
  AC_CACHE_VAL(ac_cv_path_boost_lib,[
  if test x"${with_boost_lib}" != x ; then
     ac_cv_path_boost_lib=`(cd ${with_boost_lib}; pwd)`
  fi
])

  dnl This is the default list of names to search for. The function needs to be
  dnl a C function, as double colons screw up autoconf. We also force the probable 
  boostnames="boost_thread-gcc-mt boost_thread boost-thread boost_thread-mt boost-thread-gcc-mt"
  version_suffix=`echo ${gnash_boost_version} | tr '_' '.'`
  dnl AC_MSG_CHECKING([for Boost thread library])
  AC_LANG_PUSH(C++)
  if test x"${ac_cv_path_boost_lib}" = x; then
  AC_SEARCH_LIBS(cleanup_slots, ${boostnames}, [ac_cv_path_boost_lib="$ac_cv_search_cleanup_slots"],[
      libslist="${prefix}/lib64 ${prefix}/lib32 ${prefix}/lib /usr/lib64 /usr/lib32 /usr/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib /opt/local/lib /usr/pkg/lib .. ../.."
      for i in $libslist; do
        boostnames=`ls -dr $i/libboost?thread*.so`
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
dnl  AC_MSG_RESULT(${ac_cv_path_boost_lib})
  
  BOOST_LIBS="$ac_cv_path_boost_lib"
  
 
  AC_SUBST(BOOST_LIBS)

  AM_CONDITIONAL(HAVE_BOOST, [test x${ac_cv_path_boost_incl} != x]) 
])
