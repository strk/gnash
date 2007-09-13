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

AC_DEFUN([GNASH_PATH_FREETYPE2],
[
  dnl Look for the header
  AC_ARG_WITH(freetype_incl, AC_HELP_STRING([--with-freetype-incl], [directory where libfreetype header is (w/out the freetype/ prefix)]), with_freetype_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_freetype_incl,[
    if test x"${with_freetype_incl}" != x ; then
      if test -f ${with_freetype_incl}/freetype/freetype.h ; then
	      ac_cv_path_freetype_incl=`(cd ${with_freetype_incl}; pwd)`
      else
	      AC_MSG_ERROR([${with_freetype_incl} directory doesn't contain freetype/freetype.h])
      fi
    fi
  ])

  if test x$cross_compiling = xno; then
    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_freetype_incl}" = x; then
      $PKG_CONFIG --exists freetype2 && ac_cv_path_freetype_incl=`$PKG_CONFIG --cflags freetype2`
    fi
  fi

  dnl If the path hasn't been specified, go look for it.
  if test x"${ac_cv_path_freetype_incl}" = x; then
    for i in $incllist; do
      if test -f $i/freetype2/freetype/freetype.h; then
        ac_cv_path_freetype_incl="-I$i/freetype2"
	      break
      fi
    done

    if test x"${ac_cv_path_freetype_incl}" = x ; then
      AC_CHECK_HEADERS(freetype2/freetype/freetype.h)
    fi

    AC_MSG_CHECKING([for libfreetype header])
    if test x"${ac_cv_path_freetype_incl}" != x ; then
      AC_MSG_RESULT(yes)
    else
      AC_MSG_RESULT(no)
    fi
  fi


  if test x"${ac_cv_path_freetype_incl}" != x"/usr/include"; then
    ac_cv_path_freetype_incl="${ac_cv_path_freetype_incl}"
  else
    ac_cv_path_freetype_incl=""
  fi

dnl   if test x"${darwin}" = xyes; then
     libname=freetype
dnl   else
dnl     libname=freetype2
dnl   fi
  AC_ARG_WITH(freetype_lib, AC_HELP_STRING([--with-freetype-lib], [directory where freetype library is]), with_freetype_lib=${withval})
    AC_CACHE_VAL(ac_cv_path_freetype_lib,[
    if test x"${with_freetype_lib}" != x ; then
      if test -f ${with_freetype_lib}/lib${libname}.a -o -f ${with_freetype_lib}/lib${libname}.${shlibext}; then
        ac_cv_path_freetype_lib="-L`(cd ${with_freetype_lib}; pwd)` -l${libname}"
      else
        AC_MSG_ERROR([${with_freetype_lib} directory doesn't contain libfreetype.])
      fi
    fi
  ])

  dnl Look for the library
  if test x$cross_compiling = xno; then
    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_freetype_lib}" = x; then
      $PKG_CONFIG --exists freetype2 && ac_cv_path_freetype_lib=`$PKG_CONFIG --libs freetype2`
    fi
  fi

  dnl If the path hasn't been specified, go look for it.
  if test x"${ac_cv_path_freetype_lib}" = x; then
    dnl freetype-config gives us way to many libraries, which create nasty linking
    dnl dependancy issue, so we strip them off here. The real dependencies are
    dnl are taken care of by other config tests.
    AC_MSG_CHECKING([for ${libname} library])
    for i in $libslist; do
      if test -f $i/lib${libname}.a -o -f $i/lib${libname}.${shlibext}; then
        if test x"$i" != x"/usr/lib"; then
          ac_cv_path_freetype_lib="-L$i -l${libname}"
          AC_MSG_RESULT(${ac_cv_path_freetype_lib})
          break
        else
          ac_cv_path_freetype_lib="-l${libname}"
          AC_MSG_RESULT(yes)
          break
        fi
      fi
    done
  fi

  if test x"${ac_cv_path_freetype_incl}" != x ; then
    FREETYPE2_CFLAGS="${ac_cv_path_freetype_incl}"
  else
    FREETYPE2_CFLAGS=""
  fi

  if test x"${ac_cv_path_freetype_lib}" != x ; then
    FREETYPE2_LIBS="${ac_cv_path_freetype_lib}"
  else
    FREETYPE2_LIBS=""
  fi

  AM_CONDITIONAL(FREETYPE, [test -n "$FREETYPE_LIBS"])

  if test -n "$FREETYPE2_LIBS"; then
    AC_DEFINE(USE_FREETYPE, [1], [Define this if you want to enable freetype usage])
  fi

  AC_SUBST(FREETYPE2_CFLAGS)
  AC_SUBST(FREETYPE2_LIBS)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
