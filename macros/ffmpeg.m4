dnl  
dnl    Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

dnl $Id: ffmpeg.m4,v 1.50 2007/07/29 04:40:34 rsavoye Exp $

AC_DEFUN([GNASH_PATH_FFMPEG],
[

  dnl Backup LIBS and CFLAGS vars, we'll add user-specified dirs there
  backupLIBS="$LIBS"
  backupCFLAGS="$CFLAGS"

  dnl Look for the header
  AC_ARG_WITH(ffmpeg_incl, AC_HELP_STRING([--with-ffmpeg-incl], [directory where ffmpeg headers are]), with_ffmpeg_incl=${withval})
  AC_CACHE_VAL(ac_cv_path_ffmpeg_incl,[
    if test x"${with_ffmpeg_incl}" != x ; then
      if test -f ${with_ffmpeg_incl}/ffmpeg/avcodec.h ; then
        ac_cv_path_ffmpeg_incl="-I`(cd ${with_ffmpeg_incl}; pwd)`"
        avcodec_h="${with_ffmpeg_incl}/ffmpeg/avcodec.h"
        CFLAGS="$ac_cv_path_ffmpeg_incl $CFLAGS"
      else
        AC_MSG_ERROR([${with_ffmpeg_incl} directory doesn't contain the ffmpeg/avcodec.h header])
      fi
    fi
  ])

  if test x${cross_compiling} = xno; then
    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_ffmpeg_incl}" = x; then
      if $PKG_CONFIG --exists libavcodec; then
	# Some systems return /usr/include/ffmpeg, others /usr/include.
	# We use #include <ffmpeg/avcodec.h> everywhere so weed out funny
	# values into the short form.

	# Here pkg-config outputs two spaces on the end, so match those too!
	ac_cv_path_ffmpeg_incl=`$PKG_CONFIG --cflags libavcodec | sed 's:/ffmpeg *$::'`
        CFLAGS="$ac_cv_path_ffmpeg_incl $CFLAGS"
        # ac_cv_path_ffmpeg_incl might include several paths (e.g. pointers to
        # external libraries used by ffmpeg). Let's find the right one.
        for i in `$PKG_CONFIG --cflags-only-I libavcodec |sed -e 's:-I::g'`; do
          if test -e "$i"/avcodec.h -o -e "$i"/ffmpeg/avcodec.h; then
            topdir="$i"
            break
          fi
        done
      else
        # Let's see if ffmpeg is installed without using pkgconfig...
        for i in /usr/include /usr/local/include /opt/ffmpeg/include; do
          if test -e "$i"/ffmpeg/avcodec.h; then
            topdir="$i"
            break
          fi
        done
      fi
      # Again adjust for ffmpeg/ foolery
      topdir=`echo "$topdir" | sed 's:/ffmpeg *$::'`
      # Gets "" if not installed
      if test x"$topdir" != x; then
	      avcodec_h="$topdir/ffmpeg/avcodec.h"
      fi
    fi
  fi

  dnl incllist is inherited from configure.ac.
  if test x"${ac_cv_path_ffmpeg_incl}" = x ; then
    for i in $incllist; do
      if test -f $i/ffmpeg/avcodec.h; then
        ac_cv_path_ffmpeg_incl="-I$i"
        CFLAGS="$ac_cv_path_ffmpeg_incl $CFLAGS"
        topdir=$i
	        avcodec_h="$i/ffmpeg/avcodec.h"
        break
      fi
    done
  fi

  if test x"${ac_cv_path_ffmpeg_incl}" = x; then
    if test x${cross_compiling} = xno; then
      AC_CHECK_HEADERS(ffmpeg/avcodec.h, [ac_cv_path_ffmpeg_incl=""])
    fi
  else
      AC_DEFINE(HAVE_FFMPEG_AVCODEC_H, 1, [Define if you have avcodec.h installed.])
  fi

dnl Find and check libavcodec version number to make sure we have a usable
dnl version and to enable/disable features according to version.
dnl We need LIBAVCODEC VERSION of at least 51.29.0 to get avcodec_decode_audio2

dnl We try to get the avcodec version as a decimal number
dnl so that we can compare it numerically against what we require.
dnl
dnl This previous version makes ffmpeg fail and then grubs the version string
dnl out of the resulting usage message e.g. "  libavcodec version: 51.40.2"
dnl Early versions of ffmpeg do not output this string at all.
dnl
dnl   AC_PATH_PROG(FFMPEG, ffmpeg, ,[${pathlist}])
dnl   if test "x$FFMPEG" = "x" ; then
dnl     ffmpeg_version=`$FFMPEG uglyhack 2>&1 | grep "libavcodec version" | cut -d ' ' -f 5 | tr -d '.'`
dnl     if test "$ffmpeg_version" -lt 51290; then
dnl       AC_MSG_ERROR([])
dnl     fi
dnl   fi
dnl
dnl These days we check avcodec.h and pick the version out of the constants,
dnl simply throwing away all non-digits.
dnl
dnl In earlier versions we have:
dnl #define FFMPEG_VERSION_INT     0x000409
dnl #define FFMPEG_VERSION         "0.4.9-pre1"
dnl #define LIBAVCODEC_BUILD       4731
dnl #define LIBAVCODEC_VERSION_INT FFMPEG_VERSION_INT
dnl #define LIBAVCODEC_VERSION     FFMPEG_VERSION
dnl
dnl elsewhere, FFMPEG_VERSION may also be the quoted string "CVS"
dnl
dnl This changed from the above to
dnl #define LIBAVCODEC_VERSION_INT ((49<<16)+(0<<8)+0)
dnl #define LIBAVCODEC_VERSION     49.0.0
dnl #define LIBAVCODEC_BUILD       LIBAVCODEC_VERSION_INT
dnl (note, LIBAVCODEC_VERSION also changes from a quoted string to unquoted)
dnl see http://lists.mplayerhq.hu/pipermail/ffmpeg-cvslog/2005-July/000570.html
dnl
dnl Those deb-heads at Debian redefine LIBAVCODEC_VERSION in their versions to
dnl (e.g.) 1d.51.38.0 or dnl 0d.51.11.0 - we need to discard the prefixed
dnl rubbish.
dnl
dnl Other values or LIBAVCODEC_VERSION spotted in the wild:
dnl 50.0.0  50.5.0  51.7.0  51.8.0  0d.51.8.0  51.40.4
dnl presumably this will change to 52.0.0 at some point...
dnl
dnl We can ignore the very early versions because 51.10.0 is necessary to decode
dnl Flash video at all. However the current solution will incorrectly succeed
dnl on early debian/ubuntu 1d.* version numbers and incorrectly fail
dnl when 52.0.0 happens.
dnl
dnl The most reliable way is to compile a test program to print the value of
dnl LIBAVCODEC_BUILD, which got up to dnl 4718 at version 0.4.9-pre1,
dnl then changed to (major<<16 + minor<<8 + micro) from then on.
dnl There is code to do this in transcode's configure.in, but when
dnl cross-compiling we cannot run a test program, which suggests that
dnl a modified form of grepping may be better, making sure all old kinds of
dnl version numbering fail gracefully.

# Check avcodec version number, if it was found
  if test x"${avcodec_h}" != x; then

    AC_MSG_CHECKING([for ffmpeg version])

    ffmpeg_num_version=`$EGREP "define LIBAVCODEC_VERSION " ${avcodec_h} | sed -e "s%[[^0-9]]%%g"`
    ffmpeg_version=`$EGREP "define LIBAVCODEC_VERSION " ${avcodec_h} | sed -e "s%[[^0-9.]]%%g"`

    if test x"${ffmpeg_version}" = x ; then
      ffmpeg_version=`$EGREP "define LIBAVCODEC_BUILD " ${avcodec_h} | sed -e "s%[[^0-9.]]%%g"`
      ffmpeg_num_version=`$EGREP "define LIBAVCODEC_BUILD " ${avcodec_h} | sed -e "s%[[^0-9]]%%g"`
    fi

    AC_MSG_RESULT($ffmpeg_num_version)

dnl   AC_EGREP_HEADER(avcodec_decode_audio2, ${avcodec_h}, [avfound=yes], [avfound=no])
  
    if test "$ffmpeg_num_version" -lt 51110; then
      AC_MSG_WARN([Wrong ffmpeg/libavcodec version! 51.11.0 or greater required])
    else
      ffmpeg_version=ok
    fi

    if test "$ffmpeg_num_version" -gt 51280; then
      AC_DEFINE(FFMPEG_AUDIO2, 1, [Define if avcodec_decode_audio2 can be used.])
    fi

    if test "$ffmpeg_num_version" -lt 51270; then
      AC_MSG_WARN([This version of ffmpeg/libavcodec is not able to play VP6 encoded video!])
    else
      AC_DEFINE(FFMPEG_VP6, 1, [Define if ffmpeg can play VP6.])
    fi
  else
    AC_MSG_WARN([Could not check ffmpeg version (dunno where avcodec.h is)])
    ffmpeg_version=ok # trust the user-specified dir
  fi

  # Eliminate the pointless -I/usr/include, which can happen
  if test x"${ac_cv_path_ffmpeg_incl}" != x \
       -a x"${ac_cv_path_ffmpeg_incl}" != x-I/usr/include ; then
    FFMPEG_CFLAGS="${ac_cv_path_ffmpeg_incl}"
  else
    FFMPEG_CFLAGS=""
  fi

  if test x"$avcodec_h" != x; then
    swscale_h="`dirname $avcodec_h`/swscale.h"
    if test -f "$swscale_h" -a $ffmpeg_num_version -gt 51403; then
      AC_DEFINE(HAVE_SWSCALE_H, 1, [Define if swscale.h is found])
    fi
  fi

  dnl Look for the library
  AC_ARG_WITH(ffmpeg_lib, AC_HELP_STRING([--with-ffmpeg-lib], [directory where ffmpeg libraries are]), with_ffmpeg_lib=${withval})
  AC_CACHE_VAL(ac_cv_path_ffmpeg_lib, [
    if test x"${with_ffmpeg_lib}" != x ; then
      if test -f ${with_ffmpeg_lib}/libavcodec.a -o -f ${with_ffmpeg_lib}/libavcodec.${shlibext}; then
	      ac_cv_path_ffmpeg_lib="-L`(cd ${with_ffmpeg_lib}; pwd)`"
              LIBS="${ac_cv_path_ffmpeg_lib} $LIBS"
      else
	      AC_MSG_ERROR([${with_ffmpeg_lib} directory doesn't contain ffmpeg libraries.])
      fi
    fi
  ])

  dnl Try with pkg-config
  if test x"${cross_compiling}" = xno; then
    if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_ffmpeg_lib}" = x; then
      $PKG_CONFIG --exists libavcodec && libavcodec=`$PKG_CONFIG --libs libavcodec`
    fi
  fi

  if test x"${libavcodec}" != x; then
    ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} ${libavcodec}"
  else
    AC_MSG_CHECKING([for libavcodec library])
    topdir=""
    for i in $libslist; do
      if test -f $i/libavcodec.a -o -f $i/libavcodec.${shlibext}; then
        topdir=$i
        AC_MSG_RESULT(${topdir}/libavcodec)
	      if test x"$i" != x"/usr/lib"; then
	        ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -L$i -lavcodec"
       	  break
        else
	        ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -lavcodec"
	        break
       fi
      fi
    done

    if test x"${ac_cv_path_ffmpeg_lib}" = x; then
      AC_MSG_RESULT(no)
      if test x${cross_compiling} = xno; then
        dnl avcodec_decode_audio2 starts 51.29.0
        AC_CHECK_LIB(avcodec, ff_eval, [ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -lavcodec"])
      fi
    fi
  fi

  if test x"$PKG_CONFIG" != x -a x"${ac_cv_path_ffmpeg_lib}" != x; then
    $PKG_CONFIG --exists libavcodec && topdir=`$PKG_CONFIG --libs-only-L libavcodec | sed -e 's/-L//' | cut -d ' ' -f 1`
  fi

  dnl Look for the DTS library, which is required on some systems.
  if test x"${ac_cv_path_ffmpeg_lib}" != x; then
    AC_MSG_CHECKING([for libdts library])
    if test x"$PKG_CONFIG" != x -a x${cross_compiling} = xno; then
      $PKG_CONFIG --exists libdts && libdts=`$PKG_CONFIG --libs libdts`
    else
      libdts=""
    fi
    if test x"${libdts}" = x; then
      if test -f ${topdir}/libdts.a -o -f ${topdir}/libdts.${shlibext}; then
        ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -ldts"
        AC_MSG_RESULT(${topdir}/libdts)
      else
        AC_MSG_RESULT(no)
        if test x${cross_compiling} = xno; then
          AC_CHECK_LIB(dts, dts_init, [ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -ldts"])
        fi
      fi
    else
      AC_MSG_RESULT(${libdts})
      ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} ${libdts}"      
    fi
	
    dnl Look for the VORBISENC library, which is required on some systems.
    AC_MSG_CHECKING([for libvorbisenc library])
    if test x"$PKG_CONFIG" != x -a x${cross_compiling} = xno; then
      $PKG_CONFIG --exists vorbisenc && libvorbisenc=`$PKG_CONFIG --libs vorbisenc`
    else
      libvorbisenc=""
    fi
    if test x"${libvorbisenc}" = x; then
      if test -f ${topdir}/libvorbisenc.a -o -f ${topdir}/libvorbisenc.${shlibext}; then
        ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -lvorbisenc"
        AC_MSG_RESULT(${topdir}/libvorbisenc)
      else
        AC_MSG_RESULT(no)
        if test x${cross_compiling} = xno; then
          AC_CHECK_LIB(vorbisenc, vorbis_encode_init, [ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -lvorbisenc"])
        fi
      fi
    else
      AC_MSG_RESULT(${libvorbisenc})
      ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} ${libvorbisenc}"
    fi
	
    dnl Look for the AVFORMAT library, which is required on some systems.
    AC_MSG_CHECKING([for libavformat library])
    if test x"$PKG_CONFIG" != x -a x${cross_compiling} = xno; then
      $PKG_CONFIG --exists libavformat && libavformat=`$PKG_CONFIG --libs libavformat`
    else
      libavformat=""
    fi
    if test x"${libavformat}" = x; then
      if test -f ${topdir}/libavformat.a -o -f ${topdir}/libavformat.${shlibext}; then
        ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -lavformat" 
        AC_MSG_RESULT(${topdir}/libavformat)
      else
        AC_MSG_RESULT(no)
        if test x${cross_compiling} = xno; then
          AC_CHECK_LIB(libavformat, av_open_input_file, [ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -lavformat"])
        fi
      fi
    else
      AC_MSG_RESULT(${libavformat})
      ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} ${libavformat}"      
    fi

    dnl Look for the AVUTIL library, which is required on some systems.
    AC_MSG_CHECKING([for libavutil library])
    if test x"$PKG_CONFIG" != x -a x${cross_compiling} = xno; then
      $PKG_CONFIG --exists libavutil && libavutil=`$PKG_CONFIG --libs libavutil`
    else
      libavutil=""
    fi
    if test x"${libavutil}" = x; then
      if test -f ${topdir}/libavutil.a -o -f ${topdir}/libavutil.${shlibext}; then
        ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -lavutil"
        AC_MSG_RESULT(${topdir}/libavutil)
      else
        AC_MSG_RESULT(no)
        if test x${cross_compiling} = xno; then
           AC_CHECK_LIB(avutil, av_log, [ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -lavutil"])
        fi
      fi
    else
      AC_MSG_RESULT(${libavutil})
      ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} ${libavutil}"
    fi
	
    dnl Look for the THEORA library, which is required on some systems.
    AC_MSG_CHECKING([for libtheora library])
    if test x"$PKG_CONFIG" != x -a x${cross_compiling} = xno; then
      $PKG_CONFIG --exists theora && libtheora=`$PKG_CONFIG --libs theora`
    else
      libtheora=""
    fi
    if test x"${libtheora}" = x; then
      if test -f ${topdir}/libtheora.a -o -f ${topdir}/libtheora.${shlibext}; then
        ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -ltheora"
        AC_MSG_RESULT(${topdir}/libtheora)
      else
        AC_MSG_RESULT(no)
        if test x${cross_compiling} = xno; then
          AC_CHECK_LIB(theora, theora_encode_init, [ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -ltheora"])
        fi
      fi
    else
      AC_MSG_RESULT(${libtheora})
      ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} ${libtheora}"      
    fi

    dnl Look for the GSM library, which is required on some systems.
    AC_MSG_CHECKING([for libgsm library])
    if test x"$PKG_CONFIG" != x -a x${cross_compiling} = xno; then
      $PKG_CONFIG --exists gsm && libgsm=`$PKG_CONFIG --libs gsm`
    else
      libgsm=""
    fi
    if test x"${libgsm}" = x; then
      if test -f ${topdir}/libgsm.a -o -f ${topdir}/libgsm.${shlibext}; then
        ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -lgsm"
        AC_MSG_RESULT(${topdir}/libgsm)
      else
        AC_MSG_RESULT(no)
        if test x${cross_compiling} = xno; then
          AC_CHECK_LIB(gsm, gsm_destroy, [ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -lgsm"])
        fi
      fi
    else
      AC_MSG_RESULT(${libgsm})
      ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} ${libgsm}"      
    fi
    
    dnl Look for the DC1394 library, which is required on some systems.
    AC_MSG_CHECKING([for libdc1394 library])
    if test x"$PKG_CONFIG" != x -a x${cross_compiling} = xno; then
      $PKG_CONFIG --exists libdc  && libdc=`$PKG_CONFIG --libs libdc1394`
    else
      libtdc=""
    fi
    if test x"${libdc}" = x; then
      if test -f ${topdir}/libdc1394.a -o -f ${topdir}/libdc1394.${shlibext}; then
        ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -ldc1394"
        AC_MSG_RESULT(${topdir}/libdc1394)
      else
        AC_MSG_RESULT(no)
        if test x${cross_compiling} = xno; then
          AC_CHECK_LIB(dc1394_control, dc1394_is_camera, [ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -ldc1394_control"])
        fi
      fi
    else
      AC_MSG_RESULT(${libdc})
      ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} ${libdc}"
    fi

    dnl Look for the swscale library, which is required on some system if ffmpeg is
    dnl configured with --enable-gpl --enable-swscale.
    AC_MSG_CHECKING([for libswscale library])
    if test x"$PKG_CONFIG" != x -a x${cross_compiling} = xno; then
      $PKG_CONFIG --exists libswscale  && libsws=`$PKG_CONFIG --libs libswscale`
    else
      libsws=""
    fi
    if test x"${libsws}" = x; then
      if test -f ${topdir}/libswscale.a -o -f ${topdir}/libswscale.${shlibext}; then
        ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -lswscale"
        AC_MSG_RESULT(yes)
      else
        AC_MSG_RESULT(no)
        if test x${cross_compiling} = xno; then
          AC_CHECK_LIB(swscale, sws_scale, [ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -lswscale"])
        fi
      fi
    else
      AC_MSG_RESULT(${libsws})
      ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} ${libsws}"
    fi

  fi                            dnl end of all optional library tests

  # Set final compilation flags, eliminating the pointless "-I/usr/include"
  if test x"${ac_cv_path_ffmpeg_lib}" != x ; then
    FFMPEG_LIBS="${ac_cv_path_ffmpeg_lib}"
  else
    FFMPEG_LIBS=""
  fi

  AC_SUBST(FFMPEG_CFLAGS)  
  AC_SUBST(FFMPEG_LIBS)

  LIBS="$backupLIBS"
  CFLAGS="$backupCFLAGS"
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
