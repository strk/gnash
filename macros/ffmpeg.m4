dnl  
dnl    Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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


AC_DEFUN([GNASH_PATH_FFMPEG],
[

  dnl Backup LIBS and CFLAGS vars, we'll add user-specified dirs there
  backupLIBS="$LIBS"
  backupCFLAGS="$CFLAGS"

  dnl ---------------------------------
  dnl
  dnl FFMPEG headers checking
  dnl
  dnl ---------------------------------

  top_incl_dir=""


  dnl Did they tell us where the headers are ?
  AC_ARG_WITH(ffmpeg_incl, AC_HELP_STRING([--with-ffmpeg-incl], [directory where ffmpeg headers are]), with_ffmpeg_incl=${withval})
  AC_CACHE_VAL(ac_cv_path_ffmpeg_incl,[
    if test x"${with_ffmpeg_incl}" != x ; then
      avcodec_h=""
      if test -f ${with_ffmpeg_incl}/ffmpeg/avcodec.h ; then
        ac_cv_path_ffmpeg_incl="-I`(cd ${with_ffmpeg_incl}; pwd)`"
        avcodec_h="${with_ffmpeg_incl}/ffmpeg/avcodec.h"
      fi
      if test -f ${with_ffmpeg_incl}/libavcodec/avcodec.h ; then
        ac_cv_path_ffmpeg_incl="-I`(cd ${with_ffmpeg_incl}; pwd)`"
        if test x$avcodec_h != x; then
          AC_MSG_ERROR([${with_ffmpeg_incl} directory contains both the ffmpeg/avcodec.h and libavcodec/avcodec.h headers])
        fi
        avcodec_h="${with_ffmpeg_incl}/libavcodec/avcodec.h"
      fi
      if test x$avcodec_h != x; then
        CFLAGS="$ac_cv_path_ffmpeg_incl $CFLAGS"
      else
        AC_MSG_ERROR([${with_ffmpeg_incl} directory contains neither the ffmpeg/avcodec.h nor libavcodec/avcodec.h header])
      fi
      top_incl_dir=${with_ffmpeg_incl}
    fi
  ])

  dnl Try to find avcodec.h ourselves, if not cross compiling {
  if test x$top_incl_dir = x -a x${cross_compiling} = xno; then

    AC_MSG_CHECKING([location of avcodec.h])

    dnl
    dnl Try PKG_CONFIG if available
    dnl
    if test x"$PKG_CONFIG" != x; then dnl {
      if $PKG_CONFIG --exists libavcodec; then dnl {

        # Some systems return /usr/include/ffmpeg, others /usr/include.
        # We use #include <ffmpeg/avcodec.h> everywhere so weed out funny
        # values into the short form.

        # Here pkg-config outputs two spaces on the end, so match those too!
        ac_cv_path_ffmpeg_incl=`$PKG_CONFIG --cflags-only-I libavcodec | sed 's:/ffmpeg *$::'`
        CFLAGS="$ac_cv_path_ffmpeg_incl $CFLAGS"

        # ac_cv_path_ffmpeg_incl might include several paths (e.g. pointers to
        # external libraries used by ffmpeg). Let's find the right one.
        for i in `echo ${ac_cv_path_ffmpeg_incl} | sed -e 's:-I::g'`; do
          if test -e "$i"/avcodec.h -o \
                  -e "$i"/ffmpeg/avcodec.h -o \
                  -e "$i"/libavcodec/avcodec.h; then
            top_incl_dir="$i"
            break
          fi
        done

      fi dnl }
    fi dnl }

    dnl
    dnl Try manual scan if PKG_CONFIG wasn't available or could figure
    dnl
    if test x$top_incl_dir = x; then dnl {
        for i in /usr/include /usr/local/include /opt/ffmpeg/include; do
          if test -e "$i"/avcodec.h -o \
                  -e "$i"/ffmpeg/avcodec.h -o \
                  -e "$i"/libavcodec/avcodec.h; then
            top_incl_dir="$i"
            break
          fi
        done
    fi dnl }

    dnl
    dnl See what we got
    dnl
    if test x"$top_incl_dir" != x; then dnl {
        if test -e "$top_incl_dir/avcodec.h"; then
          avcodec_h="$top_incl_dir/avcodec.h"
        elif test -e "$top_incl_dir/ffmpeg/avcodec.h"; then
          avcodec_h="$top_incl_dir/ffmpeg/avcodec.h"
        elif test -e "$top_incl_dir/libavcodec/avcodec.h"; then
          avcodec_h="$top_incl_dir/libavcodec/avcodec.h"
        fi
    fi dnl }

    AC_MSG_RESULT($avcodec_h)

  fi
  dnl End of attempt at finding avcodec.h }

  dnl incllist is inherited from configure.ac.
  if test x"${top_incl_dir}" = x ; then
    AC_MSG_CHECKING([location of avcodec.h using incllist])
    for i in $incllist; do
      if test -f $i/ffmpeg/avcodec.h; then
        ac_cv_path_ffmpeg_incl="-I$i"
        CFLAGS="$ac_cv_path_ffmpeg_incl $CFLAGS"
        top_incl_dir=$i
        avcodec_h="$i/ffmpeg/avcodec.h"
        break
      fi
      if test -f $i/libavcodec/avcodec.h; then
        ac_cv_path_ffmpeg_incl="-I$i"
        CFLAGS="$ac_cv_path_ffmpeg_incl $CFLAGS"
        top_incl_dir=$i
        avcodec_h="$i/libavcodec/avcodec.h"
        break
      fi
    done
    AC_MSG_RESULT($avcodec_h)
  fi

  if test x$top_incl_dir != x; then
    AC_MSG_NOTICE([ffmpeg top include dir is $top_incl_dir])
  fi

  if test x"${avcodec_h}" = x; then
     dnl We want to bail out all at the end, so it comes with tips about how to fix etc.etc.
     AC_MSG_WARN([Cannot find ffmpeg/avcodec.h.  Use --with-ffmpeg-incl= to specify the location of the *directory* holding avcodec.h])
  else
    if echo $avcodec_h | grep -q ffmpeg/avcodec.h; then
      AC_DEFINE(HAVE_FFMPEG_AVCODEC_H, 1, [Define if you have avcodec.h installed.])
    else
      AC_DEFINE(HAVE_LIBAVCODEC_AVCODEC_H, 1, [Define if you have avcodec.h installed.])
    fi
  fi

  dnl Find and check libavcodec version number to make sure we have a usable
  dnl version and to enable/disable features according to version.
  dnl We need LIBAVCODEC VERSION of at least 51.29.0 to get avcodec_decode_audio2
  dnl
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
  dnl
  dnl This changed from the above to
  dnl #define LIBAVCODEC_VERSION_MAJOR 51
  dnl #define LIBAVCODEC_VERSION_MINOR 54
  dnl #define LIBAVCODEC_VERSION_MICRO  0
  dnl 
  dnl #define LIBAVCODEC_VERSION_INT  AV_VERSION_INT(LIBAVCODEC_VERSION_MAJOR, \
  dnl                                                LIBAVCODEC_VERSION_MINOR, \
  dnl                                                LIBAVCODEC_VERSION_MICRO)
  dnl #define LIBAVCODEC_VERSION      AV_VERSION(LIBAVCODEC_VERSION_MAJOR,    \
  dnl                                            LIBAVCODEC_VERSION_MINOR,    \
  dnl                                            LIBAVCODEC_VERSION_MICRO)
  
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

  dnl Check avcodec version number, if it was found
  if test x"${avcodec_h}" != x; then

    AC_MSG_CHECKING([ffmpeg version])

    ffmpeg_major_version=`$EGREP "define LIBAVCODEC_VERSION_MAJOR " ${avcodec_h} | sed -e "s%[[^0-9]]%%g"`
    ffmpeg_minor_version=`$EGREP "define LIBAVCODEC_VERSION_MINOR " ${avcodec_h} | sed -e "s%[[^0-9]]%%g"`
    ffmpeg_micro_version=`$EGREP "define LIBAVCODEC_VERSION_MICRO " ${avcodec_h} | sed -e "s%[[^0-9]]%%g"`

    if test x"${ffmpeg_major_version}" != x ; then

      ffmpeg_version="${ffmpeg_major_version}.${ffmpeg_minor_version}.${ffmpeg_micro_version}"

    else

      dnl #define LIBAVCODEC_VERSION_TRIPLET 51,50,1
      ffmpeg_version=`$EGREP "define LIBAVCODEC_VERSION_TRIPLET " ${avcodec_h} | awk '{print $'3'}' | sed -e "s%,%.%g"`

      if test x"${ffmpeg_version}" = x ; then

        dnl NOTE: the [0-9]*d. pattern discards deb-heads rubbish prefix
        ffmpeg_version=`$EGREP "define LIBAVCODEC_VERSION " ${avcodec_h} | awk '{print $'3'}' | sed -e "s%^[[0-9]]d\.%%"` 

        if test x"${ffmpeg_version}" = x ; then
          ffmpeg_version=`$EGREP "define LIBAVCODEC_BUILD " ${avcodec_h} | awk '{print $'3'}'`
        fi
      fi

      if test x"${ffmpeg_version}" != x ; then
        ffmpeg_major_version=`echo ${ffmpeg_version} | cut -d. -f1`
        ffmpeg_minor_version=`echo ${ffmpeg_version} | cut -d. -f2`
        ffmpeg_micro_version=`echo ${ffmpeg_version} | cut -d. -f3`
      fi

    fi

    ffmpeg_num_version=`printf %2.2d%2.2d%2.2d $ffmpeg_major_version $ffmpeg_minor_version $ffmpeg_micro_version`

    AC_MSG_RESULT($ffmpeg_version ($ffmpeg_num_version))


dnl   AC_EGREP_HEADER(avcodec_decode_audio2, ${avcodec_h}, [avfound=yes], [avfound=no])
  
    if test -z "$ffmpeg_num_version" -o "$ffmpeg_num_version" -lt 511100; then
      AC_MSG_WARN([Wrong ffmpeg/libavcodec version! 51.11.0 or greater required, $ffmpeg_version detected.])
    else
      ffmpeg_version_check=ok
    fi

    if test ! -z "$ffmpeg_num_version" -a "$ffmpeg_num_version" -gt 512800; then
      dnl 51.28.0 or higher required
      AC_DEFINE(FFMPEG_AUDIO2, 1, [Define if avcodec_decode_audio2 can be used.])
    fi

    if test -z "$ffmpeg_num_version" -o "$ffmpeg_num_version" -lt 512700; then
      dnl 51.27.0 or higher required
      AC_MSG_WARN([This version of ffmpeg/libavcodec ($ffmpeg_version) is not able to play VP6 encoded video: 51.27.0 or higher required!])
    else
      AC_DEFINE(FFMPEG_VP6, 1, [Define if ffmpeg can play VP6.])
    fi

    if test -z "$ffmpeg_num_version" -o "$ffmpeg_num_version" -lt 514900; then
      dnl 51.49.0 or higher required
      AC_MSG_WARN([This version of ffmpeg/libavcodec ($ffmpeg_version) is not able to play VP6A encoded video: 51.49.0 or higher required!])
    else
      AC_DEFINE(FFMPEG_VP6A, 1, [Define if ffmpeg can play VP6A.])
    fi

    if test -z "$ffmpeg_num_version" -o "$ffmpeg_num_version" -lt 514600; then
      dnl 51.46.0 (r10741) or higher required for CODEC_ID_NELLYMOSER
      AC_MSG_WARN([This version of ffmpeg/libavcodec ($ffmpeg_version) is not able to decode NELLYMOSER encoded audio: 51.46.0 (r10741) or higher required!])
    else
      AC_DEFINE(FFMPEG_NELLYMOSER, 1, [Define if ffmpeg can decode NELLYMOSER audio.])
    fi

  else

    AC_MSG_WARN([Could not check ffmpeg version (dunno where avcodec.h is)])
    # ffmpeg_version_check=ok # this is NOT ok, why would it be ?! 

  fi

  # Eliminate the pointless -I/usr/include, which can happen
  if test x"${ac_cv_path_ffmpeg_incl}" != x \
       -a x"${ac_cv_path_ffmpeg_incl}" != x-I/usr/include ; then
    FFMPEG_CFLAGS="${ac_cv_path_ffmpeg_incl}"
  else
    FFMPEG_CFLAGS=""
  fi

  AC_MSG_CHECKING([for avformat.h])
  if test -f "${top_incl_dir}/ffmpeg/avformat.h"; then
    AC_DEFINE(HAVE_FFMPEG_AVFORMAT_H, 1, [Define if avformat.h is found])
    avformat_h="${top_incl_dir}/ffmpeg/avformat.h"
  else
    if test -f "${top_incl_dir}/libavformat/avformat.h"; then
      AC_DEFINE(HAVE_LIBAVFORMAT_AVFORMAT_H, 1, [Define if avformat.h is found])
      avformat_h="${top_incl_dir}/libavformat/avformat.h"
    else
      avformat_h=""
    fi
  fi
  AC_MSG_RESULT($avformat_h)

  have_ffmpeg_swscale=no

  dnl look for swscale.h, but ignore versions older than 51.40.3
  if test $ffmpeg_num_version -gt 514003; then
    if test -f "${top_incl_dir}/ffmpeg/swscale.h"; then
      have_ffmpeg_swscale=yes
      AC_DEFINE(HAVE_FFMPEG_SWSCALE_H, 1, [Define if swscale.h is found])
    fi
    if test -f "${top_incl_dir}/libswscale/swscale.h"; then
      have_ffmpeg_swscale=yes
      AC_DEFINE(HAVE_LIBSWSCALE_SWSCALE_H, 1, [Define if swscale.h is found])
    fi
  fi

  if test x"$have_ffmpeg_swscale" = "xno" -a $ffmpeg_num_version -ge 520000; then
     AC_MSG_WARN([Cannot find swscale.h, required for ffmpeg versions >= 52.0.0 (detected version: $ffmpeg_version)])
     ffmpeg_version_check=
  fi

  dnl ---------------------------------
  dnl
  dnl FFMPEG libs checking
  dnl
  dnl ---------------------------------

  top_lib_dir=""

  dnl Did they tell us where the libs are ?
  AC_ARG_WITH(ffmpeg_lib, AC_HELP_STRING([--with-ffmpeg-lib], [directory where ffmpeg libraries are]), with_ffmpeg_lib=${withval})
  AC_CACHE_VAL(ac_cv_path_ffmpeg_lib, [
    if test x"${with_ffmpeg_lib}" != x ; then
      if test -f ${with_ffmpeg_lib}/libavcodec.a -o -f ${with_ffmpeg_lib}/libavcodec.${shlibext}; then
	      ac_cv_path_ffmpeg_lib="-L`(cd ${with_ffmpeg_lib}; pwd)`"
          libavcodec="-lavcodec"
          LIBS="${ac_cv_path_ffmpeg_lib} $LIBS" dnl <-- What is this needed for ?
          top_lib_dir=${with_ffmpeg_lib}
      else
	      AC_MSG_ERROR([${with_ffmpeg_lib} directory doesn't contain libavcodec libraries.])
      fi
      if test -f ${with_ffmpeg_lib}/libavformat.a -o -f ${with_ffmpeg_lib}/libavformat.${shlibext}; then
	      ac_cv_path_ffmpeg_lib="-L`(cd ${with_ffmpeg_lib}; pwd)`"
          libavformat="-lavformat"
          top_lib_dir=${with_ffmpeg_lib}
      else
	      AC_MSG_ERROR([${with_ffmpeg_lib} directory doesn't contain libavformat libraries.])
      fi
    fi
  ])

  dnl Look for the AVCODEC library

  dnl Try with pkg-config (if not cross-compiling)
  if test x"${cross_compiling}" = xno; then
    if test x"$PKG_CONFIG" != x -a x"${libavcodec}" = x; then
      $PKG_CONFIG --exists libavcodec && libavcodec=`$PKG_CONFIG --libs-only-l libavcodec`
      dnl
      dnl WARNING: we won't be able to set top_lib_dir here, as pkg-config doesn't
      dnl          return any -L when not neeed.
      dnl          We won't need top_lib_dir either, as the only use for it
      dnl          is to look for other required libraries, while pkg-config 
      dnl          probably include them all, so not a big deal.
      dnl
    fi
  fi

  if test x"${libavcodec}" != x; then
    ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} ${libavcodec}"
  else
    AC_MSG_CHECKING([for libavcodec library])
    for i in $libslist; do
      if test -f $i/libavcodec.a -o -f $i/libavcodec.${shlibext}; then
        if test x${top_lib_dir} = x; then top_lib_dir=$i; fi
        AC_MSG_RESULT(${top_lib_dir}/libavcodec)
	    if test ! x"$i" = x"/usr/lib" -a ! x"$i" = x"/usr/lib64"; then
          libavcodec="-L$i -lavcodec"
       	  break
        else
          libavcodec="-lavcodec"
	      break
        fi
	    ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} ${libavcodec}"
      fi
    done

    if test x"${libavcodec}" = x; then
      AC_MSG_RESULT(no)
      if test x${cross_compiling} = xno; then
        dnl avcodec_decode_audio2 starts 51.29.0
        AC_CHECK_LIB(avcodec, ff_eval, [ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -lavcodec"])
      fi
    fi
  fi

  if test x$top_lib_dir != x; then
    AC_MSG_NOTICE([ffmpeg top lib dir is $top_lib_dir])
  fi

  dnl Start of all optional library tests {
  dnl
  dnl TODO: skip these tests if we got ac_cv_path_ffmpeg_lib
  dnl       from pkg-config, as it would likely bring all
  dnl       required libs in. 
  dnl
  if test x"${ac_cv_path_ffmpeg_lib}" != x; then

    dnl Look for the DTS library, which is required on some systems. {
    dnl
    dnl TODO: skip this if -ldts is already in due to pkg-config 
    dnl
    AC_MSG_CHECKING([for libdts library])
    if test x"$PKG_CONFIG" != x -a x${cross_compiling} = xno; then
      $PKG_CONFIG --exists libdts && libdts=`$PKG_CONFIG --libs-only-l libdts`
    else
      libdts=""
    fi
    if test x"${libdts}" = x; then
      if test -f ${top_lib_dir}/libdts.a -o -f ${top_lib_dir}/libdts.${shlibext}; then
        ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -ldts"
        AC_MSG_RESULT(${top_lib_dir}/libdts)
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
    dnl End of DTS library looking }
	
    dnl Look for the VORBISENC library, which is required on some systems. {
    AC_MSG_CHECKING([for libvorbisenc library])
    if test x"$PKG_CONFIG" != x -a x${cross_compiling} = xno; then
      $PKG_CONFIG --exists vorbisenc && libvorbisenc=`$PKG_CONFIG --libs-only-l vorbisenc`
    else
      libvorbisenc=""
    fi
    if test x"${libvorbisenc}" = x; then
      if test -f ${top_lib_dir}/libvorbisenc.a -o -f ${top_lib_dir}/libvorbisenc.${shlibext}; then
        ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -lvorbisenc"
        AC_MSG_RESULT(${top_lib_dir}/libvorbisenc)
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
    dnl End of VORBINSEC library looking }
	

    dnl Look for the AVFORMAT library {
    dnl
    dnl TODO: libavformat be mandatory, thus linked in already ?
    dnl

    AC_MSG_CHECKING([for libavformat library])

    dnl Try with pkg-config (if not cross-compiling)
    if test x"${cross_compiling}" = xno; then
      if test x"$PKG_CONFIG" != x -a x${libavformat} = x; then
        $PKG_CONFIG --exists libavformat && libavformat=`$PKG_CONFIG --libs-only-l libavformat`
      fi
    fi

    if test x"${libavformat}" = x; then
      if test -f ${top_lib_dir}/libavformat.a -o -f ${top_lib_dir}/libavformat.${shlibext}; then
        ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -lavformat" 
        AC_MSG_RESULT(${top_lib_dir}/libavformat)
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

    dnl End of AVFORMAT library looking }

    dnl Look for the AVUTIL library, which is required on some systems. {
    dnl
    dnl TODO: skip this if -lavutil is already in due to pkg-config 
    dnl
    AC_MSG_CHECKING([for libavutil library])
    if test x"$PKG_CONFIG" != x -a x${cross_compiling} = xno; then
      $PKG_CONFIG --exists libavutil && libavutil=`$PKG_CONFIG --libs-only-l libavutil`
    else
      libavutil=""
    fi
    if test x"${libavutil}" = x; then
      if test -f ${top_lib_dir}/libavutil.a -o -f ${top_lib_dir}/libavutil.${shlibext}; then
        ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -lavutil"
        AC_MSG_RESULT(${top_lib_dir}/libavutil)
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
    dnl End of AVUTIL library looking }
	
    dnl Look for the THEORA library, which is required on some systems. {
    dnl
    dnl TODO: skip this if -ltheora is already in due to pkg-config 
    dnl
    AC_MSG_CHECKING([for libtheora library])
    if test x"$PKG_CONFIG" != x -a x${cross_compiling} = xno; then
      $PKG_CONFIG --exists theora && libtheora=`$PKG_CONFIG --libs-only-l theora`
    else
      libtheora=""
    fi
    if test x"${libtheora}" = x; then
      if test -f ${top_lib_dir}/libtheora.a -o -f ${top_lib_dir}/libtheora.${shlibext}; then
        ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -ltheora"
        AC_MSG_RESULT(${top_lib_dir}/libtheora)
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
    dnl End of THEORA library looking }

    dnl Look for the GSM library, which is required on some systems. {
    AC_MSG_CHECKING([for libgsm library])
    if test x"$PKG_CONFIG" != x -a x${cross_compiling} = xno; then
      $PKG_CONFIG --exists gsm && libgsm=`$PKG_CONFIG --libs-only-l gsm`
    else
      libgsm=""
    fi

    dnl OpenBSD seems to have a problem with libgsm.
    if test x$openbsd_os != xopenbsd; then
      if test x"${libgsm}" = x; then
        if test -f ${top_lib_dir}/libgsm.a -o -f ${top_lib_dir}/libgsm.${shlibext}; then
          ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -lgsm"
          AC_MSG_RESULT(${top_lib_dir}/libgsm)
        else
          AC_MSG_RESULT(no)
          if test x${cross_compiling} = xno; then
            AC_CHECK_LIB(gsm, gsm_destroy, [ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -lgsm"])
          fi
        fi
      fi
    else
      AC_MSG_RESULT(${libgsm})
      ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} ${libgsm}"      
    fi
    dnl End of GSM library looking }
    
    dnl Look for the DC1394 library, which is required on some systems. {
    dnl
    dnl TODO: skip this if -ldc1394 is already in due to pkg-config 
    dnl
    AC_MSG_CHECKING([for libdc1394 library])
    if test x"$PKG_CONFIG" != x -a x${cross_compiling} = xno; then
      $PKG_CONFIG --exists libdc  && libdc=`$PKG_CONFIG --libs-only-l libdc1394`
    else
      libtdc=""
    fi
    if test x"${libdc}" = x; then
      if test -f ${top_lib_dir}/libdc1394.a -o -f ${top_lib_dir}/libdc1394.${shlibext}; then
        ac_cv_path_ffmpeg_lib="${ac_cv_path_ffmpeg_lib} -ldc1394"
        AC_MSG_RESULT(${top_lib_dir}/libdc1394)
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
    dnl End of DC1394 library looking }

    dnl Look for the SWSCALE library {
    dnl
    dnl This is required on some system if ffmpeg is
    dnl configured with --enable-gpl --enable-swscale.
    AC_MSG_CHECKING([for libswscale library])
    if test x"$PKG_CONFIG" != x -a x${cross_compiling} = xno; then
      $PKG_CONFIG --exists libswscale  && libsws=`$PKG_CONFIG --libs-only-l libswscale`
    else
      libsws=""
    fi
    if test x"${libsws}" = x; then
      if test -f ${top_lib_dir}/libswscale.a -o -f ${top_lib_dir}/libswscale.${shlibext}; then
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
    dnl End of SWSCALE library looking }

  fi
  dnl End of all optional library tests }

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
