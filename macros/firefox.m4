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

dnl Configure paths for Firefox. We used to run firefox-config, but that
dnl got too messy. Then with a little experimentation we determined
dnl most of the options weren't actually needed... so now the handful
dnl of critical headers are part of the plugin/mozilla-sdk sources
dnl copied out of a current Firefox release. This greatly simplified
dnl both the configuration and compilation processes.
AC_DEFUN([GNASH_PATH_FIREFOX],
[dnl 

dnl !!
dnl !! Moved to configure.ac to allow builds where plugins don't work
dnl !!
dnl   AC_ARG_ENABLE(plugin, [  --disable-plugin         Enable support for being a plugin],
dnl   [case "${enableval}" in
dnl     yes) plugin=yes ;;
dnl     no)  plugin=no ;;
dnl     *)   AC_MSG_ERROR([bad value ${enableval} for disable-plugin option]) ;;
dnl   esac], plugin=yes)

  FIREFOX_PLUGINS=""
  if test x"${nsapi}" = x"yes"; then

    AC_ARG_WITH(plugindir, AC_HELP_STRING([--with-plugindir=DIR], [Directory to install Firefox plugin in]),
      [FIREFOX_PLUGINS=$withval]
    )

dnl  GNASHEXE=""
dnl     dnl Allow setting a path for the Gnash executable to be different from the prefix. This
dnl     dnl is mostly only used for cross compiling.
dnl     AC_ARG_WITH(gnashexe, AC_HELP_STRING([--with-gnashexe=DIR], [Directory to where the gnash executable is]),
dnl       [gnashbindir=$withval]
dnl     )
dnl 
dnl     dnl default to the prefix if no path is specified. As $prefix isn't set at this time by
dnl     dnl configure, we set this to the variable itself so it gets resolved at make time.
dnl     if test x"${gnashbindir}" = "x" ; then
dnl       GNASHEXE="\${prefix}/bin"
dnl     else
dnl       GNASHEXE=${gnashbindir}
dnl     fi
dnl  AC_SUBST(GNASHEXE)

    dnl Always install the plugin in the users home directory. We
    dnl always use .mozilla instead of .firefox, as this directoryis
    dnl used by all mozilla derived browsers.
    if test x"${FIREFOX_PLUGINS}" = "x" ; then
       FIREFOX_PLUGINS=$HOME/.mozilla/plugins
    fi
  fi

  AC_SUBST(FIREFOX_PLUGINS)

])dnl end of GNASH_PATH_FIREFOX
# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
