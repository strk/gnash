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


dnl $Id: ffmpeg.m4,v 1.26 2007/01/08 15:30:57 bjacques Exp $

dnl struct AVFormatParameters' has no member named 'prealloced_context'

AC_DEFUN([GNASH_PATH_FFMPEG],
[
  dnl Lool for the header
  AC_ARG_WITH(ffmpeg_incl, AC_HELP_STRING([--with-ffmpeg-incl], [directory where ffmpeg headers are]), with_ffmpeg_incl=${withval})
  AC_CACHE_VAL(ac_cv_path_ffmpeg_incl,[
    if test x"${with_ffmpeg_incl}" != x ; then
      if test -f ${with_ffmpeg_incl}/avcodec.h ; then
        ac_cv_path_ffmpeg_incl="-I`(cd ${with_ffmpeg_incl}; pwd)`"
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
    FFMPEG_CFLAGS="${ac_cv_path_ffmpeg_incl}"
    AC_MSG_RESULT(yes)
  else
    FFMPEG_CFLAGS=""
    AC_MSG_RESULT(no)
  fi

  dnl Look for the library
  AC_ARG_WITH(ffmpeg_lib, AC_HELP_STRING([--with-ffmpeg-lib], [directory where ffmpeg libraries are]), with_ffmpeg_lib=${withval})
    AC_CACHE_VAL(ac_cv_path_ffmpeg_lib,[
    if test x"${with_ffmpeg_lib}" != x ; then
      if test -f ${with_ffmpeg_lib}/libavcodec.a -o -f ${with_ffmpeg_lib}/libavcodec.so; then
	ac_cv_path_ffmpeg_lib="-L`(cd ${with_ffmpeg_lib}; pwd)`"
      else
	AC_MSG_ERROR([${with_ffmpeg_lib} directory doesn't contain ffmpeg libraries.])
      fi
    fi
  ])

  dnl Try with pkg-config
  if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_ffmpeg_lib}" = x; then
    $PKG_CONFIG --exists libavcodec && ac_cv_path_ffmpeg_lib=`$PKG_CONFIG --libs libavcodec`
  fi

  if test x"${ac_cv_path_ffmpeg_lib}" = x; then #{

    topdir=""

    AC_CHECK_LIB(avcodec, ff_eval, [ac_cv_path_ffmpeg_lib="-lavcodec"], [
      libslist="${prefix}/lib64 ${prefix}/lib /usr/lib64 /usr/lib /sw/lib /usr/local/lib /home/latest/lib /opt/lib /opt/local/lib /usr/pkg/lib .. ../.."
      for i in $libslist; do
	if test -f $i/libavcodec.a -o -f $i/libavcodec.so; then
          topdir=$i
	  if test x"$i" != x"/usr/lib"; then
	    ac_cv_path_ffmpeg_lib="-L$i -lavcodec"
       	    break
          else
	    ac_cv_path_ffmpeg_lib="-lavcodec"
	    break
          fi
        fi
      done
    ])
    AC_MSG_CHECKING([for libavcodec library])
    AC_MSG_RESULT(${ac_cv_path_ffmpeg_lib})
  fi #}

  if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_ffmpeg_lib}" != x; then
    $PKG_CONFIG --exists libavcodec && topdir=`$PKG_CONFIG --libs-only-L libavcodec | sed -e 's/-L//' | cut -d ' ' -f 1`
  fi

  if test x"${ac_cv_path_ffmpeg_lib}" != x; then
    if test x"$PKG_CONFIG" != x; then
      $PKG_CONFIG --exists libdts && libdts=`$PKG_CONFIG --libs libdts`
    else
      libdts=""
    fi

    if test x"${libdts}" = x; then
      AC_CHECK_LIB(dts, dts_init, 
        [ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -ldts"],
        [
        if test -f $topdir/libdts.a -o -f $topdir/libdts.so; then
          ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -ldts"
        fi
      ])
    else
      ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} ${libdts}"      
    fi
	AC_MSG_CHECKING([for libdts library])
	AC_MSG_RESULT(${ac_cv_path_ffmpeg_lib})
	
    if test x"$PKG_CONFIG" != x; then
      $PKG_CONFIG --exists libavutil && libavutil=`$PKG_CONFIG --libs libavutil`
    else
      libavutil=""
    fi

    if test x"${libavutil}" = x; then
      AC_CHECK_LIB(avutil, av_log,
        [ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -lavutil"],
        [
        if test -f $topdir/libavutil.a -o -f $topdir/libavutil.so; then
          ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -lavutil"
        fi
      ])
    else
      ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} ${libavutil}"

    fi
	AC_MSG_CHECKING([for libavutil library])
	AC_MSG_RESULT(${ac_cv_path_ffmpeg_lib})	
	
    if test x"$PKG_CONFIG" != x; then
      $PKG_CONFIG --exists vorbisenc && vorbisenc=`$PKG_CONFIG --libs vorbisenc`
    else
      vorbisenc=""
    fi

    if test x"${libvorbisenc}" = x; then
      AC_CHECK_LIB(vorbisenc, vorbis_encode_init, 
        [ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -lvorbisenc"],
        [
        if test -f $topdir/libvorbis.so; then
          ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -lvorbisenc"
        fi
      ])
    else
      ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} ${vorbisenc}"    
    fi
    AC_MSG_CHECKING([for libvorbisenc library])
    AC_MSG_RESULT(${ac_cv_path_ffmpeg_lib})
    
    AC_CHECK_LIB(gsm, gsm_encode, 
      [ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -lgsm"],
      [
      AC_MSG_CHECKING([for libgsm library])
      if test -f $topdir/libgsm.so; then
        ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -lgsm"
        AC_MSG_RESULT(${ac_cv_path_ffmpeg_lib})
      fi
    ])

    if test x"$PKG_CONFIG" != x; then
      $PKG_CONFIG --exists libavformat && libavformat=`$PKG_CONFIG --libs libavformat`
    else
      libavformat=""
    fi

    if test x"${libavformat}" = x; then
      AC_CHECK_LIB(avformat, av_open_input_file, 
        [ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -lavformat"],
        [
        if test -f $topdir/libavformat.so; then
          ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -lavformat"
        fi
      ])
    else
      ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} ${libavformat}"
    fi
    
    AC_MSG_CHECKING([for libavformat library])
    AC_MSG_RESULT(${ac_cv_path_ffmpeg_lib})    
    
    if test x"$PKG_CONFIG" != x; then
      $PKG_CONFIG --exists theora && libtheora=`$PKG_CONFIG --libs theora`
    else
      libtheora=""
    fi

    if test x"${libtheora}" = x; then
      AC_CHECK_LIB(theora, theora_encode_init, 
        [ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -ltheora"],
        [
        if test -f $topdir/libtheora.so; then
          ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -ltheora"
        fi
      ])
    else
      ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} ${libtheora}"
    fi
     AC_MSG_CHECKING([for libtheora library])
     AC_MSG_RESULT(${ac_cv_path_ffmpeg_lib})

    AC_MSG_CHECKING([for dc1394 library])
    AC_MSG_RESULT(${ac_cv_path_ffmpeg_lib})    
    
    if test x"$PKG_CONFIG" != x; then
      $PKG_CONFIG --exists libdc1394  && libtheora=`$PKG_CONFIG --libs libdc1394`
    else
      libtdc1394=""
    fi

    if test x"${libdc1394}" = x; then
      AC_CHECK_LIB(dc1394_control, dc1394_is_camera, 
        [ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -ldc1394_control"],
        [
        if test -f $topdir/libdc1394_control.so; then
          ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -ldc1394_control"
        fi
      ])
    else
      ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} ${libdc1394_control}"
    fi
     AC_MSG_CHECKING([for the dc1394_control library])
     AC_MSG_RESULT(${ac_cv_path_ffmpeg_lib})

  fi

  if test x"${ac_cv_path_ffmpeg_lib}" != x; then
    FFMPEG_LIBS="${ac_cv_path_ffmpeg_lib}"
  else
    FFMPEG_LIBS=""
  fi

  AC_SUBST(FFMPEG_CFLAGS)  
  AC_SUBST(FFMPEG_LIBS)
])
