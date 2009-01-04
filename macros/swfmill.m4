dnl
dnl    Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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
#	AC_PATH_SWFMILL
#
# Provides:
#	SWFMILL   - Path to swfmill executable
#

AC_DEFUN([AC_PATH_SWFMILL], [
  SWFMILL=""

  AC_ARG_WITH(swfmill, AC_HELP_STRING([--with-swfmill=<path>], [Path to the swfmill executable]), [
    case "${withval}" in
      yes|no) ;;
      *) SWFMILL=${withval} ;;
    esac
  ], SWFMILL="")

  if test x"$SWFMILL" = "x"; then
    AC_PATH_PROG(SWFMILL, swfmill, ,[${pathlist}])
  fi

  AC_SUBST(HAXE)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
