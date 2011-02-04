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


dnl ~{rob@ute} pts/8> QtCore  QtSvg Qtnetwork QtXml 
dnl QtCore: Command not found.
dnl ~{rob@ute} pts/8> -L/home/rob/projects/gnu/i686-pc-linux-gnulibc1/qtopia/lib -lQtSvg -L/home/rob/projects/gnu/i686-pc-linux-gnulibc1/qtopia/qtopiacore/target/lib -lQtSql -lQtXml -lQtGui -lpng -lQtNetwork -lQtCore -lz -lm -lrt -ldl -lpthread -lqtopiabase -lqtopia -lmd5 -lqtopia-sqlite -lqtopiasecurity -Wl,-rpath,/usr/local/qtopia-prefix/lib
dnl -L/home/rob/projects/gnu/i686-pc-linux-gnulibc1/qtopia/lib: Command not found.

AC_DEFUN([GNASH_PATH_QTOPIA],
[

  has_qtopia=no
  gnash_qtopia_version=0

  dnl the list of Qtopia headers we need
  dnl Look for the header
  AC_ARG_WITH(qtopia, AC_HELP_STRING([--with-qtopia], [directory where Qtopia is installed]), with_qtopia=${withval})
  AC_CACHE_VAL(ac_cv_path_qtopia_incl,[
    if test x"${with_qtopia}" != x ; then
      if test -f ${with_qtopia}/include/qtopiamail/qtopiamail.h ; then
        gnash_qtopia_topdir="${with_qtopia}"
        ac_cv_path_qtopia_incl="-I`(cd ${with_qtopia}/include; pwd)`"
        gnash_qtopia_version=4
      else
        gnash_qtopia_topdir="${with_qtopia}"
        if test -f ${with_qtopia}/include/qtopia/mail/qtopiamail.h ; then
          ac_cv_path_qtopia_incl="-I`(cd ${with_qtopia}/include; pwd)`"
          gnash_qtopia_version=2
        else
          AC_MSG_ERROR([${with_qtopia} directory doesn't contain Qtopia headers])
        fi
      fi
      if test -f ${with_qtopia}/lib/libqtopiamail.a -o -f ${with_qtopia}/lib/libqtopiamail.${shlibext}; then
	      ac_cv_path_qtopia_lib="-L`(cd ${with_qtopia}/lib; pwd)`"
      else
	      AC_MSG_ERROR([${with_qtopia}/lib directory doesn't contain Qtopia libraries])
      fi
    fi
  ])

  
  if test x"${QPEDIR}" != x; then
    gnash_qtopia_topdir=$QPEDIR
  else
    if test x"${gnash_qtopia_topdir}" = x; then
      AC_MSG_ERROR([QPEDIR must be set in your shell environment or use --with-qtopia=])
    fi
  fi

  if test x"${ac_cv_path_qtopia_incl}" = x; then
    dnl Attempt to find the top level directory, which unfortunately
    dnl has a x version number attached. At least on Debain based
    dnl systems, this doesn't seem to get a directory that is
    dnl unversioned. 
    if test x"${gnash_qtopia_version}" = x; then
      AC_MSG_CHECKING([for libqtopia header])
      if test -f ${gnash_qtopia_topdir}/include/qtopiamail/qtopiamail.h ; then
        ac_cv_path_qtopia_incl="-I`(cd ${gnash_qtopia_topdir}/include; pwd)`"
        gnash_qtopia_version=4
      else
        if test -f ${gnash_qtopia_topdir}/include/qtopia/mail/qtopiamail.h ; then
          ac_cv_path_qtopia_incl="-I`(cd ${gnash_qtopia_topdir}/include; pwd)`"
          gnash_qtopia_version=2
        else
          AC_MSG_ERROR([${gnash_qtopia_topdir} directory doesn't contain qtopia])
        fi
      fi
      AC_MSG_RESULT(${ac_cv_path_qtopia_incl}) 
    fi                          dnl end of gnash_qtopia_version
  fi                            dnl end of ac_cv_path_qtopia_incl

  AC_MSG_CHECKING([for the Qtopia Version])
  AC_MSG_RESULT(${gnash_qtopia_version}) 

  dnl this a sanity check for Qtopia 2
  AC_MSG_CHECKING([Sanity checking the Qtopia header installation])
  qt_headers="qmainwindow.h qmenubar.h qpopupmenu.h qapplication.h"
  if test ${gnash_qtopia_version} -eq 2; then
    if test x"${ac_cv_path_qtopia_incl}" != x; then
      for i in $qt_headers; do
        if ! test -f  ${gnash_qtopia_topdir}/include/$i; then
          AC_MSG_WARN([${gnash_qtopia_topdir}/include/$i not found!])
          qtopia_nogo=yes
        fi
      done
    fi
  fi
  
  dnl this a sanity check for Qtopia 4
  qt_headers="QtXml QtGui QtCore QtNetwork QtSql"
  if test ${gnash_qtopia_version} -eq 4; then
    if test x"${ac_cv_path_qtopia_incl}" != x; then
      for i in $qt_headers; do
        if ! test -d ${gnash_qtopia_topdir}/qtopiacore/target/include/$i; then
          AC_MSG_WARN([${gnash_qtopia_topdir}/qtopiacore/target/include/$i not found!])
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

  if test ${gnash_qtopia_version} -eq 0; then
    AC_MSG_RESULT(none)
  else
    AC_MSG_RESULT([${gnash_qtopia_version}])
  fi
  
  dnl the list of Qtopia libs we need
  qt_libs="qtopiabase qtopia qpe qt"

  if test x"${ac_cv_path_qtopia_lib}" = x; then
    if test x"${ac_cv_path_qtopia_lib}" = x; then
      AC_MSG_CHECKING([for libqtopia library])
      if test -f $gnash_qtopia_topdir/lib/libqpe.a -o -f $gnash_qtopia_topdir/lib/libqpe.${shlibext}; then
        if test ! x"$i" = x"/usr/lib" -a ! x"$i" = x"/usr/lib64"; then
         ac_cv_path_qtopia_lib="-L$gnash_qtopia_topdir/lib -lqpe"
         break
        else
         ac_cv_path_qtopia_lib="-lqpe"
         break
       fi
      fi
    fi
    AC_MSG_RESULT(${ac_cv_path_qtopia_lib})
	fi
  
  AC_MSG_CHECKING([Sanity checking the Qtopia installation])
  dnl this a sanity check for Qtopia 2
  qt_libs="libqtopia libqpe"
  if test ${gnash_qtopia_version} -eq 2; then
    AC_DEFINE([GNASH_QTOPIA_VERSION], 2, [The Qtopia version])
    if test x"${ac_cv_path_qtopia_lib}" != x; then
      for i in $qt_libs; do
        if ! test -f  ${gnash_qtopia_topdir}/lib/$i.${shlibext}; then
          AC_MSG_WARN([${gnash_qtopia_topdir}/lib/$i.${shlibext} not found!])
          qtopia_nogo=yes
        fi
      done
    fi
  fi
  
  dnl this a sanity check for Qtopia 4
  qt_libs="libqtopia libqtopiabase"
  if test ${gnash_qtopia_version} -eq 4; then
    AC_DEFINE([GNASH_QTOPIA_VERSION], 4, [The Qtopia version])
    gnash_qtopia_version=`dirname ${gnash_qtopia_topdir}`
    if test x"${ac_cv_path_qtopia_lib}" != x; then
      for i in $qt_libs; do
        if ! test -f  ${gnash_qtopia_topdir}/lib/$i.${shlibext}; then
          AC_MSG_WARN([${gnash_qtopia_topdir}/lib/$i${shlibext} not found!])
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

  if test x"${ac_cv_path_qtopia_lib}" = x; then
    AC_CHECK_LIB(qtopia-${gnash_qtopia_version}, qtopia_engine_shape_class_init, [ac_cv_path_qtopia_lib="-lqtopia"])
  fi

  if test x"${ac_cv_path_qtopia_incl}" != x; then
    QTOPIA_CFLAGS="${ac_cv_path_qtopia_incl}"
  else
    QTOPIA_CFLAGS=""
  fi

  if test x"${ac_cv_path_qtopia_lib}" != x; then
    QTOPIA_LIBS="${ac_cv_path_qtopia_lib}"
    AC_DEFINE(HAVE_QTOPIA, 1, [has the Qtopia framework])
    has_qtopia="yes"
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
