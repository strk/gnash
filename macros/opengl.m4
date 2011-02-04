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

AC_DEFUN([GNASH_PATH_OPENGL],
[
  opengl=yes
  if test x"$opengl" = x"yes"; then
    dnl Look for the headers.
    AC_ARG_WITH(opengl_includes, AC_HELP_STRING([--with-opengl-includes], [directory where OpenGL headers are]), with_opengl_includes=${withval})
    AC_CACHE_VAL(ac_cv_path_opengl_includes,[
      if test x"${with_opengl_includes}" != x ; then
        if test -f ${with_opengl_includes}/GL/gl.h -a -f ${with_opengl_includes}/GL/glu.h ; then
          ac_cv_path_opengl_includes="`(cd ${with_opengl_includes}; pwd)`"
        else
          AC_MSG_ERROR([${with_opengl_includes} directory doesn't contain GL/gl.h])
        fi
      fi
    ])

    dnl If the include path hasn't been specified, go look for it.
    if test x"${darwin}" = xyes; then
      ac_cv_path_opengl_includes="-framework Carbon -framework ApplicationServices -framework OpenGL -framework AGL -I/System/Library/Frameworks/OpenGL.framework/Headers"
    else
      notsgigl=no
      if test x"${ac_cv_path_opengl_includes}" = x; then
        AC_MSG_CHECKING([for OpenGL headers])
        newlist="/System/Library/Frameworks/OpenGL.framework/Headers ${incllist}"
        for i in $newlist; do
          if test -f $i/GL/gl.h -o -f $i/OpenGL.h; then
            if test -f $i/OpenGL.h; then
              notsgigl=yes
              AC_DEFINE(NOT_SGI_GL, [1], [Is not based on the SGI GL])
            fi
            if test x"$i" != x"/usr/include"; then
              ac_cv_path_opengl_includes="-I$i"
              break
            else
              ac_cv_path_opengl_includes="default"
              break
            fi
	          if test -f $i/GL/glu.h; then
	            AC_MSG_WARN([GL/glu.h not installed!])
	          fi
          fi
        done
      fi
    fi

    if test x"${ac_cv_path_opengl_includes}" = x; then
      AC_CHECK_HEADERS([GL/gl.h], [ac_cv_path_opengl_includes=""])
    fi

    if test x"${ac_cv_path_opengl_includes}" != x -a x"${ac_cv_path_opengl_includes}" != x"default"; then
      OPENGL_CFLAGS="${ac_cv_path_opengl_includes}"
    else
      OPENGL_CFLAGS=""
    fi
    AC_MSG_RESULT(${ac_cv_path_opengl_includes})

    dnl Look for the libraries.
    AC_ARG_WITH(opengl_lib, AC_HELP_STRING([--with-opengl-lib], [directory where OpenGL libraries are]), with_opengl_lib=${withval})
    AC_CACHE_VAL(ac_cv_path_opengl_lib,[
      if test x"${with_opengl_lib}" != x ; then
        if test -f ${with_opengl_lib}/libGL.a -o -f ${with_opengl_lib}/libGL.${shlibext}; then
          ac_cv_path_opengl_lib="-L`(cd ${with_opengl_lib}; pwd)` -lGL -lGLU"
        else
          if test -f ${with_opengl_lib}/libopengl32.a; then
            ac_cv_path_opengl_lib="-L`(cd ${with_opengl_lib}; pwd)` -lopengl32 -lglu32"
          else
            AC_MSG_ERROR([${with_opengl_lib} directory doesn't contain libGL.])
          fi
        fi
      fi
    ])

    if test x"${darwin}" = xyes; then
      ac_cv_path_opengl_lib="-framework Carbon -framework ApplicationServices -framework OpenGL -framework AGL"
    else
      if test x"${ac_cv_path_opengl_lib}" = x; then
        newlist="/System/Library/Frameworks/OpenGL.framework/Versions/Current/Libraries /System/Library/Frameworks/OpenGL.framework/Libraries ${libslist}"
        for i in $newlist; do
          if test -f $i/libGL.${shlibext} -o -f $i/libGL.a; then
            if test ! x"$i" = x"/usr/lib" -a ! x"$i" = x"/usr/lib64"; then
              ac_cv_path_opengl_lib="-L$i -lGL"
              break
	          else
              ac_cv_path_opengl_lib="-lGL"
              break
	          fi
          fi
        done
        if test x"${ac_cv_path_opengl_lib}" != x; then
          if test -f $i/libGLU.${shlibext} -o -f $i/libGLU.a; then
            ac_cv_path_opengl_lib="${ac_cv_path_opengl_lib} -lGLU"
          else
            AC_WARN([No GLU library found!])
          fi
        else                      dnl nothing found, check for the win32 names
          for i in $newliblist; do
            if test -f $i/libopengl32.${shlibext} -o -f $i/libopengl32.a; then
              ac_cv_path_opengl_lib="-L$i -lopengl32 =lopenglu32"
              break
            fi
          done
        fi
      fi                        dnl end of if ac_cv_path_opengl_lib
    fi                          dnl end of if darwin
  fi                            dnl end of if $opengl

  if test x"${ac_cv_path_opengl_lib}" = x; then
    AC_CHECK_LIB([GL], [glBegin], [ac_cv_path_opengl_lib="-lGL -lGLU"])
  fi

  if test x"${ac_cv_path_opengl_lib}" != x ; then
      OPENGL_LIBS="${ac_cv_path_opengl_lib}"
      has_opengl="yes"
  else
      OPENGL_LIBS=""
  fi
  
  if test x"${darwin}" = xyes; then
	  OPENGL_LIBS="${OPENGL_LIBS} -dylib_file /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib"
  fi

  AC_SUBST(OPENGL_CFLAGS)
  AC_SUBST(OPENGL_LIBS)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
