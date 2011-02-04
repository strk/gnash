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
dnl


#
# Use: 
#	AC_PATH_HAXE
#
# Provides:
#	HAXE          	 - Path to haxe executable
#	HAXE_CLASSPATH   - Classpath for haxe
#

AC_DEFUN([AC_PATH_HAXE], [
  HAXE=""
  HAXE_CLASSPATH=""

  AC_ARG_WITH(haxe, AC_HELP_STRING([--with-haxe=<path>], [Path to the haxe executable]), [
    case "${withval}" in
      yes|no) ;;
      *) HAXE=${withval} ;;
    esac
  ], HAXE="")

  AC_ARG_WITH(haxe-classpath, AC_HELP_STRING([--with-haxe-classpath=<path>], [CLASSPATH for haxe]), [
    case "${withval}" in
      yes|no) ;;
      *) HAXE_CLASSPATH=${withval} ;;
    esac
  ], HAXE_CLASSPATH="")

  if test x"$HAXE" = "x"; then
    AC_PATH_PROG(HAXE, haxe, ,[${pathlist}])
  fi

  if test x"$HAXE" != "x"; then
    AC_MSG_CHECKING([for HAXE version])
    HAXE_VERSION=`$HAXE -help | grep -i ^haxe | awk '{print $'3'}'`
    AC_MSG_RESULT([${HAXE_VERSION}])

    major=`echo $HAXE_VERSION | cut -d '.' -f 1`
    minor=`echo $HAXE_VERSION | cut -d '.' -f 2`

    dnl
    dnl we need 2.00 or higher
    dnl 1.19 was tested as failing to build some testcases
    dnl preventing 'make check' from completing
    dnl
    if test $major -lt 2; then
        AC_MSG_WARN([Haxe ${HAXE_VERSION} is too old to be used])
        unset HAXE
    fi
  fi

  if test x"$HAXE" != "x" -a x"$HAXE_CLASSPATH" = "x"; then
    # cross your fingers !
    if test -d "`dirname ${HAXE}`/std"; then
    dir="`dirname ${HAXE}`/std"
    elif test -d "/usr/share/haxe"; then
    dir="/usr/share/haxe/"
    else
    dnl FIXME
    dir="/usr/share/haxe/"
    fi
    HAXE_CLASSPATH=$dir
  fi


  AC_SUBST(HAXE)
  AC_SUBST(HAXE_CLASSPATH)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
