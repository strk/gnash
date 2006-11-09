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

dnl $Id: sdl.m4,v 1.34 2006/11/09 18:28:15 nihilus Exp $

AC_DEFUN([GNASH_PATH_SDL],
[dnl 
  has_sdl=no
  dnl Lool for the header
  AC_ARG_WITH(sdl_incl, AC_HELP_STRING([--with-sdl-incl], [directory where sdl header is]), with_sdl_incl=${withval})
  AC_CACHE_VAL(ac_cv_path_sdl_incl,[
  if test x"${with_sdl_incl}" != x ; then
    if test -f ${with_sdl_incl}/SDL.h ; then
      ac_cv_path_sdl_incl=-I`(cd ${with_sdl_incl}; pwd)`
    else
      AC_MSG_ERROR([${with_sdl_incl} directory doesn't contain SDL.h])
    fi
  fi
  ])

if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_sdl_incl}" = x; then
	$PKG_CONFIG --exists sdl && ac_cv_path_sdl_incl=`$PKG_CONFIG --cflags sdl` 
fi

  dnl Attempt to find the top level directory, which unfortunately has a
  dnl version number attached. At least on Debain based systems, this
  dnl doesn't seem to get a directory that is unversioned.

AC_MSG_CHECKING([for the SDL Version])
  
if test x"$PKG_CONFIG" != x; then
	$PKG_CONFIG --exists sdl && gnash_sdl_version=`$PKG_CONFIG --modversion sdl`
fi

  if test x"${gnash_sdl_version}" = x; then
    pathlist="${prefix}/${target_alias}/include /sw/include /usr/local/include /opt/local/include /home/latest/include /opt/include /opt/local/include /usr/include /usr/pkg/include .. ../.."

    gnash_sdl_topdir=""
    gnash_sdl_version=""
    for i in $pathlist; do
      for j in `ls -dr $i/SDL-[[0-9]].[[0-9]] 2>/dev/null`; do
 	if test -f $j/SDL.h; then
	  gnash_sdl_topdir=`basename $j`
	  gnash_sdl_version=`echo ${gnash_sdl_topdir} | sed -e 's:SDL-::'`
          break
        fi
      done
      dnl This is a special caze for FreeBSD, that uses SDL11 instead of SDL-1.1.
      for j in `ls -dr $i/SDL[[0-9]][[0-9]] 2>/dev/null`; do
 	if test -f $j/SDL.h; then
	  gnash_sdl_topdir=`basename $j`
	  gnash_sdl_version=`echo ${gnash_sdl_topdir} | sed -e 's:SDL::'`
          break
        fi
      done
    done
  fi
  AC_MSG_RESULT(${gnash_sdl_version})
 
  AC_MSG_CHECKING([for SDL header])  
  if test x"${ac_cv_path_sdl_incl}" = x; then
    incllist="${prefix}/${target_alias} ${prefix} /usr /usr/pkg /sw /opt/local /opt/local/include /usr/local /home/latest /opt /usr .. ../.."

    for i in $incllist; do
      if test -f $i/include/SDL/SDL.h; then
        ac_cv_path_sdl_incl=$i/include/SDL
        break
      fi
      if test -f $i/include/SDL-${gnash_sdl_version}/SDL.h; then
        ac_cv_path_sdl_incl=$i/include/SDL-${gnash_sdl_version}
        break
      else
        if test -f $i/include/SDL${gnash_sdl_version}/SDL.h; then
          ac_cv_path_sdl_incl=$i/include/SDL${gnash_sdl_version}
          break
	fi
      fi
    done

    SDL_CFLAGS=""
    if test x"${ac_cv_path_sdl_incl}" = x ; then
      AC_CHECK_HEADERS(SDL.h, [ac_cv_path_sdl_incl=""])
    else
      if test x"${ac_cv_path_sdl_incl}" != x"/usr/include"; then
        ac_cv_path_sdl_incl="-I${ac_cv_path_sdl_incl}"
      else
        ac_cv_path_sdl_incl=""
      fi
    fi
  fi
  AC_MSG_RESULT(${ac_cv_path_sdl_incl})
  
  dnl Look for the library
  AC_ARG_WITH(sdl_lib, AC_HELP_STRING([--with-sdl-lib], [directory where sdl library is]), with_sdl_lib=${withval})
dnl  AC_MSG_CHECKING([for sdl library])
  AC_CACHE_VAL(ac_cv_path_sdl_lib,[
  if test x"${with_sdl_lib}" != x ; then
    if test -f ${with_sdl_libs}/libSDL.a -o -f ${with_sdl_lib}/libSDL.so; then
      ac_cv_path_sdl_lib="-L`(cd ${with_sdl_lib}; pwd)` -lSDL"
    else
      if test -f ${with_sdl_libs}/libSDL-1.1.a -o -f ${with_sdl_lib}/libSDL-1.1.so; then
        ac_cv_path_sdl_lib="-L`(cd ${with_sdl_lib}; pwd)` -lSDL"
      else
        AC_MSG_ERROR([${with_sdl_lib} directory doesn't contain libSDL-1.1.a])
      fi
    fi
  fi
  ])

  SDL_LIBS=""
  
  if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_sdl_lib}" = x; then
    $PKG_CONFIG --exists sdl && ac_cv_path_sdl_lib=`$PKG_CONFIG --libs sdl`
    if test x"$ac_cv_path_sdl_lib" != x; then
      has_sdl=yes
    fi
  fi
  
  if test x"${ac_cv_path_sdl_lib}" = x ; then
    AC_CHECK_LIB(SDL, SDL_Init, [ac_cv_path_sdl_lib="-lSDL"],[
      liblist="${prefix}/${target_alias}/lib ${prefix}/lib64 ${prefix}/lib /opt/local/lib /usr/lib64 /usr/lib /usr/pkg/lib /sw/lib /usr/local/lib /opt/local/lib /home/latest/lib /opt/lib.. ../.."
      for i in $liblist; do
        if test -f $i/libSDL.a -o -f $i/libSDL.so; then
          if test x"$i" != x"/usr/lib"; then
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
          if test -f $i/libSDL-1.1.a -o -f $i/libSDL-1.1.so; then
            if test x"$i" != x"/usr/lib"; then
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
    ])
  fi  
  AC_MSG_CHECKING([for SDL library])
  AC_MSG_RESULT(${ac_cv_path_sdl_lib})
  
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
