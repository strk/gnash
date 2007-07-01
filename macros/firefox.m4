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
  GNASHEXE=""
  if test x"${plugin}" = x"yes"; then

    AC_ARG_WITH(plugindir, AC_HELP_STRING([--with-plugindir=DIR], [Directory to install Firefox plugin in]),
      [FIREFOX_PLUGINS=$withval]
    )

    dnl Allow setting a path for the Gnash executable to be different from the prefix. This
    dnl is mostly only used for cross compiling.
    AC_ARG_WITH(gnashexe, AC_HELP_STRING([--with-gnashexe=DIR], [Directory to where the gnash executable is]),
      [gnashbindir=$withval]
    )

    dnl default to the prefix if no path is specified. As $prefix isn't set at this time by
    dnl configure, we set this to the variable itself so it gets resolved at make time.
    if test x"${gnashbindir}" = "x" ; then
      GNASHEXE="\${prefix}/bin"
    else
      GNASHEXE=${gnashbindir}
    fi
    dnl Always install the plugin in the users home directory.
    if test x"${FIREFOX_PLUGINS}" = "x" ; then
      if test -d $HOME/.mozilla/plugins; then
        FIREFOX_PLUGINS=$HOME/.mozilla/plugins
      else
        FIREFOX_PLUGINS=$HOME/.firefox/plugins
      fi
    fi
  fi

  AC_SUBST(GNASHEXE)
  AC_SUBST(FIREFOX_PLUGINS)

])dnl end of GNASH_PATH_FIREFOX

dnl This is the old version which we're keeping around for now. It
dnl would be very useful if the plugin does develop more of a
dnl dependancy on the mozilla development package.
AC_DEFUN([GNASH_PATH_FIREFOX_FULL],
[dnl 
dnl Get the cflags and libraries
dnl
dnl This enables or disables the support to make Gnash function as a
dnl Mozilla or Firefox plugin.
  AC_ARG_ENABLE(plugin, AC_HELP_STRING([--enable-plugin-full], [Enable
support for being a plugin using Firefox development packages]),
  [case "${enableval}" in
    yes) plugin=yes ;;
    no)  plugin=no ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for disable-plugin option]) ;;
  esac], plugin=yes)

  if test x"$plugin" = x"yes"; then
    AC_DEFINE([PLUGIN_SUPPORT], [], [Build plugin support for Mozilla/Firefox])

    AC_ARG_WITH(firefox, AC_HELP_STRING([--with-firefox=PFX], [Prefix where firefox is installed (optional)]), 
      [firefox_prefix=$withval], [firefox_prefix=""]
    )
    AC_ARG_WITH(firefox-libraries, AC_HELP_STRING([--with-firefox-libraries=DIR], [Directory where firefox library is installed (optional)]), 
      [firefox_libraries=$withval], [firefox_libraries=""]
    )
    AC_ARG_WITH(firefox-includes,AC_HELP_STRING([--with-firefox-includes=DIR], [Directory where firefox header files are installed (optional)]), 
      [firefox_includes=$withval], [firefox_includes=""]
    )
    AC_ARG_WITH(plugin-dir, AC_HELP_STRING([--with-plugin-dir=DIR], [Mozilla plugin dir]),
      [FIREFOX_PLUGINS=$withval]
    )
 
    if test "x${firefox_libraries}" != "x" ; then
      FIREFOX_LIBS="-L$firefox_libraries"
    elif test "x${firefox_prefix}" != "x" ; then
      FIREFOX_LIBS="-L${firefox_prefix}/lib"
    fi

    if test "x$firefox_includes" != "x" ; then
      FIREFOX_CFLAGS="-I$firefox_includes"
    elif test "x$firefox_prefix" != "x" ; then
      FIREFOX_CFLAGS="-I$firefox_prefix/include"
    fi

    no_firefox=""
    mconfig=""

    AC_CHECK_PROG(mconfig, firefox-config, firefox-config)

    if test x"${mconfig}" = "x" ; then
      AC_CHECK_PROG(mconfig, mozilla-config, mozilla-config)
    fi

    if test x"${mconfig}" = "x" ; then
      plugin="no"
      FIREFOX_CFLAGS=""
      FIREFOX_LIBS=""
      FIREFOX_DEFS=""
    else
      AC_MSG_CHECKING([for Firefox/Mozilla SDK])
      if test "x${FIREFOX_CFLAGS}" = "x" ; then
dnl -I/usr/include/mozilla/java
dnl -I/usr/include/mozilla/plugin 
dnl -I/usr/include/mozilla/nspr
dnl -I/usr/include/mozilla
        FIREFOX_CFLAGS=`${mconfig} --cflags java plugin`
      fi

      if test "x${FIREFOX_LIBS}" = "x" ; then
dnl -L/usr/lib/mozilla
        FIREFOX_LIBS=`${mconfig} --libs java plugin`
      fi

      if test "x${FIREFOX_LIBS}" != "x" ; then
dnl -DMOZ_X11=1 is all that's really needed
        FIREFOX_DEFS=`${mconfig} --defines java plugin`
dnl   if we don't have a path for the plugin by now, pick a default one
        if test x"${FIREFOX_PLUGINS}" = "x" ; then
	        FIREFOX_PLUGINS=`echo ${FIREFOX_LIBS} | sed -e 's:-L\(@<:@^ @:>@*\) .*$:\1:' -e  's:^-L::'`/plugins
        fi
      fi
    fi

    if test x"${FIREFOX_CFLAGS}" != "x" -a  x"${FIREFOX_LIBS}" != "x"; then
      AC_MSG_RESULT(yes)
      AC_DEFINE(HAVE_FIREFOX,1,[Define this if you have firefox support available])
    fi
  fi

  AM_CONDITIONAL(PLUGIN, [test x$plugin = xyes])

  AC_SUBST(FIREFOX_CFLAGS)
  AC_SUBST(FIREFOX_LIBS)
  AC_SUBST(FIREFOX_DEFS)
  AC_SUBST(FIREFOX_PLUGINS)
])
dnl end of GNASH_PATH_FIREFO

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
