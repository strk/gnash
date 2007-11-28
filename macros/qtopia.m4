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

dnl $Id: qtopia.m4,v 1.1 2007/11/28 03:34:27 rsavoye Exp $

dnl ~{rob@ute} pts/8> QtCore  QtSvg Qtnetwork QtXml 
dnl QtCore: Command not found.
dnl ~{rob@ute} pts/8> -L/home/rob/projects/gnu/i686-pc-linux-gnulibc1/qtopia/lib -lQtSvg -L/home/rob/projects/gnu/i686-pc-linux-gnulibc1/qtopia/qtopiacore/target/lib -lQtSql -lQtXml -lQtGui -lpng -lQtNetwork -lQtCore -lz -lm -lrt -ldl -lpthread -lqtopiabase -lqtopia -lmd5 -lqtopia-sqlite -lqtopiasecurity -Wl,-rpath,/usr/local/qtopia-prefix/lib
dnl -L/home/rob/projects/gnu/i686-pc-linux-gnulibc1/qtopia/lib: Command not found.

AC_DEFUN([GNASH_PATH_QTOPIA],
[

  has_qtopia=no
  dnl the list of Qtopia headers we need
  dnl Look for the header
  AC_ARG_WITH(qtopia_incl, AC_HELP_STRING([--with-qtopia-incl], [directory where libqtopia header is]), with_qtopia_incl=${withval})
  AC_CACHE_VAL(ac_cv_path_qtopia_incl,[
    if test x"${with_qtopia_incl}" != x ; then
      if test -f ${with_qtopia_incl}/qtopiamail/qtopiamail.h ; then
        gnash_qtopia_topdir="${with_qtopia_incl}"
        ac_cv_path_qtopia_incl="-I`(cd ${with_qtopia_incl}; pwd)`"
        gnash_qtopia_version=4
      else
        gnash_qtopia_topdir="${with_qtopia_incl}"
        if test -f ${with_qtopia_incl}/qtopia/mail/qtopiamail.h ; then
          ac_cv_path_qtopia_incl="-I`(cd ${with_qtopia_incl}; pwd)`"
          gnash_qtopia_version=2
        else
          AC_MSG_ERROR([${with_qtopia_incl} directory doesn't contain qtopiamail.h])
        fi
      fi
    fi
  ])

  if test x"${ac_cv_path_qtopia_incl}" = x; then
    if test x$cross_compiling = xno; then
      if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_qtopia_incl}" = x; then
        $PKG_CONFIG --exists qt-mt && ac_cv_path_qtopia_incl="`$PKG_CONFIG --cflags-only-I qt-mt | cut -d ' ' -f 1`"
      fi
    fi

    dnl Attempt to find the top level directory, which unfortunately has a
    dnl version number attached. At least on Debain based systems, this
    dnl doesn't seem to get a directory that is unversioned.

    AC_MSG_CHECKING([for the Qtopia Version])

    if test x$cross_compiling = xno; then
      if test x"$PKG_CONFIG" != x; then
        $PKG_CONFIG --exists qtopia-1 && gnash_qtopia_version="`$PKG_CONFIG --modversion qtopia-1 | cut -d '.' -f 1`"
      fi
    fi

    if test x"${gnash_qtopia_version}" = x; then
      gnash_qtopia_topdir=""
      gnash_qtopia_version=""
      for i in $incllist; do
        for j in `ls -dr $i/qtopia-[[0-9]] 2>/dev/null`; do
          if test -f $j/qtopia/qtopia-program.h; then
	          gnash_qtopia_topdir="`basename $j`"
	          gnash_qtopia_version="`echo ${gnash_qtopia_topdir} | sed -e 's:qtopia-::'`"
 	          ac_cv_path_qtopia_incl="-I$i/${gnash_qtopia_topdir}"
	          break
	        fi
        done
	      if test x$gnash_qtopia_version != x; then
	        break;
	      fi
      done
    fi      
  fi                            dnl end of ${ac_cv_path_qtopia_incl}

  dnl this a sanity check for Qtopia 2
  AC_MSG_CHECKING([Sanity checking the Qtopia header installation])
  qt_headers="qpen.h qpixmap.h"
  if test $gnash_qtopia_version -eq 2; then
    gnash_qtopia_topdir=`dirname ${gnash_qtopia_topdir}`
    if test x"${ac_cv_path_qtopia_incl}" != x; then
      for i in $qt_headers; do
        if ! test -f  ${gnash_qtopia_topdir}/include/$i; then
          AC_MSG_WARN([$i not found!])
          qtopia_nogo=yes
        fi
      done
    fi
  fi
  
  dnl this a sanity check for Qtopia 4
  qt_headers="QtXml QtGui QtCore"
  if test $gnash_qtopia_version -eq 4; then
    gnash_qtopia_topdir=`dirname ${gnash_qtopia_topdir}`
    if test x"${ac_cv_path_qtopia_incl}" != x; then
      for i in $qt_headers; do
        if ! test -f  ${gnash_qtopia_topdir}/qtopiacore/target/include/Qt/$i; then
          AC_MSG_WARN([$i not found!])
          qtopia_nogo=yes
        fi
      done
    fi
  fi

  if test x"${qtopia_nogo}" = xyes; then
    AC_MSG_ERROR([Broken Qtopia header installation!])
  else
    AC_MSG_RESULT([fine])
  fi

  if test x"${gnash_qtopia_version}" = x; then
    AC_MSG_RESULT(none)
  else
    AC_MSG_RESULT([${gnash_qtopia_version}])
  fi
  
  AC_MSG_CHECKING([for libqtopia header])
  AC_MSG_RESULT(${ac_cv_path_qtopia_incl}) 

  dnl the list of Qtopia libs we need
  qt_libs="qtopiabase qtopia md5 qtopia-sqlite qtopiasecurity"

  dnl Look for the library
  AC_ARG_WITH(qtopia_lib, AC_HELP_STRING([--with-qtopia-lib], [directory where qtopia library is]), with_qtopia_lib=${withval})
  AC_CACHE_VAL(ac_cv_path_qtopia_lib,[
    if test x"${with_qtopia_lib}" != x ; then
      if test -f ${with_qtopia_lib}/libqtopiamail.a -o -f ${with_qtopia_lib}/libqtopiamail.${shlibext}; then
	      ac_cv_path_qtopia_lib="-L`(cd ${with_qtopia_lib}; pwd)`"
      else
	      AC_MSG_ERROR([${with_qtopia_lib} directory doesn't contain libqtopiaqtopia.])
      fi
    fi
  ])
  
  if test x"${ac_cv_path_qtopia_lib}" = x; then
    if test x$cross_compiling = xno; then
      if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_qtopia_lib}" = x; then
        $PKG_CONFIG --exists qt-mt && ac_cv_path_qtopia_lib="`$PKG_CONFIG --libs-only-l qt-mt | cut -d ' ' -f 1`"
      fi
    fi

    if test x"${ac_cv_path_qtopia_lib}" = x; then
      for i in $libslist; do
        if test -f $i/libqtopiamail.a -o -f $i/libqtopiamail.${shlibext}; then
          if test ! x"$i" = x"/usr/lib" -a ! x"$i" = x"/usr/lib64"; then
	          ac_cv_path_qtopia_lib="-L$i -lqtopiamail"
	          break
          else
	          ac_cv_path_qtopia_lib="-lqtopiamail"
	          break
	        fi
        fi
      done
    else
      qtopia_lib_path=`$PKG_CONFIG --libs-only-L qtopia`
      if test ! $qtopia_lib_path = "-L/usr/lib" -o $qtopia_lib_path = "-L/usr/lib64"; then
        ac_cv_path_qtopia_lib="${qtopia_lib_path} ${ac_cv_path_qtopia_lib}"
      fi
    fi
	fi

  dnl this a sanity check for Qtopia 2
  qt_libs="libqt libqtopia"
  if test $gnash_qtopia_version -eq 2; then
    if test x"${ac_cv_path_qtopia_lib}" != x; then
      for i in $qt_libs; do
        if ! test -f  ${gnash_qtopia_topdir}/lib/$i.${shlibext}; then
          AC_MSG_WARN([$i not found!])
          qtopia_nogo=yes
        fi
      done
    fi
  fi
  
  dnl this a sanity check for Qtopia 4
  AC_MSG_CHECKING([Sanity checking the Qtopia installation])
  qt_libs="libqtopia libqtopiabase"
  if test $gnash_qtopia_version -eq 4; then
    if test x"${ac_cv_path_qtopia_lib}" != x; then
      for i in $qt_libs; do
        if ! test -f  ${gnash_qtopia_topdir}/lib/$i.${shlibext}; then
          AC_MSG_WARN([$i not found!])
          qtopia_nogo=yes
        fi
      done
    fi
  fi
  if test x"${qtopia_nogo}" = xyes; then
    AC_MSG_ERROR([Broken Qtopia library installation!])
  else
    AC_MSG_RESULT([fine])
  fi

  AC_DEFINE([QTOPIA_VERSION], [${gnash_qtopia_version}], [The Qtopia version])

  AC_MSG_CHECKING([for libqtopia library])
  AC_MSG_RESULT(${ac_cv_path_qtopia_lib})
  
  if test x"${ac_cv_path_qtopia_lib}" = x; then
    AC_CHECK_LIB(qtopia-${gnash_qtopia_version}, qtopia_engine_shape_class_init, [ac_cv_path_qtopia_lib="-lqtopia-${gnash_qtopia_version}"])
  fi

  if test x"${ac_cv_path_qtopia_incl}" != x; then
    QTOPIA_CFLAGS="${ac_cv_path_qtopia_incl}"
  else
    QTOPIA_CFLAGS=""
  fi

  if test x"${ac_cv_path_qtopia_lib}" != x; then
    QTOPIA_LIBS="${ac_cv_path_qtopia_lib}"
    AC_DEFINE(HAVE_QTOPIA, [1], [has the Qtopia framework])
    has_qtopia=yes
  else
    QTOPIA_LIBS=""
  fi

  AC_SUBST(QTOPIA_CFLAGS)
  AC_SUBST(QTOPIA_LIBS)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
