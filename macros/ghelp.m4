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

AC_DEFUN([GNASH_PATH_GHELP],
[
  AC_ARG_ENABLE(ghelp, [  --enable-ghelp            Enable support for the GNOME help system],
  [case "${enableval}" in
    yes) ghelp=yes ;;
    no)  ghelp=no ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for enable-ghelp option]) ;;
  esac], ghelp=no)

  if test x"$ghelp" = x"yes" ; then
    AC_PATH_PROG(SCROLLKEEPER, scrollkeeper-config, [],
	[$PATH:/usr/bin/X11:/usr/local/bin/X11:/opt/X11])
    AC_PATH_PROG(SCROLLUPDATE, scrollkeeper-update, [],
	[$PATH:/usr/bin/X11:/usr/local/bin/X11:/opt/X11])
    AC_PATH_PROG(SCROLLINSTALL, scrollkeeper-preinstall, [],
	[$PATH:/usr/bin/X11:/usr/local/bin/X11:/opt/X11])
    if test x"${SCROLLKEEPER}" != x"" ; then
      AC_MSG_CHECKING([the path to install under scrollkeeper])
      ghelp_install=`$SCROLLKEEPER --prefix`
      AC_MSG_RESULT(${ghelp_install})
      if test x"${ghelp_install}" = x"/usr"; then
	AC_MSG_WARN([You will need to be root to install under scrollkeeper])
      fi
    fi
  fi
  if test x"$SCROLLKEEPER" = x -o x"$SCROLLUPDATE" = x -o x"$SCROLLINSTALL" = x ; then
    ghelp=no
    AC_MSG_WARN([You need to install scrollkeeper for gnome help])
  fi
])
