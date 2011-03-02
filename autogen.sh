#!/bin/sh

# 
# Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

DIE=0

#Always use our macros
#ACLOCAL_FLAGS="-I macros $ACLOCAL_FLAGS"

if test "`uname`" = "Darwin"; then
LIBTOOLIZE=glibtoolize
fi

(test -f $srcdir/configure.ac) || {
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level package directory"
    exit 1
}

(${AUTOCONF:-autoconf} --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: You must have \`autoconf' installed."
  echo "Download the appropriate package for your distribution,"
  echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
  DIE=1
}

(grep "^AC_PROG_INTLTOOL" $srcdir/configure.ac >/dev/null) && {
  (${INTLTOOLIZE:-intltoolize} --version) < /dev/null > /dev/null 2>&1 || {
    echo 
    echo "**Error**: You must have \`intltool' installed."
    echo "You can get it from:"
    echo "  ftp://ftp.gnome.org/pub/GNOME/"
    DIE=1
  }
}

(grep "^AM_PROG_XML_I18N_TOOLS" $srcdir/configure.ac >/dev/null) && {
  (xml-i18n-toolize --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "**Error**: You must have \`xml-i18n-toolize' installed."
    echo "You can get it from:"
    echo "  ftp://ftp.gnome.org/pub/GNOME/"
    DIE=1
  }
}

(grep "^AM_PROG_LIBTOOL" $srcdir/configure.ac >/dev/null) && {
  (${LIBTOOL:-libtool} --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "**Error**: You must have \`libtool' installed."
    echo "You can get it from: ftp://ftp.gnu.org/pub/gnu/"
    DIE=1
  }
}

(grep "^AM_GLIB_GNU_GETTEXT" $srcdir/configure.ac >/dev/null) && {
  (grep "sed.*POTFILES" $srcdir/configure.ac) > /dev/null || \
  (glib-gettextize --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "**Error**: You must have \`glib' installed."
    echo "You can get it from: ftp://ftp.gtk.org/pub/gtk"
    DIE=1
  }
}

(${AUTOMAKE:-automake} --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: You must have \`automake' installed."
  echo "You can get it from: ftp://ftp.gnu.org/pub/gnu/"
  DIE=1
  NO_AUTOMAKE=yes
}


# if no automake, don't bother testing for aclocal
test -n "$NO_AUTOMAKE" || (${ACLOCAL:-aclocal} --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: Missing \`aclocal'.  The version of \`automake'"
  echo "installed doesn't appear recent enough."
  echo "You can get automake from ftp://ftp.gnu.org/pub/gnu/"
  DIE=1
}

if test "$DIE" -eq 1; then
  exit 1
fi

case $CC in
xlc )
  am_opt=--include-deps;;
esac

if test -z "$NO_LIBTOOLIZE" ; then
  libtoolflags="--force --copy"
  echo "Running libtoolize ${libtoolflags} ..."
  rm -fr macros/libtool.m4 macros/ltoptions.m4
  rm -fr macros/ltsugar.m4 macros/ltversion.m4 macros/lt~obsolete.m4
  if ! ${LIBTOOLIZE:-libtoolize} ${libtoolflags}; then
    echo
    echo "**Error**: libtoolize failed, do you have libtool and libltdl3-dev packages installed?"
    exit 1
  fi
fi

#for coin in `find $srcdir -name CVS -prune -o -name configure.ac -print`
for coin in configure.ac
do 
  dr=`dirname $coin`
  if test -f $dr/NO-AUTO-GEN; then
    echo skipping $dr -- flagged as no auto-gen
  else
    echo processing $dr
    ( cd $dr

     if test -d macros; then
        aclocalinclude="-I macros $ACLOCAL_FLAGS"
     else
        aclocalinclude="$ACLOCAL_FLAGS"
     fi

     if test -d cygnal; then
        aclocalinclude="-I cygnal ${aclocalinclude}"
     fi

     if grep "^AM_GLIB_GNU_GETTEXT" configure.ac >/dev/null; then
       echo "Creating $dr/aclocal.m4 ..."
       test -r $dr/aclocal.m4 || touch $dr/aclocal.m4
       echo "Making $dr/aclocal.m4 writable ..."
       test -r $dr/aclocal.m4 && chmod u+w $dr/aclocal.m4
     fi
     if grep "^AC_PROG_INTLTOOL" configure.ac >/dev/null; then
       echo "Running intltoolize --copy --force --automake ..."
       ${INTLTOOLIZE:-intltoolize} --copy --force --automake
     fi
     if grep "^AM_PROG_XML_I18N_TOOLS" configure.ac >/dev/null; then
       echo "Running xml-i18n-toolize --copy --force --automake..."
       xml-i18n-toolize --copy --force --automake
     fi
#       if grep "^AC_PROG_LIBTOOL" configure.ac >/dev/null; then
# 	if test -z "$NO_LIBTOOLIZE" ; then 
# 	  echo "Running libtoolize --force --copy ..."
# 	  ${LIBTOOLIZE:-libtoolize} --force --copy
# 	fi
#       fi
      echo "Running aclocal $aclocalinclude ..."
      ${ACLOCAL:-aclocal} ${aclocalinclude}
      if grep "^A[CM]_CONFIG_HEADER" configure.ac >/dev/null; then
	echo "Running autoheader..."
	${AUTOHEADER:-autoheader}
      fi
      # This is a hack. Any command line arguments maens don't run Automake.
      # This is to prevent regenerating and checking in a pile of Makefiles
      # that haven't really changed. They clutter up the checkin messages.
      if test x"$1" = x ; then
        echo "Running automake --add-missing --copy $am_opt ..."
        ${AUTOMAKE:-automake} --add-missing --copy $am_opt
      fi
      echo "Running autoconf ..."
      ${AUTOCONF:-autoconf}
    )
  fi
done

conf_flags="--enable-maintainer-mode"

