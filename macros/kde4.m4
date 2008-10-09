AC_DEFUN([GNASH_PATH_QT4], [
	AC_ARG_WITH(qt4, AS_HELP_STRING([--with-qt4=DIR], [Location of Qt 4.x]))
	if test "$with_qt4" = "yes" -o "$with_qt4" = "no"; then
		with_qt4=""
	fi
	AC_MSG_CHECKING([for Qt 4.x])
	LibDir="lib"
	if uname -m |grep -q 64 && test -d /usr/lib64; then
		LibDir="lib64"
	fi
	gnash_cv_lib_qt_dir=no
	for dir in $with_qt4 $QTDIR /usr/$LibDir/qt4 /usr/local/$LibDir/qt4 /usr/local/$LibDir/qt /opt/qt /opt/qt4 /usr/$LibDir/qt /usr/local/$LibDir/qt /opt/qt $libdir; do
		if test -e $dir/lib/libQtCore.so; then
			gnash_cv_lib_qt_dir="$dir"
			break
		fi
	done
	if test "$gnash_cv_lib_qt_dir" = "no"; then
		dnl We might be on OSX...
		if test -d /Library/Frameworks/QtCore.framework; then
			gnash_cv_lib_qt_dir=/Library/Frameworks
			LDFLAGS="-L/Library/Frameworks -F/Library/Frameworks"
		fi
	fi
	if test -x "$gnash_cv_lib_qt_dir/bin/moc"; then
		MOC4="$gnash_cv_lib_qt_dir/bin/moc"
		UIC4="$gnash_cv_lib_qt_dir/bin/uic"
		RCC4="$gnash_cv_lib_qt_dir/bin/rcc"
	else
		MOC4="moc"
		UIC4="uic"
		RCC4="rcc"
	fi
	if test "$gnash_cv_lib_qt_dir" = "no"; then
		AC_MSG_RESULT([not found])
	else
		AC_MSG_RESULT([$gnash_cv_lib_qt_dir])
		QT="$gnash_cv_lib_qt_dir"
		AC_SUBST([QT])
		AC_SUBST([MOC4])
		AC_SUBST([UIC4])
		AC_SUBST([RCC4])
	fi
])

AC_DEFUN([GNASH_QT4_CORE], [
	AC_REQUIRE([GNASH_PATH_QT4])
	if test "$gnash_cv_lib_qt_dir" = "/Library/Frameworks"; then
		CPPFLAGS="$CPPFLAGS -D_REENTRANT -DQT_SHARED -I$gnash_cv_lib_qt_dir/QtCore.framework/Headers"
		QTCORE="-framework QtCore"
	else
		CPPFLAGS="$CPPFLAGS -D_REENTRANT -DQT_SHARED -I$gnash_cv_lib_qt_dir/include -I$gnash_cv_lib_qt_dir/include/QtCore"
		LDFLAGS="$LDFLAGS -L$gnash_cv_lib_qt_dir/lib"
		QTCORE="-lQtCore"
	fi
	if test "$enable_qt_debug" = "no"; then
		CPPFLAGS="$CPPFLAGS -DQT_NO_DEBUG"
	fi
	AC_SUBST(QTCORE)
])

AC_DEFUN([GNASH_QT4_GUI], [
	AC_REQUIRE([GNASH_QT4_CORE])
	if test "$gnash_cv_lib_qt_dir" = "/Library/Frameworks"; then
		CPPFLAGS="$CPPFLAGS -I$gnash_cv_lib_qt_dir/QtGui.framework/Headers"
		QTGUI="-framework QtGui"
	else
		CPPFLAGS="$CPPFLAGS -I$gnash_cv_lib_qt_dir/include/QtGui"
		QTGUI="-lQtGui"
	fi
	AC_SUBST(QTGUI)
])

AC_DEFUN([GNASH_QT4_XML], [
	AC_REQUIRE([GNASH_QT4_CORE])
	if test "$gnash_cv_lib_qt_dir" = "/Library/Frameworks"; then
		CPPFLAGS="$CPPFLAGS -I$gnash_cv_lib_qt_dir/QtXml.framework/Headers"
		QTXML="-framework QtXml"
	else
		CPPFLAGS="$CPPFLAGS -I$gnash_cv_lib_qt_dir/include/QtXml"
		QTXML="-lQtXml"
	fi
	AC_SUBST(QTXML)
])

AC_DEFUN([GNASH_QT4_DBUS], [
	AC_REQUIRE([GNASH_QT4_XML])
	CPPFLAGS="$CPPFLAGS -I$gnash_cv_lib_qt_dir/include/QtDBus"
	QTDBUS="-lQtDBus"
	AC_SUBST(QTDBUS)
])

AC_DEFUN([GNASH_QT4_NETWORK], [
	AC_REQUIRE([GNASH_QT4_CORE])
	if test "$gnash_cv_lib_qt_dir" = "/Library/Frameworks"; then
		CPPFLAGS="$CPPFLAGS -I$gnash_cv_lib_qt_dir/QtNetwork.framework/Headers"
		QTNETWORK="-framework QtNetwork"
	else
		CPPFLAGS="$CPPFLAGS -I$gnash_cv_lib_qt_dir/include/QtNetwork"
		QTNETWORK="-lQtNetwork"
	fi
	AC_SUBST(QTNETWORK)
])

AC_DEFUN([GNASH_QT4_OPENGL], [
	AC_REQUIRE([GNASH_QT4_GUI])
	if test "$gnash_cv_lib_qt_dir" = "/Library/Frameworks"; then
		CPPFLAGS="$CPPFLAGS -I$gnash_cv_lib_qt_dir/QtOpenGL.framework/Headers"
		QTOPENGL="-framework QtOpenGL"
	else
		CPPFLAGS="$CPPFLAGS -I$gnash_cv_lib_qt_dir/include/QtOpenGL"
		QTOPENGL="-lQtOpenGL"
	fi
	AC_SUBST(QTOPENGL)
])

AC_DEFUN([GNASH_QT4_SCRIPT], [
	AC_REQUIRE([GNASH_QT4_CORE])
	if test "$gnash_cv_lib_qt_dir" = "/Library/Frameworks"; then
		CPPFLAGS="$CPPFLAGS -I$gnash_cv_lib_qt_dir/QtScript.framework/Headers"
		QTSCRIPT="-framework QtScript"
	else
		CPPFLAGS="$CPPFLAGS -I$gnash_cv_lib_qt_dir/include/QtScript"
		QTSCRIPT="-lQtScript"
	fi
	AC_SUBST(QTSCRIPT)
])

AC_DEFUN([GNASH_QT4_SQL], [
	AC_REQUIRE([GNASH_QT4_CORE])
	if test "$gnash_cv_lib_qt_dir" = "/Library/Frameworks"; then
		CPPFLAGS="$CPPFLAGS -I$gnash_cv_lib_qt_dir/QtSql.framework/Headers"
		QTSQL="-framework QtSql"
	else
		CPPFLAGS="$CPPFLAGS -I$gnash_cv_lib_qt_dir/include/QtSql"
		QTSQL="-lQtSql"
	fi
	AC_SUBST(QTSQL)
])

AC_DEFUN([GNASH_QT4_SVG], [
	AC_REQUIRE([GNASH_QT4_GUI])
	AC_REQUIRE([GNASH_QT4_XML])
	if test "$gnash_cv_lib_qt_dir" = "/Library/Frameworks"; then
		CPPFLAGS="$CPPFLAGS -I$gnash_cv_lib_qt_dir/QtSvg.framework/Headers"
		QTSVG="-framework QtSvg"
	else
		CPPFLAGS="$CPPFLAGS -I$gnash_cv_lib_qt_dir/include/QtSvg"
		QTSVG="-lQtSvg"
	fi
])

AC_DEFUN([GNASH_QT4_TEST], [
	AC_REQUIRE([GNASH_QT4_CORE])
	if test "$gnash_cv_lib_qt_dir" = "/Library/Frameworks"; then
		CPPFLAGS="$CPPFLAGS -I$gnash_cv_lib_qt_dir/QtTest.framework/Headers"
		QTTEST="-framework QtTest"
	else
		CPPFLAGS="$CPPFLAGS -I$gnash_cv_lib_qt_dir/include/QtTest"
		QTTEST="-lQtTest"
	fi
	AC_SUBST(QTTEST)
])

AC_DEFUN([GNASH_QT4_COMPAT], [
	AC_REQUIRE([GNASH_QT4_CORE])
	AC_REQUIRE([GNASH_QT4_GUI])
	AC_REQUIRE([GNASH_QT4_NETWORK])
	AC_REQUIRE([GNASH_QT4_SQL])
	AC_REQUIRE([GNASH_QT4_XML])
	if test "$gnash_cv_lib_qt_dir" = "/Library/Frameworks"; then
		CPPFLAGS="$CPPFLAGS -I$gnash_cv_lib_qt_dir/Qt3Support.framework/Headers"
		QT3SUPPORT="-framework Qt3Support"
	else
		CPPFLAGS="$CPPFLAGS -I$gnash_cv_lib_qt_dir/include/Qt3Support"
		QT3SUPPORT="-lQt3Support"
	fi
	AC_SUBST(QT3SUPPORT)
])

AC_DEFUN([GNASH_QT4_DESIGNER], [
	AC_REQUIRE([GNASH_QT4_CORE])
	AC_REQUIRE([GNASH_QT4_GUI])
	AC_REQUIRE([GNASH_QT4_NETWORK])
	AC_REQUIRE([GNASH_QT4_XML])
	if test "$gnash_cv_lib_qt_dir" = "/Library/Frameworks"; then
		CPPFLAGS="$CPPFLAGS -I$gnash_cv_lib_qt_dir/QtDesigner.framework/Headers"
		QTDESIGNER="-framework QtDesigner"
	else
		CPPFLAGS="$CPPFLAGS -I$gnash_cv_lib_qt_dir/include/QtDesigner"
		QTDESIGNER="-lQtDesigner"
	fi
	AC_SUBST(QTDESIGNER)
])

AC_DEFUN([GNASH_QT_WEBKIT], [
	AC_REQUIRE([GNASH_QT4_GUI])
	AC_MSG_CHECKING([for WebKit])
	SAVE_CPPFLAGS="$CPPFLAGS"
	SAVE_LDFLAGS="$LDFLAGS"
	if test "$gnash_cv_lib_qt_dir" = "/Library/Frameworks"; then
		CPPFLAGS="$CPPFLAGS -I$gnash_cv_lib_qt_dir/QtWebKit.framework/Headers"
		QTWEBKIT="-framework QtWebKit"
	else
		CPPFLAGS="$CPPFLAGS -I$gnash_cv_lib_qt_dir/include/QtWebKit"
		QTWEBKIT="-lQtWebKit"
	fi
	AC_LANG_PUSH([C++])
	AC_COMPILE_IFELSE([
		#include <qwebpage.h>
	], [found=yes], [found=no])
	AC_MSG_RESULT([$found])
	AC_LANG_POP
	if test "$found" = "yes"; then
		AC_DEFINE([HAVE_WEBKIT], [1], [Define if you have WebKit])
		AC_SUBST(QTWEBKIT)
	else
		CPPFLAGS="$SAVE_CPPFLAGS"
		LDFLAGS="$SAVE_LDFLAGS"
	fi
])

AC_DEFUN([GNASH_QT4], [
	AC_REQUIRE([GNASH_QT4_CORE])
	AC_REQUIRE([GNASH_QT4_GUI])
	AC_REQUIRE([GNASH_QT4_XML])
	AC_REQUIRE([GNASH_QT4_NETWORK])
	AC_REQUIRE([GNASH_QT4_OPENGL])
	AC_REQUIRE([GNASH_QT4_DBUS])
	AC_REQUIRE([GNASH_QT4_SCRIPT])
	AC_REQUIRE([GNASH_QT4_SQL])
	AC_REQUIRE([GNASH_QT4_SVG])
	AC_REQUIRE([GNASH_QT4_TEST])
])

AC_DEFUN([GNASH_PATH_KDE4], [
	AC_ARG_WITH(kde4, AS_HELP_STRING([--with-kde4=DIR], [Location of KDE 4.x]))
	if test "$with_kde4" = "yes"; then
		with_kde4=""
	fi
	AC_MSG_CHECKING([for KDE 4.x])
	if test "$with_kde4" = "no"; then
		AC_MSG_RESULT([disabled by user])
	else
		gnash_cv_lib_kde_dir=no
		if uname -m |grep -q 64 && test -d /usr/lib64; then
			LibDir="lib64"
		fi
		for dir in $with_kde4 $prefix /opt/kde4 /opt/kde /usr /usr/local; do
			if test -e $dir/$LibDir/libkhtml.so.5; then
				gnash_cv_lib_kde_dir="$dir"
				break
			fi
		done
		if test "$gnash_cv_lib_kde_dir" = "no"; then
			with_kde4=no
			AC_MSG_RESULT([not found])
		else
			AC_DEFINE_UNQUOTED([WITH_KDE4], [1], [Define to use KDE])
			AC_MSG_RESULT([$gnash_cv_lib_kde_dir])
			KDE="$gnash_cv_lib_kde_dir"
			if test "$KDE" != "/usr"; then
				KDE4_INCLUDES="-I$KDE/include"
				KDECORE="-L$KDE/$LibDir -Wl,--rpath -Wl,$KDE/$LibDir -lkdecore"
			else
				KDECORE="-lkdecore"
			fi
			AC_SUBST([KDE4])
			AC_SUBST([KDE4_INCLUDES])
			KDEUI="$KDECORE -lkdeui"
			KHTML="$KDECORE -lkhtml"
			AC_SUBST([KDECORE])
			AC_SUBST([KHTML])
			AC_SUBST([KDEUI])

			KDE4_PLUGINDIR="$gnash_cv_lib_kde_dir/$LibDir/kde4"
			KDE4_SERVICESDIR="$gnash_cv_lib_kde_dir/share/kde4/services"
			KDE4_APPSDATADIR="$gnash_cv_lib_kde_dir/share/apps/klash4"
			KDE4_CONFIGDIR="$gnash_cv_lib_kde_dir/share/config/"
			AC_SUBST([KDE4_PLUGINDIR])
			AC_SUBST([KDE4_SERVICESDIR])
			AC_SUBST([KDE4_APPSDATADIR])
			AC_SUBST([KDE4_CONFIGDIR])
		fi
	fi
	AM_CONDITIONAL(WITH_KDE4, test "$with_kde4" != "no")
])

AC_DEFUN([GNASH_QT_PLATFORM], [
	AC_REQUIRE([GNASH_QT4_CORE])
	AC_MSG_CHECKING([Qt platform])
	platform=unknown
	AC_COMPILE_IFELSE([
		#include <qglobal.h>
		#ifndef Q_WS_X11
		#error Not X11
		#endif
	], [platform=X11], [])
	if test "$platform" = "unknown"; then
		AC_COMPILE_IFELSE([
			#include <qglobal.h>
			#ifndef Q_WS_QWS
			#error Not Qtopia
			#endif
		], [platform=Qtopia], [])
	fi
	if test "$platform" = "unknown"; then
		AC_COMPILE_IFELSE([
			#include <qglobal.h>
			#ifndef Q_WS_MACX
			#error Not OSX
			#endif
		], [platform=OSX], [])
	fi
	if test "$platform" = "unknown"; then
		AC_COMPILE_IFELSE([
			#include <qglobal.h>
			#ifndef Q_WS_MAC9
			#error Not OS9
			#endif
		], [platform=OS9], [])
	fi
	if test "$platform" = "unknown"; then
		AC_COMPILE_IFELSE([
			#include <qglobal.h>
			#ifndef Q_WS_WIN32
			#error No dirty Nazi junk
			#endif
		], [platform=Win32], [])
	fi
	AM_CONDITIONAL([QT_X11], [test "$platform" = "X11"])
	AM_CONDITIONAL([QTOPIA], [test "$platform" = "Qtopia"])
	AM_CONDITIONAL([QT_OSX], [test "$platform" = "OSX"])
	AM_CONDITIONAL([QT_OS9], [test "$platform" = "OS9"])
	AM_CONDITIONAL([QT_WIN32], [test "$platform" = "Win32"])
	AC_MSG_RESULT([$platform])
])
