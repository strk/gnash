dnl Process this file with autoconf to produce a configure script.
dnl  
dnl    Copyright (C) 2005, 2006, 2009, 2010 Free Software Foundation, Inc.
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

dnl FIXME: This should really do tests for this data, and not
dnl just hardcode it based on the OS. This currently depends on
dnl AC_EXEEXT being called first.

AC_DEFUN([AM_COMPILER_LIB],
[dnl 
  AC_MSG_CHECKING(for library file name specifics)
  dnl These are the same for most platforms
  LIBEXT="a"
  LIBPRE="lib"

  if test "x$LIBPRE" != "x" ; then
    if test x"$EXEEXT"	= "exe"; then
      LIBPRE="lib"
    fi
  fi

  if test "x$LIBEXT" != "x" ; then
    if test x"$EXEEXT"	= "exe"; then
      LIBEXT="dll"
    fi
  fi

  if test "x$LIBPRE" != "x" -a  "x$LIBEXE" != "x"; then
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
  fi

AC_SUBST(LIBPRE)
AC_SUBST(LIBEXT)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
