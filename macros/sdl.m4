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

AC_DEFUN([GNASH_PATH_SDL],
[dnl 
  has_sdl=no
  dnl Lool for the header
  AC_ARG_WITH(sdl_incl, [  --with-sdl-incl   directory where sdl header is], with_sdl_incl=${withval})
  AC_CACHE_VAL(ac_cv_path_sdl_incl,[
  if test x"${with_sdl_incl}" != x ; then
    if test -f ${with_sdl_incl}/SDL.h ; then
      ac_cv_path_sdl_incl=`(cd ${with_sdl_incl}; pwd)`
    else
      AC_MSG_ERROR([${with_sdl_incl} directory doesn't contain SDL.h])
    fi
  fi
  ])

  dnl Attempt to find the top level directory, which unfortunately has a
  dnl version number attached. At least on Debain based systems, this
  dnl doesn't seem to get a directory that is unversioned.
  if test x"${ac_cv_path_sdl_incl}" = x; then
    AC_MSG_CHECKING([for the SDL Version])
    pathlist="/sw/include /usr/local/include /home/latest/include /opt/include /usr/include /usr/pkg/include .. ../.."

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

  if test x"${ac_cv_path_sdl_incl}" = x ; then
    AC_MSG_CHECKING([for SDL header])
    incllist="${prefix} /usr /usr/pkg /sw /usr/local /home/latest /opt /usr .. ../.."

    for i in $incllist; do
      if test -f $i/SDL/include/SDL.h; then
        ac_cv_path_sdl_incl=$i/SDL/include
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
      AC_MSG_RESULT(none)
      AC_CHECK_HEADERS(SDL.h, [ac_cv_path_sdl_incl=""])
    else
      AC_MSG_RESULT(${ac_cv_path_sdl_incl})
      if test x"${ac_cv_path_sdl_incl}" != x"/usr/include"; then
        ac_cv_path_sdl_incl="${ac_cv_path_sdl_incl}"
      else
        ac_cv_path_sdl_incl=""
      fi
    fi
  fi

  dnl Look for the library
  AC_ARG_WITH(sdl_lib, [  --with-sdl-lib    directory where sdl library is], with_sdl_lib=${withval})
  AC_MSG_CHECKING([for sdl library])
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
  if test x"${ac_cv_path_sdl_lib}" = x ; then
    AC_CHECK_LIB(SDL, SDL_Init, [ac_cv_path_sdl_lib="-lSDL"],[
      AC_MSG_CHECKING([for SDL library])
      liblist="${prefix}/lib64 ${prefix}/lib /usr/lib64 /usr/lib /usr/pkg/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib.. ../.."
      for i in $liblist; do
        if test -f $i/libSDL.a -o -f $i/libSDL.so; then
          if test x"$i" != x"/usr/lib"; then
            ac_cv_path_sdl_lib="-L$i -lSDL"
            AC_MSG_RESULT(${ac_cv_path_sdl_lib})
            break
          else
            ac_cv_path_sdl_lib="-lSDL"
            AC_MSG_RESULT([yes])
	    has_sdl=yes
            break
          fi
	else
          if test -f $i/libSDL-1.1.a -o -f $i/libSDL-1.1.so; then
            if test x"$i" != x"/usr/lib"; then
              ac_cv_path_sdl_lib="-L$i -lSDL-1.1"
              AC_MSG_RESULT(${ac_cv_path_sdl_lib})
              break
            else
              ac_cv_path_sdl_lib="-lSDL-1.1"
              AC_MSG_RESULT([yes])
	      has_sdl=yes
              break
            fi
	  fi
        fi
      done
    ])
  fi

  if test x"${ac_cv_path_sdl_incl}" != x ; then
    SDL_CFLAGS="-I${ac_cv_path_sdl_incl}"
  fi

  if test x"${ac_cv_path_sdl_lib}" != x ; then
    SDL_LIBS="${ac_cv_path_sdl_lib}"
    has_sdl=yes
    AC_DEFINE(HAVE_SDL_H, [], [We have SDL support])
  else
    has_sdl=no
  fi

  AC_SUBST(SDL_CFLAGS)
  AC_SUBST(SDL_LIBS)
])

AC_DEFUN([GNASH_PATH_SDL_MIXER],
[
  dnl Lool for the header
  AC_ARG_WITH(sdl_mixer_incl, [  --with-sdl-mixer-incl   directory where sdl_mixer header is], with_sdl_mixer_incl=${withval})
  AC_CACHE_VAL(ac_cv_path_sdl_mixer_incl,[
  if test x"${with_sdl_mixer_incl}" != x ; then
    if test -f ${with_sdl_mixer_incl}/SDL_mixer.h ; then
      ac_cv_path_sdl_mixer_incl=`(cd ${with_sdl_mixer_incl}; pwd)`
    else
      AC_MSG_ERROR([${with_sdl_mixer_incl} directory doesn't contain SDL_mixer.h])
    fi
  fi
  ])
  if test x"${ac_cv_path_sdl_mixer_incl}" = x ; then
    AC_MSG_CHECKING([for SDL_mixer header])
    incllist="${prefix} /usr/pkg /sw /usr/local /home/latest /opt /usr .. ../.."

    for i in $incllist; do
      if test -f $i/SDL/include/SDL_mixer.h; then
        ac_cv_path_sdl_mixer_incl=$i/SDL/include
        break
      fi
      if test -f $i/include/SDL/SDL_mixer.h; then
        ac_cv_path_sdl_mixer_incl=$i/include/SDL
        break
      fi
    done

    SDL_MIXER_CFLAGS=""
    if test x"${ac_cv_path_sdl_mixer_incl}" = x ; then
      AC_MSG_RESULT(none)
      AC_CHECK_HEADERS(SDL_mixer.h, [ac_cv_path_sdl_mixer_incl=""])
    else
      AC_MSG_RESULT(${ac_cv_path_sdl_mixer_incl})
    fi
  fi

  if test x"${ac_cv_path_sdl_mixer_incl}" != x ; then
    if test x"${ac_cv_path_sdl_mixer_incl}" != x"/usr/include"; then
      ac_cv_path_sdl_mixer_incl="${ac_cv_path_sdl_mixer_incl}"
      SDL_MIXER_CFLAGS="-I${ac_cv_path_sdl_mixer_incl}"
      AC_DEFINE(HAVE_SDL_MIXER_H, [], [We have SDL Mixer support])
    else
      ac_cv_path_sdl_mixer_incl=""
    fi
  fi

  dnl Look for the library
  AC_ARG_WITH(sdl_mixer_lib, [  --with-sdl-mixer-lib    directory where sdl_mixer library is], with_sdl_mixer_lib=${withval})
  AC_MSG_CHECKING([for sdl_mixer library])
  AC_CACHE_VAL(ac_cv_path_sdl_mixer_lib,[
  if test x"${with_sdl_mixer_lib}" != x ; then
    if test -f ${with_sdl_mixer_lib}/libSDL_mixer.a -o -f ${with_sdl_mixer_lib}/libSDL_mixer.so -o -f $i/libSDL_mixer-1.2.a -o -f $i/libSDL_mixer-1.2.so; then
      ac_cv_path_sdl_mixer_lib=-L`(cd ${with_sdl_mixer_lib}; pwd)` -lSDL_mixer
    else
      AC_MSG_ERROR([${with_sdl_mixer_lib} directory doesn't contain libsdl_mixer.a])
    fi
  fi
  ])

      dnl Some systems ubnfortunately use the version on this library,
      dnl and don't have a symbolic link without a version number,
dnl     if test -f $i/libSDL_mixer-1.2.a -o -f $i/libSDL_mixer-1.2.so; then
dnl          ac_cv_path_sdl_mixer_lib=$i
dnl 	 withver="-1.2"

  SDL_MIXER_LIBS=""

  if test x"${ac_cv_path_sdl_mixer_lib}" = x ; then
    AC_CHECK_LIB(SDL_mixer, Mix_Linked_Version, [], AC_MSG_RESULT([no]))
    if test x"${ac_cv_path_sdl_mixer_lib}" = x ; then
      AC_CHECK_LIB(SDL_mixer-1.2, Mix_Linked_Version, [ac_cv_path_sdl_mixer_lib="-lSDL_mixer-1.2"])
    fi
  fi

  if test x"${ac_cv_path_sdl_mixer_lib}" = x ; then
    AC_MSG_CHECKING([for SDL_mixer library])
    liblist="${prefix}/lib64 ${prefix}/lib /usr/lib64 /usr/lib /usr/pkg/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib.. ../.."
    for i in $liblist; do
      if test -f $i/libSDL_mixer.a -o -f $i/libSDl_mixer.so; then
        if test x"$i" != x"/usr/lib"; then
          ac_cv_path_sdl_mixer_lib="-L$i -lSDL_mixer"
dnl          AC_MSG_RESULT(${ac_cv_path_sdl_mixer_lib})
          break
        else
          ac_cv_path_sdl_mixer_lib="-lSDL_mixer"
dnl          AC_MSG_RESULT([yes])
          break
        fi
      fi
      if test -f $i/libSDL_mixer-1.2.a -o -f $i/libSDl_mixer-1.2.so; then
        if test x"$i" != x"/usr/lib"; then
          ac_cv_path_sdl_mixer_lib="-L$i -lSDL_mixer-1.2"
          AC_MSG_RESULT(${ac_cv_path_sdl_mixer_lib})
          break
        else
          ac_cv_path_sdl_mixer_lib="-lSDL_mixer-1.2"
          AC_MSG_RESULT([yes])
          break
        fi
      fi
    done
  fi

  if test x"${ac_cv_path_sdl_mixer_lib}" != x ; then
    SDL_MIXER_LIBS="${ac_cv_path_sdl_mixer_lib}"
    AC_DEFINE(HAVE_SDL_MIXER, [], [We have full SDL Mixer support])
  fi

  AC_SUBST(SDL_MIXER_CFLAGS)
  AC_SUBST(SDL_MIXER_LIBS)
])
