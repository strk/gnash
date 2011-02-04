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
#	AC_PATH_MTASC
#
# Provides:
#	MTASC          	  - Path to mtasc executable
#	MTASC_CLASSPATH   - Classpath for mtasc
#

AC_DEFUN([AC_PATH_MTASC], [
  MTASC=""
  MTASC_CLASSPATH=""

  AC_ARG_WITH(mtasc, AC_HELP_STRING([--with-mtasc=<path>], [Path to the mtasc executable]), [
    case "${withval}" in
      yes|no) ;;
      *) MTASC=${withval} ;;
    esac
  ], MTASC="")

  AC_ARG_WITH(mtasc-classpath, AC_HELP_STRING([--with-mtasc-classpath=<path>], [CLASSPATH for mtasc]), [
    case "${withval}" in
      yes|no) ;;
      *) MTASC_CLASSPATH=${withval} ;;
    esac
  ], MTASC_CLASSPATH="")

  if test x"$MTASC" = "x"; then
    AC_PATH_PROG(MTASC, mtasc, ,[${pathlist}])
  fi

  if test x"$MTASC" != "x" -a x"$MTASC_CLASSPATH" = "x"; then
    # cross your fingers !
    if test -d "`dirname ${MTASC}`/std"; then
    dir="`dirname ${MTASC}`/std"
    elif test -d "/usr/share/mtasc/std"; then
    dir="/usr/share/mtasc/std"
    else
    dnl FIXME
    dir="/usr/share/ocaml/mtasc/std"
    fi
    MTASC_CLASSPATH=$dir
  fi


  AC_SUBST(MTASC)
  AC_SUBST(MTASC_CLASSPATH)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
