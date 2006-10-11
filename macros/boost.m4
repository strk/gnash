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

dnl $Id: boost.m4,v 1.12 2006/10/11 01:27:23 nihilus Exp $

dnl Boost modules are:
dnl date-time, filesystem. graph. iostreams, program options, python,
dnl regex, serialization, signals, unit test, thead, and wave.

AC_DEFUN([GNASH_PATH_BOOST],
[
  dnl Lool for the header
  AC_ARG_WITH(boost_incl, [  --with-boost-incl        directory where boost headers are], with_boost_incl=${withval})
  AC_CACHE_VAL(ac_cv_path_boost_incl,[
  if test x"${with_boost_incl}" != x ; then
    if test -f ${with_boost_incl}/thread/mutex.hpp ; then
      ac_cv_path_boost_incl=`(cd ${with_boost_incl}; pwd)`
    elif test -f ${with_boost_incl}/thread/mutex.hpp ; then
      ac_cv_path_boost_incl=`(cd ${with_boost_incl}; pwd)`
    else
      AC_MSG_ERROR([${with_boost_incl} directory doesn't contain any headers])
    fi
  fi
  ])

  if test x"${ac_cv_path_boost_incl}" = x ; then
    AC_MSG_CHECKING([for boost header])
    incllist="${prefix}/include /sw/include /usr/local/include /home/latest/include /usr/pkg/include /opt/include /opt/local/include /usr/include .. ../.."

    for i in $incllist; do
      if test -f $i/boost/detail/lightweight_mutex.hpp; then
        ac_cv_path_boost_incl="$i"
	break;
      fi
    done
  fi

  if test x"${ac_cv_path_boost_incl}" != x ; then
      BOOST_CFLAGS="-I${ac_cv_path_boost_incl}"
      AC_MSG_RESULT(${ac_cv_path_boost_incl})
  else
    AC_MSG_RESULT(no)
    BOOST_CFLAGS=""
    AC_MSG_WARN([Boost header files not found!])
  fi
  AC_SUBST(BOOST_CFLAGS)


  dnl Look for the library
  AC_ARG_WITH(boost_lib, [  --with-boost-lib         directory where boost libraries are], with_boost_lib=${withval})
  if test x"${with_boost_lib}" != x ; then
     ac_cv_path_boost_lib=`(cd ${with_boost_lib}; pwd)`
  fi

dnl  boostnames="boost_thread boost-thread boost_thread-mt boost_thread-gcc-mt boost-thread-gcc-m"
boostnames=""
dnl  AC_MSG_CHECKING([for libboost library])
  for j in $boostnames; do
  if test x"${ac_cv_path_boost_lib}" = x; then
    AC_CHECK_LIB(${j}, cleanup_slots, [ac_cv_path_boost_lib="-l${j}"],[
      libslist="${prefix}/lib64 ${prefix}/lib /usr/lib64 /usr/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib /opt/local/lib /usr/pkg/lib .. ../.."
      for i in $libslist; do
	if test -f $i/lib${j}.a -o -f $i/lib${j}.so; then
	  if test x"$i" != x"/usr/lib"; then
	    ac_cv_path_boost_lib="-L$i -l${j}"
	    break
          else
	    ac_cv_path_boost_lib="-l${j}"
	    break
          fi
        fi
      done])
    if test x"${ac_cv_path_boost_lib}" != x ; then 
      break; 
    fi
  else
    if test -f ${ac_cv_path_boost_lib}/lib${j}.a -o -f ${ac_cv_path_boost_lib}/lib${j}.so; then
      if test x"${ac_cv_path_boost_lib}" != x"/usr/lib"; then
	ac_cv_path_boost_lib="-L${ac_cv_path_boost_lib} -l${j}"
      else
        ac_cv_path_boost_lib="-l${j}"
      fi
    fi
  fi
  done
dnl  AC_MSG_RESULT(${ac_cv_path_boost_lib})
  
  if test x"${ac_cv_path_boost_lib}" != x ; then
      BOOST_LIBS="${ac_cv_path_boost_lib}"
  else
      BOOST_LIBS=""
  fi
  AC_SUBST(BOOST_LIBS)

  AM_CONDITIONAL(HAVE_BOOST, [test x${ac_cv_path_boost_incl} != x]) 
])
