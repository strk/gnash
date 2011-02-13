dnl  
dnl    Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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

AC_DEFUN([GNASH_DOCBOOK], [

  AC_ARG_ENABLE(docbook, AC_HELP_STRING([--enable-docbook], [Enable support for building documentation with "make html" and "make pdf"]),
  [case "${enableval}" in
    yes) docbook=yes ;;
    no)  docbook=no ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for enable-docbook option]) ;;
  esac], docbook=no)

  DB2X_VERSION=
  if test x"$docbook" = x"yes"; then dnl## {
    dnl install-info is used to update entries in the dirs file, used by
    dnl info to find all of it files.
    AC_PATH_PROG(INSTALL_INFO, install-info, $PATH:/usr/sbin)
    AC_SUBST(INSTALL_INFO)
    AM_CONDITIONAL(ENABLE_INFO, test x${INSTALL_INFO} != x)

    docbook_styles=
    AC_ARG_WITH(docbook_styles, AC_HELP_STRING([--with-docbook-styles], [directory where Docbook stylesheets are]), with_docbook_styles=${withval})
    if test x"${with_docbook_styles}" != x ; then dnl## {

      if test -f ${with_docbook_styles}/html/docbook.xsl ; then dnl## {
        docbook_styles="`(cd ${with_docbook_styles}; pwd)`"
      else dnl## }{
        AC_MSG_ERROR([${with_docbook_styles}/html directory doesn't contain docbook.xsl])
      fi dnl## }

    else dnl## }{
      AC_CACHE_CHECK([for docbook styles path],[gnash_cv_path_docbook_styles],[
      dirlist="/usr/share/xml/docbook/stylesheet/nwalsh /usr/share/xml/docbook/stylesheet/nwalsh/current /usr/share/sgml/docbook/xsl-stylesheets /usr/local/share/sgml/docbook/xsl-stylesheets /opt/share/sgml/docbook/xsl-stylesheets /home/latest/share/sgml/docbook/xsl-stylesheets /usr/share/sgml/docbook/stylesheet/xsl/nwalsh"
      for i in $dirlist; do
        if test -f $i/html/docbook.xsl; then
          gnash_cv_path_docbook_styles="`(cd $i; pwd)`"
          break
        fi
      done
      ])

      if test x$gnash_cv_path_docbook_styles != x; then dnl## {
        docbook_styles=$gnash_cv_path_docbook_styles
      fi dnl## }

    fi dnl## }


    AC_MSG_NOTICE([checking for other programs needed to process the DocBook files])
    AC_PATH_PROG(FOP, fop.sh, [],
    	[$PATH:/usr/local/fop-0.20.5/:/usr/fop-0.20.5/:/usr/local/fop:/usr/lib/java/fop])
    if test x"$FOP" = x; then dnl## {
        AC_PATH_PROG(FOP, fop, [],
            [$PATH:/usr/local/fop-0.20.5/:/usr/fop-0.20.5/:/usr/local/fop:/usr/lib/java/fop])
    fi dnl## }

    if test x"$FOP" != x; then dnl## {
      dirlist="/usr /usr/lib/jre /usr/jre /opt/local/Java/JavaSDK ~/ReQuest/jre $J2REDIR"
      JAVA=
      for i in $dirlist; do
        if test -f $i/bin/java; then dnl## {
          version=`$i/bin/java -version 2>&1`
          dnl See if it's Sun Java
          tmp="`echo $version | grep -c "java version" `"
          if test $tmp -gt 0; then dnl## {
            version=sun
            JAVA=$i/bin/java
            break;
          fi dnl## }

          dnl See if it's GCJ
          tmp="`echo $version | grep -c "java version" `"
          if test $tmp -gt 0; then dnl## {
            AC_MSG_WARN([$i/bin/java not Sun version!])
            version=gcj
          fi dnl## }

        fi dnl## }

      done

      AC_MSG_CHECKING(for Sun java runtime)
      if test x"$JAVA" = x; then dnl## {
        AC_MSG_RESULT(not found)
        AC_MSG_WARN([You need to install Sun Java and the JAI toolkit to run fop])
      else dnl## }{
        AC_MSG_RESULT([$JAVA])
      fi dnl## }

    fi dnl## }

    AC_PATH_PROG(PDFXMLTEX, pdfxmltex, [],
    	[$PATH:/usr/bin:/usr/bin/X11:/usr/local/X11/bin])

dnl    AC_PATH_PROG(DBLATEX, dblatex, [],
dnl    	[$PATH:/usr/bin:/usr/bin/X11:/usr/local/X11/bin])

    AC_PATH_PROG(XSLTPROC, xsltproc, [],
    	[$PATH:/usr/bin:/usr/bin/X11:/usr/local/X11/bin])

    AC_PATH_PROG(DB2X_XSLTPROC, db2x_xsltproc, [],
     [$PATH:/usr/bin:/usr/bin/X11:/usr/local/X11/bin])

    if test x$DB2X_XSLTPROC = x; then dnl## {
      AC_PATH_PROG(DB2X_XSLTPROC, db2x_xsltproc.pl, [],
    	  [$PATH:/usr/bin:/usr/bin/X11:/usr/local/X11/bin])
    fi dnl## }

    dnl Find the programs we need to convert docbook into Texi for
    dnl making info pages. The first catagory are the wrapper
    dnl utilities included in most docbook2x packages.
    dnl It turns out there are two sets of wrapper functions, the good
    dnl ones from the newer DocBook2X tools are written in perl, and
    dnl actually work correctly. There are other versions of the same
    dnl tools ,but they are merely a 1 line wrapper for the OpenJade
    dnl tools. These versions have big problems, namely they don't
    dnl support the encoding of entities, so we get massive warnings
    dnl about entities in included files we never heard about.
    scripts="db2x_docbook2texi docbook2texi docbook2texi.pl"
    for i in $scripts; do dnl## {
      AC_PATH_PROG(DB2X_TEXI, $i, [], [$PATH:/usr/bin:/usr/bin/X11:/usr/local/X11/bin])
      if test x$DB2X_TEXI != x; then
        type="`file $DB2X_TEXI  | grep -ic "perl " 2>&1`"
        if test $type -gt 0; then
          break
        else
          DB2X_TEXI=
        fi
      fi
    done dnl## }

    dnl These look for the seperate utilities used by the wrapper
    dnl scripts. If we don't find the wrappers, then we use the lower
    dnl level utilities directly. 

    scripts="db2x_texixml db2x_texixml.pl"
    for i in $scripts; do
      AC_PATH_PROG(DB2X_TEXIXML, $i, [], [$PATH:/usr/bin:/usr/bin/X11:/usr/local/X11/bin])
      if test x$DB2X_TEXIXML != x; then
        break
      fi
    done

    dnl Find the programs we need to convert docbook into man pages.
    scripts="db2x_docbook2man docbook2man docbook2man.pl"
    for i in $scripts; do
      AC_PATH_PROG(DB2X_MAN, $i, [], [$PATH:/usr/bin:/usr/bin/X11:/usr/local/X11/bin])
      if test x$DB2X_MAN != x; then 
        type="`file $DB2X_MAN  | grep -ic "perl " 2>&1`"
        if test $type -gt 0; then
          break
        else
          DB2X_MAN=
        fi
      fi
    done

    scripts="db2x_manxml db2x_manxml.pl"
    for i in $scripts; do
      AC_PATH_PROG(DB2X_MANXML, $i, [], [$PATH:/usr/bin:/usr/bin/X11:/usr/local/X11/bin])
      if test x$DB2X_MANXML != x; then
        break
      fi
    done

    AC_PATH_PROG(MAKEINFO, makeinfo, [], [$PATH:/usr/bin:/usr/bin/X11:/usr/local/X11/bin])

    if test x"$XSLTPROC" = x -o x"$docbook_styles" = x ; then 
      AC_MSG_WARN([You need to install xsltproc and docbook style sheets before HTML output can be generated])
    fi

    if test x"$DB2X_XSLTPROC" = x -o x"$DB2X_TEXIXML" = x  -o x"$DB2X_MANXML" = x -o x"$MAKEINFO" = x ; then
      AC_MSG_WARN([You need to install the docbook2X package before Texi output can be generated])
    fi

    AC_PATH_PROG(DB2X_PDF, docbook2pdf, [], [$PATH:/usr/bin:/usr/bin/X11:/usr/local/X11/bin])


    if test x"$FOP" != x -a x"$docbook_styles" != x ; then :
    elif test x"$PDFXMLTEX" != x -a x"$XSLTPROC" != x -a x"$docbook_styles" != x ; then :
    else
      AC_MSG_WARN([No suitable fop nor pdfxmltex, PDF format files can't be generated - hint: apt-get install xmltex fop])
    fi

  fi dnl## }

dnl  AM_CONDITIONAL(HAVE_JAVA, [test x$JAVA != x])
  AM_CONDITIONAL(ENABLE_TEXI, [ test x"$DB2X_TEXI" != x -o x"$DB2X_TEXIXML" != x ])
  AM_CONDITIONAL(ENABLE_PDF, [ test x"$DB2X_PDF" ])
  AM_CONDITIONAL(ENABLE_HTML, [ test x"$XSLTPROC" != x -a x"$docbook_styles" != x ])
  AM_CONDITIONAL(ENABLE_FOP, [ test x"$FOP" != x -a x"$docbook_styles" != x ])
  AM_CONDITIONAL(ENABLE_XMLTEX, [ test x"$PDFXMLTEX" != x -a x"$XSLTPROC" != x -a x"$docbook_styles" != x ])
dnl  AM_CONDITIONAL(ENABLE_DBLATEX, [ test x"$DBLATEX" != x ])
  AM_CONDITIONAL(ENABLE_MAN, [ test x"$DB2X_MAN" != x -o x"$DB2X_MANXML" != x ])
  AC_SUBST(JAVA)

dnl See which version of the DocBook2x tools we have, because it
dnl forces a command line change in the Makefile.

dnl db2x_texixml (part of docbook2X 0.8.3)
dnl db2x_texixml (part of docbook2X 0.8.5)
dnl docbook2texi (part of docbook2X 0.8.8)
  if test x"${DB2X_TEXIXML}" != x; then dnl## {
    db2x_version=`${DB2X_TEXIXML} --version | head -1 | sed -e 's/^.*docbook2X //' -e 's/).*$//' 2>&1`
    DB2X_VERSION="${db2x_version}"
    AC_SUBST(DB2X_VERSION)
  fi dnl## }

dnl  AM_CONDITIONAL(NEW_DOCBOOK2X, [test "$db2x_version" = "0.8.5"])
  AC_SUBST(docbook_styles)
  AC_SUBST(XSLTPROC)
  AC_SUBST(DB2X_XSLTPROC)
  AC_SUBST(DB2X_TEXIXML)
  AC_SUBST(PDFXMLTEX)
  AC_SUBST(FOP)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
