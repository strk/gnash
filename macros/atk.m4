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
dnl  
dnl  Linking Gnash statically or dynamically with other modules is making
dnl  a combined work based on Gnash. Thus, the terms and conditions of
dnl  the GNU General Public License cover the whole combination.
dnl  
dnl  In addition, as a special exception, the copyright holders of Gnash give
dnl  you permission to combine Gnash with free software programs or
dnl  libraries that are released under the GNU LGPL and/or with Mozilla, 
dnl  so long as the linking with Mozilla, or any variant of Mozilla, is
dnl  through its standard plug-in interface. You may copy and distribute
dnl  such a system following the terms of the GNU GPL for Gnash and the
dnl  licenses of the other code concerned, provided that you include the
dnl  source code of that other code when and as the GNU GPL requires
dnl  distribution of source code. 
dnl  
dnl  Note that people who make modified versions of Gnash are not obligated
dnl  to grant this special exception for their modified versions; it is
dnl  their choice whether to do so.  The GNU General Public License gives
dnl  permission to release a modified version without this exception; this
dnl  exception also makes it possible to release a modified version which
dnl  carries forward this exception.
dnl 

AC_DEFUN([GNASH_PATH_ATK],
[
  dnl Look for the header
  AC_ARG_WITH(atk_incl, [  --with-atk-incl        directory where libatk header is], with_atk_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_atk_incl,[
    if test x"${with_atk_incl}" != x ; then
      if test -f ${with_atk_incl}/atk/atkgl.h ; then
	ac_cv_path_atk_incl=`(cd ${with_atk_incl}; pwd)`
      else
	AC_MSG_ERROR([${with_atk_incl} directory doesn't contain atk/atkgl.h])
      fi
    fi])

  if test x"$glext" = x"yes"; then
    dnl Attempt to find the top level directory, which unfortunately has a
    dnl version number attached. At least on Debain based systems, this
    dnl doesn't seem to get a directory that is unversioned.
    if test x"${ac_cv_path_atk_incl}" = x; then
      AC_MSG_CHECKING([for the Atk Version])
      pathlist="${prefix}/include /sw/include /usr/local/include /home/latest/include /opt/include /usr/include /usr/pkg/include .. ../.."

      topdir=""
      version=""
      for i in $pathlist; do
	for j in `ls -dr $i/atk-[[0-9]].[[0-9]] 2>/dev/null`; do
 	  if test -f $j/atk/atk.h; then
	    topdir=`basename $j`
	    version=`echo ${topdir} | sed -e 's:atk-::'`
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
    if test x"${ac_cv_path_atk_incl}" = x; then
      AC_CHECK_HEADERS(atk/atkgl.h, [ac_cv_path_atk_incl=""],[
      if test x"${ac_cv_path_atk_incl}" = x; then
        AC_MSG_CHECKING([for libatk header])
        incllist="/sw/include /usr/local/include /home/latest/include /usr/X11R6/include /opt/include /usr/include /usr/pkg/include .. ../.."

        for i in $incllist; do
	  if test -f $i/atk/atk.h; then
	    ac_cv_path_atk_incl="$i"
	    break
	  else
	    if test -f $i/${topdir}/atk/atk.h; then
	      ac_cv_path_atk_incl="$i/${topdir}"
	      break
	    fi
	  fi
        done
      fi])
    fi

    if test x"${ac_cv_path_atk_incl}" != x ; then
      AC_MSG_RESULT(yes)
    else
      AC_MSG_RESULT(no)
    fi

      dnl Look for the library
    AC_ARG_WITH(atk_lib, [  --with-atk-lib         directory where atk library is], with_atk_lib=${withval})
      AC_CACHE_VAL(ac_cv_path_atk_lib,[
      if test x"${with_atk_lib}" != x ; then
        if test -f ${with_atk_lib}/libatkatk-x11-${version}.a -o -f ${with_atk_lib}/libatkatk-x11-${version}.so; then
	  ac_cv_path_atk_lib=`(cd ${with_atk_incl}; pwd)`
        else
	  AC_MSG_ERROR([${with_atk_lib} directory doesn't contain libatkatk.])
        fi
      fi
      ])

dnl If the header doesn't exist, there is no point looking for
dnl the library. 
      if test x"${ac_cv_path_atk_incl}" != x; then
        AC_CHECK_LIB(atk-${version}, atk_focus_tracker_init, [ac_cv_path_atk_lib="-latk-${version}"],[
          AC_MSG_CHECKING([for libatk library])
          libslist="${prefix}/lib64 ${prefix}/lib /usr/lib64 /usr/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib /usr/pkg/lib .. ../.."
          for i in $libslist; do
	    if test -f $i/libatk-${version}.a -o -f $i/libatk-${version}.so; then
	      if test x"$i" != x"/usr/lib"; then
	        ac_cv_path_atk_lib="-L$i -latk-${version}"
	        break
              else
	        ac_cv_path_atk_lib=""
	        break
	      fi
	    else
	      if test -f $i/libatk-${version}.a -o -f $i/libatk-${version}.so; then
		ac_cv_path_atk_lib="$i/${topdir}"
		break
	      fi
	    fi
          done])
      else
	if test -f $i/libatk-${version}.a -o -f $i/libatk-${version}.so; then
          if test x"${ac_cv_path_atk_lib}" != x"/usr/lib"; then
	    ac_cv_path_atk_lib="-L${ac_cv_path_atk_lib}"
           else
	    ac_cv_path_atk_lib=""
          fi
        fi
      fi
    fi

  if test x"${ac_cv_path_atk_incl}" != x ; then
    ATK_CFLAGS="-I${ac_cv_path_atk_incl}"
  else
    ATK_CFLAGS=""
  fi

  if test x"${ac_cv_path_atk_lib}" != x ; then
    ATK_LIBS="${ac_cv_path_atk_lib}"
  else
    ATK_LIBS=""
  fi

  AC_SUBST(ATK_CFLAGS)
  AC_SUBST(ATK_LIBS)
])
