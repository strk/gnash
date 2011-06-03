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

dnl Configuing QT is a pain in the neck if you want to support all
dnl versions, as the layout of the API has changed significantly
dnl between Qt3 and Qt4. In Qt4, all the top level includes are in a
dnl 'Qt' subdirectory now. Since it's best for now to handle the path
dnl changes in the source files so we're concious of the changes, we
dnl set version flags in config.h and use Automake conditionals to
dnl force one behaviour or the other.

AC_DEFUN([GNASH_PATH_QT3],
[
  gnash_qt3_topdir=""
  has_qt3=no

  dnl Look for the header
  AC_ARG_WITH(qt3_incl, AC_HELP_STRING([--with-qt3-incl], [directory where QT 3.x headers are]), with_qt3_incl=${withval})

  AC_CACHE_VAL(ac_cv_path_qt3_incl,[
    if test x"${with_qt3_incl}" != x; then
      if test -f ${with_qt3_incl}/qobject.h; then
        ac_cv_path_qt3_incl="-I`(cd ${with_qt3_incl}; pwd)`"
      else
        AC_MSG_ERROR([${with_qt3_incl} directory doesn't contain any QT 3.x headers])
      fi
    fi
  ])				dnl end of cache ac_cv_path_qt4_incl

  dnl Only run these tests if this version was specified by the user, and they
  dnl haven't spcified a custom path.
  if test x"${ac_cv_path_qt3_incl}" = x; then
    dnl if QTDIR is set in the users environment, use that, as that's what
    dnl most QT programmers do, as it's required by the QT build system.
    qt_pkg="qt-mt"
    if test x$QTDIR != x; then
      if test -f $QTDIR/include/qobject.h; then
        qt_base=`basename $QTDIR`
        if test x"${qt_base}" = "qt4"; then
          AC_MSG_ERROR([Mismatched QT versions! Reset QTDIR in your environment!])
        else
          gnash_qt_version=3
        fi
      fi
    fi

    if test x$cross_compiling = xno; then
      if test x"$PKG_CONFIG" != x; then
        gnash_qt_version="`$PKG_CONFIG --modversion $qt_pkg | cut -d '.' -f 1`"
      fi
    fi

    AC_MSG_CHECKING([for QT 3.x header])
    if test x$cross_compiling = xno; then
      if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_qt3_incl}" = x; then
        $PKG_CONFIG --exists $qt_pkg && ac_cv_path_qt3_incl="`$PKG_CONFIG --cflags-only-I $qt_pkg | cut -d ' ' -f 1`"
      fi
    fi

    dnl QT 3,x stores it's headers in ../lib/qt-3.3/include for example, so
    dnl use libslist instead of incllist
    if test x"${ac_cv_path_qt3_incl}" = x; then
      for i in $QTDIR $libslist $incllist; do
        for j in `ls -dr $i/qt[[0-9]] 2>/dev/null`; do
          if test -f $j/include/qobject.h; then
            gnash_qt3_topdir="$j"
            ac_cv_path_qt3_incl="-I$j/include"
            break
          fi
        done
        if test "x$gnash_qt3_topdir" != x; then
          break;
        fi
      done
    else
      gnash_qt3_topdir="`echo ${ac_cv_path_qt3_incl} | sed -e 's:-I::' -e 's:/lib/qt.*::'`"
    fi

    if test x"${ac_cv_path_qt3_incl}" = x; then
      QT3_CFLAGS=""
      AC_MSG_RESULT(no)
    else
      QT3_CFLAGS="${ac_cv_path_qt3_incl}"
      AC_MSG_RESULT(${ac_cv_path_qt3_incl})
    fi

dnl   # QT_LIBS =  -lqtui -lqtcore -lqtprint -L/usr/lib/qt-3.3/lib -lqt-mt
  dnl Look for the libraries
    AC_ARG_WITH(qt3_lib, AC_HELP_STRING([--with-qt3-lib], [directory where QT 3.x libraries are]), with_qt3_lib=${withval})
    AC_CACHE_VAL(ac_cv_path_qt3_lib, [
      if test x"${with_qt3_lib}" != x ; then
        if test `ls -C1 ${with_qt3_lib}/lib/libqt*-mt.* | wc -l` -gt 0 ; then
         ac_cv_path_qt3_lib="-L`(cd ${with_qt3_lib}; pwd)` ${qt3support} -lqt-mt"
        else
	        AC_MSG_ERROR([${with_qt3_lib} directory doesn't contain QT 3.x libraries.])
        fi
      fi
    ])

    if test x$cross_compiling = xno; then
      if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_qt3_lib}" = x; then
        $PKG_CONFIG --exists $qt_pkg && ac_cv_path_qt3_lib="`$PKG_CONFIG --libs-only-l $qt_pkg | cut -d ' ' -f 1`"
      fi
    fi

    if test x"${ac_cv_path_qt3_lib}" = x; then
      AC_MSG_CHECKING([for QT 3.x libraries])
      if test `ls -C1 ${gnash_qt3_topdir}/lib/libqt*-mt.*| wc -l` -gt 0 ; then
        dnl Qtopia (embedded QT) uses libqte-mt instead of libqt-mt
        if test -f ${gnash_qt3_topdir}/lib/libqte-mt.${shlibext}; then
          ac_cv_path_qt3_lib="-L${gnash_qt3_topdir}/lib ${qt3support} -lqte-mt"
         else
          ac_cv_path_qt3_lib="-L${gnash_qt3_topdir}/lib ${qt3support} -lqt-mt"
        fi
      fi
      if test x"${ac_cv_path_qt3_lib}" != x; then
        AC_MSG_RESULT(${ac_cv_path_qt3_lib})
      else
        AC_MSG_RESULT(none)
      fi
    fi

    if test x"${ac_cv_path_qt3_lib}" != x; then
      AC_MSG_RESULT(${ac_cv_path_qt3_lib})
    else
      AC_MSG_RESULT(no)
    fi

    dnl Both the headers and the library must be installed
    if test x"${ac_cv_path_qt3_incl}" != x -a x"${ac_cv_path_qt3_lib}" != x; then
      AC_DEFINE(HAVE_QT3, 1, [Have QT 3.x installed])
      QT3_LIBS="${ac_cv_path_qt3_lib}"
      has_qt3="yes"
    else
      has_qt3="no"
      QT3_LIBS=""
    fi

    AC_PATH_PROG(MOC3, moc, ,[ /usr/lib/qt-3.3/bin ${QTDIR}/bin${pathlist}])

  fi                              dnl end of ${ac_cv_path_qt3_incl} empty

  AC_SUBST(MOC3)
  AC_SUBST(QT3_CFLAGS)  
  AC_SUBST(QT3_LIBS)
])


# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
