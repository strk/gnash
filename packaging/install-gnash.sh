#!/bin/sh
# 
#   Copyright (C) 2008, 2009, 2010 Free Software Foundation, Inc.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
# 

#
# install script for Gnash binary tarballs.
#

genericdeps="boost gstreamer libpng libjpeg libxml2 libstdc++ curl agg"
gtkdeps="gtk2 gtkglext pango atk"
kdedeps="kdebase qt"
missing=""

os=`./config.guess`

case $os in
  *i*86-*-linux*)
    ;;
  *64-*-openbsd*|i*86-*-openbsd*)
    if [ x${BASH} = x ]; then
      echo "Sorry, you need a more modern shell like bash to run this script"
      exit
    fi
    genericdeps="boost gstreamer png jpeg libxml-2 libstdc++ curl agg"
    ;;
  *)
    ;;
esac

# set to any command to execute to make the shell commands not
# make any actual changes on disk. 'echo' is the usual value here
# for this, as it just displays the commands to the screen.
debug=

# set 'yes' to 'yes', and you don't get asked for prompts. Set to 'no'
# for the default behaviour. Use with care!!!
yes=no


# aliases for a few commands to force debug mode while still leaving
# the rest of this code readable.
COPY="$debug cp -f"
REMOVE="$debug rm -f"
MOVE="$debug mv -f"

#
# Find an executable file that needs to be in our shell PATH.
#
findbin ()
{
    ret=0

    newpath=`echo $PATH | tr : ' '`
    for i in ${newpath}; do
      if [ -x $i/$1 ]; then
	ret=1
	break
      fi
    done

    echo $ret
    return 
}

#
# See if we can figure out which GNU/Linux or BSD distribution we are running
#
findrelease ()
{
    # Ubuntu uses /etc/lsb-release
    if [ -f /etc/lsb-release ]; then
	release_name="`grep DISTRIB_ID /etc/lsb-release`"
	release_version="`grep DISTRIB_RELEASE /etc/lsb-release`"
	return
    fi

    # Redhat, Fedora, and Centos all use /etc/redhat-release
    if [ -f /etc/redhat-release ]; then
	release_name="`cat /etc/redhat-release | cut -d ' ' -f 1`"
	release_version="`cat /etc/redhat-release | cut -d ' ' -f 3`"
	return
    fi

    # Debian uses /etc/issue. Fedora uses /etc/issue too, but we
    # don't need it now.
    if [ -f /etc/issue ]; then
	release_name="`cat /etc/issue | cut -d ' ' -f 1`"
	release_version="`cat /etc/issue | cut -d ' ' -f 3`"
	return
    fi

    # OpenBSD doesn't appear to use anything, but by default motd does
    # have the distro name and version, so we try that as most people
    # never bother to change their motd message anyway.
    if [ -f /etc/motd ]; then
	release_name="`cat /etc/motd | head -1 | cut -d ' ' -f 1`"
	release_version="`cat /etc/motd | head -1 | cut -d ' ' -f 2`"
	return
    fi

}

# This script has a split personality in that it works in an
# unconfigured tree, as well as a version with the paths munged
# by the normal Gnash configuration process. This way we can
# have the final version as included in the tarball have the
# paths Gnash was configured with for that build, which seems
# a good idea. Running this unconfigured is mostly a maintainer
# feature, it's much faster to be avbke to rerun this script than
# constantely reconfiguring gnash, which isn't real fast...
checkdirs ()
{
  # unfortunately the field used for the user name varies depending
  # on the system.
  field=0

  # Root directory used to look for pre existing installations,
  # as well as were these files get installed to.
  rootdir="@prefix@"
  if [ x"${rootdir}" = x"@prefix@" ]; then
    rootdir="/usr"
  fi

  # This is where the NSAPI (any Geeko based browser like Mozilla,
  # Netscape, Firefox, Galeon, Ephipany, XO Web Activity, gets
  # installed. Most browsers, whether proprietary or open software
  # support NSAPI, but your mileage may vary...
  nsapidir="@FIREFOX_PLUGINS@"
  if [ x"${nsapidir}" = x"@FIREFOX_PLUGINS@" ]; then
    # OpenBSD 4.2, at the least. uses this path for the plugin
    if [ -d /usr/local/mozilla-firefox/plugins ]; then
      nsapidir="/usr/local/mozilla-firefox/plugin"s
      field=4
    fi
    # this is the standard defalt path, common on most GNU/Linux distros
    if [ -d /usr/lib/mozilla/plugins ]; then
      nsapidir="/usr/lib/mozilla/plugins"
      field=3
    fi
  fi
  if [ ! -w ${nsapidir} ]; then
    needstobe="`ls -ld ${nsapidir} | cut -f ${field} -d ' ' 2>&1`"
    echo "Sorry, you need to be have write permissions to \"${nsapidir}\" to install the Gnash NSAPI plugin"
    echo "Or be the user \"${needstobe}\""
    $debug exit
  fi

  # This is where the Kparts plugin gets installed.
  kpartsplugindir="@KDE_PLUGINDIR@"
  if [ x"${kpartsplugindir}" = x"@KDE_PLUGINDIR@" ]; then
    kpartsplugindir="/usr/lib/kde3"
  fi
  if [ -d ${kpartsplugindir} -a ! -w ${kpartsplugindir} ]; then
    needstobe="`ls -ld ${kpartsplugindir} | cut -f ${field} -d ' ' 2>&1`"
    echo "Sorry, you need to be have write permissions to \"${kpartsplugindir}\" to install the Gnash Kparts plugin"
    echo "Or be the user \"${needstobe}\""
    $debug exit
  fi

  # This is where the Kparts services gets installed.
  kpartsservicesdir="@KDE_SERVICESDIR@"
  if [ x"${kpartsservicesdir}" = x"@KDE_SERVICESDIR@" ]; then
    kpartsservicesdir="/usr/share/services"
  fi
  if [ -d ${kpartsservicesdir} -a ! -w ${kpartsservicesdir} ]; then
    needstobe="`ls -ld ${kpartsservicesdir} | cut -f ${field} -d ' ' 2>&1`"
    echo "Sorry, you need to be have write permissions to \"${kpartsservicesdir}\" to install the Gnash Kparts services"
    echo "Or be the user \"${needstobe}\""
    $debug exit
  fi
  
  # This is where the Kparts config gets installed.
  kpartsconfigdir="@KDE_CONFIGDIR@"
  if [ x"${kpartsconfigdir}" = x"@KDE_CONFIGDIR@" ]; then
    kpartsconfigdir="/etc/kde3"
  fi
  if [ -d ${kpartsconfigdir} -a ! -w ${kpartsconfigdir} ]; then
    needstobe="`ls -ld ${kpartsconfigdir} | cut -f ${field} -d ' ' 2>&1`"
    echo "Sorry, you need to be have write permissions to \"${kpartsconfigdir}\" to install the Gnash Kparts Config Data"
    echo "Or be the user \"${needstobe}\""
    $debug exit
  fi
  
  # This is where the Kparts appdata gets installed.
  kpartappsdir="@KDE_APPSDATADIR@"
  if [ x"${kpartappsdir}" = x"@KDE_APPSDATADIR@" ]; then
    kpartappsdir="/usr/share/apps/klash"
  fi
  if [ -d ${kpartappsdir} -a ! -w ${kpartappsdir} ]; then
    needstobe="`ls -ld ${kpartappsdir} | cut -f ${field} -d ' ' 2>&1`"
    echo "Sorry, you need to be have write permissions to \"${kpartappsdir}\" to install the Gnash Kparts Application Data"
    echo "Or be the user \"${needstobe}\""
    $debug exit
  fi
  
  if [ x"${debug}" != x ]; then
    echo "NOTE: You have debug mode set to \"${debug}\", no real commands will be executed"
  fi
  
  if [ ! -w ${rootdir} ]; then
    needstobe="`ls -ld ${rootdir} | cut -f ${field} -d ' ' 2>&1`"
    if [ x`whoami` != xroot ]; then
      echo "Sorry dude, you need to be have write permissions to \"${rootdir}\" to install Gnash"
      echo "Or be the user \"${needstobe}\""
      $debug exit
    fi
  fi

#   sodir=/etc/ld.so.conf.d
#   if [ ! -w ${sodir} ]; then
#     needstobe="`ls -ld ${sodir} | cut -f ${field} -d ' ' 2>&1`"
#     if [ x`whoami` != xroot ]; then
#       echo "Sorry dude, you need to be have write permissions to \"${sodir}\" to install Gnash"
#       echo "Or be the user \"${needstobe}\""
#       $debug exit
#     fi
#   fi

  # documentation gets installed here
  docdir="/usr/share/doc"

  if [ ! -w ${docdir} ]; then
    needstobe="`ls -ld ${docdir} | cut -f ${field} -d ' ' 2>&1`"
    if [ x`whoami` != xroot ]; then
      echo "Sorry dude, you need to be have write permissions to \"${docdir}\" to install Gnash"
      echo "Or be the user \"${needstobe}\""
      $debug exit
    fi
  fi
}
  
# look for existing installtions of snapshots. As these are date stamped,
# it's possible to get many of them from repeated weekly installs of
# these Gnash snapshots. So Look for them, and let the user remove pieces
# of them to return to a stable state.
preexisting ()
{
  if [ -d ${rootdir}/lib/gnash -o -d ${rootdir}/lib ]; then
    existing=`ls -d ${rootdir}/lib/gnash/libgnashbase*.so* ${rootdir}/lib/libgnashbase*.so 2> /dev/null`
    if [ -n "${existing}" ]; then
      echo ""
      echo "You have previous installations of Gnash"
      file=""
      for i in $existing; do
        files="$files `echo $i | sed -e 's:.*gnashbase-::' -e 's:.so::'`"
      done
      echo "version: $files"
      echo "These should to be removed for the best stability of Gnash"
      if [ x$yes != xyes ]; then
        read -p "Do you wish to remove these old versions ?  [yes] " answer
      fi
      if [ x$answer = xyes  -o x$answer = xy ]; then
        for i in $files; do
          if [ x$yes != xyes ]; then
	    read -p "Remove Gnash version \"$i\"? [yes] " answer
          fi
          if [ x$answer = xyes -o x$answer = xy ]; then
            ${REMOVE} $rootdir/lib/libgnash*
	  fi
        done
      fi
    fi
    existing=`ls -d ${rootdir}/bin/*-gnash 2> /dev/null`
    if [ -n "${existing}" ]; then
      echo "You have previous installations of the Gnash GUI"
      files=""
      for i in $existing; do
        files="$files `echo $i | sed -e 's:-gnash::' -e "s:${rootdir}/bin/::"`"
      done
      echo "GUIS: $files"
      echo "These should to be removed for the best stability of Gnash"
      if [ x$yes != xyes ]; then
        read -p "Do you wish to remove these old versions ? [yes] " answer
      fi
      if [ x$answer = xyes  -o x$answer = xy ]; then
        for i in $files; do
          if [ x$yes != xyes ]; then
	    read -p "Remove Gnash GUI \"$i\"? [yes] " answer
	  fi
          if [ x$answer = xyes  -o x$answer = xy ]; then
            ${REMOVE} $rootdir/bin/$i-gnash
	  fi
        done
      fi
      if [ x$yes != xyes ]; then
        read -p "Do you wish to remove the gnash shell script too ? [yes] " answer
      fi
      if [ x$answer = xyes  -o x$answer = xy ]; then
	  ${REMOVE} $rootdir/bin/gnash
      fi
    fi
  else
      echo "Cool, you don\'t have any preexisting installations of Gnash"
  fi

  return `true`
}

# install all the files to their proper location.
# this basically dumplicates what "make install install-plugin"
# does, but it's more oriented towards developers than end users.
# so this is the more user friendly way to install a binary tarball.
install ()
{
  # install the NSAPI (Mozilla/Firefox) plugin
  if [ -e plugins/libgnashplugin.so -o x$yes = xyes ]; then
    ${COPY} plugins/libgnashplugin.so ${nsapidir}
  else
    echo "You don't have the NSAPI plugin, installation can't continue"
    exit
  fi

  # install the Kparts (KDE/Konqueror) plugin
  if [ -e lib/kde3/libklashpart.so -o x$yes = xyes ]; then
    ${COPY} lib/kde3/libklashpart.so ${kpartsplugindir}
  else
    echo "You don't have the Kparts plugin, installation can't continue"
    exit
  fi

  # install the libraries
  if [ -e lib/gnash/libgnashbase.so -o x$yes = xyes ]; then
    for i in lib/gnash/libgnash*trunk.so; do
      ${COPY} $i /usr/lib/
      linkname=`echo $i | sed -e 's:\-.*.so:.so:' -e 's:\.3.*::'`
      linkname=`basename ${linkname}`
      ln -fs /usr/lib/`basename $i` /usr/lib/${linkname}
    done
    ${COPY} lib/gnash/libltdl.so.3.* /usr/lib/
    ln -fs /usr/lib/libltdl.so.3.1.4 /usr/lib/libltdl.so.3
    ln -fs /usr/lib/libltdl.so.3.1.4 /usr/lib/libltdl.so
  else
    echo "You don't have the Gnash libraries, installation can't continue"
    exit
  fi

  # install the executables
  if [ -e bin/gtk-gnash -o x$yes = xyes ]; then
    ${COPY} bin/*gnash bin/gprocessor bin/dumpshm bin/soldumper /usr/bin/
  else
    echo "You don't have the Gnash executable, installation can't continue"
    exit
  fi

  # copy the ld.so.conf file
#   if [ ! -f /etc/ld.so.conf.d/gnash.conf ]; then
#     echo " /usr/lib/gnash" > /etc/ld.so.conf.d/gnash.conf
#   fi

#   # run ldconfig so the shared libraries can be found.
  ldconfig
}

#
# Here's where we make an attempt to see if the dependant 
# packages Gnash needs are installed. This has a pile of
# system dependant code in it, so for now we can only support
# the more popular packaging types, namely rpm,.deb, ipkg, and
# BSD packages.
#
checkdeps()
{
  # first see which packaging system we have to deal with.
  # First we go through and see what program we can use to
  # check for installed packages. Note that this does not
  # currently support finding packages you've built from
  # source, or installed via a tarball, because neither will
  # have entries into the package DB.
  pkginfo="rpm dpkg ipkg pkg_info"
  for i in $pkginfo; do
    ret=`findbin $i`
    if [ $ret -eq 1 ]; then
      pkginfo=$i
      echo "Yes, found $i, so we'll use that for listing packages"
      break
      fi
  done

  if [ ${pkginfo} = "pkg_info" ]; then
    # BSD needs PKG_PATH set to load anything over the net.
    if [ x${PKG_PATH} = x ]; then
      echo "Pleaase set the environment variable PKG_PATH and try again."
      exit 1
    fi
  fi
  # Usually the program that installs packages isn't the same one
  # that we can use to check the packaging DB.
  pkgnet="yum apt-get ipkg pkg_add"
  for i in ${pkgnet}; do
    ret=`findbin $i`
    if [ $ret -eq 1 ]; then
      pkgnet=$i
      echo "Yes, found $i, so we'll use that to install packages"
      break
    fi
  done

  echo "Looking for dependent packages for GTK2 and KDE..."
  # first find the packages all configurations of Gnash need
  for i in ${genericdeps} ${gtkdeps} ${kdedeps}; do
    case $pkginfo in
      dpkg)
        deps="`dpkg -l "*$i*" | grep -- "^ii" | cut -d ' ' -f 3`"
        ;;
      rpm)
	deps="`rpm -q $i`"
        ;;
      pkg_info)
	 deps="`pkg_info | grep "$i" | sed -e 's: .*$::'`"
	 ;;
      ipkg)
	 deps="todo"
         ;;
       *)
         echo "ERROR: No package manager found!"
         exit 1
         ;;
      esac
      found=`echo ${deps} | grep -v 'not installed' | grep -c "${i}" 2>&1`
      if [ $found -gt 0 ]; then
	echo "Yes, found $i"
      else
	echo "Nope, $i appears to not be installed"
	missing="${missing} $i"
      fi
    done

  if [ -n "${missing}" ]; then
    echo "package(s)\"${missing}\" are missing!"
    echo "You will need sudo priviledges to install the packages"
    if [ x$yes = xno ]; then
      $debug sudo ${pkgnet} install ${missing}
    else
      $debug sudo ${pkgnet} -y install ${missing}
    fi
  fi
}

#
# Get codecs
#
getcodecs()
{
    # sudo apt-get install gstreamer0.10-ffmpegsudo apt-get install gstreamer0.10-ffmpeg gstreamer0.10-gl gstreamer0.10-plugins-base gstreamer0.10-plugins-good gstreamer0.10-plugins-bad gstreamer0.10-plugins-bad-multiverse gstreamer0.10-plugins-ugly gstreamer0.10-plugins-ugly-multiverse libxine-extracodecs w32codecs
    # deb http://archive.ubuntu.com/ubuntu dapper universe multiverse
    # deb-src http://archive.ubuntu.com/ubuntu dapper universe multiverse



    echo $0
}

#
# Here's where all the actions happens
#

# if we have any argument, process them
while [ ! -z $1 ]; do
  case "$1" in
    rootdir=*)
      rootdir=`echo $1 | sed -e 's:rootdir=::'`
      if [ -z "${rootdir}" ]; then
        echo "ERROR: You need to specify an argument for rootdir="
      fi
      ;;
    uninstall)
      checkdirs
      preexisting
      exit 0
      ;;
    dump)
      yes=yes
      findrelease
      echo "Release name is: ${release_name}"
      echo "Release version is: ${release_version}"
      checkdirs
      echo ""
      echo "Default paths are: "
      echo "	Root directory is: ${rootdir}"
      echo "	NSAPI directory is: ${nsapidir}"
      echo "	Kparts plugin directory is: ${kpartsplugindir}"
      echo "	Kparts services directory is: ${kpartsservicesdir}"
      echo "	Kparts config data directory is: ${kpartsconfigdir}"
      echo "	Kparts application data directory is: ${kpartappsdir}"
      echo ""
      echo "The 'debug' option is set to: \"${debug}\""
      echo "The 'yes' option is set to: \"${yes}\""
      echo "The 'COPY' command is set to: \"${COPY}\""
      echo "The 'REMOVE' command is set to: \"${REMOVE}\""
      echo "The 'MOVE' command is set to: \"${MOVE}\""
      exit 0
      ;;
    deps)
      checkdeps
      exit 1
      ;;
    *)
      echo "Usage: $0 [rootdir=path dump deps]"
      echo "	\"$1\" isn't a supported option"
      exit 1
      ;;
  esac
done

#
# Here's where everything actually gets executed
#

# Check the directory permissions first
checkdirs

# Look for preexisting installations of Gnash. letting the user
# remove them if they so choose
preexisting

# Install Gnash
install
