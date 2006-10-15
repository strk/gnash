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

AC_DEFUN([GNASH_DOCBOOK], [

  AC_ARG_ENABLE(docbook, AC_HELP_STRING([--disable-docbook], [Disable support for building documentation]),
  [case "${enableval}" in
    yes) docbook=yes ;;
    no)  docbook=no ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for enable-docbook option]) ;;
  esac], docbook=no)

  if test x"$docbook" = x"yes"; then
    docbook_styles=
    AC_ARG_WITH(docbook_styles, AC_HELP_STRING([--with-docbook-styles], [directory where Docbook stylesheets are]), with_docbook_styles=${withval})
    if test x"${with_docbook_styles}" != x ; then
      if test -f ${with_docbook_styles}/html/docbook.xsl ; then
        docbook_styles=`(cd ${with_docbook_styles}; pwd)`
      else
        AC_MSG_ERROR([${with_docbook_styles} directory doesn't contain docbook.xsl])
      fi
    else
      AC_CACHE_CHECK([for docbook styles path],[gnash_cv_path_docbook_styles],[
      dirlist="/usr/share/xml/docbook/stylesheet/nwalsh /usr/share/sgml/docbook/xsl-stylesheets /usr/local/share/sgml/docbook/xsl-stylesheets /opt/share/sgml/docbook/xsl-stylesheets /home/latest/share/sgml/docbook/xsl-stylesheets /usr/share/sgml/docbook/stylesheet/xsl/nwalsh"
      for i in $dirlist; do
        if test -f $i/html/docbook.xsl; then
          gnash_cv_path_docbook_styles=`(cd $i; pwd)`
        break
        fi
      done
      ])
      if test x$gnash_cv_path_docbook_styles != x; then
        docbook_styles=$gnash_cv_path_docbook_styles
      fi
    fi


    AC_MSG_NOTICE([checking for other programs needed to process the DocBook files])
    AC_PATH_PROG(FOP, fop.sh, [],
    	[$PATH:/usr/local/fop-0.20.5/:/usr/fop-0.20.5/:/usr/local/fop:/usr/lib/java/fop])
    if test x"$FOP" != x; then
      dirlist="/usr/lib/jre /usr/jre /opt/local/Java/JavaSDK ~/ReQuest/jre $J2REDIR"
      JAVA=
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
    fi

    AC_PATH_PROG(PDFXMLTEX, pdfxmltex, [],
    	[$PATH:/usr/bin:/usr/bin/X11:/usr/local/X11/bin])

dnl    AC_PATH_PROG(DBLATEX, dblatex, [],
dnl    	[$PATH:/usr/bin:/usr/bin/X11:/usr/local/X11/bin])

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

    if test x"$XSLTPROC" = x -o x"$docbook_styles" = x ; then
      AC_MSG_WARN([You need to install xsltproc and docbook style sheets before HTML output can be generated])
    fi
    if test x"$DB2X_XSLTPROC" = x -o x"$DB2X_TEXIXML" = x  -o x"$DB2X_MANXML" = x -o x"$MAKEINFO" = x ; then
      AC_MSG_WARN([You need to install the docbook2X package before Texi output can be generated])
    fi
    if test x"$FOP" != x -a x"$docbook_styles" != x ; then :
    elif test x"$PDFXMLTEX" != x -a x"$XSLTPROC" != x -a x"$docbook_styles" != x ; then :
    else
      AC_MSG_WARN([No suitable fop nor pdfxmltex, PDF format files can't be generated])
    fi
  fi

dnl  AM_CONDITIONAL(HAVE_JAVA, [test x$JAVA != x])
  AM_CONDITIONAL(ENABLE_TEXI, [ test x"$DB2X_XSLTPROC" != x -a x"$DB2X_TEXIXML" != x -a x"$MAKEINFO" != x ])
  AM_CONDITIONAL(ENABLE_HTML, [ test x"$XSLTPROC" != x -a x"$docbook_styles" != x ])
  AM_CONDITIONAL(ENABLE_FOP, [ test x"$FOP" != x -a x"$docbook_styles" != x ])
  AM_CONDITIONAL(ENABLE_XMLTEX, [ test x"$PDFXMLTEX" != x -a x"$XSLTPROC" != x -a x"$docbook_styles" != x ])
dnl  AM_CONDITIONAL(ENABLE_DBLATEX, [ test x"$DBLATEX" != x ])
  AM_CONDITIONAL(ENABLE_MAN, [ test x"$DB2X_XSLTPROC" != x -a x"$DB2X_MANXML" != x ])
  AC_SUBST(JAVA)

dnl See which version of the DocBook2x tools we have, because it
dnl forces a command line change in the Makefile.

dnl db2x_texixml (part of docbook2X 0.8.3)
dnl db2x_texixml (part of docbook2X 0.8.5)
  DB2X_VERSION=
  if test x"${DB2X_TEXIXML}" != x; then
    db2x_version=`${DB2X_TEXIXML} --version | head -1 | sed -e 's/^.*docbook2X //' -e 's/).*$//'`
    DB2X_VERSION="${db2x_version}"
    AC_SUBST(DB2X_VERSION)
  fi

dnl  AM_CONDITIONAL(NEW_DOCBOOK2X, [test "$db2x_version" = "0.8.5"])
  AC_SUBST(docbook_styles)
  AC_SUBST(XSLTPROC)
  AC_SUBST(DB2X_XSLTPROC)
  AC_SUBST(DB2X_TEXIXML)
  AC_SUBST(PDFXMLTEX)
  AC_SUBST(FOP)
])

