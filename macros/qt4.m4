dnl  
dnl Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
dnl 2011 Free Software Foundation, Inc.
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

AC_DEFUN([GNASH_PATH_QT4], [
  gnash_qt4_topdir=

  dnl Look for the header
  AC_ARG_WITH(qt4_incl, AC_HELP_STRING([--with-qt4-incl],
             [directory where QT 4.x headers are]),
	     with_qt4_incl=${withval})

  AC_CACHE_VAL(ac_cv_path_qt4_incl,[
    if test x"${with_qt4_incl}" != x; then
      if test -f ${with_qt4_incl}/QtCore/qobject.h; then
        qt4_include_subdirs="QtCore QtGui QtOpenGL"

        ac_cv_path_qt4_incl="${ac_cv_path_qt4_incl} -I`(cd ${with_qt4_incl}; pwd)`"
        for i in ${qt4_include_subdirs}; do
          if test -d "${with_qt4_incl}/$i"; then
            ac_cv_path_qt4_incl="${ac_cv_path_qt4_incl} -I`(cd ${with_qt4_incl}\$i; pwd)`"
          fi
        done
      else
        AC_MSG_ERROR([${with_qt4_incl}/QtCore directory doesn't contain QT 4.x headers])
      fi
    fi
  ])				dnl end of cache ac_cv_path_qt4_incl

  dnl Only run these tests if this version was specified by the user.

    if test x"${ac_cv_path_qt4_incl}" = x; then
      dnl if QTDIR is set in the users environment, use that, as that's what
      dnl most QT programmers do, as it's required by the QT build system.
      if test x$QTDIR != x; then
        if test -f $QTDIR/include/QtCore/qobject.h; then
          qt_base=`basename $QTDIR`
          dnl Only support version 4 in this file.
          if test x"${qt_base}" != x"qt4"; then
            AC_MSG_ERROR([Mismatched QT versions! Reset QTDIR in your environment!])
          fi
        fi
      fi
    fi

dnl FIXME: do we even need the version anymore ?
dnl     if test x"${cross_compiling} = xno; then
dnl       if test x"${PKG_CONFIG}" != x; x1then
dnl         gnash_qt_version="`${PKG_CONFIG} --modversion QtCore | cut -d '.' -f 1`"
dnl       fi
dnl     fi

     AC_MSG_CHECKING([for QT 4.x headers])
    if test x$cross_compiling = xno; then
      if test x"${PKG_CONFIG}" != x -a x"${ac_cv_path_qt4_incl}" = x; then
        for i in QtCore QtGui QtOpenGL; do
          if ${PKG_CONFIG} --exists $i; then
            ac_cv_path_qt4_incl="`$PKG_CONFIG --cflags-only-I $i`"
          fi
        done
	gnash_qt4_topdir=`echo "${ac_cv_path_qt4_incl}"`
      fi
    fi

    dnl QT 4.x stores it's headers in ../lib/qt4/include for example, so
    dnl use libslist instead of incllist
    if test x"${ac_cv_path_qt4_incl}" = x; then
      for i in ${incllist} ${libslist}; do
        dnl Some distributions put the QT headers directly in the directory
        if test -f $i/Qt/qobject.h; then
          gnash_qt4_topdir="$i"
	  if test x"$i" != x"/usr/include"; then
            ac_cv_path_qt4_incl="-I$i"
	  fi
          break
        fi
	dnl Some distributions put the QT headers in a sub directory
        if test -f $i/qt4/Qt/qobject.h; then
          gnash_qt4_topdir="$i/qt4"
          ac_cv_path_qt4_incl="-I$i/qt4"
          break
        fi
      done
    fi

    dnl this is a list of al the libraries supported by QT 4.x, but we don't need all of
    dnl then, but we might as well get all the paths, as header files ofteninclude other
    dnl header files.
    all_qt4_libs="QtCore QtGui QtOpenGL QtXml QtDBus QtNetwork QtScript QtSql QtTest QtSvg QtWebKit"
    if test x"${ac_cv_path_qt4_incl}" = x; then
      for i in ${all_qt4_libs}; do
        dnl Darwin is easy, everything is in the same location on all machines.
        if test x"${darwin}" = xyes; then
          if test -d /Library/Frameworks; then
            ac_cv_path_qt4_incl="${ac_cv_path_qt4_incl} /Library/Frameworks/$i.framework/Headers"
          fi
        else
          if test -d ${gnash_qt4_topdir}/$i; then
            ac_cv_path_qt4_incl="${ac_cv_path_qt4_incl} -I${gnash_qt4_topdir}/$i"
          fi
        fi
      done
    fi
    if test x"${ac_cv_path_qt4_incl}" = x; then
      QT4_CFLAGS=""
      AC_MSG_RESULT(no)
    else
      QT4_CFLAGS="${ac_cv_path_qt4_incl}"
      AC_MSG_RESULT(${gnash_qt4_topdir})
    fi

dnl   if test "$gnash_cv_lib_qt_dir" = "no"; then
dnl     dnl We might be on OSX...
dnl     if test -d /Library/Frameworks/QtCore.framework; then
dnl       gnash_cv_lib_qt_dir=/Library/Frameworks
dnl       KDE_LIBS="-L/Library/Frameworks -F/Library/Frameworks"
dnl     fi
dnl   fi

  dnl Look for the libraries
    AC_ARG_WITH(qt4_lib, AC_HELP_STRING([--with-qt4-lib], [directory where QT 4.x libraries are]), with_qt4_lib=${withval})
    AC_CACHE_VAL(ac_cv_path_qt4_lib, [
      if test x"${with_qt4_lib}" != x ; then
        if test `ls -C1 ${with_qt4_lib}/libQtCore.${shlibext}* | wc -l` -gt 0 ; then
         ac_cv_path_qt4_lib="-L`(cd ${with_qt4_lib}; pwd)` ${qt4support} -lQtCore -lQtGui"
        else
          AC_MSG_ERROR([${with_qt4_lib} directory doesn't contain QT 4.x libraries.])
        fi
      fi
    ])

    if test x$cross_compiling = xno; then
      if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_qt4_lib}" = x; then
        $PKG_CONFIG --exists QtCore QtGui && ac_cv_path_qt4_lib="`$PKG_CONFIG --libs QtCore QtGui`"
      fi
    fi

    AC_MSG_CHECKING([for QT 4.x libraries])
    if test x"${ac_cv_path_qt4_lib}" = x; then
      for i in $libslist; do
        if test -f $i/libQtCore.${shlibext}; then
	  gnash_qt4_topdir=$i
	  if test x"$i" != x"/usr/lib"; then
	    ac_cv_path_qt4_lib="-L$i"
	  fi
	  break
	fi
      done
      dnl redefine the list of libraries, as we don't need all of them, so why bother.
      all_qt4_libs="QtCore QtGui"
      for i in ${all_qt4_libs}; do
        if test -f ${gnash_qt4_topdir}/lib$i.${shlibext}; then
	    ac_cv_path_qt4_lib="${ac_cv_path_qt4_lib} -l$i"
	fi
      done
    fi

    if test x"${ac_cv_path_qt4_lib}" != x; then
      AC_MSG_RESULT(${ac_cv_path_qt4_lib})
    else
      AC_MSG_RESULT(no)
    fi

    dnl Both the headers and the library must be installed
    if test x"${ac_cv_path_qt4_incl}" != x -a x"${ac_cv_path_qt4_lib}" != x; then
      AC_DEFINE(HAVE_QT4, 1, [Have QT 4.x installed])
      QT4_LIBS="${ac_cv_path_qt4_lib}"
      has_qt4="yes"
    else
      has_qt4="no"
      QT4_LIBS=""
    fi

    AC_PATH_PROGS(MOC4, [moc-qt4 moc moc4], ,[/usr/bin ${QTDIR}/bin /usr/lib/qt4/bin /usr/share/qt4/bin/ ${pathlist}])
    AC_PATH_PROGS(UIC4, [uic-qt4 uic uic4], ,[/usr/bin ${QTDIR}/bin /usr/lib/qt4/bin /usr/share/qt4/bin/ ${pathlist}])


    AC_SUBST([QT4_CFLAGS])
    AC_SUBST([QT4_LIBS])
    AC_SUBST([MOC4])
    AC_SUBST([UIC4])
])				dnl end of defun GNASH_PATH_QT4

dnl dnl Find the QT 4.x libQtCore library
dnl AC_DEFUN([GNASH_QT4_CORE], [
dnl   AC_REQUIRE([GNASH_PATH_QT4])
dnl   if test x"$gnash_qt4_topdir" = x"/Library/Frameworks"; then
dnl     QT_CFLAGS="${QT_CFLAGS} -D_REENTRANT -DQT_SHARED ${gnash_qt4_topdir}/QtCore.framework/Headers"
dnl     QTCORE="-framework QtCore"
dnl   else
dnl     QT_CFLAGS="${QT_CFLAGS} -D_REENTRANT -DQT_SHARED -I${gnash_qt4_topdir}/QtCore"
dnl     QT_LIBS="$QT_LIBS -lQtCore"
dnl   fi
dnl   if test "$enable_qt_debug" = "no"; then
dnl     KDE_CFLAGS="$CPPFLAGS -DQT_NO_DEBUG"
dnl   fi
dnl   AC_SUBST(QTCORE)
dnl ])

dnl FIXME: I don't think we need this.
dnl AC_DEFUN([GNASH_QT_PLATFORM], [
dnl 	AC_REQUIRE([GNASH_QT4_CORE])
dnl 	AC_MSG_CHECKING([Qt platform])
dnl 	platform=unknown
dnl 	AC_COMPILE_IFELSE([
dnl 		#include <qglobal.h>
dnl 		#ifndef Q_WS_X11
dnl 		#error Not X11
dnl 		#endif
dnl 	], [platform=X11], [])
dnl 	if test "$platform" = "unknown"; then
dnl 		AC_COMPILE_IFELSE([
dnl 			#include <qglobal.h>
dnl 			#ifndef Q_WS_QWS
dnl 			#error Not Qtopia
dnl 			#endif
dnl 		], [platform=Qtopia], [])
dnl 	fi
dnl 	if test "$platform" = "unknown"; then
dnl 		AC_COMPILE_IFELSE([
dnl 			#include <qglobal.h>
dnl 			#ifndef Q_WS_MACX
dnl 			#error Not OSX
dnl 			#endif
dnl 		], [platform=OSX], [])
dnl 	fi
dnl 	if test "$platform" = "unknown"; then
dnl 		AC_COMPILE_IFELSE([
dnl 			#include <qglobal.h>
dnl 			#ifndef Q_WS_MAC9
dnl 			#error Not OS9
dnl 			#endif
dnl 		], [platform=OS9], [])
dnl 	fi
dnl 	if test "$platform" = "unknown"; then
dnl 		AC_COMPILE_IFELSE([
dnl 			#include <qglobal.h>
dnl 			#ifndef Q_WS_WIN32
dnl 			#error No dirty Nazi junk
dnl 			#endif
dnl 		], [platform=Win32], [])
dnl 	fi
dnl 	AC_MSG_RESULT([$platform])
dnl ])
