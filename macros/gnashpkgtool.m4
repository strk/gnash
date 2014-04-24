dnl  
dnl  Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
dnl  2011, 2014 Free Software Foundation, Inc.
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


dnl Generic macros for finding and setting include-paths and library-path
dnl for packages. Implements GNASH_PKG_INCLUDES() and GNASH_PKG_LIBS().
dnl 
dnl TODO:
dnl
dnl   - always run AC_CHECK_HEADERS and AC_CHECK_LIB so that config.h end
dnl     up with correct information about what's available and what not
dnl     and every provided info is verified before acceptance.
dnl
dnl   - Document the interface of the macro!
dnl

AC_DEFUN([GNASH_PKG_INCLUDES],
[
  pushdef([UP], translit([$1], [a-z], [A-Z]))dnl Uppercase
  pushdef([DOWN], translit([$1], [A-Z], [a-z]))dnl Lowercase
  pushdef([DASHDOWN], translit([$1], [A-Z_], [a-z-]))dnl Lowercase
  pushdef([UPHEADER], translit([$2], [a-z./-], [A-Z___]))dnl Uppercase header

    $1=yes
    if test x$4 = x; then
      name=DASHDOWN
    else
      name=DASHDOWN-$4
    fi
    	    
    dnl Look for the header
    if test x"${$1}" = x"yes"; then
      AC_ARG_WITH($1_incl, AC_HELP_STRING([--with-$1-incl], [directory where $2 is]), with_$1_incl=${withval})
      AC_CACHE_VAL(ac_cv_path_$1_incl, [
      if test x"${with_$1_incl}" != x ; then
        AC_MSG_CHECKING([for $2 header in specified directory])
        if test -f ${with_$1_incl}/$2 ; then
          ac_cv_path_$1_incl="-I`(cd ${with_$1_incl}; pwd)`"
          found_$1_incl="yes"
          AC_MSG_RESULT([yes])
        else
          AC_MSG_ERROR([${with_$1_incl} directory doesn't contain $2.])
          AC_MSG_RESULT([no])
        fi
      fi
    ])

  if test x$cross_compiling = xno; then
    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_$1_incl}" = x; then
      AC_MSG_CHECKING([for $2 header using pkg-config])
      $PKG_CONFIG --exists [lib]DASHDOWN && ac_cv_path_$1_incl="`$PKG_CONFIG --cflags [lib]DASHDOWN`"
      $PKG_CONFIG --exists DASHDOWN[] && ac_cv_path_$1_incl="`$PKG_CONFIG --cflags DASHDOWN[]`"
      $PKG_CONFIG --exists lib$name && ac_cv_path_$1_incl="`$PKG_CONFIG --cflags lib$name`"
      $PKG_CONFIG --exists $name && ac_cv_path_$1_incl="`$PKG_CONFIG --cflags $name`"
      if test x"${ac_cv_path_$1_incl}" != x; then
        AC_MSG_RESULT(${ac_cv_path_$1_incl})
        found_$1_incl="yes"
      else
        AC_MSG_RESULT([not found])
      fi
    fi
    if test x"${ac_cv_path_$1_incl}" = x; then
      AC_PATH_PROG(UP[]_CONFIG, $1-config)
      if test x"${UP[]_CONFIG}" != x; then
        AC_MSG_CHECKING([for $2 header using $1-config])
        ac_cv_path_$1_incl="`${UP[]_CONFIG} --cxxflags 2>/dev/null`"
        if test x"${ac_cv_path_$1_incl}" = x; then
          ac_cv_path_$1_incl="`${UP[]_CONFIG} --cflags 2>/dev/null`"
        fi
        if test x"${ac_cv_path_$1_incl}" != x; then
        	AC_MSG_RESULT(${ac_cv_path_$1_incl})
        	found_$1_incl="yes"
      	else
        	AC_MSG_RESULT([not found])
      	fi
      fi
    fi
  fi

  AC_LIB_WITH_FINAL_PREFIX([
    eval additional_includedir=\"$includedir\"
  ])

  dnl If the path hasn't been specified, go look for it.
  if test x"${ac_cv_path_$1_incl}" = x; then
    AC_CHECK_HEADER(${additional_includedir}/$2, [ac_cv_path_$1_incl=""; found_$1_incl="yes"], [
      AC_CHECK_HEADER(${additional_includedir}/$1/$2, [ac_cv_path_$1_incl="-I${additional_includedir}/$1"; found_$1_incl="yes"], [
        AC_CHECK_HEADER(${additional_includedir}/$name/$2, [ac_cv_path_$1_incl="-I${additional_includedir}/$name"; found_$1_incl="yes"], [
          AC_CHECK_HEADER(${additional_includedir}/$2, [ac_cv_path_$1_incl="-I${additional_includedir}/$2"; found_$1_incl="yes"], [
          if test x"${ac_cv_path_$1_incl}" = x; then
            for i in $incllist; do
              if test -f $i/$name; then
                found_$1_incl="yes"
                if test x"$i" != x"${additional_includedir}"; then
                  ac_cv_path_$1_incl="-I$i"
                else
                  ac_cv_path_$1_incl="-I${additional_includedir}"
                fi
                break
              else
                if test -f $i/$name/$2; then
                  found_$1_incl="yes"
                  ac_cv_path_$1_incl="-I$i/$name"
                  break
                else
                  if test -f $i/$2; then
                    found_$1_incl="yes" 
                    ac_cv_path_$1_incl="-I$i"
                    break
                  fi
                fi
              fi
            done
          fi
        ])
      ])
    ])
  ])
  fi
  
  AC_MSG_CHECKING([for $2 header]) 
  if test x"${found_$1_incl}" = "xyes"; then

      dnl It seems we need to explicitly call AC_DEFINE as AC_CHECK_HEADER doesn't
      dnl do this automatically. AC_CHECK_HEADERS (not the final S) would do it.
      AC_DEFINE([HAVE_]UPHEADER, 1, [Define if you have the $2 header])
      AC_MSG_RESULT(${ac_cv_path_$1_incl})
      if test x"${ac_cv_path_$1_incl}" != x -a x"${ac_cv_path_$1_incl}" != x"-I${additional_includedir}"; then
        UP[]_CFLAGS="${ac_cv_path_$1_incl}"        
      else
        UP[]_CFLAGS=""
      fi
  else
  	  AC_MSG_RESULT([not found])
    fi
  fi
  AC_SUBST(UP[]_CFLAGS)

  popdef([UP])
  popdef([DOWN])
  popdef([DASHDOWN])
  popdef([UPHEADER])
])

AC_DEFUN([GNASH_PKG_LIBS], dnl GNASH_PKG_LIBS(cairo, cairo_status, [cairo render library.])
[
pushdef([UP], translit([$1], [a-z], [A-Z]))dnl Uppercase
pushdef([DOWN], translit([$1], [A-Z], [a-z]))dnl Lowercase
pushdef([DASHDOWN], translit([$1], [A-Z_], [a-z-]))dnl Lowercase

has_$1=no
ac_manual=yes

if test x"${$1}" = x"yes" -a x"${found_$1_incl}" = "xyes"; then
  dnl Look for the library
  AC_ARG_WITH($1_lib, AC_HELP_STRING([--with-$1-lib], [directory where $1 library is]), with_$1_lib=${withval})
  AC_CACHE_VAL(ac_cv_path_$1_lib,[
  if test x"${with_$1_lib}" != x ; then
    AC_MSG_CHECKING([for lib$1 library in specified directory])
    dnl look for the library with the lower case name
    if test -f ${with_$1_lib}/lib$name.a -o -f ${with_$1_lib}/lib$name.${shlibext}; then
      tmp="`(cd ${with_$1_lib}; pwd)`"
      ac_cv_path_$1_lib="-L${tmp} -l$name"
      AC_MSG_RESULT([yes])
    else
      dnl look for the library with the unchanged case name
      if test x"${ac_cv_path_$1_lib}" = x -a -f ${with_$1_lib}/lib$1.a -o -f ${with_$1_lib}/lib$1.${shlibext}; then
        tmp="`(cd ${with_$1_lib}; pwd)`"
        ac_cv_path_$1_lib="-L${tmp} -l$1"
        AC_MSG_RESULT([yes])
      else
        AC_MSG_ERROR([${with_$1_lib} directory doesn't contain library $name.])
        AC_MSG_RESULT([no])
      fi
    fi
  fi
  ])

  dnl If the header doesn't exist, there is no point looking for the library.
  if test x$cross_compiling = xno; then
    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_$1_lib}" = x; then
      AC_MSG_CHECKING([for lib$1 library using pkg-config])
      $PKG_CONFIG --exists [lib]DASHDOWN && ac_cv_path_$1_lib="`$PKG_CONFIG --libs [lib]DASHDOWN`"
      $PKG_CONFIG --exists DASHDOWN && ac_cv_path_$1_lib="`$PKG_CONFIG --libs DASHDOWN`"
      $PKG_CONFIG --exists lib$name && ac_cv_path_$1_lib="`$PKG_CONFIG --libs lib$name`"
      $PKG_CONFIG --exists $name && ac_cv_path_$1_lib="`$PKG_CONFIG --libs $name`"
      if test x"${ac_cv_path_$1_lib}" != x; then
        AC_MSG_RESULT(${ac_cv_path_$1_lib})
        ac_manual=no
      else
        AC_MSG_RESULT([not found])
      fi
    fi
    if test x"${ac_cv_path_$1_lib}" = x; then
      AC_PATH_PROG(UP[]_CONFIG, $1-config)
      if test x"${UP[]_CONFIG}" != x; then
        AC_MSG_CHECKING([for lib$1 library using $1-config])
        ac_cv_path_$1_lib="`${UP[]_CONFIG} --libs`"
        AC_MSG_RESULT(${ac_cv_path_$1_lib})
      fi
      if test x"${ac_cv_path_$1_lib}" = x; then
      		ac_manual=no
      fi
    fi
  fi
  
  if test x"${ac_manual}" != xyes -a x"${ac_cv_path_$1_lib}" = x; then
	ac_manual=yes
  fi  

  AC_LIB_WITH_FINAL_PREFIX([
    eval additional_libdir=\"$libdir\"
  ])
  
  if test x"${ac_cv_path_$1_lib}" = x; then
    ac_save_LIBS=$LIBS
    LIBS=""
    for i in $libslist; do
      if test -f $i/lib$1.a -o -f $i/lib$1.${shlibext}; then
        if test -f "$i/lib$1.a" -o -f "$i/lib$1.${shlibext}"; then
          if test ! x"$i" = x"${additional_libdir}"; then
            ac_cv_path_$1_lib="-L$i -l$1 $5"
          else
            ac_cv_path_$1_lib="-L${additional_libdir} -l$1 $5"
          fi
          break
        fi
      else
        if test -f "$i/lib$name.a" -o -f "$i/lib$name.${shlibext}"; then
          if test ! x"$i" = x"${additional_libdir}"; then
            ac_cv_path_$1_lib="-L$i -l$name $5"
          else
            ac_cv_path_$1_lib="-L${additional_libdir} -l$name $5"
          fi
        break
        fi
      fi
    done
    LIBS=$ac_save_LIBS
  fi

  if test x"${ac_cv_path_$1_lib}" = x ; then
    AC_SEARCH_LIBS($2, $1 $name, [ac_cv_path_$1_lib="$LIBS $5"])
  fi
  
  if test x"${ac_manual}" = xyes ; then
  	AC_MSG_CHECKING([for lib$1 library]) 
	if test x"${ac_cv_path_$1_lib}" != x ; then
    AC_MSG_RESULT(${ac_cv_path_$1_lib})
	else
    AC_MSG_RESULT([not found])
    fi  	
  fi
  
  if test x"${ac_cv_path_$1_lib}" != x ; then
    UP[]_LIBS="${ac_cv_path_$1_lib}"
    has_$1=yes
  else
    UP[]_LIBS=""
  fi
fi

AC_SUBST(UP[]_LIBS)

popdef([UP])
popdef([DOWN])
popdef([DASHDOWN])
])

dnl Example: GNASH_PKG_FIND(fltk, [FL_API.h], [fltk gui], fl_xmap, [], [-lfltk_gl])
AC_DEFUN([GNASH_PKG_FIND],
[
GNASH_PKG_INCLUDES($1, $2, $3, $5)
GNASH_PKG_LIBS($1, $4, $3, $5, $6)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
