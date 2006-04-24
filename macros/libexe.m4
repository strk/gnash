dnl Process this file with autoconf to produce a configure script.
dnl  
dnl    Copyright (C) 2005, 2006 Free Software Foundation, Inc.
dnl  
dnl  This program is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU General Public License as published by
dnl  the Free Software Foundation; either version 2 of the License, or
dnl  (at your option) any later version.
dnl  
dnl  This program is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl  GNU General Public License for more details.
dnl  You should have received a copy of the GNU General Public License
dnl  along with this program; if not, write to the Free Software
dnl  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

dnl Linking Gnash statically or dynamically with other modules is making a
dnl combined work based on Gnash. Thus, the terms and conditions of the GNU
dnl General Public License cover the whole combination.
dnl
dnl As a special exception, the copyright holders of Gnash give you
dnl permission to combine Gnash with free software programs or libraries
dnl that are released under the GNU LGPL and with code included in any
dnl release of Talkback distributed by the Mozilla Foundation. You may
dnl copy and distribute such a system following the terms of the GNU GPL
dnl for all but the LGPL-covered parts and Talkback, and following the
dnl LGPL for the LGPL-covered parts.
dnl
dnl Note that people who make modified versions of Gnash are not obligated
dnl to grant this special exception for their modified versions; it is their
dnl choice whether to do so. The GNU General Public License gives permission
dnl to release a modified version without this exception; this exception
dnl also makes it possible to release a modified version which carries
dnl forward this exception.
dnl  
dnl 

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
	if test x"$EXEEXT"	== "exe"; then
	   LIBPRE="lib"
	fi
  fi

  if test "x$LIBEXT" != "x" ; then
	if test x"$EXEEXT"	== "exe"; then
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
