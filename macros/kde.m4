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

dnl $Id: kde.m4,v 1.23 2006/11/29 21:11:06 rsavoye Exp $

dnl ------------------------------------------------------------------------
dnl Find a file (or one of more files in a list of dirs)
dnl ------------------------------------------------------------------------
dnl
AC_DEFUN([AC_FIND_FILE],
[
$3=NO
for i in $2;
do
  for j in $1;
  do
    echo "configure: __oline__: $i/$j" >&AC_FD_CC
    if test -r "$i/$j"; then
      echo "taking that" >&AC_FD_CC
      $3=$i
      break 2
    fi
  done
done
])

dnl KDE_FIND_PATH(program-name, variable-name, list-of-dirs,
dnl	if-not-found, test-parameter, prepend-path)
dnl
dnl Look for program-name in list-of-dirs+$PATH.
dnl If prepend-path is set, look in $PATH+list-of-dirs instead.
dnl If found, $variable-name is set. If not, if-not-found is evaluated.
dnl test-parameter: if set, the program is executed with this arg,
dnl                 and only a successful exit code is required.
AC_DEFUN([KDE_FIND_PATH],
[
   AC_MSG_CHECKING([for $1])
   if test -n "$$2"; then
        kde_cv_path="$$2";
   else
        kde_cache=`echo $1 | sed 'y%./+-%__p_%'`

        AC_CACHE_VAL(kde_cv_path_$kde_cache,
        [
        kde_cv_path="NONE"
	kde_save_IFS=$IFS
	IFS=':'
	dirs=""
	for dir in $PATH; do
	  dirs="$dirs $dir"
	done
	if test -z "$6"; then  dnl Append dirs in PATH (default)
	  dirs="$3 $dirs"
        else  dnl Prepend dirs in PATH (if 6th arg is set)
	  dirs="$dirs $3"
	fi
	IFS=$kde_save_IFS

        for dir in $dirs; do
	  if test -x "$dir/$1"; then
	    if test -n "$5"
	    then
              evalstr="$dir/$1 $5 2>&1 "
	      if eval $evalstr; then
                kde_cv_path="$dir/$1"
                break
	      fi
            else
		kde_cv_path="$dir/$1"
                break
	    fi
          fi
        done

        eval "kde_cv_path_$kde_cache=$kde_cv_path"

        ])

      eval "kde_cv_path=\"`echo '$kde_cv_path_'$kde_cache`\""

   fi

   if test -z "$kde_cv_path" || test "$kde_cv_path" = NONE; then
      AC_MSG_RESULT(not found)
      $4
   else
      AC_MSG_RESULT($kde_cv_path)
      $2=$kde_cv_path

   fi
])

AC_DEFUN([KDE_MOC_ERROR_MESSAGE],
[
    AC_MSG_NOTICE([No Qt meta object compiler (moc) found])
])

AC_DEFUN([KDE_UIC_ERROR_MESSAGE],
[
    AC_MSG_NOTICE([No Qt ui compiler (uic) found])
])


AC_DEFUN([KDE_CHECK_UIC_FLAG],
[
    AC_MSG_CHECKING([whether uic supports -$1 ])
    kde_cache=`echo $1 | sed 'y% .=/+-%____p_%'`
    AC_CACHE_VAL(kde_cv_prog_uic_$kde_cache,
    [
        cat >conftest.ui <<EOT
        <!DOCTYPE UI><UI version="3" stdsetdef="1"></UI>
EOT
        ac_uic_testrun="$UIC_PATH -$1 $2 conftest.ui >/dev/null"
        if AC_TRY_EVAL(ac_uic_testrun); then
            eval "kde_cv_prog_uic_$kde_cache=yes"
        else
            eval "kde_cv_prog_uic_$kde_cache=no"
        fi
        rm -f conftest*
    ])

    if eval "test \"`echo '$kde_cv_prog_uic_'$kde_cache`\" = yes"; then
        AC_MSG_RESULT([yes])
        :
        $3
    else
        AC_MSG_RESULT([no])
        :
        $4
    fi
])


dnl ------------------------------------------------------------------------
dnl Find the meta object compiler and the ui compiler in the PATH,
dnl in $QTDIR/bin, and some more usual places
dnl ------------------------------------------------------------------------
dnl
AC_DEFUN([AC_PATH_QT_MOC_UIC],
[
   AC_REQUIRE([KDE_CHECK_PERL])
 if test x"$klash" = x"yes"; then

   qt_bindirs=""
   for dir in $kde_qt_dirs; do
      qt_bindirs="$qt_bindirs $dir/bin $dir/src/moc"
   done
   qt_bindirs="$qt_bindirs /usr/bin /usr/X11R6/bin /usr/local/qt/bin"
   if test ! "$ac_qt_bindir" = "NO"; then
      qt_bindirs="$ac_qt_bindir $qt_bindirs"
   fi

   KDE_FIND_PATH(moc, MOC, [$qt_bindirs], [KDE_MOC_ERROR_MESSAGE])
   if test -z "$UIC_NOT_NEEDED"; then
     KDE_FIND_PATH(uic, UIC_PATH, [$qt_bindirs], [UIC_PATH=""])
     if test -z "$UIC_PATH" ; then
       KDE_UIC_ERROR_MESSAGE
#       exit 1
     else
       UIC=$UIC_PATH

       if test $kde_qtver = 3; then
         KDE_CHECK_UIC_FLAG(L,[/nonexistent],ac_uic_supports_libpath=yes,ac_uic_supports_libpath=no)
         KDE_CHECK_UIC_FLAG(nounload,,ac_uic_supports_nounload=yes,ac_uic_supports_nounload=no)

         if test x$ac_uic_supports_libpath = xyes; then
             UIC="$UIC -L \$(kde_widgetdir)"
         fi
         if test x$ac_uic_supports_nounload = xyes; then
             UIC="$UIC -nounload"
         fi
       fi
     fi
   else
     UIC="echo uic not available: "
   fi
fi
   AC_SUBST(MOC)
   AC_SUBST(UIC)

   UIC_TR="i18n"
   if test $kde_qtver = 3; then
     UIC_TR="tr2i18n"
   fi

   AC_SUBST(UIC_TR)
])

AC_DEFUN([KDE_1_CHECK_PATHS],
[
  KDE_1_CHECK_PATH_HEADERS

  KDE_TEST_RPATH=

  if test x"$klash" = x"yes"; then
    if test -n "$USE_RPATH"; then

       if test -n "$kde_libraries"; then
         KDE_TEST_RPATH="-R $kde_libraries"
       fi

       if test -n "$qt_libraries"; then
         KDE_TEST_RPATH="$KDE_TEST_RPATH -R $qt_libraries"
       fi

       if test -n "$x_libraries"; then
         KDE_TEST_RPATH="$KDE_TEST_RPATH -R $x_libraries"
       fi

       KDE_TEST_RPATH="$KDE_TEST_RPATH $KDE_EXTRA_RPATH"
    fi

    AC_MSG_CHECKING([for KDE libraries installed])
ac_link='$LIBTOOL_SHELL --silent --mode=link ${CXX-g++} -o conftest $CXXFLAGS $all_includes $CPPFLAGS $LDFLAGS $all_libraries conftest.$ac_ext $LIBS -lkdecore $LIBQT $KDE_TEST_RPATH 1>&5'

    if AC_TRY_EVAL(ac_link) && test -s conftest; then
      AC_MSG_RESULT(yes)
    else
      AC_MSG_NOTICE([your system fails at linking a small KDE application!.])
    fi
  fi

if eval `KDEDIR= ./conftest 2>&5`; then
  kde_result=done
else
  kde_result=problems
fi

KDEDIR= ./conftest 2> /dev/null >&5 # make an echo for config.log
kde_have_all_paths=yes

KDE_SET_PATHS($kde_result)

])

AC_DEFUN([KDE_SET_PATHS],
[
  kde_cv_all_paths="kde_have_all_paths=\"yes\" \
	kde_htmldir=\"$kde_htmldir\" \
	kde_appsdir=\"$kde_appsdir\" \
	kde_icondir=\"$kde_icondir\" \
	kde_sounddir=\"$kde_sounddir\" \
	kde_datadir=\"$kde_datadir\" \
	kde_locale=\"$kde_locale\" \
	kde_cgidir=\"$kde_cgidir\" \
	kde_confdir=\"$kde_confdir\" \
	kde_kcfgdir=\"$kde_kcfgdir\" \
	kde_mimedir=\"$kde_mimedir\" \
	kde_toolbardir=\"$kde_toolbardir\" \
	kde_wallpaperdir=\"$kde_wallpaperdir\" \
	kde_templatesdir=\"$kde_templatesdir\" \
	kde_bindir=\"$kde_bindir\" \
	kde_servicesdir=\"$kde_servicesdir\" \
	kde_servicetypesdir=\"$kde_servicetypesdir\" \
	kde_moduledir=\"$kde_moduledir\" \
	kde_styledir=\"$kde_styledir\" \
	kde_widgetdir=\"$kde_widgetdir\" \
	xdg_appsdir=\"$xdg_appsdir\" \
	xdg_menudir=\"$xdg_menudir\" \
	xdg_directorydir=\"$xdg_directorydir\" \
	kde_result=$1"
])

AC_DEFUN([KDE_SET_DEFAULT_PATHS],
[
if test "$1" = "default"; then

  if test -z "$kde_htmldir"; then
    kde_htmldir='\${datadir}/doc/HTML'
  fi
  if test -z "$kde_appsdir"; then
    kde_appsdir='\${datadir}/applnk'
  fi
  if test -z "$kde_icondir"; then
    kde_icondir='\${datadir}/icons'
  fi
  if test -z "$kde_sounddir"; then
    kde_sounddir='\${datadir}/sounds'
  fi
  if test -z "$kde_datadir"; then
    kde_datadir='\${datadir}/apps'
  fi
  if test -z "$kde_locale"; then
    kde_locale='\${datadir}/locale'
  fi
  if test -z "$kde_cgidir"; then
    kde_cgidir='\${exec_prefix}/cgi-bin'
  fi
  if test -z "$kde_confdir"; then
    kde_confdir='\${datadir}/config'
  fi
  if test -z "$kde_kcfgdir"; then
    kde_kcfgdir='\${datadir}/config.kcfg'
  fi
  if test -z "$kde_mimedir"; then
    kde_mimedir='\${datadir}/mimelnk'
  fi
  if test -z "$kde_toolbardir"; then
    kde_toolbardir='\${datadir}/toolbar'
  fi
  if test -z "$kde_wallpaperdir"; then
    kde_wallpaperdir='\${datadir}/wallpapers'
  fi
  if test -z "$kde_templatesdir"; then
    kde_templatesdir='\${datadir}/templates'
  fi
  if test -z "$kde_bindir"; then
    kde_bindir='\${exec_prefix}/bin'
  fi
  if test -z "$kde_servicesdir"; then
    kde_servicesdir='\${datadir}/services'
  fi
  if test -z "$kde_servicetypesdir"; then
    kde_servicetypesdir='\${datadir}/servicetypes'
  fi
  if test -z "$kde_moduledir"; then
    if test "$kde_qtver" = "2"; then
      kde_moduledir='\${libdir}/kde2'
    else
      kde_moduledir='\${libdir}/kde3'
    fi
  fi
  if test -z "$kde_styledir"; then
    kde_styledir='\${libdir}/kde3/plugins/styles'
  fi
  if test -z "$kde_widgetdir"; then
    kde_widgetdir='\${libdir}/kde3/plugins/designer'
  fi
  if test -z "$xdg_appsdir"; then
    xdg_appsdir='\${datadir}/applications/kde'
  fi
  if test -z "$xdg_menudir"; then
    xdg_menudir='\${sysconfdir}/xdg/menus'
  fi
  if test -z "$xdg_directorydir"; then
    xdg_directorydir='\${datadir}/desktop-directories'
  fi

  KDE_SET_PATHS(defaults)

else

  if test $kde_qtver = 1; then
     AC_MSG_RESULT([compiling])
     KDE_1_CHECK_PATHS
  else
     AC_MSG_ERROR([path checking not yet supported for KDE 2])
  fi

fi
])

AC_DEFUN([KDE_CHECK_PATHS_FOR_COMPLETENESS],
[ if test -z "$kde_htmldir" || test -z "$kde_appsdir" ||
   test -z "$kde_icondir" || test -z "$kde_sounddir" ||
   test -z "$kde_datadir" || test -z "$kde_locale"  ||
   test -z "$kde_cgidir"  || test -z "$kde_confdir" ||
   test -z "$kde_kcfgdir" ||
   test -z "$kde_mimedir" || test -z "$kde_toolbardir" ||
   test -z "$kde_wallpaperdir" || test -z "$kde_templatesdir" ||
   test -z "$kde_bindir" || test -z "$kde_servicesdir" ||
   test -z "$kde_servicetypesdir" || test -z "$kde_moduledir" ||
   test -z "$kde_styledir" || test -z "kde_widgetdir" ||
   test -z "$xdg_appsdir" || test -z "$xdg_menudir" || test -z "$xdg_directorydir" ||
   test "x$kde_have_all_paths" != "xyes"; then
     kde_have_all_paths=no
   else
     AC_DEFINE([HAVE_KDE], ,[Has KDE installed])
     kde=yes
  fi
])

AC_DEFUN([KDE_MISSING_PROG_ERROR],
[
    AC_MSG_NOTICE([$1 not found])
    AC_MSG_NOTICE([WARNING: klash disabled due to this])
    klash=no
])

AC_DEFUN([KDE_MISSING_ARTS_ERROR],
[
    AC_MSG_NOTICE([$1 not found])
])

AC_DEFUN([KDE_SET_DEFAULT_BINDIRS],
[
    kde_default_bindirs="/usr/bin /usr/local/bin /opt/local/bin /usr/X11R6/bin /opt/kde/bin /opt/kde3/bin /usr/kde/bin /usr/local/kde/bin"
    test -n "$KDEDIR" && kde_default_bindirs="$KDEDIR/bin $kde_default_bindirs"
    if test -n "$KDEDIRS"; then
       kde_save_IFS=$IFS
       IFS=:
       for dir in $KDEDIRS; do
            kde_default_bindirs="$dir/bin $kde_default_bindirs "
       done
       IFS=$kde_save_IFS
    fi
])

AC_DEFUN([KDE_SUBST_PROGRAMS],
[
    AC_ARG_WITH(arts,
        AC_HELP_STRING([--without-arts],[build without aRts [default=no]]),
        [build_arts=$withval],
        [build_arts=yes]
    )
    if test "$build_arts" = "no"; then
        AC_DEFINE(WITHOUT_ARTS, 1, [Defined if compiling without arts])
    fi

  if test x"$klash" = x"yes"; then
        KDE_SET_DEFAULT_BINDIRS
        kde_default_bindirs="$exec_prefix/bin $prefix/bin $kde_libs_prefix/bin $kde_default_bindirs"
        KDE_FIND_PATH(dcopidl, DCOPIDL, [$kde_default_bindirs], [KDE_MISSING_PROG_ERROR(dcopidl)])
        KDE_FIND_PATH(dcopidl2cpp, DCOPIDL2CPP, [$kde_default_bindirs], [KDE_MISSING_PROG_ERROR(dcopidl2cpp)])
        if test "$build_arts" '!=' "no"; then
          KDE_FIND_PATH(mcopidl, MCOPIDL, [$kde_default_bindirs], [KDE_MISSING_ARTS_ERROR(mcopidl)])
          KDE_FIND_PATH(artsc-config, ARTSCCONFIG, [$kde_default_bindirs], [KDE_MISSING_ARTS_ERROR(artsc-config)])
        fi
        KDE_FIND_PATH(meinproc, MEINPROC, [$kde_default_bindirs])

        kde32ornewer=1
        kde33ornewer=1
        if test -n "$kde_qtver" && test "$kde_qtver" -lt 3; then
            kde32ornewer=
            kde33ornewer=
        else
            if test "$kde_qtver" = "3"; then
              if test "$kde_qtsubver" -le 1; then
                kde32ornewer=
              fi
              if test "$kde_qtsubver" -le 2 -o `$KDECONFIG --version | grep KDE | sed 's/KDE: \(...\).*/\1/'` = 3.2; then
                kde33ornewer=
              fi
            fi
        fi

        if test -n "$kde32ornewer"; then
            KDE_FIND_PATH(kconfig_compiler, KCONFIG_COMPILER, [$kde_default_bindirs], [KDE_MISSING_PROG_ERROR(kconfig_compiler)])
            KDE_FIND_PATH(dcopidlng, DCOPIDLNG, [$kde_default_bindirs], [KDE_MISSING_PROG_ERROR(dcopidlng)])
        fi
        if test -n "$kde33ornewer"; then
            KDE_FIND_PATH(makekdewidgets, MAKEKDEWIDGETS, [$kde_default_bindirs], [KDE_MISSING_PROG_ERROR(makekdewidgets)])
            AC_SUBST(MAKEKDEWIDGETS)
        fi
        KDE_FIND_PATH(xmllint, XMLLINT, [${prefix}/bin ${exec_prefix}/bin], [XMLLINT=""])

        if test -n "$MEINPROC" -a "$MEINPROC" != "compiled"; then
 	    kde_sharedirs="/usr/share/kde /usr/local/share /usr/share /opt/kde3/share /opt/kde/share $prefix/share"
            test -n "$KDEDIR" && kde_sharedirs="$KDEDIR/share $kde_sharedirs"
            AC_FIND_FILE(apps/ksgmltools2/customization/kde-chunk.xsl, $kde_sharedirs, KDE_XSL_STYLESHEET)
	    if test "$KDE_XSL_STYLESHEET" = "NO"; then
		KDE_XSL_STYLESHEET=""
	    else
                KDE_XSL_STYLESHEET="$KDE_XSL_STYLESHEET/apps/ksgmltools2/customization/kde-chunk.xsl"
	    fi
        fi

        DCOP_DEPENDENCIES='$(DCOPIDL)'
        if test -n "$kde32ornewer"; then
            KCFG_DEPENDENCIES='$(KCONFIG_COMPILER)'
            DCOP_DEPENDENCIES='$(DCOPIDL) $(DCOPIDLNG)'
            AC_SUBST(KCONFIG_COMPILER)
            AC_SUBST(KCFG_DEPENDENCIES)
            AC_SUBST(DCOPIDLNG)
        fi
        AC_SUBST(DCOPIDL)
        AC_SUBST(DCOPIDL2CPP)
        AC_SUBST(DCOP_DEPENDENCIES)
        AC_SUBST(MCOPIDL)
        AC_SUBST(ARTSCCONFIG)
	AC_SUBST(MEINPROC)
 	AC_SUBST(KDE_XSL_STYLESHEET)
	AC_SUBST(XMLLINT)
  fi
])dnl

AC_DEFUN([AC_CREATE_KFSSTND],
[
AC_REQUIRE([AC_CHECK_RPATH])

AC_MSG_CHECKING([for KDE paths])
kde_result=""
kde_cached_paths=yes
AC_CACHE_VAL(kde_cv_all_paths,
[
  KDE_SET_DEFAULT_PATHS($1)
  kde_cached_paths=no
])
eval "$kde_cv_all_paths"
KDE_CHECK_PATHS_FOR_COMPLETENESS
if test "$kde_have_all_paths" = "no" && test "$kde_cached_paths" = "yes"; then
  # wrong values were cached, may be, we can set better ones
  kde_result=
  kde_htmldir= kde_appsdir= kde_icondir= kde_sounddir=
  kde_datadir= kde_locale=  kde_cgidir=  kde_confdir= kde_kcfgdir=
  kde_mimedir= kde_toolbardir= kde_wallpaperdir= kde_templatesdir=
  kde_bindir= kde_servicesdir= kde_servicetypesdir= kde_moduledir=
  kde_have_all_paths=
  kde_styledir=
  kde_widgetdir=
  xdg_appsdir = xdg_menudir= xdg_directorydir= 
  KDE_SET_DEFAULT_PATHS($1)
  eval "$kde_cv_all_paths"
  KDE_CHECK_PATHS_FOR_COMPLETENESS
  kde_result="$kde_result (cache overridden)"
fi
if test "$kde_have_all_paths" = "no"; then
  AC_MSG_ERROR([configure could not run a little KDE program to test the environment.
Since it had compiled and linked before, it must be a strange problem on your system.
Look at config.log for details. If you are not able to fix this, look at
http://www.kde.org/faq/installation.html or any www.kde.org mirror.
(If you're using an egcs version on Linux, you may update binutils!)
])
else
  rm -f conftest*
  AC_MSG_RESULT($kde_result)
fi

bindir=$kde_bindir

KDE_SUBST_PROGRAMS

])

AC_DEFUN([AC_SUBST_KFSSTND],
[
AC_SUBST(kde_htmldir)
AC_SUBST(kde_appsdir)
AC_SUBST(kde_icondir)
AC_SUBST(kde_sounddir)
AC_SUBST(kde_datadir)
AC_SUBST(kde_locale)
AC_SUBST(kde_confdir)
AC_SUBST(kde_kcfgdir)
AC_SUBST(kde_mimedir)
AC_SUBST(kde_wallpaperdir)
AC_SUBST(kde_bindir)
dnl X Desktop Group standards
AC_SUBST(xdg_appsdir)
AC_SUBST(xdg_menudir)
AC_SUBST(xdg_directorydir)
dnl for KDE 2
AC_SUBST(kde_templatesdir)
AC_SUBST(kde_servicesdir)
AC_SUBST(kde_servicetypesdir)
AC_SUBST(kde_moduledir)
AC_SUBST(kdeinitdir, '$(kde_moduledir)')
AC_SUBST(kde_styledir)
AC_SUBST(kde_widgetdir)
if test "$kde_qtver" = 1; then
  kde_minidir="$kde_icondir/mini"
else
# for KDE 1 - this breaks KDE2 apps using minidir, but
# that's the plan ;-/
  kde_minidir="/dev/null"
fi
dnl AC_SUBST(kde_minidir)
dnl AC_SUBST(kde_cgidir)
dnl AC_SUBST(kde_toolbardir)
])

AC_DEFUN([KDE_MISC_TESTS],
[
   dnl Checks for libraries.
   AC_CHECK_LIB(util, main, [LIBUTIL="-lutil"]) dnl for *BSD 
   AC_SUBST(LIBUTIL)
   AC_CHECK_LIB(compat, main, [LIBCOMPAT="-lcompat"]) dnl for *BSD
   AC_SUBST(LIBCOMPAT)
   kde_have_crypt=
   AC_CHECK_LIB(crypt, crypt, [LIBCRYPT="-lcrypt"; kde_have_crypt=yes],
      AC_CHECK_LIB(c, crypt, [kde_have_crypt=yes], [
        AC_MSG_WARN([you have no crypt in either libcrypt or libc.
You should install libcrypt from another source or configure with PAM
support])
	kde_have_crypt=no
      ]))
   AC_SUBST(LIBCRYPT)
   if test $kde_have_crypt = yes; then
      AC_DEFINE_UNQUOTED(HAVE_CRYPT, 1, [Defines if your system has the crypt function])
   fi
   AC_CHECK_SOCKLEN_T
   AC_CHECK_LIB(dnet, dnet_ntoa, [X_EXTRA_LIBS="$X_EXTRA_LIBS -ldnet"])
   if test $ac_cv_lib_dnet_dnet_ntoa = no; then
      AC_CHECK_LIB(dnet_stub, dnet_ntoa,
        [X_EXTRA_LIBS="$X_EXTRA_LIBS -ldnet_stub"])
   fi
   AC_CHECK_FUNC(inet_ntoa)
   if test $ac_cv_func_inet_ntoa = no; then
     AC_CHECK_LIB(nsl, inet_ntoa, X_EXTRA_LIBS="$X_EXTRA_LIBS -lnsl")
   fi
   AC_CHECK_FUNC(connect)
   if test $ac_cv_func_connect = no; then
      AC_CHECK_LIB(socket, connect, X_EXTRA_LIBS="-lsocket $X_EXTRA_LIBS", ,
        $X_EXTRA_LIBS)
   fi

   AC_CHECK_FUNC(remove)
   if test $ac_cv_func_remove = no; then
      AC_CHECK_LIB(posix, remove, X_EXTRA_LIBS="$X_EXTRA_LIBS -lposix")
   fi

   # BSDI BSD/OS 2.1 needs -lipc for XOpenDisplay.
   AC_CHECK_FUNC(shmat, ,
     AC_CHECK_LIB(ipc, shmat, X_EXTRA_LIBS="$X_EXTRA_LIBS -lipc"))
   
   # more headers that need to be explicitly included on darwin
   AC_CHECK_HEADERS(sys/types.h stdint.h)

   # sys/bitypes.h is needed for uint32_t and friends on Tru64
   AC_CHECK_HEADERS(sys/bitypes.h)

   # darwin requires a poll emulation library
   AC_CHECK_LIB(poll, poll, LIB_POLL="-lpoll")

   # for some image handling on Mac OS X
   AC_CHECK_HEADERS(Carbon/Carbon.h)

   # CoreAudio framework
   AC_CHECK_HEADER(CoreAudio/CoreAudio.h, [
     AC_DEFINE(HAVE_COREAUDIO, 1, [Define if you have the CoreAudio API])
     FRAMEWORK_COREAUDIO="-Xlinker -framework -Xlinker CoreAudio"
   ])

   AC_CHECK_RES_INIT
   AC_SUBST(LIB_POLL)
   AC_SUBST(FRAMEWORK_COREAUDIO)
   LIBSOCKET="$X_EXTRA_LIBS"
   AC_SUBST(LIBSOCKET)
   AC_SUBST(X_EXTRA_LIBS)
   AC_CHECK_LIB(ucb, killpg, [LIBUCB="-lucb"]) dnl for Solaris2.4
   AC_SUBST(LIBUCB)

   case $host in  dnl this *is* LynxOS specific
   *-*-lynxos* )
        AC_MSG_CHECKING([LynxOS header file wrappers])
        [CFLAGS="$CFLAGS -D__NO_INCLUDE_WARN__"]
        AC_MSG_RESULT(disabled)
        AC_CHECK_LIB(bsd, gethostbyname, [LIBSOCKET="-lbsd"]) dnl for LynxOS
         ;;
    esac

   KDE_CHECK_TYPES
   KDE_CHECK_LIBDL
   KDE_CHECK_STRLCPY

# darwin needs this to initialize the environment
AC_CHECK_HEADERS(crt_externs.h)
AC_CHECK_FUNC(_NSGetEnviron, [AC_DEFINE(HAVE_NSGETENVIRON, 1, [Define if your system needs _NSGetEnviron to set up the environment])])
 
AH_VERBATIM(_DARWIN_ENVIRON,
[
#if defined(HAVE_NSGETENVIRON) && defined(HAVE_CRT_EXTERNS_H)
# include <sys/time.h>
# include <crt_externs.h>
# define environ (*_NSGetEnviron())
#endif
])

AH_VERBATIM(_AIX_STRINGS_H_BZERO,
[
/*
 * AIX defines FD_SET in terms of bzero, but fails to include <strings.h>
 * that defines bzero.
 */

#if defined(_AIX)
#include <strings.h>
#endif
])

])

dnl ------------------------------------------------------------------------
dnl Find the header files and libraries for X-Windows. Extended the
dnl macro AC_PATH_X
dnl ------------------------------------------------------------------------
dnl
AC_DEFUN([K_PATH_X],
[
AC_REQUIRE([KDE_MISC_TESTS])dnl
AC_REQUIRE([KDE_CHECK_LIB64])

AC_ARG_ENABLE(
  embedded,
  AC_HELP_STRING([--enable-embedded],[link to Qt-embedded, don't use X]),
  kde_use_qt_emb=$enableval,
  kde_use_qt_emb=no
)

AC_ARG_ENABLE(
  qtopia,
  AC_HELP_STRING([--enable-qtopia],[link to Qt-embedded, link to the Qtopia Environment]),
  kde_use_qt_emb_palm=$enableval,
  kde_use_qt_emb_palm=no
)

AC_ARG_ENABLE(
  mac,
  AC_HELP_STRING([--enable-mac],[link to Qt/Mac (don't use X)]),
  kde_use_qt_mac=$enableval,
  kde_use_qt_mac=no
)

if test "$kde_use_qt_emb" = "no" && test "$kde_use_qt_mac" = "no"; then

AC_MSG_CHECKING(for X)

AC_CACHE_VAL(kde_cv_have_x,
[ dnl One or both of the vars are not set, and there is no cached value.
if test "{$x_includes+set}" = set || test "$x_includes" = NONE; then
   kde_x_includes=NO
else
   kde_x_includes=$x_includes
fi
if test "{$x_libraries+set}" = set || test "$x_libraries" = NONE; then
   kde_x_libraries=NO
else
   kde_x_libraries=$x_libraries
fi

# when the user already gave --x-includes, we ignore
# what the standard autoconf macros told us.
if test "$kde_x_includes" = NO; then
  kde_x_includes=$x_includes
fi

# for --x-libraries too
if test "$kde_x_libraries" = NO; then
  kde_x_libraries=$x_libraries
fi

if test "$kde_x_includes" = NO; then
  AC_MSG_ERROR([Can't find X includes. Please check your installation and add the correct paths!])
fi

if test "$kde_x_libraries" = NO; then
  AC_MSG_ERROR([Can't find X libraries. Please check your installation and add the correct paths!])
fi

# Record where we found X for the cache.
kde_cv_have_x="have_x=yes \
         kde_x_includes=$kde_x_includes kde_x_libraries=$kde_x_libraries"
])dnl

eval "$kde_cv_have_x"

if test "$have_x" != yes; then
  AC_MSG_RESULT($have_x)
  no_x=yes
else
  AC_MSG_RESULT([libraries $kde_x_libraries, headers $kde_x_includes])
fi

if test -z "$kde_x_includes" || test "x$kde_x_includes" = xNONE; then
  X_INCLUDES=""
  x_includes="."; dnl better than nothing :-
 else
  x_includes=$kde_x_includes
  X_INCLUDES="-I$x_includes"
fi

if test -z "$kde_x_libraries" || test "x$kde_x_libraries" = xNONE; then
  X_LDFLAGS=""
  x_libraries="/usr/lib"; dnl better than nothing :-
 else
  x_libraries=$kde_x_libraries
  X_LDFLAGS="-L$x_libraries"
fi
all_includes="$X_INCLUDES"
all_libraries="$X_LDFLAGS $LDFLAGS_AS_NEEDED $LDFLAGS_NEW_DTAGS"

# Check for libraries that X11R6 Xt/Xaw programs need.
ac_save_LDFLAGS="$LDFLAGS"
LDFLAGS="$LDFLAGS $X_LDFLAGS"
# SM needs ICE to (dynamically) link under SunOS 4.x (so we have to
# check for ICE first), but we must link in the order -lSM -lICE or
# we get undefined symbols.  So assume we have SM if we have ICE.
# These have to be linked with before -lX11, unlike the other
# libraries we check for below, so use a different variable.
#  --interran@uluru.Stanford.EDU, kb@cs.umb.edu.
AC_CHECK_LIB(ICE, IceConnectionNumber,
  [LIBSM="-lSM -lICE"], , $X_EXTRA_LIBS)
LDFLAGS="$ac_save_LDFLAGS"

LIB_X11='-lX11 $(LIBSOCKET)'

AC_MSG_CHECKING(for libXext)
AC_CACHE_VAL(kde_cv_have_libXext,
[
kde_ldflags_safe="$LDFLAGS"
kde_libs_safe="$LIBS"

LDFLAGS="$LDFLAGS $X_LDFLAGS $USER_LDFLAGS"
LIBS="-lXext -lX11 $LIBSOCKET"

AC_TRY_LINK([
#include <stdio.h>
#ifdef STDC_HEADERS
# include <stdlib.h>
#endif
],
[
printf("hello Xext\n");
],
kde_cv_have_libXext=yes,
kde_cv_have_libXext=no
)

LDFLAGS=$kde_ldflags_safe
LIBS=$kde_libs_safe
])

AC_MSG_RESULT($kde_cv_have_libXext)

if test "$kde_cv_have_libXext" = "no"; then
  AC_MSG_ERROR([We need a working libXext to proceed. Since configure
can't find it itself, we stop here assuming that make wouldn't find
them either.])
fi

LIB_XEXT="-lXext"
QTE_NORTTI=""

elif test "$kde_use_qt_emb" = "yes"; then
  dnl We're using QT Embedded
  CPPFLAGS=-DQWS
  CXXFLAGS="$CXXFLAGS -fno-rtti"
  QTE_NORTTI="-fno-rtti -DQWS"
  X_PRE_LIBS=""
  LIB_X11=""
  LIB_XEXT=""
  LIB_XRENDER=""
  LIBSM=""
  X_INCLUDES=""
  X_LDFLAGS=""
  x_includes=""
  x_libraries=""
elif test "$kde_use_qt_mac" = "yes"; then
  dnl We're using QT/Mac (I use QT_MAC so that qglobal.h doesn't *have* to
  dnl be included to get the information) --Sam
  CXXFLAGS="$CXXFLAGS -DQT_MAC -no-cpp-precomp"
  CFLAGS="$CFLAGS -DQT_MAC -no-cpp-precomp"
  X_PRE_LIBS=""
  LIB_X11=""
  LIB_XEXT=""
  LIB_XRENDER=""
  LIBSM=""
  X_INCLUDES=""
  X_LDFLAGS=""
  x_includes=""
  x_libraries=""
fi
AC_SUBST(X_PRE_LIBS)
AC_SUBST(LIB_X11)
AC_SUBST(LIB_XRENDER)
AC_SUBST(LIBSM)
AC_SUBST(X_INCLUDES)
AC_SUBST(X_LDFLAGS)
AC_SUBST(x_includes)
AC_SUBST(x_libraries)
AC_SUBST(QTE_NORTTI)
AC_SUBST(LIB_XEXT)

])

AC_DEFUN([KDE_PRINT_QT_PROGRAM],
[
AC_REQUIRE([KDE_USE_QT])
cat > conftest.$ac_ext <<EOF
#include "confdefs.h"
#include <qglobal.h>
#include <qapplication.h>
EOF
if test "$kde_qtver" = "2"; then
cat >> conftest.$ac_ext <<EOF
#include <qevent.h>
#include <qstring.h>
#include <qstyle.h>
EOF

if test $kde_qtsubver -gt 0; then
cat >> conftest.$ac_ext <<EOF
#if QT_VERSION < 210
#error 1
#endif
EOF
fi
fi

if test "$kde_qtver" = "3"; then
cat >> conftest.$ac_ext <<EOF
#include <qcursor.h>
#include <qstylefactory.h>
#include <private/qucomextra_p.h>
EOF
fi

echo "#if ! ($kde_qt_verstring)" >> conftest.$ac_ext
cat >> conftest.$ac_ext <<EOF
#error 1
#endif

int main() {
EOF
if test "$kde_qtver" = "2"; then
cat >> conftest.$ac_ext <<EOF
    QStringList *t = new QStringList();
    Q_UNUSED(t);
EOF
if test $kde_qtsubver -gt 0; then
cat >> conftest.$ac_ext <<EOF
    QString s;
    s.setLatin1("Elvis is alive", 14);
EOF
fi
fi
if test "$kde_qtver" = "3"; then
cat >> conftest.$ac_ext <<EOF
    (void)QStyleFactory::create(QString::null);
    QCursor c(Qt::WhatsThisCursor);
EOF
fi
cat >> conftest.$ac_ext <<EOF
    return 0;
}
EOF
])

AC_DEFUN([KDE_USE_QT],
[
if test -z "$1"; then
  # Current default Qt version: 3.3
  kde_qtver=3
  kde_qtsubver=3
else
  kde_qtsubver=`echo "$1" | sed -e 's#[0-9][0-9]*\.\([0-9][0-9]*\).*#\1#'`
  # following is the check if subversion isnt found in passed argument
  if test "$kde_qtsubver" = "$1"; then
    kde_qtsubver=1
  fi
  kde_qtver=`echo "$1" | sed -e 's#^\([0-9][0-9]*\)\..*#\1#'`
  if test "$kde_qtver" = "1"; then
    kde_qtsubver=42
  fi
fi

if test -z "$2"; then
  if test "$kde_qtver" = "2"; then
    if test $kde_qtsubver -gt 0; then
      kde_qt_minversion=">= Qt 2.2.2"
    else
      kde_qt_minversion=">= Qt 2.0.2"
    fi
  fi
  if test "$kde_qtver" = "3"; then
    if test $kde_qtsubver -gt 0; then
	 if test $kde_qtsubver -gt 1; then
	    if test $kde_qtsubver -gt 2; then
		kde_qt_minversion=">= Qt 3.3 and < 4.0"
	    else
	        kde_qt_minversion=">= Qt 3.2 and < 4.0"
	    fi
	 else
            kde_qt_minversion=">= Qt 3.1 (20021021) and < 4.0"
         fi
    else
      kde_qt_minversion=">= Qt 3.0 and < 4.0"
    fi
  fi
  if test "$kde_qtver" = "1"; then
    kde_qt_minversion=">= 1.42 and < 2.0"
  fi
else
   kde_qt_minversion="$2"
fi

if test -z "$3"; then
   if test $kde_qtver = 3; then
     if test $kde_qtsubver -gt 0; then
       kde_qt_verstring="QT_VERSION >= 0x03@VER@00 && QT_VERSION < 0x040000"
       qtsubver=`echo "00$kde_qtsubver" | sed -e 's,.*\(..\)$,\1,'`
       kde_qt_verstring=`echo $kde_qt_verstring | sed -e "s,@VER@,$qtsubver,"`
     else
       kde_qt_verstring="QT_VERSION >= 300 && QT_VERSION < 0x040000"
     fi
   fi
   if test $kde_qtver = 2; then
     if test $kde_qtsubver -gt 0; then
       kde_qt_verstring="QT_VERSION >= 222"
     else
       kde_qt_verstring="QT_VERSION >= 200"
     fi
   fi
   if test $kde_qtver = 1; then
    kde_qt_verstring="QT_VERSION >= 142 && QT_VERSION < 200"
   fi
else
   kde_qt_verstring="$3"
fi

if test $kde_qtver = 4; then
  kde_qt_dirs="$QTDIR /usr/lib/qt4 /usr/lib/qt4 /usr/lib/qt /usr/share/qt4"
fi
if test $kde_qtver = 3; then
  kde_qt_dirs="$QTDIR /usr/lib64/qt-3.3 /usr/lib/qt-3.3 /usr/lib/qt3 /usr/lib/qt /usr/share/qt3"
fi
if test $kde_qtver = 2; then
   kde_qt_dirs="$QTDIR /usr/lib/qt2 /usr/lib/qt"
fi
if test $kde_qtver = 1; then
   kde_qt_dirs="$QTDIR /usr/lib/qt"
fi
])

AC_DEFUN([KDE_CHECK_QT_DIRECT],
[
AC_REQUIRE([KDE_USE_QT])
AC_MSG_CHECKING([if Qt compiles without flags])
AC_CACHE_VAL(kde_cv_qt_direct,
[
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
ac_LD_LIBRARY_PATH_safe=$LD_LIBRARY_PATH
ac_LIBRARY_PATH="$LIBRARY_PATH"
ac_cxxflags_safe="$CXXFLAGS"
ac_ldflags_safe="$LDFLAGS"
ac_libs_safe="$LIBS"

CXXFLAGS="$CXXFLAGS -I$qt_includes"
LDFLAGS="$LDFLAGS $X_LDFLAGS"
if test "x$kde_use_qt_emb" != "xyes" && test "x$kde_use_qt_mac" != "xyes"; then
LIBS="$LIBQT -lXext -lX11 $LIBSOCKET"
else
LIBS="$LIBQT $LIBSOCKET"
fi
LD_LIBRARY_PATH=
export LD_LIBRARY_PATH
LIBRARY_PATH=
export LIBRARY_PATH

KDE_PRINT_QT_PROGRAM

if AC_TRY_EVAL(ac_link) && test -s conftest; then
  kde_cv_qt_direct="yes"
else
  kde_cv_qt_direct="no"
  echo "configure: failed program was:" >&AC_FD_CC
  cat conftest.$ac_ext >&AC_FD_CC
fi

rm -f conftest*
CXXFLAGS="$ac_cxxflags_safe"
LDFLAGS="$ac_ldflags_safe"
LIBS="$ac_libs_safe"

LD_LIBRARY_PATH="$ac_LD_LIBRARY_PATH_safe"
export LD_LIBRARY_PATH
LIBRARY_PATH="$ac_LIBRARY_PATH"
export LIBRARY_PATH
AC_LANG_RESTORE
])

if test "$kde_cv_qt_direct" = "yes"; then
  AC_MSG_RESULT(yes)
  $1
else
  AC_MSG_RESULT(no)
  $2
fi
])

dnl ------------------------------------------------------------------------
dnl Try to find the Qt headers and libraries.
dnl $(QT_LDFLAGS) will be -Lqtliblocation (if needed)
dnl and $(QT_INCLUDES) will be -Iqthdrlocation (if needed)
dnl ------------------------------------------------------------------------
dnl
AC_DEFUN([AC_PATH_QT_1_3],
[
AC_REQUIRE([K_PATH_X])
AC_REQUIRE([KDE_USE_QT])
AC_REQUIRE([KDE_CHECK_LIB64])

dnl ------------------------------------------------------------------------
dnl Add configure flag to enable linking to MT version of Qt library.
dnl ------------------------------------------------------------------------

AC_ARG_ENABLE(
  mt,
  AC_HELP_STRING([--disable-mt],[link to non-threaded Qt (deprecated)]),
  kde_use_qt_mt=$enableval,
  [
    if test $kde_qtver = 3; then
      kde_use_qt_mt=yes
    else
      kde_use_qt_mt=no
    fi
  ]
)

USING_QT_MT=""

dnl ------------------------------------------------------------------------
dnl If we not get --disable-qt-mt then adjust some vars for the host.
dnl ------------------------------------------------------------------------

KDE_MT_LDFLAGS=
KDE_MT_LIBS=
if test "x$kde_use_qt_mt" = "xyes"; then
  KDE_CHECK_THREADING
  if test "x$kde_use_threading" = "xyes"; then
    CPPFLAGS="$USE_THREADS -DQT_THREAD_SUPPORT $CPPFLAGS"
    KDE_MT_LDFLAGS="$USE_THREADS"
    KDE_MT_LIBS="$LIBPTHREAD"
  else
    kde_use_qt_mt=no
  fi
fi
AC_SUBST(KDE_MT_LDFLAGS)
AC_SUBST(KDE_MT_LIBS)

kde_qt_was_given=yes

dnl ------------------------------------------------------------------------
dnl If we haven't been told how to link to Qt, we work it out for ourselves.
dnl ------------------------------------------------------------------------
if test -z "$LIBQT_GLOB"; then
  if test "x$kde_use_qt_emb" = "xyes"; then
    LIBQT_GLOB="libqte.*"
  else
    LIBQT_GLOB="libqt.*"
  fi
fi

if test -z "$LIBQT"; then
dnl ------------------------------------------------------------
dnl If we got --enable-embedded then adjust the Qt library name.
dnl ------------------------------------------------------------
  if test "x$kde_use_qt_emb" = "xyes"; then
    qtlib="qte"
  else
    qtlib="qt"
  fi

  kde_int_qt="-l$qtlib"
else
  kde_int_qt="$LIBQT"
  kde_lib_qt_set=yes
fi

if test -z "$LIBQPE"; then
dnl ------------------------------------------------------------
dnl If we got --enable-palmtop then add -lqpe to the link line
dnl ------------------------------------------------------------
  if test "x$kde_use_qt_emb" = "xyes"; then
    if test "x$kde_use_qt_emb_palm" = "xyes"; then
      LIB_QPE="-lqpe"
    else
      LIB_QPE=""
    fi
  else
    LIB_QPE=""
  fi
fi

dnl ------------------------------------------------------------------------
dnl If we got --enable-qt-mt then adjust the Qt library name for the host.
dnl ------------------------------------------------------------------------

if test "x$kde_use_qt_mt" = "xyes"; then
  LIBQT="-l$qtlib-mt"
  kde_int_qt="-l$qtlib-mt"
  LIBQT_GLOB="lib$qtlib-mt.*"
  USING_QT_MT="using -mt"
else
  LIBQT="-l$qtlib"
fi

if test $kde_qtver != 1; then

  AC_REQUIRE([AC_FIND_PNG])
  AC_REQUIRE([AC_FIND_JPEG])
  LIBQT="$LIBQT $LIBPNG $LIBJPEG"
fi

if test $kde_qtver = 3; then
  AC_REQUIRE([KDE_CHECK_LIBDL])
  LIBQT="$LIBQT $LIBDL"
fi

AC_MSG_CHECKING([for Qt])

if test "x$kde_use_qt_emb" != "xyes" && test "x$kde_use_qt_mac" != "xyes"; then
LIBQT="$LIBQT $X_PRE_LIBS -lXext -lX11 $LIBSM $LIBSOCKET"
fi
ac_qt_includes=NO ac_qt_libraries=NO ac_qt_bindir=NO
qt_libraries=""
qt_includes=""
AC_ARG_WITH(qt-dir,
    AC_HELP_STRING([--with-qt-dir=DIR],[where the root of Qt is installed ]),
    [  ac_qt_includes="$withval"/include
       ac_qt_libraries="$withval"/lib${kdelibsuff}
       ac_qt_bindir="$withval"/bin
    ])

AC_ARG_WITH(qt-includes,
    AC_HELP_STRING([--with-qt-includes=DIR],[where the Qt includes are. ]),
    [
       ac_qt_includes="$withval"
    ])

kde_qt_libs_given=no

AC_ARG_WITH(qt-libraries,
    AC_HELP_STRING([--with-qt-libraries=DIR],[where the Qt library is installed.]),
    [  ac_qt_libraries="$withval"
       kde_qt_libs_given=yes
    ])

AC_CACHE_VAL(ac_cv_have_qt,
[#try to guess Qt locations

qt_incdirs=""
for dir in $kde_qt_dirs; do
   qt_incdirs="$qt_incdirs $dir/include $dir"
done
qt_incdirs="$QTINC $qt_incdirs /usr/local/qt/include /usr/include/qt /usr/include /usr/X11R6/include/X11/qt /usr/X11R6/include/qt /usr/X11R6/include/qt2 /usr/include/qt3 $x_includes "
if test ! "$ac_qt_includes" = "NO"; then
   qt_incdirs="$ac_qt_includes $qt_incdirs"
fi

if test "$kde_qtver" != "1"; then
  kde_qt_header=qstyle.h
else
  kde_qt_header=qglobal.h
fi

AC_FIND_FILE($kde_qt_header, $qt_incdirs, qt_incdir)
ac_qt_includes="$qt_incdir"

qt_libdirs=""
for dir in $kde_qt_dirs; do
   qt_libdirs="$qt_libdirs $dir/lib${kdelibsuff} $dir"
done
qt_libdirs="$QTLIB $qt_libdirs /usr/X11R6/lib /usr/lib /usr/local/qt/lib $x_libraries"
if test ! "$ac_qt_libraries" = "NO"; then
  qt_libdir=$ac_qt_libraries
else
  qt_libdirs="$ac_qt_libraries $qt_libdirs"
  # if the Qt was given, the chance is too big that libqt.* doesn't exist
  qt_libdir=NONE
  for dir in $qt_libdirs; do
    try="ls -1 $dir/${LIBQT_GLOB}"
    if test -n "`$try 2> /dev/null`"; then qt_libdir=$dir; break; else echo "tried $dir" >&AC_FD_CC ; fi
  done
fi
for a in $qt_libdir/lib`echo ${kde_int_qt} | sed 's,^-l,,'`_incremental.*; do
  if test -e "$a"; then
    LIBQT="$LIBQT ${kde_int_qt}_incremental"
    break
  fi
done

ac_qt_libraries="$qt_libdir"

AC_LANG_SAVE
AC_LANG_CPLUSPLUS

ac_cxxflags_safe="$CXXFLAGS"
ac_ldflags_safe="$LDFLAGS"
ac_libs_safe="$LIBS"

CXXFLAGS="$CXXFLAGS -I$qt_incdir $all_includes"
LDFLAGS="$LDFLAGS -L$qt_libdir $all_libraries $USER_LDFLAGS $KDE_MT_LDFLAGS"
LIBS="$LIBS $LIBQT $KDE_MT_LIBS"

KDE_PRINT_QT_PROGRAM

if AC_TRY_EVAL(ac_link) && test -s conftest; then
  rm -f conftest*
else
  echo "configure: failed program was:" >&AC_FD_CC
  cat conftest.$ac_ext >&AC_FD_CC
  ac_qt_libraries="NO"
fi
rm -f conftest*
CXXFLAGS="$ac_cxxflags_safe"
LDFLAGS="$ac_ldflags_safe"
LIBS="$ac_libs_safe"

AC_LANG_RESTORE
if test "$ac_qt_includes" = NO || test "$ac_qt_libraries" = NO; then
  ac_cv_have_qt="have_qt=no"
  ac_qt_notfound=""
  missing_qt_mt=""
  if test "$ac_qt_includes" = NO; then
    if test "$ac_qt_libraries" = NO; then
      ac_qt_notfound="(headers and libraries)";
    else
      ac_qt_notfound="(headers)";
    fi
  else
    if test "x$kde_use_qt_mt" = "xyes"; then
       missing_qt_mt="
Make sure that you have compiled Qt with thread support!"
       ac_qt_notfound="(library $qtlib-mt)";
    else
       ac_qt_notfound="(library $qtlib)";
    fi
  fi

  AC_MSG_NOTICE([Qt $ac_qt_notfound not found.])
else
  have_qt="yes"
fi
])

eval "$ac_cv_have_qt"

if test "$have_qt" != yes; then
  AC_MSG_RESULT([$have_qt]);
else
  ac_cv_have_qt="have_qt=yes \
    ac_qt_includes=$ac_qt_includes ac_qt_libraries=$ac_qt_libraries"
  AC_MSG_RESULT([libraries $ac_qt_libraries, headers $ac_qt_includes $USING_QT_MT])

  qt_libraries="$ac_qt_libraries"
  qt_includes="$ac_qt_includes"
fi

if test ! "$kde_qt_libs_given" = "yes" && test ! "$kde_qtver" = 3; then
     KDE_CHECK_QT_DIRECT(qt_libraries= ,[])
fi

AC_SUBST(qt_libraries)
AC_SUBST(qt_includes)

if test "$qt_includes" = "$x_includes" || test -z "$qt_includes"; then
 QT_INCLUDES=""
else
 QT_INCLUDES="-I$qt_includes"
 all_includes="$QT_INCLUDES $all_includes"
fi

if test "$qt_libraries" = "$x_libraries" || test -z "$qt_libraries"; then
 QT_LDFLAGS=""
else
 QT_LDFLAGS="-L$qt_libraries"
 all_libraries="$QT_LDFLAGS $all_libraries"
fi
test -z "$KDE_MT_LDFLAGS" || all_libraries="$all_libraries $KDE_MT_LDFLAGS"

AC_SUBST(QT_INCLUDES)
AC_SUBST(QT_LDFLAGS)
AC_PATH_QT_MOC_UIC

KDE_CHECK_QT_JPEG

if test "x$kde_use_qt_emb" != "xyes" && test "x$kde_use_qt_mac" != "xyes"; then
LIB_QT="$kde_int_qt $LIBJPEG_QT "'$(LIBZ) $(LIBPNG) -lXext $(LIB_X11) $(LIBSM)'
else
LIB_QT="$kde_int_qt $LIBJPEG_QT "'$(LIBZ) $(LIBPNG)'
fi
test -z "$KDE_MT_LIBS" || LIB_QT="$LIB_QT $KDE_MT_LIBS"
for a in $qt_libdir/lib`echo ${kde_int_qt} | sed 's,^-l,,'`_incremental.*; do
  if test -e "$a"; then
     LIB_QT="$LIB_QT ${kde_int_qt}_incremental"
     break
  fi
done

AC_SUBST(LIB_QT)
AC_SUBST(LIB_QPE)

AC_SUBST(kde_qtver)
])

AC_DEFUN([AC_PATH_QT],
[
AC_PATH_QT_1_3
])

AC_DEFUN([KDE_CHECK_UIC_PLUGINS],
[
AC_REQUIRE([AC_PATH_QT_MOC_UIC])

if test x$ac_uic_supports_libpath = xyes; then

AC_MSG_CHECKING([if UIC has KDE plugins available])
AC_CACHE_VAL(kde_cv_uic_plugins,
[
cat > actest.ui << EOF
<!DOCTYPE UI><UI version="3.0" stdsetdef="1">
<class>NewConnectionDialog</class>
<widget class="QDialog">
   <widget class="KLineEdit">
        <property name="name">
           <cstring>testInput</cstring>
        </property>
   </widget>
</widget>
</UI>
EOF
       


kde_cv_uic_plugins=no
kde_line="$UIC_PATH -L $kde_widgetdir"
if test x$ac_uic_supports_nounload = xyes; then
   kde_line="$kde_line -nounload"
fi
kde_line="$kde_line -impl actest.h actest.ui > actest.cpp"
if AC_TRY_EVAL(kde_line); then
	# if you're trying to debug this check and think it's incorrect,
	# better check your installation. The check _is_ correct - your
	# installation is not.
	if test -f actest.cpp && grep klineedit actest.cpp > /dev/null; then
		kde_cv_uic_plugins=yes
	fi
fi
rm -f actest.ui actest.cpp
])

AC_MSG_RESULT([$kde_cv_uic_plugins])
if test "$kde_cv_uic_plugins" != yes; then
	AC_MSG_NOTICE([you need to install kdelibs first.])
fi
fi
])

AC_DEFUN([KDE_EXPAND_MAKEVAR], [

if test x"$klash" = x"yes"; then
savex=$exec_prefix
test "x$exec_prefix" = xNONE && exec_prefix=$prefix
tmp=$$2
while $1=`eval echo "$tmp"`; test "x$$1" != "x$tmp"; do tmp=$$1; done
exec_prefix=$savex
fi
])

dnl ------------------------------------------------------------------------
dnl Now, the same with KDE
dnl $(KDE_LDFLAGS) will be the kdeliblocation (if needed)
dnl and $(kde_includes) will be the kdehdrlocation (if needed)
dnl ------------------------------------------------------------------------
dnl
AC_DEFUN([AC_BASE_PATH_KDE],
[
AC_REQUIRE([KDE_CHECK_STL])
AC_REQUIRE([AC_PATH_QT])dnl
AC_REQUIRE([KDE_CHECK_LIB64])

AC_CHECK_RPATH
AC_MSG_CHECKING([for KDE])

if test "${prefix}" != NONE; then
  kde_includes=${includedir}
  KDE_EXPAND_MAKEVAR(ac_kde_includes, includedir)

  kde_libraries=${libdir}
  KDE_EXPAND_MAKEVAR(ac_kde_libraries, libdir)

else
  ac_kde_includes=
  ac_kde_libraries=
  kde_libraries=""
  kde_includes=""
fi

AC_CACHE_VAL(ac_cv_have_kde,
[#try to guess kde locations

if test "$kde_qtver" = 1; then
  kde_check_header="ksock.h"
  kde_check_lib="libkdecore.la"
else
  kde_check_header="ksharedptr.h"
  kde_check_lib="libkio.la"
fi

if test -z "$1"; then

kde_incdirs="$kde_libs_prefix/include /usr/lib/kde/include /usr/local/kde/include /usr/local/include /usr/kde/include /usr/include/kde /usr/include /opt/kde3/include /opt/kde/include $x_includes $qt_includes"
test -n "$KDEDIR" && kde_incdirs="$KDEDIR/include $KDEDIR/include/kde $KDEDIR $kde_incdirs"
kde_incdirs="$ac_kde_includes $kde_incdirs"
AC_FIND_FILE($kde_check_header, $kde_incdirs, kde_incdir)
ac_kde_includes="$kde_incdir"

if test -n "$ac_kde_includes" && test ! -r "$ac_kde_includes/$kde_check_header"; then
  AC_MSG_NOTICE([no KDE headers installed.])
fi

kde_libdirs="$kde_libs_prefix/lib${kdelibsuff} /usr/lib/kde/lib${kdelibsuff} /usr/local/kde/lib${kdelibsuff} /usr/kde/lib${kdelibsuff} /usr/lib${kdelibsuff}/kde /usr/lib${kdelibsuff}/kde3 /usr/lib${kdelibsuff} /usr/X11R6/lib${kdelibsuff} /usr/local/lib${kdelibsuff} /opt/kde3/lib${kdelibsuff} /opt/kde/lib${kdelibsuff} /usr/X11R6/kde/lib${kdelibsuff}"
test -n "$KDEDIR" && kde_libdirs="$KDEDIR/lib${kdelibsuff} $KDEDIR $kde_libdirs"
kde_libdirs="$ac_kde_libraries $libdir $kde_libdirs"
AC_FIND_FILE($kde_check_lib, $kde_libdirs, kde_libdir)
ac_kde_libraries="$kde_libdir"

kde_widgetdir=NO
dnl this might be somewhere else
AC_FIND_FILE("kde3/plugins/designer/kdewidgets.la", $kde_libdirs, kde_widgetdir)

if test -n "$ac_kde_libraries" && test ! -r "$ac_kde_libraries/$kde_check_lib"; then
AC_MSG_NOTICE([no KDE libraries installed.])
fi

if test -n "$kde_widgetdir" && test ! -r "$kde_widgetdir/kde3/plugins/designer/kdewidgets.la"; then
AC_MSG_NOTICE([I can't find the KDE designer plugins.])
fi

if test -n "$kde_widgetdir"; then
    kde_widgetdir="$kde_widgetdir/kde3/plugins/designer"
fi


if test "$ac_kde_includes" = NO || test "$ac_kde_libraries" = NO || test "$kde_widgetdir" = NO; then
  ac_cv_have_kde="have_kde=no"
else
  ac_cv_have_kde="have_kde=yes \
    ac_kde_includes=$ac_kde_includes ac_kde_libraries=$ac_kde_libraries"
fi

else dnl test -z $1, e.g. from kdelibs

  ac_cv_have_kde="have_kde=no"

fi
])dnl

eval "$ac_cv_have_kde"

if test "$have_kde" != "yes"; then
 if test "${prefix}" = NONE; then
  ac_kde_prefix="$ac_default_prefix"
 else
  ac_kde_prefix="$prefix"
 fi
 if test "$exec_prefix" = NONE; then
  ac_kde_exec_prefix="$ac_kde_prefix"
  AC_MSG_RESULT([will be installed in $ac_kde_prefix])
 else
  ac_kde_exec_prefix="$exec_prefix"
  AC_MSG_RESULT([will be installed in $ac_kde_prefix and $ac_kde_exec_prefix])
 fi

 kde_libraries="${libdir}"
 kde_includes="${includedir}"

else
  ac_cv_have_kde="have_kde=yes \
    ac_kde_includes=$ac_kde_includes ac_kde_libraries=$ac_kde_libraries"
  AC_MSG_RESULT([libraries $ac_kde_libraries, headers $ac_kde_includes])

  kde_libraries="$ac_kde_libraries"
  kde_includes="$ac_kde_includes"
fi
AC_SUBST(kde_libraries)
AC_SUBST(kde_includes)

if test "$kde_includes" = "$x_includes" || test "$kde_includes" = "$qt_includes"  || test "$kde_includes" = "/usr/include"; then
 KDE_INCLUDES=""
else
 KDE_INCLUDES="-I$kde_includes"
 all_includes="$KDE_INCLUDES $all_includes"
fi

KDE_DEFAULT_CXXFLAGS="-DQT_CLEAN_NAMESPACE -DQT_NO_ASCII_CAST -DQT_NO_STL -DQT_NO_COMPAT -DQT_NO_TRANSLATION"

dnl We don't want the default system path as it screws up specify directories.
if test x"${kde_libraries}" = x"/usr/lib" -o x"${kde_libraries}" = x"/usr/lib64"; then
  KDE_LDFLAGS=""
else
  KDE_LDFLAGS="-L${kde_libraries}"
fi

if test ! "$kde_libraries" = "$x_libraries" && test ! "$kde_libraries" = "$qt_libraries" ; then 
 all_libraries="$KDE_LDFLAGS $all_libraries"
fi

AC_SUBST(KDE_LDFLAGS)
AC_SUBST(KDE_INCLUDES)

AC_REQUIRE([KDE_CHECK_EXTRA_LIBS])

all_libraries="$all_libraries $USER_LDFLAGS"
all_includes="$all_includes $USER_INCLUDES"
AC_SUBST(all_includes)
AC_SUBST(all_libraries)

if test -z "$1"; then
KDE_CHECK_UIC_PLUGINS
fi

ac_kde_libraries="$kde_libdir"

AC_SUBST(AUTODIRS)


])

AC_DEFUN([KDE_CHECK_EXTRA_LIBS],
[
AC_MSG_CHECKING(for extra includes)
AC_ARG_WITH(extra-includes,AC_HELP_STRING([--with-extra-includes=DIR],[adds non standard include paths]),
  kde_use_extra_includes="$withval",
  kde_use_extra_includes=NONE
)
if test x"$klash" = x"yes"; then
kde_extra_includes=
if test -n "$kde_use_extra_includes" && \
   test "$kde_use_extra_includes" != "NONE"; then

   ac_save_ifs=$IFS
   IFS=':'
   for dir in $kde_use_extra_includes; do
     kde_extra_includes="$kde_extra_includes $dir"
     USER_INCLUDES="$USER_INCLUDES -I$dir"
   done
   IFS=$ac_save_ifs
   kde_use_extra_includes="added"
else
   kde_use_extra_includes="no"
fi
AC_SUBST(USER_INCLUDES)

AC_MSG_RESULT($kde_use_extra_includes)

kde_extra_libs=
AC_MSG_CHECKING(for extra libs)
AC_ARG_WITH(extra-libs,AC_HELP_STRING([--with-extra-libs=DIR],[adds non standard library paths]),
  kde_use_extra_libs=$withval,
  kde_use_extra_libs=NONE
)
if test -n "$kde_use_extra_libs" && \
   test "$kde_use_extra_libs" != "NONE"; then

   ac_save_ifs=$IFS
   IFS=':'
   for dir in $kde_use_extra_libs; do
     kde_extra_libs="$kde_extra_libs $dir"
     KDE_EXTRA_RPATH="$KDE_EXTRA_RPATH -R $dir"
     USER_LDFLAGS="$USER_LDFLAGS -L$dir"
   done
   IFS=$ac_save_ifs
   kde_use_extra_libs="added"
else
   kde_use_extra_libs="no"
fi
fi
AC_SUBST(USER_LDFLAGS)

AC_MSG_RESULT($kde_use_extra_libs)

])

AC_DEFUN([KDE_1_CHECK_PATH_HEADERS],
[
    AC_MSG_CHECKING([for KDE headers installed])
    AC_LANG_SAVE
    AC_LANG_CPLUSPLUS
cat > conftest.$ac_ext <<EOF
#ifdef STDC_HEADERS
# include <stdlib.h>
#endif
#include <stdio.h>
#include "confdefs.h"
#include <kapp.h>

int main() {
    printf("kde_htmldir=\\"%s\\"\n", KApplication::kde_htmldir().data());
    printf("kde_appsdir=\\"%s\\"\n", KApplication::kde_appsdir().data());
    printf("kde_icondir=\\"%s\\"\n", KApplication::kde_icondir().data());
    printf("kde_sounddir=\\"%s\\"\n", KApplication::kde_sounddir().data());
    printf("kde_datadir=\\"%s\\"\n", KApplication::kde_datadir().data());
    printf("kde_locale=\\"%s\\"\n", KApplication::kde_localedir().data());
    printf("kde_cgidir=\\"%s\\"\n", KApplication::kde_cgidir().data());
    printf("kde_confdir=\\"%s\\"\n", KApplication::kde_configdir().data());
    printf("kde_mimedir=\\"%s\\"\n", KApplication::kde_mimedir().data());
    printf("kde_toolbardir=\\"%s\\"\n", KApplication::kde_toolbardir().data());
    printf("kde_wallpaperdir=\\"%s\\"\n",
	KApplication::kde_wallpaperdir().data());
    printf("kde_bindir=\\"%s\\"\n", KApplication::kde_bindir().data());
    printf("kde_partsdir=\\"%s\\"\n", KApplication::kde_partsdir().data());
    printf("kde_servicesdir=\\"/tmp/dummy\\"\n");
    printf("kde_servicetypesdir=\\"/tmp/dummy\\"\n");
    printf("kde_moduledir=\\"/tmp/dummy\\"\n");
    printf("kde_styledir=\\"/tmp/dummy\\"\n");
    printf("kde_widgetdir=\\"/tmp/dummy\\"\n");
    printf("xdg_appsdir=\\"/tmp/dummy\\"\n");
    printf("xdg_menudir=\\"/tmp/dummy\\"\n");
    printf("xdg_directorydir=\\"/tmp/dummy\\"\n");
    printf("kde_kcfgdir=\\"/tmp/dummy\\"\n");
    return 0;
    }
EOF

 ac_save_CPPFLAGS=$CPPFLAGS
 CPPFLAGS="$all_includes $CPPFLAGS"
 if AC_TRY_EVAL(ac_compile); then
   AC_MSG_RESULT(yes)
 else
   AC_MSG_ERROR([your system is not able to compile a small KDE application!
Check, if you installed the KDE header files correctly.
For more details about this problem, look at the end of config.log.])
  fi
  CPPFLAGS=$ac_save_CPPFLAGS

  AC_LANG_RESTORE
])

AC_DEFUN([KDE_CHECK_KDEQTADDON],
[
AC_MSG_CHECKING(for kde-qt-addon)
AC_CACHE_VAL(kde_cv_have_kdeqtaddon,
[
 kde_ldflags_safe="$LDFLAGS"
 kde_libs_safe="$LIBS"
 kde_cxxflags_safe="$CXXFLAGS"

 LIBS="-lkde-qt-addon $LIBQT $LIBS"
 CXXFLAGS="$CXXFLAGS -I$prefix/include -I$prefix/include/kde $all_includes"
 LDFLAGS="$LDFLAGS $all_libraries $USER_LDFLAGS"

 AC_TRY_LINK([
   #include <qdom.h>
 ],
 [
   QDomDocument doc;
 ],
  kde_cv_have_kdeqtaddon=yes,
  kde_cv_have_kdeqtaddon=no
 )

 LDFLAGS=$kde_ldflags_safe
 LIBS=$kde_libs_safe
 CXXFLAGS=$kde_cxxflags_safe
])

AC_MSG_RESULT($kde_cv_have_kdeqtaddon)

if test "$kde_cv_have_kdeqtaddon" = "no"; then
  AC_MSG_ERROR([Can't find libkde-qt-addon. You need to install it first.
It is a separate package (and CVS module) named kde-qt-addon.])
fi
])

AC_DEFUN([KDE_CREATE_LIBS_ALIASES],
[
   AC_REQUIRE([KDE_MISC_TESTS])
   AC_REQUIRE([KDE_CHECK_LIBDL])
   AC_REQUIRE([K_PATH_X])

if test $kde_qtver = 3; then
   AC_SUBST(LIB_KDECORE, "-lkdecore")
   AC_SUBST(LIB_KDEUI, "-lkdeui")
   AC_SUBST(LIB_KIO, "-lkio")
   AC_SUBST(LIB_KJS, "-lkjs")
   AC_SUBST(LIB_SMB, "-lsmb")
   AC_SUBST(LIB_KAB, "-lkab")
   AC_SUBST(LIB_KABC, "-lkabc")
   AC_SUBST(LIB_KHTML, "-lkhtml")
   AC_SUBST(LIB_KSPELL, "-lkspell")
   AC_SUBST(LIB_KPARTS, "-lkparts")
   AC_SUBST(LIB_KDEPRINT, "-lkdeprint")
   AC_SUBST(LIB_KUTILS, "-lkutils")
   AC_SUBST(LIB_KDEPIM, "-lkdepim")
   AC_SUBST(LIB_KIMPROXY, "-lkimproxy")
   AC_SUBST(LIB_KNEWSTUFF, "-lknewstuff")
   AC_SUBST(LIB_KDNSSD, "-lkdnssd")
   AC_SUBST(LIB_KUNITTEST, "-lkunittest")
# these are for backward compatibility
   AC_SUBST(LIB_KSYCOCA, "-lkio")
   AC_SUBST(LIB_KFILE, "-lkio")
elif test $kde_qtver = 2; then
   AC_SUBST(LIB_KDECORE, "-lkdecore")
   AC_SUBST(LIB_KDEUI, "-lkdeui")
   AC_SUBST(LIB_KIO, "-lkio")
   AC_SUBST(LIB_KSYCOCA, "-lksycoca")
   AC_SUBST(LIB_SMB, "-lsmb")
   AC_SUBST(LIB_KFILE, "-lkfile")
   AC_SUBST(LIB_KAB, "-lkab")
   AC_SUBST(LIB_KHTML, "-lkhtml")
   AC_SUBST(LIB_KSPELL, "-lkspell")
   AC_SUBST(LIB_KPARTS, "-lkparts")
   AC_SUBST(LIB_KDEPRINT, "-lkdeprint")
else
   AC_SUBST(LIB_KDECORE, "-lkdecore -lXext $(LIB_QT)")
   AC_SUBST(LIB_KDEUI, "-lkdeui $(LIB_KDECORE)")
   AC_SUBST(LIB_KFM, "-lkfm $(LIB_KDECORE)")
   AC_SUBST(LIB_KFILE, "-lkfile $(LIB_KFM) $(LIB_KDEUI)")
   AC_SUBST(LIB_KAB, "-lkab $(LIB_KIMGIO) $(LIB_KDECORE)")
fi
])

AC_DEFUN([AC_PATH_KDE],
[
  AC_BASE_PATH_KDE
  AC_ARG_ENABLE(path-check,AC_HELP_STRING([--disable-path-check],[don't try to find out, where to install]),
  [
  if test "$enableval" = "no";
    then ac_use_path_checking="default"
    else ac_use_path_checking=""
  fi
  ],
  [
  if test "$kde_qtver" = 1;
    then ac_use_path_checking=""
    else ac_use_path_checking="default"
  fi
  ]
  )

  AC_CREATE_KFSSTND($ac_use_path_checking)

  AC_SUBST_KFSSTND
  KDE_CREATE_LIBS_ALIASES
])

dnl KDE_CHECK_FUNC_EXT(<func>, [headers], [sample-use], [C prototype], [autoheader define], [call if found])
AC_DEFUN([KDE_CHECK_FUNC_EXT],
[
AC_MSG_CHECKING(for $1)
AC_CACHE_VAL(kde_cv_func_$1,
[
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
save_CXXFLAGS="$CXXFLAGS"
kde_safe_LIBS="$LIBS"
LIBS="$LIBS $X_EXTRA_LIBS"
if test "$GXX" = "yes"; then
CXXFLAGS="$CXXFLAGS -pedantic-errors"
fi
AC_TRY_COMPILE([
$2
],
[
$3
],
kde_cv_func_$1=yes,
kde_cv_func_$1=no)
CXXFLAGS="$save_CXXFLAGS"
LIBS="$kde_safe_LIBS"
AC_LANG_RESTORE
])

AC_MSG_RESULT($kde_cv_func_$1)

AC_MSG_CHECKING([if $1 needs custom prototype])
AC_CACHE_VAL(kde_cv_proto_$1,
[
if test "x$kde_cv_func_$1" = xyes; then
  kde_cv_proto_$1=no
else
  case "$1" in
	setenv|unsetenv|usleep|random|srandom|seteuid|mkstemps|mkstemp|revoke|vsnprintf|strlcpy|strlcat)
		kde_cv_proto_$1="yes - in libkdefakes"
		;;
	*)
		kde_cv_proto_$1=unknown
		;;
  esac
fi

if test "x$kde_cv_proto_$1" = xunknown; then

AC_LANG_SAVE
AC_LANG_CPLUSPLUS
  kde_safe_libs=$LIBS
  LIBS="$LIBS $X_EXTRA_LIBS"
  AC_TRY_LINK([
$2

extern "C" $4;
],
[
$3
],
[ kde_cv_func_$1=yes
  kde_cv_proto_$1=yes ],
  [kde_cv_proto_$1="$1 unavailable"]
)
LIBS=$kde_safe_libs
AC_LANG_RESTORE
fi
])
AC_MSG_RESULT($kde_cv_proto_$1)

if test "x$kde_cv_func_$1" = xyes; then
  AC_DEFINE(HAVE_$5, 1, [Define if you have $1])
  $6
fi
if test "x$kde_cv_proto_$1" = xno; then
  AC_DEFINE(HAVE_$5_PROTO, 1,
  [Define if you have the $1 prototype])
fi

AH_VERBATIM([_HAVE_$5_PROTO],
[
#if !defined(HAVE_$5_PROTO)
#ifdef __cplusplus
extern "C" {
#endif
$4;
#ifdef __cplusplus
}
#endif
#endif
])
])

AC_DEFUN([AC_CHECK_RES_INIT],
[
  AC_MSG_CHECKING([if res_init needs -lresolv])
  kde_libs_safe="$LIBS"
  LIBS="$LIBS $X_EXTRA_LIBS -lresolv"
  AC_TRY_LINK(
    [
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
    ],
    [
      res_init(); 
    ],
    [
      LIBRESOLV="-lresolv"
      AC_MSG_RESULT(yes)
      AC_DEFINE(HAVE_RES_INIT, 1, [Define if you have the res_init function])
    ],
    [ AC_MSG_RESULT(no) ]
  )
  LIBS=$kde_libs_safe
  AC_SUBST(LIBRESOLV)

  KDE_CHECK_FUNC_EXT(res_init,
    [
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
    ],
    [res_init()],
    [int res_init(void)],
    [RES_INIT])
])

AC_DEFUN([AC_CHECK_STRLCPY],
[
	KDE_CHECK_FUNC_EXT(strlcpy, [
#include <string.h>
],
[ char buf[20];
  strlcpy(buf, "KDE function test", sizeof(buf));
],
 	[unsigned long strlcpy(char*, const char*, unsigned long)],
	[STRLCPY])
])

AC_DEFUN([AC_CHECK_STRLCAT],
[
	KDE_CHECK_FUNC_EXT(strlcat, [
#include <string.h>
],
[ char buf[20];
  buf[0]='\0';
  strlcat(buf, "KDE function test", sizeof(buf));
],
 	[unsigned long strlcat(char*, const char*, unsigned long)],
	[STRLCAT])
])

AC_DEFUN([KDE_FIND_JPEG_HELPER],
[
AC_MSG_CHECKING([for libjpeg$2])
AC_CACHE_VAL(ac_cv_lib_jpeg_$1,
[
ac_save_LIBS="$LIBS"
LIBS="$all_libraries $USER_LDFLAGS -ljpeg$2 -lm"
ac_save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS $all_includes $USER_INCLUDES"
AC_TRY_LINK(
[/* Override any gcc2 internal prototype to avoid an error.  */
struct jpeg_decompress_struct;
typedef struct jpeg_decompress_struct * j_decompress_ptr;
typedef int size_t;
#ifdef __cplusplus
extern "C" {
#endif
    void jpeg_CreateDecompress(j_decompress_ptr cinfo,
                                    int version, size_t structsize);
#ifdef __cplusplus
}
#endif
/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
],
            [jpeg_CreateDecompress(0L, 0, 0);],
            eval "ac_cv_lib_jpeg_$1=-ljpeg$2",
            eval "ac_cv_lib_jpeg_$1=no")
LIBS="$ac_save_LIBS"
CFLAGS="$ac_save_CFLAGS"
])

if eval "test ! \"`echo $ac_cv_lib_jpeg_$1`\" = no"; then
  LIBJPEG="$ac_cv_lib_jpeg_$1"
  AC_MSG_RESULT($ac_cv_lib_jpeg_$1)
else
  AC_MSG_RESULT(no)
  $3
fi

])

AC_DEFUN([AC_FIND_JPEG],
[
dnl first look for libraries
KDE_FIND_JPEG_HELPER(6b, 6b,
   KDE_FIND_JPEG_HELPER(normal, [],
    [
       LIBJPEG=
    ]
   )
)

dnl then search the headers (can't use simply AC_TRY_xxx, as jpeglib.h
dnl requires system dependent includes loaded before it)
jpeg_incdirs="$includedir /usr/include /usr/local/include $kde_extra_includes"
AC_FIND_FILE(jpeglib.h, $jpeg_incdirs, jpeg_incdir)
test "x$jpeg_incdir" = xNO && jpeg_incdir=

dnl if headers _and_ libraries are missing, this is no error, and we
dnl continue with a warning (the user will get no jpeg support in khtml)
dnl if only one is missing, it means a configuration error, but we still
dnl only warn
if test -n "$jpeg_incdir" && test -n "$LIBJPEG" ; then
  AC_DEFINE_UNQUOTED(HAVE_LIBJPEG, 1, [Define if you have libjpeg])
else
dnl   if test -n "$jpeg_incdir" || test -n "$LIBJPEG" ; then
dnl     AC_MSG_WARN([
dnl There is an installation error in jpeg support. You seem to have only one
dnl of either the headers _or_ the libraries installed. You may need to either
dnl provide correct --with-extra-... options, or the development package of
dnl libjpeg6b. You can get a source package of libjpeg from http://www.ijg.org/
dnl Disabling JPEG support.
dnl ])
dnl   else
dnl     AC_MSG_WARN([libjpeg not found. disable JPEG support.])
dnl   fi
  jpeg_incdir=
  LIBJPEG=
fi

AC_SUBST(LIBJPEG)
AH_VERBATIM(_AC_CHECK_JPEG,
[/*
 * jpeg.h needs HAVE_BOOLEAN, when the system uses boolean in system
 * headers and I'm too lazy to write a configure test as long as only
 * unixware is related
 */
#ifdef _UNIXWARE
#define HAVE_BOOLEAN
#endif
])
])

AC_DEFUN([KDE_CHECK_QT_JPEG],
[
if test -n "$LIBJPEG"; then
AC_MSG_CHECKING([if Qt needs $LIBJPEG])
AC_CACHE_VAL(kde_cv_qt_jpeg,
[
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
ac_save_LIBS="$LIBS"
LIBS="$all_libraries $USER_LDFLAGS $LIBQT"
LIBS=`echo $LIBS | sed "s/$LIBJPEG//"`
ac_save_CXXFLAGS="$CXXFLAGS"
CXXFLAGS="$CXXFLAGS $all_includes $USER_INCLUDES"
AC_TRY_LINK(
[#include <qapplication.h>],
            [
            int argc;
            char** argv;
            QApplication app(argc, argv);],
            eval "kde_cv_qt_jpeg=no",
            eval "kde_cv_qt_jpeg=yes")
LIBS="$ac_save_LIBS"
CXXFLAGS="$ac_save_CXXFLAGS"
AC_LANG_RESTORE
fi
])

if eval "test ! \"`echo $kde_cv_qt_jpeg`\" = no"; then
  AC_MSG_RESULT(yes)
  LIBJPEG_QT='$(LIBJPEG)'
else
  AC_MSG_RESULT(no)
  LIBJPEG_QT=
fi

])

AC_DEFUN([AC_FIND_ZLIB],
[
AC_REQUIRE([KDE_CHECK_EXTRA_LIBS])

GNASH_PATH_ZLIB
LIBZ=$ZLIB_LIBS
AC_SUBST(LIBZ)
])

AC_DEFUN([AC_FIND_PNG],
[
AC_REQUIRE([KDE_CHECK_EXTRA_LIBS])
AC_REQUIRE([AC_FIND_ZLIB])
AC_MSG_CHECKING([for libpng])
AC_CACHE_VAL(ac_cv_lib_png,
[
kde_save_LIBS="$LIBS"
if test "x$kde_use_qt_emb" != "xyes" && test "x$kde_use_qt_mac" != "xyes"; then
LIBS="$LIBS $all_libraries $USER_LDFLAGS -lpng $LIBZ -lm -lX11 $LIBSOCKET"
else
LIBS="$LIBS $all_libraries $USER_LDFLAGS -lpng $LIBZ -lm"
fi
kde_save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS $all_includes $USER_INCLUDES"

AC_TRY_LINK(dnl
    [
    #include<png.h>
    ],
    [
    png_structp png_ptr = png_create_read_struct(  /* image ptr */
		PNG_LIBPNG_VER_STRING, 0, 0, 0 );
    return( png_ptr != 0 );
    ],
    eval "ac_cv_lib_png='-lpng $LIBZ -lm'",
    eval "ac_cv_lib_png=no"
)
LIBS="$kde_save_LIBS"
CFLAGS="$kde_save_CFLAGS"
])dnl
if eval "test ! \"`echo $ac_cv_lib_png`\" = no"; then
  AC_DEFINE_UNQUOTED(HAVE_LIBPNG, 1, [Define if you have libpng])
  LIBPNG="$ac_cv_lib_png"
  AC_SUBST(LIBPNG)
  AC_MSG_RESULT($ac_cv_lib_png)
else
  AC_MSG_RESULT(no)
  LIBPNG=""
  AC_SUBST(LIBPNG)
fi
])


AC_DEFUN([KDE_CHECK_COMPILER_FLAG],
[
AC_MSG_CHECKING([whether $CXX supports -$1])
kde_cache=`echo $1 | sed 'y% .=/+-,%____p__%'`
AC_CACHE_VAL(kde_cv_prog_cxx_$kde_cache,
[
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  save_CXXFLAGS="$CXXFLAGS"
  CXXFLAGS="$CXXFLAGS -$1"
  AC_TRY_LINK([],[ return 0; ], [eval "kde_cv_prog_cxx_$kde_cache=yes"], [])
  CXXFLAGS="$save_CXXFLAGS"
  AC_LANG_RESTORE
])
if eval "test \"`echo '$kde_cv_prog_cxx_'$kde_cache`\" = yes"; then
 AC_MSG_RESULT(yes)
 :
 $2
else
 AC_MSG_RESULT(no)
 :
 $3
fi
])

AC_DEFUN([KDE_CHECK_C_COMPILER_FLAG],
[
AC_MSG_CHECKING([whether $CC supports -$1])
kde_cache=`echo $1 | sed 'y% .=/+-,%____p__%'`
AC_CACHE_VAL(kde_cv_prog_cc_$kde_cache,
[
  AC_LANG_SAVE
  AC_LANG_C
  save_CFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS -$1"
  AC_TRY_LINK([],[ return 0; ], [eval "kde_cv_prog_cc_$kde_cache=yes"], [])
  CFLAGS="$save_CFLAGS"
  AC_LANG_RESTORE
])
if eval "test \"`echo '$kde_cv_prog_cc_'$kde_cache`\" = yes"; then
 AC_MSG_RESULT(yes)
 :
 $2
else
 AC_MSG_RESULT(no)
 :
 $3
fi
])


AC_DEFUN([KDE_CHECK_LIB64],
[
    kdelibsuff="$kde_libs_suffix"
    if test -z "$kdelibsuff"; then
       kdelibsuff=no
    fi
    AC_ARG_ENABLE(libsuffix,
        AC_HELP_STRING([--enable-libsuffix],
            [/lib directory suffix (64,32,none[=default])]),
            kdelibsuff=$enableval)
    # TODO: add an auto case that compiles a little C app to check
    # where the glibc is
    if test "$kdelibsuff" = "no"; then
       kdelibsuff=
    fi
    if test -z "$kdelibsuff"; then
        AC_MSG_RESULT([not using lib directory suffix])
        AC_DEFINE(KDELIBSUFF, [""], Suffix for lib directories)
    else
        if test "$libdir" = '${exec_prefix}/lib'; then
            libdir="$libdir${kdelibsuff}"
            AC_SUBST([libdir], ["$libdir"])  dnl ugly hack for lib64 platforms
        fi
        AC_DEFINE_UNQUOTED(KDELIBSUFF, ["${kdelibsuff}"], Suffix for lib directories)
        AC_MSG_RESULT([using lib directory suffix $kdelibsuff])
    fi
])

AC_DEFUN([KDE_CHECK_TYPES],
[  AC_CHECK_SIZEOF(int, 4)dnl
   AC_CHECK_SIZEOF(short)dnl
  AC_CHECK_SIZEOF(long, 4)dnl
  AC_CHECK_SIZEOF(char *, 4)dnl
])dnl

AC_DEFUN([AC_CHECK_RPATH],
[
AC_MSG_CHECKING(for rpath)
AC_ARG_ENABLE(rpath,
      AC_HELP_STRING([--disable-rpath],[do not use the rpath feature of ld]),
      USE_RPATH=$enableval, USE_RPATH=yes)

if test x"$klash" = x"yes"; then

if test -z "$KDE_RPATH" && test "$USE_RPATH" = "yes"; then

  KDE_RPATH="-R \$(libdir)"

  if test "$kde_libraries" != "$libdir"; then
      KDE_RPATH="$KDE_RPATH -R \$(kde_libraries)"
  fi

  if test -n "$qt_libraries"; then
    KDE_RPATH="$KDE_RPATH -R \$(qt_libraries)"
  fi
  dnl $x_libraries is set to /usr/lib in case
  if test -n "$X_LDFLAGS"; then
    X_RPATH="-R \$(x_libraries)"
    KDE_RPATH="$KDE_RPATH $X_RPATH"
  fi
  if test -n "$KDE_EXTRA_RPATH"; then
    KDE_RPATH="$KDE_RPATH \$(KDE_EXTRA_RPATH)"
  fi
fi
fi
AC_SUBST(KDE_EXTRA_RPATH)
AC_SUBST(KDE_RPATH)
AC_SUBST(X_RPATH)
AC_MSG_RESULT($USE_RPATH)
])

dnl Check for the type of the third argument of getsockname
AC_DEFUN([AC_CHECK_SOCKLEN_T],
[
   AC_MSG_CHECKING(for socklen_t)
   AC_CACHE_VAL(kde_cv_socklen_t,
   [
      AC_LANG_PUSH(C++)
      kde_cv_socklen_t=no
      AC_TRY_COMPILE([
         #include <sys/types.h>
         #include <sys/socket.h>
      ],
      [
         socklen_t len;
         getpeername(0,0,&len);
      ],
      [
         kde_cv_socklen_t=yes
         kde_cv_socklen_t_equiv=socklen_t
      ])
      AC_LANG_POP(C++)
   ])
   AC_MSG_RESULT($kde_cv_socklen_t)
   if test $kde_cv_socklen_t = no; then
      AC_MSG_CHECKING([for socklen_t equivalent for socket functions])
      AC_CACHE_VAL(kde_cv_socklen_t_equiv,
      [
         kde_cv_socklen_t_equiv=int
         AC_LANG_PUSH(C++)
         for t in int size_t unsigned long "unsigned long"; do
            AC_TRY_COMPILE([
               #include <sys/types.h>
               #include <sys/socket.h>
            ],
            [
               $t len;
               getpeername(0,0,&len);
            ],
            [
               kde_cv_socklen_t_equiv="$t"
               break
            ])
         done
         AC_LANG_POP(C++)
      ])
      AC_MSG_RESULT($kde_cv_socklen_t_equiv)
   fi
   AC_DEFINE_UNQUOTED(kde_socklen_t, $kde_cv_socklen_t_equiv,
                     [type to use in place of socklen_t if not defined])
   AC_DEFINE_UNQUOTED(ksize_t, $kde_cv_socklen_t_equiv,
                     [type to use in place of socklen_t if not defined (deprecated, use kde_socklen_t)])
])

dnl This is a merge of some macros out of the gettext aclocal.m4
dnl since we don't need anything, I took the things we need
dnl the copyright for them is:
dnl >
dnl Copyright (C) 1994, 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
dnl This Makefile.in is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.
dnl >
dnl for this file it is relicensed under LGPL

AC_DEFUN([AM_KDE_WITH_NLS],
  [
    dnl If we use NLS figure out what method

    AM_PATH_PROG_WITH_TEST_KDE(MSGFMT, msgfmt,
        [test -n "`$ac_dir/$ac_word --version 2>&1 | grep 'GNU gettext'`"], msgfmt)
    AC_PATH_PROG(GMSGFMT, gmsgfmt, $MSGFMT)

     if test -z "`$GMSGFMT --version 2>&1 | grep 'GNU gettext'`"; then
        AC_MSG_RESULT([found msgfmt program is not GNU msgfmt; ignore it])
        GMSGFMT=":"
      fi
      MSGFMT=$GMSGFMT
      AC_SUBST(GMSGFMT)
      AC_SUBST(MSGFMT)

      AM_PATH_PROG_WITH_TEST_KDE(XGETTEXT, xgettext,
	[test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)

      dnl Test whether we really found GNU xgettext.
      if test "$XGETTEXT" != ":"; then
	dnl If it is no GNU xgettext we define it as : so that the
	dnl Makefiles still can work.
	if $XGETTEXT --omit-header /dev/null 2> /dev/null; then
	  : ;
	else
	  AC_MSG_RESULT(
	    [found xgettext programs is not GNU xgettext; ignore it])
	  XGETTEXT=":"
	fi
      fi
     AC_SUBST(XGETTEXT)

  ])

# Search path for a program which passes the given test.
# Ulrich Drepper <drepper@cygnus.com>, 1996.

dnl serial 1
# Stephan Kulow: I appended a _KDE against name conflicts

dnl AM_PATH_PROG_WITH_TEST_KDE(VARIABLE, PROG-TO-CHECK-FOR,
dnl   TEST-PERFORMED-ON-FOUND_PROGRAM [, VALUE-IF-NOT-FOUND [, PATH]])
AC_DEFUN([AM_PATH_PROG_WITH_TEST_KDE],
[# Extract the first word of "$2", so it can be a program name with args.
set dummy $2; ac_word=[$]2
AC_MSG_CHECKING([for $ac_word])
AC_CACHE_VAL(ac_cv_path_$1,
[case "[$]$1" in
  /*)
  ac_cv_path_$1="[$]$1" # Let the user override the test with a path.
  ;;
  *)
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:"
  for ac_dir in ifelse([$5], , $PATH, [$5]); do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/$ac_word; then
      if [$3]; then
	ac_cv_path_$1="$ac_dir/$ac_word"
	break
      fi
    fi
  done
  IFS="$ac_save_ifs"
dnl If no 4th arg is given, leave the cache variable unset,
dnl so AC_PATH_PROGS will keep looking.
ifelse([$4], , , [  test -z "[$]ac_cv_path_$1" && ac_cv_path_$1="$4"
])dnl
  ;;
esac])dnl
$1="$ac_cv_path_$1"
if test -n "[$]$1"; then
  AC_MSG_RESULT([$]$1)
else
  AC_MSG_RESULT(no)
fi
AC_SUBST($1)dnl
])

AC_DEFUN([KDE_CHECK_LIBDL],
[
AC_CHECK_LIB(dl, dlopen, [
LIBDL="-ldl"
ac_cv_have_dlfcn=yes
])

AC_CHECK_LIB(dld, shl_unload, [
LIBDL="-ldld"
ac_cv_have_shload=yes
])

AC_SUBST(LIBDL)
])

AC_DEFUN([KDE_CHECK_LIBPTHREAD],
[
  dnl This code is here specifically to handle the
  dnl various flavors of threading library on FreeBSD
  dnl 4-, 5-, and 6-, and the (weird) rules around it.
  dnl There may be an environment PTHREAD_LIBS that 
  dnl specifies what to use; otherwise, search for it.
  dnl -pthread is special cased and unsets LIBPTHREAD
  dnl below if found.
  LIBPTHREAD=""

  if test -n "$PTHREAD_LIBS"; then
    if test "x$PTHREAD_LIBS" = "x-pthread" ; then
      LIBPTHREAD="PTHREAD"
    else
      PTHREAD_LIBS_save="$PTHREAD_LIBS"
      PTHREAD_LIBS=`echo "$PTHREAD_LIBS_save" | sed -e 's,^-l,,g'`
      AC_MSG_CHECKING([for pthread_create in $PTHREAD_LIBS])
      KDE_CHECK_LIB($PTHREAD_LIBS, pthread_create, [
          LIBPTHREAD="$PTHREAD_LIBS_save"])
      PTHREAD_LIBS="$PTHREAD_LIBS_save"
    fi
  fi

  dnl Is this test really needed, in the face of the Tru64 test below?
  if test -z "$LIBPTHREAD"; then
    AC_CHECK_LIB(pthread, pthread_create, [LIBPTHREAD="-lpthread"])
  fi

  dnl This is a special Tru64 check, see BR 76171 issue #18.
  if test -z "$LIBPTHREAD" ; then
    AC_MSG_CHECKING([for pthread_create in -lpthread])
    kde_safe_libs=$LIBS
    LIBS="$LIBS -lpthread"
    AC_TRY_LINK([#include <pthread.h>],[(void)pthread_create(0,0,0,0);],[
        AC_MSG_RESULT(yes)
        LIBPTHREAD="-lpthread"],[
	AC_MSG_RESULT(no)])
    LIBS=$kde_safe_libs
  fi

  dnl Un-special-case for FreeBSD.
  if test "x$LIBPTHREAD" = "xPTHREAD" ; then
    LIBPTHREAD=""
  fi

  AC_SUBST(LIBPTHREAD)
])

AC_DEFUN([KDE_CHECK_PTHREAD_OPTION],
[
      USE_THREADS=""
      if test -z "$LIBPTHREAD"; then
        KDE_CHECK_COMPILER_FLAG(pthread, [USE_THREADS="-D_THREAD_SAFE -pthread"])
      fi

    AH_VERBATIM(__svr_define, [
#if defined(__SVR4) && !defined(__svr4__)
#define __svr4__ 1
#endif
])
    case $host_os in
 	solaris*)
		KDE_CHECK_COMPILER_FLAG(mt, [USE_THREADS="-mt"])
                CPPFLAGS="$CPPFLAGS -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -DUSE_SOLARIS -DSVR4"
    		;;
        freebsd*)
                CPPFLAGS="$CPPFLAGS -D_THREAD_SAFE $PTHREAD_CFLAGS"
                ;;
        aix*)
                CPPFLAGS="$CPPFLAGS -D_THREAD_SAFE"
                LIBPTHREAD="$LIBPTHREAD -lc_r"
                ;;
        linux*) CPPFLAGS="$CPPFLAGS -D_REENTRANT"
                if test "$CXX" = "KCC"; then
                  CXXFLAGS="$CXXFLAGS --thread_safe"
		  NOOPT_CXXFLAGS="$NOOPT_CXXFLAGS --thread_safe"
                fi
                ;;
	*)
		;;
    esac
    AC_SUBST(USE_THREADS)
    AC_SUBST(LIBPTHREAD)
])

AC_DEFUN([KDE_CHECK_THREADING],
[
  AC_REQUIRE([KDE_CHECK_LIBPTHREAD])
  AC_REQUIRE([KDE_CHECK_PTHREAD_OPTION])
  dnl default is yes if libpthread is found and no if no libpthread is available
  if test -z "$LIBPTHREAD"; then
    if test -z "$USE_THREADS"; then
      kde_check_threading_default=no
    else
      kde_check_threading_default=yes
    fi
  else
    kde_check_threading_default=yes
  fi
  AC_ARG_ENABLE(threading,AC_HELP_STRING([--disable-threading],[disables threading even if libpthread found]),
   kde_use_threading=$enableval, kde_use_threading=$kde_check_threading_default)
  if test "x$kde_use_threading" = "xyes"; then
    AC_DEFINE(HAVE_LIBPTHREAD, 1, [Define if you have a working libpthread (will enable threaded code)])
  fi
])

AC_DEFUN([KDE_CHECK_STL],
[
    AC_LANG_SAVE
    AC_LANG_CPLUSPLUS
    ac_save_CXXFLAGS="$CXXFLAGS"
    CXXFLAGS="`echo $CXXFLAGS | sed s/-fno-exceptions//`"

    AC_MSG_CHECKING([if C++ programs can be compiled])
    AC_CACHE_VAL(kde_cv_stl_works,
    [
      AC_TRY_COMPILE([
#include <string>
using namespace std;
],[
  string astring="Hallo Welt.";
  astring.erase(0, 6); // now astring is "Welt"
  return 0;
], kde_cv_stl_works=yes,
   kde_cv_stl_works=no)
])

   AC_MSG_RESULT($kde_cv_stl_works)

   if test "$kde_cv_stl_works" = "yes"; then
     # back compatible
	 AC_DEFINE_UNQUOTED(HAVE_SGI_STL, 1, [Define if you have a STL implementation by SGI])
   else
	 AC_MSG_ERROR([Your Installation isn't able to compile simple C++ programs.
Check config.log for details - if you're using a Linux distribution you might miss
a package named similar to libstdc++-dev.])
   fi

   CXXFLAGS="$ac_save_CXXFLAGS"
   AC_LANG_RESTORE
])

AC_DEFUN([KDE_FAST_CONFIGURE],
[
  dnl makes configure fast (needs perl)
  AC_ARG_ENABLE(fast-perl, AC_HELP_STRING([--disable-fast-perl],[disable fast Makefile generation (needs perl)]),
      with_fast_perl=$enableval, with_fast_perl=yes)
])

AC_DEFUN([KDE_CONF_FILES],
[
  val=
  if test -f $srcdir/configure.files ; then
    val=`sed -e 's%^%\$(top_srcdir)/%' $srcdir/configure.files`
  fi
  CONF_FILES=
  if test -n "$val" ; then
    for i in $val ; do
      CONF_FILES="$CONF_FILES $i"
    done
  fi
  AC_SUBST(CONF_FILES)
])dnl

dnl This sets the prefix, for arts and kdelibs
dnl Do NOT use in any other module.
dnl It only looks at --prefix, KDEDIR and falls back to /usr/local/kde

AC_DEFUN([KDE_SET_PREFIX],
[
  unset CDPATH
  dnl We can't give real code to that macro, only a value.
  dnl It only matters for --help, since we set the prefix in this function anyway.
dnl    AC_PREFIX_DEFAULT(${KDEDIR:-the kde prefix})

dnl    KDE_SET_DEFAULT_BINDIRS
    if test "x$prefix" = "xNONE"; then
    dnl no prefix given: look for kde-config in the PATH and deduce the prefix from it
      KDE_FIND_PATH(kde-config, KDECONFIG, [$kde_default_bindirs], [KDE_MISSING_PROG_ERROR(kde-config)], [], prepend)
    else
    dnl prefix given: look for kde-config, preferrably in prefix, otherwise in PATH
      kde_save_PATH="$PATH"
      PATH="$exec_prefix/bin:$prefix/bin:$PATH"
      KDE_FIND_PATH(kde-config, KDECONFIG, [$kde_default_bindirs], [KDE_MISSING_PROG_ERROR(kde-config)], [], prepend)
      PATH="$kde_save_PATH"
    fi

dnl    kde_libs_prefix=`$KDECONFIG --prefix`
dnl     if test -z "$kde_libs_prefix" || test ! -x "$kde_libs_prefix"; then
dnl       AC_MSG_NOTICE([$KDECONFIG --prefix outputed the non existant prefix '$kde_libs_prefix' for kdelibs.])
dnl     fi
dnl    kde_libs_htmldir=`$KDECONFIG --install html --expandvars`
dnl     kde_libs_suffix=`$KDECONFIG --libsuffix`

dnl     AC_MSG_CHECKING([where to install])
dnl     if test "x$prefix" = "xNONE"; then
dnl dnl      prefix=$kde_libs_prefix
dnl       AC_MSG_RESULT([$prefix (as returned by kde-config)])
dnl     else
dnl     dnl --prefix was given. Compare prefixes and warn (in configure.in.bot.end) if different
dnl       given_prefix=$prefix
dnl       AC_MSG_RESULT([$prefix (as requested)])
dnl     fi

    # And delete superfluous '/' to make compares easier
dnl     prefix=`echo "$prefix" | sed 's,//*,/,g' | sed -e 's,/$,,'`
dnl     exec_prefix=`echo "$exec_prefix" | sed 's,//*,/,g' | sed -e 's,/$,,'`
dnl     given_prefix=`echo "$given_prefix" | sed 's,//*,/,g' | sed -e 's,/$,,'`

    AC_SUBST(KDECONFIG)
dnl     AC_SUBST(kde_libs_prefix)
dnl     AC_SUBST(kde_libs_htmldir)

    KDE_FAST_CONFIGURE
    KDE_CONF_FILES
])

pushdef([AC_PROG_INSTALL],
[
  dnl our own version, testing for a -p flag
  popdef([AC_PROG_INSTALL])
  dnl as AC_PROG_INSTALL works as it works we first have
  dnl to save if the user didn't specify INSTALL, as the
  dnl autoconf one overwrites INSTALL and we have no chance to find
  dnl out afterwards
  test -n "$INSTALL" && kde_save_INSTALL_given=$INSTALL
  test -n "$INSTALL_PROGRAM" && kde_save_INSTALL_PROGRAM_given=$INSTALL_PROGRAM
  test -n "$INSTALL_SCRIPT" && kde_save_INSTALL_SCRIPT_given=$INSTALL_SCRIPT
  AC_PROG_INSTALL

  if test -z "$kde_save_INSTALL_given" ; then
    # OK, user hasn't given any INSTALL, autoconf found one for us
    # now we test, if it supports the -p flag
    AC_MSG_CHECKING(for -p flag to install)
    rm -f confinst.$$.* > /dev/null 2>&1
    echo "Testtest" > confinst.$$.orig
    ac_res=no
    if ${INSTALL} -p confinst.$$.orig confinst.$$.new > /dev/null 2>&1 ; then
      if test -f confinst.$$.new ; then
        # OK, -p seems to do no harm to install
	INSTALL="${INSTALL} -p"
	ac_res=yes
      fi
    fi
    rm -f confinst.$$.*
    AC_MSG_RESULT($ac_res)
  fi
  dnl the following tries to resolve some signs and wonders coming up
  dnl with different autoconf/automake versions
  dnl e.g.:
  dnl  *automake 1.4 install-strip sets A_M_INSTALL_PROGRAM_FLAGS to -s
  dnl   and has INSTALL_PROGRAM = @INSTALL_PROGRAM@ $(A_M_INSTALL_PROGRAM_FLAGS)
  dnl   it header-vars.am, so there the actual INSTALL_PROGRAM gets the -s
  dnl  *automake 1.4a (and above) use INSTALL_STRIP_FLAG and only has
  dnl   INSTALL_PROGRAM = @INSTALL_PROGRAM@ there, but changes the
  dnl   install-@DIR@PROGRAMS targets to explicitly use that flag
  dnl  *autoconf 2.13 is dumb, and thinks it can use INSTALL_PROGRAM as
  dnl   INSTALL_SCRIPT, which breaks with automake <= 1.4
  dnl  *autoconf >2.13 (since 10.Apr 1999) has not that failure
  dnl  *sometimes KDE does not use the install-@DIR@PROGRAM targets from
  dnl   automake (due to broken Makefile.am or whatever) to install programs,
  dnl   and so does not see the -s flag in automake > 1.4
  dnl to clean up that mess we:
  dnl  +set INSTALL_PROGRAM to use INSTALL_STRIP_FLAG
  dnl   which cleans KDE's program with automake > 1.4;
  dnl  +set INSTALL_SCRIPT to only use INSTALL, to clean up autoconf's problems
  dnl   with automake<=1.4
  dnl  note that dues to this sometimes two '-s' flags are used (if KDE
  dnl   properly uses install-@DIR@PROGRAMS, but I don't care
  dnl
  dnl And to all this comes, that I even can't write in comments variable
  dnl  names used by automake, because it is so stupid to think I wanted to
  dnl  _use_ them, therefor I have written A_M_... instead of AM_
  dnl hmm, I wanted to say something ... ahh yes: Arghhh.

  if test -z "$kde_save_INSTALL_PROGRAM_given" ; then
    INSTALL_PROGRAM='${INSTALL} $(INSTALL_STRIP_FLAG)'
  fi
  if test -z "$kde_save_INSTALL_SCRIPT_given" ; then
    INSTALL_SCRIPT='${INSTALL}'
  fi
])dnl

AC_DEFUN([KDE_CHECK_LIB],
[
     kde_save_LDFLAGS="$LDFLAGS"
     dnl AC_CHECK_LIB modifies LIBS, so save it here
     kde_save_LIBS="$LIBS"
     LDFLAGS="$LDFLAGS $all_libraries"
     case $host_os in
      aix*) LDFLAGS="-brtl $LDFLAGS"
	test "$GCC" = yes && LDFLAGS="-Wl,$LDFLAGS"
	;;
     esac
     AC_CHECK_LIB($1, $2, $3, $4, $5)
     LDFLAGS="$kde_save_LDFLAGS"
     LIBS="$kde_save_LIBS"
])

AC_DEFUN([KDE_CHECK_STRLCPY],
[
  AC_REQUIRE([AC_CHECK_STRLCAT])
  AC_REQUIRE([AC_CHECK_STRLCPY])
  AC_CHECK_SIZEOF(size_t)
  AC_CHECK_SIZEOF(unsigned long)

  AC_MSG_CHECKING([sizeof size_t == sizeof unsigned long])
  AC_TRY_COMPILE(,[
    #if SIZEOF_SIZE_T != SIZEOF_UNSIGNED_LONG
       choke me
    #endif
    ],AC_MSG_RESULT([yes]),[
      AC_MSG_RESULT(no)
      AC_MSG_ERROR([
       Apparently on your system our assumption sizeof size_t == sizeof unsigned long 
       does not apply. Please mail kde-devel@kde.org with a description of your system!
      ])
  ])
])

AC_DEFUN([KDE_CHECK_PERL],
[
	KDE_FIND_PATH(perl, PERL, [$bindir $exec_prefix/bin $prefix/bin], [
		    AC_MSG_NOTICE([No Perl found in your $PATH.])
	])
    AC_SUBST(PERL)
])
