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

dnl Check for OpenGL & [Glut | GLU]
AC_DEFUN([AM_PATH_OPENGL], [

  AC_ARG_ENABLE(opengl, AC_HELP_STRING([--disable-opengl], [do not build OpenGL]),
    [enableopengl=$enableval],
    [enableopengl="yes"]
  )

  if test x$enableopengl = "xyes"; then
    AC_CHECK_LIB(GL, glBegin,
     [AC_CHECK_HEADER(GL/gl.h,
       [ac_have_opengl="yes"
        OPENGL_LIBS="-lGL"
        dnl check for glut
        AC_CHECK_LIB(glut, glutInit,
         [ac_have_glut="yes"
          GLUT_LIBS="-lglut"
          AC_DEFINE(HAVE_GLUT,1,[Define this if you have GLut support available])
          AC_DEFINE(HAVE_OPENGL,1,[Define this if you have OpenGL support available])
         ],
         [ac_have_glut="no"
          dnl fallback, check for GLU
          AC_CHECK_LIB(GLU, gluPerspective,
           [ac_have_glu="yes"
            GLU_LIBS="-lGLU -lm" 
            AC_DEFINE(HAVE_GLU,1,[Define this if you have GLU support available])
            AC_DEFINE(HAVE_OPENGL,1,[Define this if you have OpenGL support available])
           ],
           [ac_have_glu="no"], 
           [$X_LIBS $X_PRE_LIBS $OPENGL_LIBS -lGLU -lm $X_EXTRA_LIBS]
          )
         ], 
         [$X_LIBS $X_PRE_LIBS -lglut $X_EXTRA_LIBS]
        )
       ]
     )],
     [],
     [$X_LIBS $X_PRE_LIBS -lGL $X_EXTRA_LIBS]
    )
    if test x$ac_have_opengl = "xyes" -a x$ac_have_glut = "xyes" -o x$ac_have_opengl = "xyes" -a x$ac_have_glu = "xyes"; then
      ac_use_opengl=yes
    fi
  fi

  AC_SUBST(OPENGL_CFLAGS)
  AC_SUBST(OPENGL_LIBS)
  AC_SUBST(GLUT_LIBS)
  AC_SUBST(GLU_LIBS)
  AM_CONDITIONAL(HAVE_OPENGL, [test x$ac_use_opengl = "xyes"])

  dnl result
  if test x$ac_use_opengl = "xyes"; then
    ifelse([$1], , :, [$1])
  else
    ifelse([$2], , :, [$2])
  fi

])
