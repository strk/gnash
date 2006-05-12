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

AC_DEFUN([GNASH_PATH_GLEXT],
[
  AC_ARG_ENABLE(glext, [  --disable-glext           Disable support for GTK OpenGL extension],
  [case "${enableval}" in
    yes) glext=yes ;;
    no)  glext=no ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for disable-glext option]) ;;
  esac], glext=yes)

  if test x"$plugin" = x"no"; then
    glext=no
  fi

  if test x"$glext" = x"yes"; then
    dnl Look for the header
    AC_ARG_WITH(glext_incl, [  --with-glext-incl        directory where libglext header is], with_glext_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_glext_incl,[
    if test x"${with_glext_incl}" != x ; then
      if test -f ${with_glext_incl}/gtk/gtkgl.h ; then
	ac_cv_path_glext_incl=`(cd ${with_glext_incl}; pwd)`
      else
	AC_MSG_ERROR([${with_glext_incl} directory doesn't contain gtk/gtkgl.h])
      fi
     ])

dnl Attempt to find the top level directory, which unfortunately has a
dnl version number attached. At least on Debain based systems, this
dnl doesn't seem to get a directory that is unversioned.
    if test x"${ac_cv_path_glext_incl}" = x ; then
      AC_MSG_CHECKING([for the Gtk GL Extensions Version])
      pathlist="${prefix}/include /sw/include /usr/local/include /usr/X11R6/include /home/latest/include /opt/include /usr/include /usr/pkg/include .. ../.."

      topdir=""
      version=""
      for i in $pathlist; do
	for j in `ls -dr $i/gtkglext-[[0-9]].[[0-9]] 2>/dev/null`; do
 	  if test -f $j/gtk/gtkgl.h; then
	    topdir=`basename $j`
	    version=`echo ${topdir} | sed -e 's:gtkglext-::'`
 	    break
 	  fi
	done
      done
    fi

     if test x"${topdir}" = x; then
       AC_MSG_RESULT(none)
     else
       AC_MSG_RESULT([${version}])
     fi

    dnl If the path hasn't been specified, go look for it.
    if test x"${ac_cv_path_glext_incl}" = x; then
      AC_CHECK_HEADERS(gtk/gtkgl.h, [ac_cv_path_glext_incl=""],[
      if test x"${ac_cv_path_glext_incl}" = x; then
        AC_MSG_CHECKING([for libglext header])
        incllist="${prefix}/include /sw/include /usr/local/include /usr/X11R6/include /home/latest/include /opt/include /usr/include /usr/pkg/include .. ../.."

	ac_cv_path_glext_incl=""
        for i in $incllist; do
	  if test -f $i/gtk/gtkgl.h; then
	    if test x"$i" != x"/usr/include"; then
	      ac_cv_path_glext_incl="$i"
	      break
            fi
	  else
	    if test -f $i/${topdir}/gtk/gtkgl.h; then
	      ac_cv_path_glext_incl="$i/${topdir}"
	      break
	    fi
	  fi
        done
      fi])
    fi

      dnl Look for the library
      AC_ARG_WITH(glext_lib, [  --with-glext-lib         directory where glext library is], with_glext_lib=${withval})
      AC_CACHE_VAL(ac_cv_path_glext_lib,[
      if test x"${with_glext_lib}" != x ; then
        if test -f ${with_glext_lib}/libgtkglext-x11-${version}.a -o -f ${with_glext_lib}/libgtkglext-x11-${version}.so; then
	  ac_cv_path_glext_lib=`(cd ${with_glext_lib}; pwd)`
        else
	  AC_MSG_ERROR([${with_glext_lib} directory doesn't contain libgtkglext.])
        fi
      fi
      ])

dnl If the header doesn't exist, there is no point looking for
dnl the library. 
      if test x"${ac_cv_path_glext_incl}" != x; then
        AC_CHECK_LIB(gtkglext-x11-${version}, gtk_gl_init, [ac_cv_path_glext_lib="-lgtkglext-x11-${version} -lgdkglext-x11-${version}"],[
          AC_MSG_CHECKING([for libglext library])
          libslist="${prefix}/lib64 ${prefix}/lib /usr/X11R6/lib64 /usr/X11R6/lib /usr/lib64 /usr/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib /usr/pkg/lib .. ../.."
          for i in $libslist; do
	    if test -f $i/gtkglext-${version}/include/gdkglext-config.h; then
	      ac_cv_path_glext_incl="${ac_cv_path_glext_incl} -I${i}/gtkglext-${version}/include"
	    fi
	    if test -f $i/libgtkglext-x11-${version}.a -o -f $i/libgtkglext-x11-${version}.so; then
	      if test x"$i" != x"/usr/lib"; then
	        ac_cv_path_glext_lib="-L$i -lgtkglext-x11-${version} -lgdkglext-x11-${version}"
	        break
              fi
	    else
	      if test -f $i/libgtkglext-x11-${version}.a -o -f $i/libgtkglext-x11-${version}.so; then
		ac_cv_path_glext_lib="-L$i/${topdir} -lgtkglext-x11-${version} -lgdkglext-x11-${version}"
		break
              fi
	    fi
          done])
      else
	if test -f $i/libgtkglext-x11-${version}.a -o -f $i/libgtkglext-x11-${version}.so; then
          if test x"${ac_cv_path_glext_lib}" != x"/usr/lib"; then
	    ac_cv_path_glext_lib="-L${ac_cv_path_glext_lib} -lgtkglext-x11-${version} -lgdkglext-x11-${version}"
           else
	    ac_cv_path_glext_lib=""
          fi
        fi
      fi

    fi
  fi

  if test x"${ac_cv_path_glext_incl}" != x ; then
    libincl=`echo ${ac_cv_path_glext_incl} | sed -e 's/include/lib/'`
    GLEXT_CFLAGS="-I${ac_cv_path_glext_incl} -I${libincl}/include"
    AC_DEFINE(HAVE_GTK_GTKGL_H, [1], [GTKGLExt header])
  else
    GLEXT_CFLAGS=""
  fi

  if test x"${ac_cv_path_glext_lib}" != x ; then
    AC_DEFINE(USE_GTKGLEXT,[1], [Use GtkGLExt extension])
    GLEXT_LIBS="${ac_cv_path_glext_lib}"
  else
    GLEXT_LIBS=""
dnl we can't build the plguin without GtkGlExt
    glext=no
  fi

  AM_CONDITIONAL(HAVE_GLEXT, [test x$glext = xyes])

  AC_SUBST(GLEXT_CFLAGS)
  AC_SUBST(GLEXT_LIBS)
])
