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

AC_DEFUN([GNASH_PATH_DMALLOC],
[
  AC_ARG_ENABLE(dmalloc, [  --enable-dmalloc        Enable support for dmalloc],
  [case "${enableval}" in
    yes) dmalloc=yes ;;
    no)  dmalloc=no ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for enable-dmalloc option]) ;;
  esac], dmalloc=no)

  if test x"$dmalloc" = x"yes"; then
    dnl Look for the header
  AC_ARG_WITH(dmalloc_incl, [  --with-dmalloc-incl     directory where libdmalloc header is], with_dmalloc_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_dmalloc_incl,[
    if test x"${with_dmalloc_incl}" != x ; then
      if test -f ${with_dmalloc_incl}/dmalloc.h ; then
	ac_cv_path_dmalloc_incl=`(cd ${with_dmalloc_incl}; pwd)`
      else
	AC_MSG_ERROR([${with_dmalloc_incl} directory doesn't contain dmalloc.h])
      fi
    fi
    ])

    dnl If the path hasn't been specified, go look for it.
    if test x"${ac_cv_path_dmalloc_incl}" = x; then
      AC_CHECK_HEADERS(dmalloc.h, [ac_cv_path_dmalloc_incl=""],[
      if test x"${ac_cv_path_dmalloc_incl}" = x; then
        AC_MSG_CHECKING([for libdmalloc header])
        incllist="/sw/include /usr/local/include /home/latest/include /opt/include /usr/include .. ../.."

        for i in $incllist; do
	  if test -f $i/dmalloc.h; then
	    if test x"$i" != x"/usr/include"; then
	      ac_cv_path_dmalloc_incl="-I$i"
	      break
            else
	      ac_cv_path_dmalloc_incl=""
	      break
	    fi
	  fi
        done
      fi])
    else
      AC_MSG_RESULT(-I${ac_cv_path_dmalloc_incl})
      if test x"${ac_cv_path_dmalloc_incl}" != x"/usr/include"; then
	ac_cv_path_dmalloc_incl="-I${ac_cv_path_dmalloc_incl}"
       else
	ac_cv_path_dmalloc_incl=""
      fi
    fi

    if test x"${ac_cv_path_dmalloc_incl}" != x ; then
      DMALLOC_CFLAGS="${ac_cv_path_dmalloc_incl}"
      AC_MSG_RESULT(${ac_cv_path_dmalloc_incl})
    else
      DMALLOC_CFLAGS=""
    fi

      dnl Look for the library
      AC_ARG_WITH(dmalloc_lib, [  --with-dmalloc-lib      directory where dmalloc library is], with_dmalloc_lib=${withval})
      AC_CACHE_VAL(ac_cv_path_dmalloc_lib,[
      if test x"${with_dmalloc_lib}" != x ; then
        if test -f ${with_dmalloc_lib}/libdmalloc.a -o -f ${with_dmalloc_lib}/libdmalloc.so; then
	  ac_cv_path_dmalloc_lib=`(cd ${with_dmalloc_incl}; pwd)`
        else
	  AC_MSG_ERROR([${with_dmalloc_lib} directory doesn't contain libdmalloc.])
        fi
      fi
      ])

      dnl If the header doesn't exist, there is no point looking for the library.
      if test x"${ac_cv_path_dmalloc_lib}" = x; then
        AC_CHECK_LIB(dmalloc, mallinfo, [ac_cv_path_dmalloc_lib="-ldmalloc"],[
          AC_MSG_CHECKING([for libdmalloc library])
          libslist="/usr/lib64 /usr/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib .. ../.."
          for i in $libslist; do
	    if test -f $i/libdmalloc.a -o -f $i/libdmalloc.so; then
	      if test x"$i" != x"/usr/lib"; then
	        ac_cv_path_dmalloc_lib="-L$i"
                AC_MSG_RESULT(${ac_cv_path_dmalloc_lib})
	        break
              else
	        ac_cv_path_dmalloc_lib=""
                AC_MSG_RESULT(yes)
	        break
	      fi
	    fi
          done])
      else
        if test -f ${ac_cv_path_dmalloc_lib}/libdmalloc.a -o -f ${ac_cv_path_dmalloc_lib}/libdmalloc.so; then

          if test x"${ac_cv_path_dmalloc_lib}" != x"/usr/lib"; then
	    ac_cv_path_dmalloc_lib="-I${ac_cv_path_dmalloc_lib}"
           else
	    ac_cv_path_dmalloc_lib=""
          fi
        fi
      fi

      if test x"${ac_cv_path_dmalloc_lib}" != x ; then
        DMALLOC_LIBS="${ac_cv_path_dmalloc_lib}"
      else
        DMALLOC_LIBS=""
      fi
    fi

  if test x"${ac_cv_path_dmalloc_lib}" != x ; then
      DMALLOC_LIBS="${ac_cv_path_dmalloc_lib}"
      AC_DEFINE(HAVE_DMALLOC, , [Defined if DMALLOC usage is enabled])
  fi

  AM_CONDITIONAL(DMALLOC, [test x$dmalloc = xyes])

  AC_SUBST(DMALLOC_CFLAGS)
  AC_SUBST(DMALLOC_LIBS)
])
