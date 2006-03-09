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
dnl  
dnl  Linking Gnash statically or dynamically with other modules is making
dnl  a combined work based on Gnash. Thus, the terms and conditions of
dnl  the GNU General Public License cover the whole combination.
dnl  
dnl  In addition, as a special exception, the copyright holders of Gnash give
dnl  you permission to combine Gnash with free software programs or
dnl  libraries that are released under the GNU LGPL and/or with Mozilla, 
dnl  so long as the linking with Mozilla, or any variant of Mozilla, is
dnl  through its standard plug-in interface. You may copy and distribute
dnl  such a system following the terms of the GNU GPL for Gnash and the
dnl  licenses of the other code concerned, provided that you include the
dnl  source code of that other code when and as the GNU GPL requires
dnl  distribution of source code. 
dnl  
dnl  Note that people who make modified versions of Gnash are not obligated
dnl  to grant this special exception for their modified versions; it is
dnl  their choice whether to do so.  The GNU General Public License gives
dnl  permission to release a modified version without this exception; this
dnl  exception also makes it possible to release a modified version which
dnl  carries forward this exception.
dnl 

# Configure paths for Ming
# Sandro Santilli 2006-01-24
#
# This macro uses ming-config, which was
# not available as for Ming 0.3beta1
#
#
#
# Use: 
#	AC_PATH_MING
#
# Provides:
#	MING_CFLAGS
#	MING_LIBS
#	MAKESWF
#

AC_DEFUN([GNASH_PATH_MING],
[
	MING_CFLAGS=""
	MING_LIBS=""

	AC_ARG_WITH(ming,[  --with-ming=[<ming-config>]    Use ming to build tests],
		[
		case "${withval}" in
			yes|no)
				;;
			*) MING_CONFIG=${withval}
				;;
		esac
		], MING_CONFIG="")

	if test x"$MING_CONFIG" = "x"; then
		AC_PATH_PROG(MING_CONFIG, ming-config)
	fi

	if test x"$MING_CONFIG" != "x"; then
		MING_CFLAGS=`$MING_CONFIG --cflags`
		MING_LIBS=`$MING_CONFIG --libs`
		MING_PATH=`$MING_CONFIG --bindir`
		AC_PATH_PROG([MAKESWF], [makeswf], , [$PATH:$MING_PATH])
	fi


	AC_SUBST(MING_CFLAGS)
	AC_SUBST(MING_LIBS)
	AC_SUBST(MAKESWF)
])
