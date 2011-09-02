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

AC_DEFUN([GNASH_PATH_GLES],
[
  gles=yes

  dnl This changes the default flavor of OpenGL-ES implementations. If only
  dnl one is installed, it doesn't matter, but if you are a developer with
  dnl multiple OpenGL-ES implementations, you can use this to limit the
  dnl search. 10c, mali, android are all that's supported.
  default_flavor=any
  dnl The config flavor is used for what configure thinks the default
  dnl is when there is no default flavor of OpenGL-ES library.
  config_flavor=${default_flavor}

  dnl add the default path to the Android NDK
  if test x"${android_ndk}" != xno; then
    newlist="${android_ndk}/usr/include ${incllist}"
  else
    newlist="${incllist}"
  fi

  dnl Look for the headers.
  AC_ARG_WITH(gles1_includes, AC_HELP_STRING([--with-gles1-includes], [directory where OpenGLES 1 headers are]), with_gles1_includes=${withval})
  AC_CACHE_VAL(ac_cv_path_gles1_includes,[
    if test x"${with_gles1_includes}" != x ; then
      if test -f ${with_gles1_includes}/GLES/egl.h -o -f ${with_gles1_includes}/GLES/gl.h; then
        ac_cv_path_gles1_includes="-I`(cd ${with_gles1_includes}; pwd)`"
      else
        AC_MSG_ERROR([${with_gles1_includes} directory doesn't contain egl.h or gl.h])
      fi
    fi
  ])

  AC_ARG_WITH(gles2_includes, AC_HELP_STRING([--with-gles2-includes], [directory where OpenGLES 2 headers are]), with_gles2_includes=${withval})
  AC_CACHE_VAL(ac_cv_path_gles2_includes,[
    if test x"${with_gles2_includes}" != x ; then
      if test -f ${with_gles2_includes}/GLES2/gl2.h ; then
        ac_cv_path_gles2_includes="-I`(cd ${with_gles2_includes}; pwd)`"
      else
        AC_MSG_ERROR([${with_gles2_includes} directory doesn't contain gl2.h])
      fi
    fi
  ])

  dnl This gets really fun, as all the implementations I could find
  dnl have differences. OpenGL-ES 1.x by default doesn't have
  dnl Farmebuffer support, this is added by an extension. For
  dnl OpenGL-ES 2.x, this extension is now in the standard API.
  dnl 
  dnl The three tested implementations so far are the linux gles-10c
  dnl project, which appears unmaintained these days, but lacks the
  dnl framebuffer extension. It does however work on a 64bit system,
  dnl unlike the other libraries mentioned here.
  dnl
  dnl The other primary one is the Andproid implementation, which has
  dnl OpenGL-ES 1.1 support (with the framebuffer extension) and 2.1
  dnl support.
  dnl
  dnl The final one are the Mali Developer Tools from ARM, that
  dnl contain emulator that lets you run OpenGL-ES applications on a
  dnl standard desktop under X11. The 2.0 version of the tools
  dnl includes both OpenGL-ES 1.1 and 2.1, but with a twist. The
  dnl OpenGL-ES 1.x implementation is under the EGL, instead of the
  dnl GLES top level directory.

  if test x"${ac_cv_path_gles1_includes}" = x -o x"${ac_cv_path_gles2_includes}" = x; then
    AC_MSG_CHECKING([for OpenGL-ES headers])
    for i in ${newlist}; do
      dnl OpenGL-ES 1.x
      if test x"${ac_cv_path_gles1_includes}" = x -a x"${build_gles1}" = xyes; then
        if test -f $i/GLES/gl.h; then
          AC_DEFINE(HAVE_GLES1_GL_H, [1], [Have OpenGL-ES GLES/gl.h])
          AC_DEFINE([RENDERER_GLES1], [1], [Use OpenGL-ES version 1.x])
          ac_cv_path_gles1_includes="-I$i"
          dnl Some implementations have this extension, which adds
          dnl framebuffer support.
          if test -f $i/GLES/glext.h; then
            AC_DEFINE(HAVE_GLES1_GLEXT_H, [1], [Have OpenGL-ES GLES/glext.h])
          fi
          dnl If it contains the egl.h file, it's probably gles-10c,
          dnl if not, then it's probably the Android implementation.
          if test -f $i/GLES/egl.h; then
            AC_DEFINE(HAVE_GLES1_EGL_H, [1], [Have OpenGLES 1 egl.h])
            config_flavor=10c
          else
            config_flavor=android
          fi
          if test -f $i/EGL/egl.h; then
            AC_DEFINE(HAVE_EGL_EGL_H, [1], [Have OpenGL-ES EGL/egl.h])
            ac_cv_path_gles1_includes="-I$i"
            dnl Only the Mali library has this header file.
            if test -f $i/EGL/egl_native_types.h; then
              config_flavor=mali
            else
              config_flavor=android
            fi
          fi
        fi
      fi
      dnl OpenGL-ES 2.x
      if test x"${ac_cv_path_gles2_includes}" = x -a x"${build_gles2}" = xyes; then
        if test -f $i/GLES2/gl2.h; then
          AC_DEFINE(HAVE_GLES2_GL2_H, [1], [Have OpenGLES v2 GLES2/gl2.h])
          AC_DEFINE([RENDERER_GLES2], [1], [Use OpenGLES v2 version 2])
          ac_cv_path_gles2_includes="-I$i"
          dnl Both OpenGL-ES 2.1 implementations are similar enough
          dnl the flavor doesn't matter.
          config_flavor=android
        fi
        if test -f $i/EGL/egl.h; then
          AC_DEFINE(HAVE_EGL_EGL_H, [1], [Have OpenGL-ES EGL/egl.h])
          ac_cv_path_gles2_includes="-I$i"
          dnl Only the Mali library has this header file.
          if test -f $i/EGL/egl_native_types.h; then
            config_flavor=mali
          else
            config_flavor=android
          fi
        fi
      fi
    done
  fi

  if test x"${ac_cv_path_gles1_includes}" = x; then
    AC_CHECK_HEADERS([GLES/egl.h], [ac_cv_path_gles1_includes=""])
  fi
  if test x"${ac_cv_path_gles2_includes}" = x; then
    AC_CHECK_HEADERS([GLES2/gl2.h], [ac_cv_path_gles2_includes=""])
  fi

  if test x"${ac_cv_path_gles1_includes}" != x -a x"${ac_cv_path_gles1_includes}" != x/usr/include; then
    GLES1_CFLAGS="${ac_cv_path_gles1_includes}"
  else
    GLES1_CFLAGS=""
  fi

  if test x"${ac_cv_path_gles2_includes}" != x -a x"${ac_cv_path_gles2_includes}" != x/usr/include; then
    GLES2_CFLAGS="${ac_cv_path_gles2_includes}"
  else
    GLES2_CFLAGS=""
  fi

  AC_MSG_RESULT(${ac_cv_path_gles1_includes} ${ac_cv_path_gles2_includes})

  if test x"${default_flavor}" = xany; then
    flavor="${config_flavor}"
  else
    flavor="${default_flavor}"
  fi

  dnl the library has different names depending on the release and platform
  dnl Android uses GLESv1_CM or GLESv2
  dnl The older gles-10c uses libGLES_CL, but works on an x86_64, which the
  dnl Mali tools from ARM don't.
  dnl The Mali Developer Tools 1.1 from the ARM emulator use libegl & libglesv1
  dnl The Mali Developer Tools 2.0 from the ARM emulator use libEGL &
  dnl libGLESv2. We limit the search of librairies so if we have
  dnl multiple OpenGL-ES implementations installed (for testing and
  dnl development), so we don't mix and match headers and libaries
  dnl between implementations.
  if test x"${ac_cv_path_gles1_includes}" != x; then
    AC_MSG_NOTICE([OpenGL-ES implementation being used is: ${flavor} 1.x])
    case "${flavor}" in
      mali)    gleslist1="egl glesv1"  ;;
      10c)     gleslist1="GLES_CL" ;;
      android) gleslist1="EGL GLESv1_CM" ;;
      mesa)    gleslist1="GLESv1" ;;
      *)       gleslist1="GLES_CL GLESv1_CM GLESv1" ;;
    esac
  fi

  if test x"${ac_cv_path_gles2_includes}" != x; then
    AC_MSG_NOTICE([OpenGL-ES implementation being used is: ${flavor} 2.x])
    gleslist2="EGL GLESv2"
  else
    gleslist2=""
  fi

  dnl Look for the libraries.
  AC_ARG_WITH(gles1_lib, AC_HELP_STRING([--with-gles1-lib], [directory where OpenGLES 1 libraries are]), with_gles1_lib=${withval})
  AC_CACHE_VAL(ac_cv_path_gles1_lib,[
    if test x"${with_gles1_lib}" != x ; then
      for j in $gleslist1; do
        if test -f ${with_gles1_lib}/$i.a -o -f ${with_gles1_lib}/$i.${shlibext}; then
          ac_cv_path_gles1_lib="-L`(cd ${with_gles1_lib}; pwd)` -l$i"
          break;
        fi
      done
    fi
  ])
  AC_ARG_WITH(gles2_lib, AC_HELP_STRING([--with-gles2-lib], [directory where OpenGLES 2 libraries are]), with_gles2_lib=${withval})
  AC_CACHE_VAL(ac_cv_path_gles2_lib,[
    if test x"${with_gles2_lib}" != x ; then
      for j in $gleslist2; do
        if test -f ${with_gles2_lib}/$i.a -o -f ${with_gles2_lib}/$i.${shlibext}; then
          ac_cv_path_gles2_lib="-L`(cd ${with_gles2_lib}; pwd)` -l$i"
          break;
        fi
      done
    fi
  ])

  has_gles1=no
  has_gles2=no

  dnl add the default path to the Android NDK
  newlist="${android_ndk}/usr/lib ${libslist}"
  if test x"${ac_cv_path_gles1_lib}" = x; then
    for dir in ${newlist}; do
      for lib in ${gleslist1}; do
        if test -f ${dir}/lib${lib}.${shlibext} -o -f ${dir}/lib${lib}.a; then
          has_gles1="yes"
          if test ! x"${dir}" = x"/usr/lib" -a ! x"${dir}" = x"/usr/lib64"; then
            ac_cv_path_gles1_lib="-L${dir} -l${lib}"
          else
            ac_cv_path_gles1_lib="-l${lib}"
          fi
        fi
      done
    done
  fi
  if test x"${ac_cv_path_gles2_lib}" = x; then
    for dir in ${newlist}; do
      for lib in ${gleslist2}; do
        if test -f ${dir}/lib${lib}.${shlibext} -o -f ${dir}/lib${lib}.a; then
          has_gles2="yes"
          if test ! x"${dir}" = x"/usr/lib" -a ! x"${dir}" = x"/usr/lib64"; then
            ac_cv_path_gles2_lib="-L${dir} -l${lib}"
          else
            ac_cv_path_gles2_lib="-l${lib}"
          fi
        fi
      done
    done
  fi

  if test x"${ac_cv_path_gles1_lib}" != x; then
      GLES1_LIBS="${ac_cv_path_gles1_lib}"
  else
      GLES1_LIBS=""
  fi

  if test x"${ac_cv_path_gles2_lib}" != x; then
      GLES2_LIBS="${ac_cv_path_gles2_lib}"
  else
      GLES2_LIBS=""
  fi

  AC_SUBST(GLES1_CFLAGS)
  AC_SUBST(GLES1_LIBS)

  AC_SUBST(GLES2_CFLAGS)
  AC_SUBST(GLES2_LIBS)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
