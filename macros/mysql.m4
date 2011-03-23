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


AC_DEFUN([GNASH_PATH_MYSQL],
[
  dnl Lool for the header
  AC_ARG_WITH(mysql-incl, AC_HELP_STRING([--with-mysql-incl], [directory where mysql headers are]), with_mysql_incl=${withval})
  AC_CACHE_VAL(ac_cv_path_mysql_incl,[
    if test x"${with_mysql_incl}" != x ; then
      if test -f ${with_mysql_incl}/mysql.h ; then
        ac_cv_path_mysql_incl="-I`(cd ${with_mysql_incl}; pwd)`"
      else
        AC_MSG_ERROR([${with_mysql_incl} directory doesn't contain any headers])
      fi
    fi
  ])

  if test x"${ac_cv_path_mysql_incl}" = x; then
    AC_CHECK_PROG(mconfig, mysql_config, mysql_config)
  fi
  if test x"${ac_cv_prog_mconfig}" = "x" ; then
     AC_CHECK_PROG(mconfig, mysql-config, mysql-config)
  fi

  if test x"${ac_cv_prog_mconfig}" != "x" ; then
     ac_cv_path_mysql_incl=`${mconfig} --include`
  fi

  AC_MSG_CHECKING([for MySQL headers])
  if test x"${ac_cv_path_mysql_incl}" = x ; then
    AC_MSG_CHECKING([for mysql header])
    for i in $incllist; do
      if test -f $i/mysql/mysql.h; then
        ac_cv_path_mysql_incl="-I$i/mysql"
        break
      fi
    done
  fi

  if test x"${ac_cv_path_mysql_incl}" != x ; then
    MYSQL_CFLAGS="${ac_cv_path_mysql_incl}"
    AC_MSG_RESULT(yes)
  else
    MYSQL_CFLAGS=""
    AC_MSG_RESULT(no)
  fi

  dnl Look for the library
  AC_ARG_WITH(mysql-lib, AC_HELP_STRING([--with-mysql-lib], [directory where mysql libraries are]), with_mysql_lib=${withval})
    AC_CACHE_VAL(ac_cv_path_mysql_lib,[
    if test x"${with_mysql_lib}" != x ; then
      if test -f ${with_mysql_lib}/libmysqlclient.a -o -f ${with_mysql_lib}/libmysqlclient.${shlibext}; then
	      ac_cv_path_mysql_lib="-L`(cd ${with_mysql_lib}; pwd)`"
      else
        AC_MSG_ERROR([${with_mysql_lib} directory doesn't contain mysql libraries.])
      fi
    fi
  ])

  AC_MSG_CHECKING([for MySQL libraries])
  if test x"${ac_cv_prog_mconfig}" != "x" ; then
     ac_cv_path_mysql_lib=`${mconfig} --libs`
  fi

  if test x"${ac_cv_path_mysql_lib}" = x; then #{

    topdir=""

    AC_CHECK_LIB(mysqlclient, mysql_init, [ac_cv_path_mysql_lib="-lmysqlclient"], [
      for i in $libslist; do
	      if test -f $i/libmysqlclient.a -o -f $i/libmysqlclient.${shlibext}; then
          topdir=$i
	        if test ! x"$i" = x"/usr/lib" -a ! x"$i" = x"/usr/lib64"; then
	          ac_cv_path_mysql_lib="-L$i -lmysqlclient"
       	    break
          else
	          ac_cv_path_mysql_lib="-lmysqlclient"
	          break
          fi
        fi
      done
    ])
    AC_MSG_CHECKING([for MySQL client library])
  fi #}


  if test x"${ac_cv_path_mysql_lib}" != x; then
    AC_MSG_RESULT(${ac_cv_path_mysql_lib})
    MYSQL_LIBS="${ac_cv_path_mysql_lib}"
  else
    AC_MSG_RESULT(no)
    MYSQL_LIBS=""
  fi

  if test x"${ac_cv_path_mysql_incl}" != x -a x"${ac_cv_path_mysql_lib}" != x; then
    AC_DEFINE(HAVE_MYSQL, [], [Defined if you have MySQL installed])
  fi
  AC_SUBST(MYSQL_CFLAGS)  
  AC_SUBST(MYSQL_LIBS)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
