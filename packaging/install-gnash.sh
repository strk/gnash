#!/bin/sh
# 
#   Copyright (C) 2008 Free Software Foundation, Inc.
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

if [ x${BASH} = x ]; then
  echo "Sorry, you need a more modern shell like bash to run this script"
  exit
fi

# set to any command to execute to make the shell commands not
# make any actual changes on disk. 'echo' is the usual value here
# for this, as it just displays the commands to the screen.
debug=echo

# set 'yes' to 'yes', and you don't get asked for prompts.
# Use with care!!! Note that setting this to yes obligates the user
# to having agreed to the GPLv3 license when executing this script.
yes=yes

# aliases for a few commands to force debug mode while still leaving
# the rest of this code readable.
COPY="$debug cp -f"
REMOVE="$debug rm -f"
MOVE="$debug mv -f"

#
# Display the GPLv3, and get the users agreement before proceeding
#
gpl ()
{
  # display the GPLv3
  cat COPYING

  # make sure the user agrees
  if [ x$yes = xno ]; then
    read -p "Do you agree to the terms of this license? (yes or no) " answer
  else
    answer="$yes"
  echo "NOTE: You have the install script set to \"yes always\" mode"
    echo "NOTE: This means by default you have agree to the terms of the GPLv3"
  fi

  if [ x$answer != xyes ] ; then
    echo "Sorry, then you can't install Gnash..."
    exit
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
    rootdir="/usr/local"
  fi

  # This is where the NSAPI (any Geeko based browser like Mozilla,
  # Netscape, Firefox, Galeon, Ephipany, XO Web Activity, gets
  # installed. Most browsers, whether proprietary or open software
  # support NSAPI, but your mileage may vary...
  nsapidir="@FIREFOX_PLUGINS@"
  if [ x"${nsapidir}" = x"@FIREFOX_PLUGINS@" ]; then
    # OpenBSD 4.2, at the least. uses this path for the plugin
    if [ -d /usr/local/mozilla-firefox/plugins ]; then
      nsapidir="/usr/local/mozilla-firefox/plugins"
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

  # dopcumentation gets installed here
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
  if [ -d ${rootdir}/lib/gnash ]; then
    existing=`ls -d ${rootdir}/lib/gnash/libgnashbase-*.so* 2> /dev/null`
    if [ -n "${existing}" ]; then
      echo ""
      echo "You have previous installations of Gnash"
      file=""
      for i in $existing; do
        files="$files `echo $i | sed -e 's:.*gnashbase-::' -e 's:.so::'`"
      done
      echo "version: $files"
      echo "These should to be removed for the best stability of Gnash"
      if [ x$yes = xno ]; then
        read -p "Do you wish to remove these old versions ? " answer
      else
	answer="$yes"
      fi
        if [ x$answer = xyes ]; then
          for i in $files; do
            if [ x$yes = xno ]; then
	      read -p "Remove Gnash version \"$i\"? " answer
	    else
	      answer="$yes"
            fi
            if [ x$answer = xyes ]; then
              ${REMOVE} $rootdir/lib/gnash/libgnash*-$i.*
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
      if [ x$yes = xno ]; then
        read -p "Do you wish to remove these old versions ? " answer
      else
	answer="$yes"
      fi
      if [ x$answer = xyes ]; then
        for i in $files; do
          if [ x$yes = xno ]; then
	    read -p "Remove Gnash GUI \"$i\"? " answer
	  else
	    answer="$yes"
	  fi
          if [ x$answer = xyes ]; then
            ${REMOVE} $rootdir/bin/gnash/$i-gnash
	  fi
        done
      fi
      if [ x$yes = xno ]; then
        read -p "Do you wish to remove the gnash shell script too ? " answer
      else
	answer="$yes"
      fi
      if [ x$answer = xyes ]; then
	  ${REMOVE} $rootdir/bin/gnash/gnash
      fi
    fi
  else
      echo "Cool, you don't have any preexisting installations of Gnash"
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
    ${COPY} libgnashplugin.so ${nsapidir}
  else
    echo "You don't have the NSAPI plugin, installation can't continue"
    exit
  fi

  # install the Kparts (KDE/Konqueror) plugin
  if [ -e plugins/libklashpart.so -o x$yes = xyes ]; then
    ${COPY} plugins/libklashpart.so ${kpartsplugindir}
  else
    echo "You don't have the Kparts plugin, installation can't continue"
    exit
  fi

  # install the libraries
  if [ -e lib/libgnashbase.so -o x$yes = xyes ]; then
    ${COPY} lib/libgnash*.so /usr/lib/
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

  if [ -e dumpshm -o x$yes = xyes ]; then
    ${COPY} gprocessor dumpshm soldumper /usr/bin/
  else
    echo "You don't have the Gnsh utilities, installation can't continue"
    exit
  fi

  # install the documentation
  if [ -e gnash.pdf -o x$yes = xyes ]; then
    $debug mkdir -p /usr/share/doc/gnash
    ${COPY} gnash.pdf /usr/share/doc/gnash
  fi
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
  genericdeps="boost gstreamer libpng libjpeg libxml2 libstdc++ curl agg"
  gtkdeps="gtk2 gtkglext pango atk"
  kdedeps="kdebase qt"
  missing=""
  # first see which packaging system we have to deal with.
  # First we go through and see what program we can use to
  # check for installed packages. Note that this does not
  # currently support finding packages you've built from
  # source, or installed via a tarball, because neither will
  # have entries into the package DB.
  pkginfo="rpm dpkg ipkg pkg_info"
  for i in $pkginfo; do
    ret=`which $i | grep -c 'not found'`
    if [ $ret -eq 0 ]; then
      pkginfo=$i
      echo "Yes, found $i, so we'll use that for listing packages"
      break
      fi
  done

  # tweaka few package names if we're on a BSD system
  if [ ${pkginfo} = "pkg_info" ]; then
    genericdeps="boost gstreamer png jpeg libxml-2 libstdc++ curl agg"
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
    ret=`which $i | grep -c 'not found'`
    if [ $ret -eq 0 ]; then
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
	deps="`rpm -q "$i"`"
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
    dump)
      yes=yes
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
# Go through the license agreement
gpl

# Check the directory permissions first
checkdirs

# Look for preexisting installations of Gnash. letting the user
# remove them if they so choose
preexisting

# Install Gnash
install
