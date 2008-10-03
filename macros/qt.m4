dnl  
dnl    Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

AC_DEFUN([GNASH_PATH_QT],
[
  gnash_qt_topdir=""
  gnash_qt_version=0
  qt3support=""

  dnl Lool for the header
  AC_ARG_WITH(qt_incl, AC_HELP_STRING([--with-qt-incl], [directory where qt headers are]), with_qt_incl=${withval})
  AC_CACHE_VAL(ac_cv_path_qt_incl,[
    if test x"${with_qt_incl}" != x ; then
      if test -f ${with_qt_incl}/qobject.h ; then
        ac_cv_path_qt_incl="-I`(cd ${with_qt_incl}; pwd)`"
      else
        AC_MSG_ERROR([${with_qt_incl} directory doesn't contain any headers])
      fi
    fi
  ])

  dnl if QTDIR is set, don't use pkg-config, as it screws the flags
  dnl up. Default to Qt3 for now.
  qt_pkg="qt-mt"
  if test x$QTDIR != x; then
    if test -f $QTDIR/include/qobject.h; then
      qt_pkg="qt-mt"
      if test x`basename $QTDIR` = "qt4"; then
        gnash_qt_version=4
      else
        if test x`basename $QTDIR` = "qt3"; then
          gnash_qt_version=3
 	      else
          gnash_qt_version=2
        fi
      fi
    else
      dnl Look for qt4 first, if qt3 exists, we prefer that, so override this
      if test -f /usr/include/qt4/qobject.h -o -f /usr/include/Qt/qobject.h; then
        qt_pkg="QtCore"
        gnash_qt_version=4
      fi
      if test -f /usr/include/qt3/qobject.h; then
        qt_pkg="qt-mt"       
        gnash_qt_version=3
      fi
    fi
  fi

  if test x$cross_compiling = xno; then
    if test x"$PKG_CONFIG" != x; then
      gnash_qt_version="`$PKG_CONFIG --modversion $qt_pkg | cut -d '.' -f 1`"
    fi
  fi

  AC_MSG_CHECKING([for qt header])
  if test x$cross_compiling = xno; then
    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_qt_incl}" = x; then
      $PKG_CONFIG --exists $qt_pkg && ac_cv_path_qt_incl="`$PKG_CONFIG --cflags-only-I $qt_pkg | cut -d ' ' -f 1`"
    fi
  fi

  dnl QT stores it's headers in ../lib/qt-3.3/include for example, so
  dnl use libslist instead of incllist
  if test x"${ac_cv_path_qt_incl}" = x; then
    for i in $QTDIR $libslist $incllist; do
      for j in $QTDIR `ls -dr $i/qt[[0-9]] 2>/dev/null`; do
        if test -f $j/include/qobject.h; then
          gnash_qt_topdir="$j"
          if test "${gnash_qt_version} " -eq 0; then
            gnash_qt_version=2
	        fi
          ac_cv_path_qt_incl="-I$j/include"
          break
	      else
          if test -f $j/Qt/qobject.h; then
            gnash_qt_topdir="$j"
	    if test "${gnash_qt_version} " -eq 0; then
              gnash_qt_version=4
	    fi
            ac_cv_path_qt_incl="-I$j -I$j/Qt"
	  fi
        fi
      done
      if test "x$gnash_qt_topdir" != x; then
        break;
      fi
    done
  else
    gnash_qt_topdir="`echo ${ac_cv_path_qt_incl} | sed -e 's:-I::' -e 's:/lib/qt.*::'`"
  fi
dnl On Debian the dir is /usr/include/qt3 with /usr/lib/libqt-mt.*
dnl so we set topdir to /usr so that $topdir/lib/libqt-mt.* works below,
dnl and version to 3 or whatever.
dnl incllist is inherited from configure.ac.
  if test "x$gnash_qt_version" = x; then
    for i in  $QTDIR $incllist; do
      for j in  $QTDIR `ls -dr $i/qt[[0-9]] 2>/dev/null`; do
        if test -f $j/qobject.h -o -f $j/Qt/qobject.h; then
dnl          gnash_qt_version=`echo "$j" | sed "s:$i/qt::"`
          ac_cv_path_qt_incl="-I$j -I$j/Qt"
          break
        fi
      done
      if test x$gnash_qt_version != x; then
        break;
      fi
    done
  fi

  if test x"${ac_cv_path_qt_incl}" = x; then
    AC_MSG_RESULT(no)
  else
    AC_MSG_RESULT(${ac_cv_path_qt_incl})
  fi

  if test x"${ac_cv_path_qt_incl}" != x ; then
    QT_CFLAGS="${ac_cv_path_qt_incl}"
  else
    QT_CFLAGS=""
  fi

  dnl we have to define our own config constant for this, even though
  dnl QT_VERSION exists, the header path it's in changes, so this
  dnl seemed easier.
  if test x$gnash_qt_version != x; then
    AC_MSG_NOTICE([QT version is $gnash_qt_version])
    dnl due to a weird problem with variable expansion, we have to use
    dnl a case statement to set the QT version.
    case $gnash_qt_version in
      2)
	      AC_DEFINE([GNASH_QT_VERSION], 2, [The Qtopia version])
	      ;;
      3)
	      AC_DEFINE([GNASH_QT_VERSION], 3, [The Qtopia version])
	      ;;
      4)
	      AC_DEFINE([GNASH_QT_VERSION], 4, [The Qtopia version])
	      qt3support="-lqt3support"
	      ;;
    esac
  fi

dnl   # QT_LIBS =  -lqtui -lqtcore -lqtprint -L/usr/lib/qt-3.3/lib -lqt-mt
  dnl Look for the libraries
  AC_ARG_WITH(qt_lib, AC_HELP_STRING([--with-qt-lib], [directory where qt libraries are]), with_qt_lib=${withval})
  AC_CACHE_VAL(ac_cv_path_qt_lib, [
    if test x"${with_qt_lib}" != x ; then
      if test `ls -C1 ${gnash_qt_topdir}/lib/libqt*-mt.* | wc -l` -gt 0 ; then
       ac_cv_path_qt_lib="-L`(cd ${with_qt_lib}; pwd)` ${qt3support} -lqt-mt"
      else
	      AC_MSG_ERROR([${with_qt_lib} directory doesn't contain qt libraries.])
      fi
    fi
  ])

  if test x$cross_compiling = xno; then
    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_qt_lib}" = x; then
         $PKG_CONFIG --exists $qt_pkg && ac_cv_path_qt_lib="`$PKG_CONFIG --libs-only-l $qt_pkg | cut -d ' ' -f 1`"
    fi
  fi

  if test x"${ac_cv_path_qt_lib}" = x; then
    AC_MSG_CHECKING([for qt library])
    if test `ls -C1 ${gnash_qt_topdir}/lib/libqt*-mt.*| wc -l` -gt 0 ; then
      dnl Qtopia (embedded QT) uses libqte-mt instead of libqt-mt
      if test -f ${gnash_qt_topdir}/lib/libqte-mt.${shlibext}; then
        ac_cv_path_qt_lib="-L${gnash_qt_topdir}/lib ${qt3support} -lqte-mt"
       else
        ac_cv_path_qt_lib="-L${gnash_qt_topdir}/lib ${qt3support} -lqt-mt"
      fi
    fi
    if test x"${ac_cv_path_qt_lib}" != x; then
      AC_MSG_RESULT(${ac_cv_path_qt_lib})
    else
      AC_MSG_RESULT(none)
    fi
  fi

  if test x"${ac_cv_path_qt_lib}" != x; then
    QT_LIBS="${ac_cv_path_qt_lib}"
    AC_DEFINE(HAVE_QT,1,[Have QT installed])
    has_qt="yes"
  else
    QT_LIBS=""
    has_qt="no"
  fi

  AC_PATH_PROG(MOC, moc, ,[${QTDIR}/bin ${gnash_qt_topdir}/bin ${pathlist}])

  AC_SUBST(MOC)

  AC_SUBST(QT_CFLAGS)  
  AC_SUBST(QT_LIBS)
])


# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
