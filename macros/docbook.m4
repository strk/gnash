dnl Process this file with autoconf to produce a configure script.
dnl
dnl  Copyright (C) 2005 Free Software Foundation, Inc.
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

AC_DEFUN([AC_DOCBOOK_STYLES], [
  dirlist="/usr/share/sgml/docbook/xsl-stylesheets /usr/local/share/sgml/docbook/xsl-stylesheets /opt/share/sgml/docbook/xsl-stylesheets /home/latest/share/sgml/docbook/xsl-stylesheets /usr/share/sgml/docbook/stylesheet/xsl/nwalsh"
  for i in $dirlist; do
       if test -f $i/html/docbook.xsl; then
         docbook_styles=`(cd $i; pwd)`
         break
       fi
  done

dnl FIXME: Make sure fop.sh exists too!

  AC_SUBST(docbook_styles)
])

