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

dnl Ffmpeg modules are:
dnl date-time, filesystem. graph. iostreams, program options, python,
dnl regex, serialization, signals, unit test, thead, and wave.

dnl $Id: ffmpeg.m4,v 1.13 2006/10/09 16:11:27 nihilus Exp $

AC_DEFUN([GNASH_PATH_FFMPEG],
[
  dnl Lool for the header
  AC_ARG_WITH(ffmpeg_incl, [  --with-ffmpeg-incl        directory where ffmpeg headers are], with_ffmpeg_incl=${withval})
  AC_CACHE_VAL(ac_cv_path_ffmpeg_incl,[
  if test x"${with_ffmpeg_incl}" != x ; then
    if test -f ${with_ffmpeg_incl}/avcodec.h ; then
      ac_cv_path_ffmpeg_incl=`(cd ${with_ffmpeg_incl}; pwd)`
    elif test -f ${with_ffmpeg_incl}/avcodec.h; then
      ac_cv_path_ffmpeg_incl=`(cd ${with_ffmpeg_incl}; pwd)`
    else
      AC_MSG_ERROR([${with_ffmpeg_incl} directory doesn't contain any headers])
    fi
  fi
  ])

  if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_ffmpeg_incl}" = x; then
   $PKG_CONFIG --exists libavcodec && ac_cv_path_ffmpeg_incl=`$PKG_CONFIG --cflags libavcodec`
  fi

  if test x"${ac_cv_path_ffmpeg_incl}" = x ; then
    AC_MSG_CHECKING([for ffmpeg header])
    incllist="${prefix}/include /sw/include /usr/local/include /home/latest/include /opt/include /opt/local/include /usr/include .. ../.."

    for i in $incllist; do
      if test -f $i/ffmpeg/avcodec.h; then
        ac_cv_path_ffmpeg_incl="-I$i/ffmpeg"
        break
      fi
    done
  fi

  if test x"${ac_cv_path_ffmpeg_incl}" != x ; then
    FFMPEG_CFLAGS="-I${ac_cv_path_ffmpeg_incl}"
    AC_MSG_RESULT(yes)
  else
    FFMPEG_CFLAGS=""
    AC_MSG_RESULT(no)
  fi

  dnl Look for the library
  AC_ARG_WITH(ffmpeg_lib, [  --with-ffmpeg-lib         directory where ffmpeg libraries are], with_ffmpeg_lib=${withval})
    AC_CACHE_VAL(ac_cv_path_ffmpeg_lib,[
    if test x"${with_ffmpeg_lib}" != x ; then
      if test -f ${with_ffmpeg_lib}/libavcodec.a -o -f ${with_ffmpeg_lib}/libavcodec.so; then
	ac_cv_path_ffmpeg_lib=`(cd ${with_ffmpeg_incl}; pwd)`
      else
	AC_MSG_ERROR([${with_ffmpeg_lib} directory doesn't contain ffmpeg libraries.])
      fi
    fi
  ])

  dnl Try with pkg-config
  if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_ffmpeg_lib}" = x; then
    $PKG_CONFIG --exists libavcodec && FFMPEG_LIBS=`$PKG_CONFIG --libs libavcodec`
  fi

  libn="no"
  if test x"${ac_cv_path_ffmpeg_lib}" = x -a x"$FFMPEG_LIBS" = x ; then
      AC_MSG_CHECKING([for libffmpeg library])
      libslist="${prefix}/lib64 ${prefix}/lib /usr/lib64 /usr/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib /opt/local/lib /usr/pkg/lib .. ../.."
      for i in $libslist; do
	if test -f $i/libavcodec.a -o -f $i/libavcodec.so; then
	  if test x"$i" != x"/usr/lib"; then
	    ac_cv_path_ffmpeg_lib="-L$i"
            AC_MSG_RESULT(${ac_cv_path_ffmpeg_lib})
	    break
          else
	    ac_cv_path_ffmpeg_lib=""
	    FFMPEG_LIBS="-lavcodec -ldts"
            AC_MSG_RESULT(yes)
	    libn="yes"
	    break
          fi
        fi
      done
      if test x"$libn" != x"yes"; then
        AC_MSG_RESULT(no)
      fi
  else
    if test -f ${ac_cv_path_ffmpeg_lib}/libavcodec.a -o -f ${ac_cv_path_ffmpeg_lib}/libavcodec.so; then
      if test x"${ac_cv_path_ffmpeg_lib}" != x"/usr/lib"; then
	ac_cv_path_ffmpeg_lib="-L${ac_cv_path_ffmpeg_lib} -lavcodec -ldts"
      else
        ac_cv_path_ffmpeg_lib=""
        FFMPEG_LIBS="-lavcodec"
      fi
    fi
  fi

  if test x"$FFMPEG_LIBS" = x; then
    if x"${ac_cv_path_ffmpeg_lib}" != x -o x"$libn" = x"yes"; then
      FFMPEG_LIBS="${ac_cv_path_ffmpeg_lib} -lavcodec -ldts"
    fi
  fi

  dnl
  dnl Call AC_CHECK_LIB here to
  dnl make sure ffmpeg actually works and we get standard defines
  dnl

  ac_save_LIBS="$LIBS"
  LIBS="$LIBS $FFMPEG_LIBS"
  AC_CHECK_LIB(avcodec, ff_eval, [avcodec_lib_ok="yes"], [avcodec_lib_ok="no"])
  AC_CHECK_LIB(dts, dts_init, [dts_lib_ok="yes"], [dts_lib_ok="no"])
  LIBS="$ac_save_LIBS"

  if test x"${avcodec_lib_ok}" = xno -o x"${dts_lib_ok}" = xno; then
     FFMPEG_LIBS=""
  fi

  if test x"$FFMPEG_LIBS" != x; then
    AC_DEFINE([USE_FFMPEG],  [1], [Use FFMPEG for MP3 decoding])
  fi

  AC_SUBST(FFMPEG_CFLAGS)  
  AC_SUBST(FFMPEG_LIBS)
])
