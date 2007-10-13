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

dnl $Id: libXML.m4,v 1.32 2007/10/13 23:24:08 rsavoye Exp $

AC_DEFUN([GNASH_PATH_LIBXML], [
  has_xml=no
  dnl Lool for the header
  AC_ARG_WITH(libxml-incl, AC_HELP_STRING([--with-libxml-incl], [directory where libxml2 header is]), with_libxml_incl=${withval})
  AC_CACHE_VAL(ac_cv_path_libxml_incl, [
    if test x"${with_libxml_incl}" != x ; then
      if test -f ${with_libxml_incl}/libxml/xmlmemory.h ; then
        ac_cv_path_libxml_incl=-I`(cd ${with_libxml_incl}; pwd)`
      else
        AC_MSG_ERROR([${with_libxml_incl} directory doesn't contain libxml/xmlmemory.h])
      fi
    fi
  ])

  if test x"${ac_cv_path_libxml_incl}" = x; then
    AC_PATH_PROG(XML2_CONFIG, xml2-config, ,[${pathlist}])
    if test "x$XML2_CONFIG" != "x"  -a x"${darwin}" = xno ; then
      if test "x$XML2_CFLAGS" = "x" ; then
        ac_cv_path_libxml_incl=`$XML2_CONFIG --cflags`
      fi
      if test "x$XML2_LIBS" = "x" ; then
        ac_cv_path_libxml_lib=`$XML2_CONFIG --libs | sed -e 's:-L/usr/lib::'`
      fi
    else
      AC_MSG_RESULT(no)
    fi
  fi

  gnash_libxml2_topdir=""
  gnash_libxml2_version=""
  AC_MSG_CHECKING([for libxml2 header])  
  if test x"${ac_cv_path_libxml_incl}" = x; then
    for i in ${incllist}; do
      for j in `ls -dr $i/libxml2 2>/dev/null`; do
 	      if test -f $j/libxml/xmlmemory.h; then
      	  gnash_libxml_topdir=`basename $j`
      	  gnash_libxml_version=`echo ${gnash_libxml2_topdir} | sed -e 's:libxml2::' -e 's:-::'`
      	  ac_cv_path_libxml_incl="-I$j"
          AC_MSG_RESULT(${ac_cv_path_libxml_incl})
          break
        fi
      done
    done
  fi
 
  if test x"${ac_cv_path_libxml_incl}" = x ; then
    AC_MSG_RESULT(no)
    AC_CHECK_HEADERS(libxml/xmlmemory.h, [ac_cv_path_libxml_incl=""])
  fi

  dnl Look for the library
  AC_ARG_WITH(libxml_lib, AC_HELP_STRING([--with-libxml-lib], [directory where libxml2 library is]), with_libxml_lib=${withval})
  AC_CACHE_VAL(ac_cv_path_libxml_lib, [
    if test x"${with_libxml_lib}" != x ; then
      if test -f ${with_libxml_libs}/libxml2.a -o -f ${with_libxml_lib}/libxml2.${shlibext}; then
        ac_cv_path_libxml_lib="-L`(cd ${with_libxml_lib}; pwd)` -lxml2"
      fi
    fi
  ])
  AC_MSG_CHECKING([for libxml library])
  if test x"${ac_cv_path_libxml_lib}" = x ; then
    for i in $libslist; do
      if test -f $i/libxml2.a -o -f $i/libxml2.${shlibext}; then
        if test ! x"$i" = x"/usr/lib" -a ! x"$i" = x"/usr/lib64"; then
          ac_cv_path_libxml_lib="-L$i -lxml2"
          break
        else
          ac_cv_path_libxml_lib="-lxml2"
	        has_xml=yes
          break
        fi
      fi
    done
    AC_MSG_RESULT(${ac_cv_path_libxml_lib}) 
  fi
  if test x"${ac_cv_path_libxml_lib}" = x ; then
    AC_CHECK_LIB(libxml2, libxml_Init, [ac_cv_path_libxml_lib="-lxml2"])
  fi  
  AC_MSG_CHECKING([for libxml2 library])
  AC_MSG_RESULT(${ac_cv_path_libxml_lib}) 
  if test x"${ac_cv_path_libxml_incl}" != x ; then
    LIBXML_CFLAGS="${ac_cv_path_libxml_incl}"
  else
    LIBXML_CFLAGS=""
  fi
  if test x"${ac_cv_path_libxml_lib}" != x ; then
    LIBXML_LIBS="${ac_cv_path_libxml_lib}"
    has_xml=yes
    AC_DEFINE(HAVE_LIBXML_H, [1], [We have libxml2 support])
  else
    has_xml=no
    LIBXML_LIBS=""
  fi
  AC_SUBST(LIBXML_CFLAGS)
  AC_SUBST(LIBXML_LIBS)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
