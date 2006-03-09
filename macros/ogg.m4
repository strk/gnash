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

AC_DEFUN([GNASH_PATH_OGG],
[
  AC_ARG_ENABLE(ogg, [  --enable-ogg            Enable support for playing oggs],
  [case "${enableval}" in
    yes) ogg=yes ;;
    no)  ogg=no ;;
    *)   AC_MSG_ERROR([bad value ${enableval} for enable-ogg option]) ;;
  esac], ogg=yes)

  if test x"$ogg" = x"yes"; then
    dnl Look for the header
  AC_ARG_WITH(ogg_incl, [  --with-ogg_incl         directory where libogg header is], with_ogg_incl=${withval})
    AC_CACHE_VAL(ac_cv_path_ogg_incl,[
    if test x"${with_ogg_incl}" != x ; then
      if test -f ${with_ogg_incl}/ogg.h ; then
	ac_cv_path_ogg_incl=`(cd ${with_ogg_incl}; pwd)`
      else
	AC_MSG_ERROR([${with_ogg_incl} directory doesn't contain ogg.h])
      fi
    fi
    ])

    dnl If the path hasn't been specified, go look for it.
    if test x"${ac_cv_path_ogg_incl}" = x; then
      AC_CHECK_HEADERS(ogg.h, [ac_cv_path_ogg_incl=""],[
      if test x"${ac_cv_path_ogg_incl}" = x; then
        AC_MSG_CHECKING([for libogg header])
        incllist="${prefix}/include /sw/include /usr/local/include /home/latest/include /opt/include /usr/include /usr/pkg/include .. ../.."

        for i in $incllist; do
	  if test -f $i/ogg/ogg.h; then
	    if test x"$i" != x"/usr/include"; then
	      ac_cv_path_ogg_incl="-I$i"
	      break
            else
	      ac_cv_path_ogg_incl=""
	      break
	    fi
	  fi
        done
      fi])
    else
      AC_MSG_RESULT(-I${ac_cv_path_ogg_incl})
      if test x"${ac_cv_path_ogg_incl}" != x"/usr/include"; then
	ac_cv_path_ogg_incl="-I${ac_cv_path_ogg_incl}"
       else
	ac_cv_path_ogg_incl=""
      fi
    fi

    if test x"${ac_cv_path_ogg_incl}" != x ; then
      OGG_CFLAGS="${ac_cv_path_ogg_incl}"
      AC_MSG_RESULT(${ac_cv_path_ogg_incl})
    else
      OGG_CFLAGS=""
    fi

      dnl Look for the library
      AC_ARG_WITH(ogg_lib, [  --with-ogg-lib          directory where ogg library is], with_ogg_lib=${withval})
      AC_CACHE_VAL(ac_cv_path_ogg_lib,[
      if test x"${with_ogg_lib}" != x ; then
        if test -f ${with_ogg_lib}/libogg.a -o -f ${with_ogg_lib}/libogg.so; then
	  ac_cv_path_ogg_lib=`(cd ${with_ogg_incl}; pwd)`
        else
	  AC_MSG_ERROR([${with_ogg_lib} directory doesn't contain libogg.])
        fi
      fi
      ])

      dnl If the header doesn't exist, there is no point looking for the library.
      if test x"${ac_cv_path_ogg_lib}" = x; then
        AC_CHECK_LIB(ogg, ogg_sync_init, [ac_cv_path_ogg_lib="-logg"],[
          AC_MSG_CHECKING([for libogg library])
          libslist="${prefix}/lib64 ${prefix}/lib /usr/lib64 /usr/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib /usr/pkg/lib .. ../.."
          for i in $libslist; do
	    if test -f $i/libogg.a -o -f $i/libogg.so; then
	      if test x"$i" != x"/usr/lib"; then
	        ac_cv_path_ogg_lib="-L$i"
                AC_MSG_RESULT(${ac_cv_path_ogg_lib})
	        break
              else
	        ac_cv_path_ogg_lib=""
                AC_MSG_RESULT(yes)
	        break
	      fi
	    fi
          done])
      else
        if test -f ${ac_cv_path_ogg_lib}/libogg.a -o -f ${ac_cv_path_ogg_lib}/libogg.so; then

          if test x"${ac_cv_path_ogg_lib}" != x"/usr/lib"; then
	    ac_cv_path_ogg_lib="-L${ac_cv_path_ogg_lib}"
           else
	    ac_cv_path_ogg_lib=""
          fi
        fi
      fi

      if test x"${ac_cv_path_ogg_lib}" != x ; then
        OGG_LIBS="${ac_cv_path_ogg_lib}"
      else
        OGG_LIBS=""
      fi
    fi

  if test x"${ac_cv_path_ogg_lib}" != x ; then
      OGG_LIBS="${ac_cv_path_ogg_lib}"
  fi

  AM_CONDITIONAL(OGG, [test x$ogg = xyes])

  AC_SUBST(OGG_CFLAGS)
  AC_SUBST(OGG_LIBS)
])
