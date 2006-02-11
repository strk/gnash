dnl Process this file with autoconf to produce a configure script.
dnl
dnl  Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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
dnl
dnl  You should have received a copy of the GNU General Public License
dnl  along with this program; if not, write to the Free Software
dnl  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

AC_DEFUN([GNASH_DOCBOOK], [

  AC_ARG_ENABLE(docbook, [  --disable-docbook            Disable support for building documentation],
  [case "${enableval}" in
    yes) docbook=yes ;;
    no)  docbook=no ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for enable-docbook option]) ;;
  esac], docbook=yes)

  if test x"$docbook" = x"yes"; then
    AC_ARG_WITH(docbook_styles, [  --with-docbook-styles  directory where Docbook stylesheets are], with_docbook_styles=${withval})
    AC_CACHE_VAL(ac_cv_path_docbook_styles,[
    if test x"${with_docbook_styles}" != x ; then
      if test -f ${with_docbook_styles}/html/docbook.xsl ; then
        ac_cv_path_docbook_styles=`(cd ${with_docbook_styles}; pwd)`
      else
        AC_MSG_ERROR([${with_docbook_styles} directory doesn't contain docbook.xsl])
      fi
    fi
    ])

    dirlist="/usr/share/xml/docbook/stylesheet/nwalsh /usr/share/sgml/docbook/xsl-stylesheets /usr/local/share/sgml/docbook/xsl-stylesheets /opt/share/sgml/docbook/xsl-stylesheets /home/latest/share/sgml/docbook/xsl-stylesheets /usr/share/sgml/docbook/stylesheet/xsl/nwalsh"
    for i in $dirlist; do
      if test -f $i/html/docbook.xsl; then
        docbook_styles=`(cd $i; pwd)`
      break
      fi
    done

    AC_MSG_NOTICE([checking for other programs needed to process the DocBook files])
    AC_PATH_PROG(FOP, fop.sh, [""],
    	[$PATH:/usr/local/fop-0.20.5/:/usr/fop-0.20.5/:/usr/local/fop:/usr/lib/java/fop])
    if test x"${FOP}" = x ; then
      AC_MSG_WARN([No fop.sh found! PDF format files can't be generated])
    fi

    dirlist="/usr/lib/jre /usr/jre /opt/local/Java/JavaSDK ~/ReQuest/jre"
    JAVA=""
    for i in $dirlist; do
      if test -f $i/bin/java; then
	version=`$i/bin/java -version 2>&1`
	dnl See if it's Sun Java
  	tmp=`echo $version | grep -c "java version" `
	if test $tmp -gt 0; then
	  version=sun
	  JAVA=$i/bin/java
	  break;
	fi
	dnl See if it's GCJ
  	tmp=`echo $version | grep -c "java version" `
	if test $tmp -gt 0; then
	  AC_MSG_WARN([$i/bin/java not Sun version!])
	  version=gcj
	fi
      fi
    done

    AC_MSG_CHECKING(for Sun java runtime)
    if test x"$JAVA" = x; then
      AC_MSG_RESULT(not found)
      AC_MSG_WARN([You need to install Sun Java and the JAI toolkit to run fop])
    else
      AC_MSG_RESULT([$JAVA])
    fi

    AC_PATH_PROG(XSLTPROC, xsltproc, [],
    	[$PATH:/usr/bin:/usr/bin/X11:/usr/local/X11/bin])

    AC_PATH_PROG(DB2X_XSLTPROC, db2x_xsltproc, [],
    	[$PATH:/usr/bin:/usr/bin/X11:/usr/local/X11/bin])

    AC_PATH_PROG(DB2X_TEXIXML, db2x_texixml, [],
    	[$PATH:/usr/bin:/usr/bin/X11:/usr/local/X11/bin])

    AC_PATH_PROG(DB2X_MANXML, db2x_manxml, [],
    	[$PATH:/usr/bin:/usr/bin/X11:/usr/local/X11/bin])

    AC_PATH_PROG(MAKEINFO, makeinfo, [],
    	[$PATH:/usr/bin:/usr/bin/X11:/usr/local/X11/bin])

    if test x"$XSLTPROC" = x; then
      AC_MSG_WARN([You need to install xsltproc before HTML output can be generated])
    fi
    if test x"$DB2X_XSLTPROC" = x -o x"$DB2X_TEXIXML" = x  -o x"$DB2X_MANXML" = x -o x"$MAKEINFO" = x; then
      AC_MSG_WARN([You need to install the docbook2X package before Texi output can be generated])
    fi
  fi

  AM_CONDITIONAL(HAVE_JAVA, [test x$JAVA != x])
  AM_CONDITIONAL(ENABLE_TEXI, [ test x"$DB2X_XSLTPROC" != x -a x"$DB2X_TEXIXML" != x -a x"$MAKEINFO" != x ])
  AM_CONDITIONAL(ENABLE_HTML, [ test x"$XSLTPROC" != x ])
  AM_CONDITIONAL(ENABLE_FOP, [ test x"$FOP" != x ])
  AM_CONDITIONAL(ENABLE_MAN, [ test x"$DB2X_XSLTPROC" != x -a x"$DB2X_MANXML" != x ])
  AC_SUBST(JAVA)

dnl See which version of the DocBook2x tools we have, because it
dnl forces a command line change in the Makefile.

dnl db2x_texixml (part of docbook2X 0.8.3)
dnl db2x_texixml (part of docbook2X 0.8.5)
  DB2X_VERSION=""
  if test x"${DB2X_TEXIXML}" != x; then
    db2x_version=`${DB2X_TEXIXML} --version | head -1 | sed -e 's/^.*docbook2X //' -e 's/).*$//'`
    DB2X_VERSION="${db2x_version}"
    AC_SUBST(DB2X_VERSION)
  fi

  AM_CONDITIONAL(NEW_DOCBOOK2X, [test $db2x_version = "0.8.5"])
  AC_SUBST(docbook_styles)
])

