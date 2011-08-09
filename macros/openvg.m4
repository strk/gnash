dnl  
dnl Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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

AC_DEFUN([GNASH_PATH_OPENVG],
[
  has_openvg=no
  mesavg=no
  dnl Look for the headers.
  AC_ARG_WITH(openvg_includes, AC_HELP_STRING([--with-openvg-includes], [directory where Openvg headers are]), with_openvg_includes=${withval})
  AC_CACHE_VAL(ac_cv_path_openvg_includes,[
    if test x"${with_openvg_includes}" != x; then
      if test -f ${with_openvg_includes}/openvg.h; then
        ac_cv_path_openvg_includes="`(cd ${with_openvg_includes}; pwd)`"
      else
        AC_MSG_ERROR([${with_openvg_includes} directory doesn't contain VG/openvg.h])
      fi
    fi
  ])

  dnl If the include path hasn't been specified, go look for it.
  if test x"${ac_cv_path_openvg_includes}" = x; then
    AC_MSG_CHECKING([for Openvg headers])
    for i in $incllist; do
      if test -f $i/VG/openvg.h; then
      dnl We have the libMesa version of OpenVG 1.0.1
        if test -f $i/VG/vgext.h; then
           AC_DEFINE(HAVE_VG_VGEXT_H, 1, [Have LibMESA OpenVG])
           mesavg=yes
	    else 
	      if test -f $i/VG/ext.h; then
          AC_DEFINE(HAVE_VG_EXT_H, 1, [Have Freescale OpenVG])
	      fi
        fi
        has_openvg=yes
        if test x"$i" != x"/usr/include"; then
          ac_cv_path_openvg_includes="-I$i"
          break
        else
          ac_cv_path_openvg_includes=""
          break
        fi
      fi
    done
  fi

  dnl if test x"${has_openvg}" = xno; then
  dnl   AC_CHECK_HEADER([VG/openvg.h], [ac_cv_path_openvg_includes=""])
  dnl fi

  if test x"${mesavg}" = xyes; then
    OPENVG_CFLAGS="${ac_cv_path_openvg_includes}"
  else
    OPENVG_CFLAGS="-DOPENVG_STATIC_LIBRARY ${ac_cv_path_openvg_includes}"
  fi
  AC_MSG_RESULT(${ac_cv_path_openvg_includes})

  dnl Look for the libraries.
  AC_ARG_WITH(openvg_lib, AC_HELP_STRING([--with-openvg-lib], [directory where Openvg libraries are]), with_openvg_lib=${withval})
  AC_CACHE_VAL(ac_cv_path_openvg_lib,[
    if test x"${with_openvg_lib}" != x ; then
      if test -f ${with_openvg_lib}/libOpenVG.a -o -f ${with_openvg_lib}/libOpenVG.${shlibext}; then
        ac_cv_path_openvg_lib="-L`(cd ${with_openvg_lib}; pwd)` -lOpenVG"
      fi
    fi
  ])

  if test x"${ac_cv_path_openvg_lib}" -o x"${has_openvg}" = xyes; then
    for i in $libslist; do
      if test -f $i/libOpenVG.${shlibext} -o -f $i/libOpenVG.a; then
        if test ! x"$i" = x"/usr/lib" -a ! x"$i" = x"/usr/lib64"; then
          ac_cv_path_openvg_lib="-L$i -lOpenVG"
          break
        else
          ac_cv_path_openvg_lib="-lOpenVG"
          break
        fi
      fi
      dnl Ubuntu Oneric (11.10) has moved the OpenVG libraries to an architecture
      dnl specific directory, I assume to separate the two library versions between
      dnl X11 (Mesa) and a frasmebuffer.
      if test -f $i/mesa-egl/libOpenVG.${shlibext} -o -f $i/mesa-egl/libOpenVG.a; then
        ac_cv_path_openvg_lib="-L$i/mesa-egl -lOpenVG"
        break
      fi
    done
    dnl The Babbage board wants libgsl too. We put this in a separate
    dnl vatiable because all the executables need it when statically
    dnl linking. With Ltib, when cross compiling, these are needed at
    dnl link time.
    if test -f $i/libgsl.${shlibext} -o -f $i/libgsl.a; then
      EXTRA_EGL_LIBS="${EXTRA_EGL_LIBS} -lgsl"
    fi
    dnl The Babbage board wants libssl too because libcurl
    dnl is statically linked.
    if test -f $i/libssl.${shlibext} -o -f $i/libssl.a; then
      EXTRA_EGL_LIBS="${EXTRA_EGL_LIBS} -lssl"
    fi
    if test -f $i/libcrypto.${shlibext} -o -f $i/libcrypto.a; then
      EXTRA_EGL_LIBS="${EXTRA_EGL_LIBS} -lcrypto"
    fi
    if test x"${ac_cv_path_openvg_lib}" = x; then
      AC_CHECK_LIB([OpenVG], [vgClear], [ac_cv_path_openvg_lib="-lOpenVG"],
                                        [ac_cv_path_openvg_lib=""])
    fi
  fi

  if test x"${ac_cv_path_openvg_lib}" != x ; then
    OPENVG_LIBS="${ac_cv_path_openvg_lib}"
    has_openvg="yes"
  else
    OPENVG_LIBS=""
  fi
  
  AC_SUBST(EXTRA_EGL_LIBS)
  AC_SUBST(OPENVG_CFLAGS)
  AC_SUBST(OPENVG_LIBS)
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
