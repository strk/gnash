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

dnl Configure paths for Firefox. We used to run firefox-config, but that
dnl got too messy. Then with a little experimentation we determined
dnl most of the options weren't actually needed... so now the handful
dnl of critical headers are part of the plugin/mozilla-sdk sources
dnl copied out of a current Firefox release. This greatly simplified
dnl both the configuration and compilation processes.
AC_DEFUN([GNASH_PATH_FIREFOX],
[dnl 

  FIREFOX_PLUGINS=""
  if test x"${npapi}" = x"yes"; then

  AC_ARG_WITH(npapi-install,
    AC_HELP_STRING([--with-npapi-install=system|user|prefix], [Policy for NPAPI plugin install. Default: user.]),
      [case "${withval}" in
  	  user) NPAPI_INSTALL_POLICY=user ;;
  	  system) NPAPI_INSTALL_POLICY=system ;;
  	  prefix) NPAPI_INSTALL_POLICY=prefix ;;
  	  *)  AC_MSG_ERROR([bad value ${withval} for --with-npapi-install]) ;;
  	 esac 
      ], NPAPI_INSTALL_POLICY=${PLUGINS_INSTALL_POLICY}) dnl Inherit a generic PLUGINS_INSTALL_POLICY when available


  dnl For backward compatibility, won't be advertised
  AC_ARG_WITH(plugindir, [],
    [ AC_MSG_WARN([--with-plugindir is obsoleted, use --with-npapi-plugindir instead]); FIREFOX_PLUGINS=$withval])

  AC_ARG_WITH(npapi-plugindir,
    AC_HELP_STRING([--with-npapi-plugindir=DIR], [Directory to install NPAPI plugin in]),
    [FIREFOX_PLUGINS=$withval]
  )

  dnl
  dnl If not explicitly specified, figure install dir
  dnl from policy
  dnl
  if test x"${FIREFOX_PLUGINS}" = "x" ; then

       if test "x${NPAPI_INSTALL_POLICY}" = "xuser"; then

          dnl We always use .mozilla instead of .firefox, as this directoryis
          dnl used by all mozilla derived browsers.
          FIREFOX_PLUGINS=$HOME/.mozilla/plugins

       elif test "x${NPAPI_INSTALL_POLICY}" = "xsystem"; then

          for dir in /usr/lib64/mozilla/plugins /usr/lib/mozilla/plugins /usr/lib64/firefox/plugins /usr/lib/firefox/plugins /usr/lib/mozilla-firefox/plugins /usr/lib64/iceweasel/plugins /usr/lib/iceweasel/plugins ; do
             if test -d $dir; then
                FIREFOX_PLUGINS=$dir
                break
             fi
          done
          if test "x${FIREFOX_PLUGINS}" = x; then
             FIREFOX_PLUGINS=/usr/lib/mozilla/plugins
             AC_MSG_WARN([Could not find system mozilla plugin dir, use --with-npapi-plugindir. Defaulting to ${FIREFOX_PLUGINS}]);
          fi

       elif test "x${NPAPI_INSTALL_POLICY}" = "xprefix"; then
          FIREFOX_PLUGINS="\${prefix}/npapi"
       fi
    fi
  fi

  AC_SUBST(FIREFOX_PLUGINS)

])dnl end of GNASH_PATH_FIREFOX
# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
