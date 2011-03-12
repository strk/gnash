dnl  
dnl    Copyright (C) 2005, 2006, 2009, 2010 Free Software Foundation, Inc.
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


AC_DEFUN([GNASH_PATH_PYTHON],
[

  dnl Look for the header
  AC_ARG_WITH(python_incl, AC_HELP_STRING([--with-python-incl], [directory where libpython header is (w/out the python/ prefix)]), with_python_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_python_incl,[
    if test x"${with_python_incl}" != x ; then
      if test -f ${with_python_incl}/python/python.h ; then
	      ac_cv_path_python_incl="`(cd ${with_python_incl}; pwd)`"
      else
	      AC_MSG_ERROR([${with_python_incl} directory doesn't contain python/python.h])
      fi
    fi
  ])

  if test x"${python}" = x"yes"; then
    # Look for the python-config script
    pythonconfig=""
    AC_PATH_PROG(pythonconfig, python-config, ,[${pathlist}])
    if  test x"${pythonconfig}" = x; then
      AC_MSG_CHECKING([for versioned python-config])
      for i in `echo ${pathlist} | sed  's|:| |g' 2>/dev/null`; do
        pythonconfig="`ls ${i}/python2.*-config 2>/dev/null | head -1`"
        if test x"${pythonconfig}" != x; then
          break
        fi
      done
      if test x"${pythonconfig}" != x; then
        AC_MSG_RESULT([${pythonconfig}])
      else
        AC_MSG_RESULT([no])
      fi
    fi


    dnl If the path hasn't been specified, go look for it.
    if test x"${ac_cv_path_python_incl}" = x; then
      if test x"${pythonconfig}" != "x"; then
        ac_cv_path_python_incl="`${pythonconfig} --include`"
      else
        for i in $incllist; do
          for j in `ls -dr $i/python2.* 2>/dev/null`;do
            if test -f $j/pythonrun.h; then
              ac_cv_path_python_incl="-I$j"
              break 2
            fi
          done
        done
      fi

      if test x"${ac_cv_path_python_incl}" = x ; then
        AC_CHECK_HEADERS(pythonrun.h)
      fi

      AC_MSG_CHECKING([for libpython header])
      if test x"${ac_cv_path_python_incl}" != x ; then
        AC_MSG_RESULT(yes)
      else
        AC_MSG_RESULT(no)
      fi
    fi


    if test x"${ac_cv_path_python_incl}" != x"/usr/include"; then
      ac_cv_path_python_incl="${ac_cv_path_python_incl}"
    else
      ac_cv_path_python_incl=""
    fi

    dnl Look for the library
    AC_ARG_WITH(python_lib, AC_HELP_STRING([--with-python-lib], [directory where python library is]), with_python_lib=${withval})
      AC_CACHE_VAL(ac_cv_path_python_lib,[
      if test x"${with_python_lib}" != x ; then # {
        if test -f ${with_python_lib}/libpython.a -o -f ${with_python_lib}/libpython.${shlibext}; then # {
          ac_cv_path_python_lib="-L`(cd ${with_python_lib}; pwd)`"
        else # }{
          AC_MSG_ERROR([${with_python_lib} directory doesn't contain libpython.])
        fi # }
      fi # }
    ])

    dnl If the path hasn't been specified, go look for it.
    if test x"${ac_cv_path_python_lib}" = x; then # {
      if test x"${pythonconfig}" != "x" -a x"${darwin}" = xno; then # {
        dnl python-config gives us way to many libraries, which create nasty linking
        dnl dependancy issue, so we strip them off here. The real dependencies are
        dnl are taken care of by other config tests.
        ac_cv_path_python_lib=`${pythonconfig} --libs`
      else # }{
        AC_MSG_CHECKING([for libpython library])
        for i in $libslist; do # {
          if test -f $i/libpython.a -o -f $i/libpython.${shlibext}; then # {
            if test ! x"$i" = x"/usr/lib" -a ! x"$i" = x"/usr/lib64"; then # {
              ac_cv_path_python_lib="-L$i -lpython"
              AC_MSG_RESULT(${ac_cv_path_python_lib})
              break
            else # }{
              ac_cv_path_python_lib="-lpython2.5"
              AC_MSG_RESULT(yes)
              break
            fi # }
          fi # }
        done # }
        if test x"${ac_cv_path_python_lib}" = x; then # {
          AC_MSG_RESULT(no)
        fi # }
      fi # }
    fi # }

    if test x"${ac_cv_path_python_incl}" != x ; then
      PYTHON_CFLAGS="${ac_cv_path_python_incl}"
    else
      PYTHON_CFLAGS=""
    fi

    if test x"${ac_cv_path_python_lib}" != x ; then
      PYTHON_LIBS="${ac_cv_path_python_lib}"
    else
      PYTHON_LIBS=""
    fi

    if test -n "$PYTHON_LIBS"; then
      AC_DEFINE(HAS_PYTHON, [1], [Define this if you want to enable python usage])
      has_python=yes
    else
      has_python=no
    fi

    AM_PATH_PYTHON
    AC_PATH_PROG(PYGOBJECT_CODEGEN, pygobject-codegen-2.0, no)
    if test x"${PYGOBJECT_CODEGEN}" = xno; then
      AC_MSG_WARN(could not find pygobject-codegen-2.0 script)
      AC_PATH_PROG(PYGTK_CODEGEN, pygtk-codegen-2.0, no)
      if test x"${PYGTK_CODEGEN}" = xno; then
        AC_MSG_ERROR(could not find pygtk-codegen-2.0 script)
       fi
     fi

    PKG_CHECK_MODULES(PYGTK, pygtk-2.0)

    PYGTK_DEFSDIR=`$PKG_CONFIG --variable=defsdir pygtk-2.0`
  fi

  AC_SUBST(PYGOBJECT_CODEGEN)
  AC_SUBST(PYGTK_CODEGEN)
  AC_SUBST(PYGTK_DEFSDIR)
  AC_SUBST(PYTHON_CFLAGS)  
  AC_SUBST(PYTHON_LIBS)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
