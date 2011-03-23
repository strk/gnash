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


AC_DEFUN([GNASH_PATH_SDL], [
  has_sdl=no
  dnl Lool for the header
  AC_ARG_WITH(sdl-incl, AC_HELP_STRING([--with-sdl-incl], [directory where sdl header is]), with_sdl_incl=${withval})
  AC_CACHE_VAL(ac_cv_path_sdl_incl, [
    if test x"${with_sdl_incl}" != x ; then
      if test -f ${with_sdl_incl}/SDL.h ; then
        ac_cv_path_sdl_incl="-I`(cd ${with_sdl_incl}; pwd)`"
      else
        AC_MSG_ERROR([${with_sdl_incl} directory doesn't contain SDL.h])
      fi
    fi
  ])
  if test x${cross_compiling} = xno; then
    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_sdl_incl}" = x; then
	    $PKG_CONFIG --exists sdl && ac_cv_path_sdl_incl=`$PKG_CONFIG --cflags sdl` 
    fi
  fi

  dnl Attempt to find the top level directory, which unfortunately has a
  dnl version number attached. At least on Debain based systems, this
  dnl doesn't seem to get a directory that is unversioned.

  if test x${cross_compiling} = xno; then
    if test x"$PKG_CONFIG" != x; then
      dnl What's the point of checking for SDL version here if we wipe out that info below ?? .. dropping the check
      dnl AC_MSG_CHECKING([for the SDL Version])  
      dnl $PKG_CONFIG --exists sdl && gnash_sdl_version=`$PKG_CONFIG --modversion sdl`
      $PKG_CONFIG --exists sdl && ac_cv_path_sdl_incl=`$PKG_CONFIG --cflags sdl`
      dnl AC_MSG_RESULT(${gnash_sdl_version})
    fi
  fi

  AC_PATH_PROG(SDL_CONFIG, sdl-config, , ,[${pathlist}])
  if test "x$SDL_CONFIG" != "x" ; then
    if test "x$SDL_CFLAGS" = "x" ; then
      SDL_CFLAGS=`$SDL_CONFIG --cflags`
	if test x${cross_compiling} = xno; then
      		ac_cv_path_sdl_incl=$SDL_CFLAGS
	fi
    fi
    if test "x$SDL_LIBS" = "x" ; then
      SDL_LIBS=`$SDL_CONFIG --libs | sed -e 's:-L/usr/lib\>::'`
	if test x${cross_compiling} = xno; then
		ac_cv_path_sdl_lib=$SDL_LIBS
	fi
    fi
  fi

  gnash_sdl_topdir=""
  gnash_sdl_version=""
  if test x"${ac_cv_path_sdl_incl}" = x; then
    AC_MSG_CHECKING([for SDL header])  
    for i in ${incllist}; do
      for j in `ls -dr $i/SDL* 2>/dev/null`; do
        if test -f $j/SDL.h; then
      	  gnash_sdl_topdir=`basename $j`
      	  gnash_sdl_version=`echo ${gnash_sdl_topdir} | sed -e 's:SDL::' -e 's:-::'`
      	  ac_cv_path_sdl_incl="-I$j"
          break
        fi
      done
      if test x"${ac_cv_path_sdl_incl}" != x; then
        break
      fi
    done
    if test x"${ac_cv_path_sdl_incl}" != x; then
        AC_MSG_RESULT(${ac_cv_path_sdl_incl})
    else
        AC_MSG_RESULT([not found in incllist])
    fi
  fi
 
  dnl This is sorta bogus atm.
  SDL_CFLAGS=""
  if test x"${ac_cv_path_sdl_incl}" = x ; then
    AC_CHECK_HEADERS(SDL.h, [ac_cv_path_sdl_incl=""])
  else
    if test x"${ac_cv_path_sdl_incl}" != x"/usr/include"; then
      ac_cv_path_sdl_incl="${ac_cv_path_sdl_incl}"
    else
      ac_cv_path_sdl_incl=""
    fi
  fi
  
  dnl Look for the library
  AC_ARG_WITH(sdl_lib, AC_HELP_STRING([--with-sdl-lib], [directory where sdl library is]), with_sdl_lib=${withval})
dnl  AC_MSG_CHECKING([for sdl library])
  AC_CACHE_VAL(ac_cv_path_sdl_lib, [
    if test x"${with_sdl_lib}" != x ; then
      if test -f ${with_sdl_lib}/libSDL.a -o -f ${with_sdl_lib}/libSDL.${shlibext}; then
        ac_cv_path_sdl_lib="-L`(cd ${with_sdl_lib}; pwd)` -lSDL"
      else
        if test -f ${with_sdl_lib}/libSDL-1.1.a -o -f ${with_sdl_lib}/libSDL-1.1.${shlibext}; then
          ac_cv_path_sdl_lib="-L`(cd ${with_sdl_lib}; pwd)` -lSDL"
        else
          AC_MSG_ERROR([${with_sdl_lib} directory doesn't contain libSDL])
        fi
      fi
    fi
  ])

  SDL_LIBS=""
  if test x${cross_compiling} = xno; then
    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_sdl_lib}" = x; then
      $PKG_CONFIG --exists sdl && ac_cv_path_sdl_lib=`$PKG_CONFIG --libs sdl`
      if test x"$ac_cv_path_sdl_lib" != x; then
        has_sdl=yes
      fi
    fi
  fi
  
  if test x"${ac_cv_path_sdl_lib}" = x ; then
    for i in $libslist; do
      if test -f $i/libSDL.a -o -f $i/libSDL.${shlibext}; then
        if test ! x"$i" = x"/usr/lib" -o x"$i" = x"/usr/lib64"; then
          ac_cv_path_sdl_lib="-L$i -lSDL"
          dnl AC_MSG_RESULT(${ac_cv_path_sdl_lib})
          break
        else
          ac_cv_path_sdl_lib="-lSDL"
          dnl AC_MSG_RESULT([yes])
	  has_sdl=yes
          break
        fi
      else
        if test -f $i/libSDL-1.1.a -o -f $i/libSDL-1.1.${shlibext}; then
          if test ! x"$i" = x"/usr/lib" -o x"$i" = x"/usr/lib64"; then
            ac_cv_path_sdl_lib="-L$i -lSDL-1.1"
            dnl AC_MSG_RESULT(${ac_cv_path_sdl_lib})
            break
          else
            ac_cv_path_sdl_lib="-lSDL-1.1"
            dnl AC_MSG_RESULT([yes])
            has_sdl=yes
            break
          fi
        fi
      fi
    done
  fi
  if test x"${ac_cv_path_sdl_lib}" = x ; then
    AC_CHECK_LIB(SDL, SDL_Init, [ac_cv_path_sdl_lib="-lSDL"])
  fi  
  dnl AC_MSG_CHECKING([for SDL library])
  dnl AC_MSG_RESULT(${ac_cv_path_sdl_lib}) 
  if test x"${ac_cv_path_sdl_incl}" != x ; then
    SDL_CFLAGS="${ac_cv_path_sdl_incl}"
  fi
  if test x"${ac_cv_path_sdl_lib}" != x ; then
    SDL_LIBS="${ac_cv_path_sdl_lib}"
    has_sdl=yes
    AC_DEFINE(HAVE_SDL_H, [1], [We have SDL support])
  else
    has_sdl=no
  fi
  AC_SUBST(SDL_CFLAGS)
  AC_SUBST(SDL_LIBS)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
