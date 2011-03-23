Name:           gnash
# This next field gets edited by "make gnash.spec" when building an rpm
Version:        0.8.10dev
Release:        0
Epoch: 		1
# This next field gets edited by "make gnash.spec" when building an rpm
Distribution:	fc13
Summary:        GNU SWF player

Group:          Applications/Multimedia
Vendor:		Gnash Project
Packager:	Rob Savoye <rob@welcomehome.org>
License:        GPLv3
URL:            http://www.gnu.org/software/gnash/
Source0:        http://www.getgnash.org/packages/snapshots/fedora/%{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-%{_target_cpu}

BuildRequires:  redhat-lsb mysql-devel
# bitmap libraries for loading images
BuildRequires:  libpng-devel libjpeg-devel giflib-devel
# these are needed for the python gtk widget
BuildRequires:  pygtk2-devel python-devel
BuildRequires:  gtk2-devel freetype-devel fontconfig-devel
BuildRequires:  openssl-devel curl-devel boost-devel
BuildRequires:  gstreamer-devel >= 0.10, gstreamer-plugins-base-devel >= 0.10
# these are for the kde4 support
BuildRequires:  kdelibs-devel >= 4.0, kdebase-devel >= 4.0, qt-devel >= 4.0
# these are needed for the various renderers, which now all get built
BuildRequires:  libXt-devel agg-devel gtkglext-devel libstdc++

# The default Gnash package only includes the GTK parts, the rest
# is in gnash-common.
Requires:  gtkglext gtk2 pygtk2 python
Requires:  gnash-common

# Fedora 12 packages the boost libraries as separate packages,
# whereas Fedora 11 has just the one dependency on boost-devel.
%if %{distribution} != "fc11"
BuildRequires:   boost-date-time boost-thread
Requires:   boost-date-time boost-thread
%endif

# BuildRequires:  scrollkeeper

#Requires(post): scrollkeeper
#Requires(postun): scrollkeeper
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
#Requires(post): /sbin/install-info
#Requires(preun): /sbin/install-info

%description
Gnash is a GNU SWF movie player that supports many SWF v7 features,
with growing support for swf v8, v9, and v10.

%package common
Summary:   Web-client SWF player plugin 
Group:     Applications/Multimedia
# Installation requirements
Requires:  libpng libjpeg giflib
Requires:  boost agg cairo libGL libXt libX11 libXv 
Requires:  freetype fontconfig libstdc++
Requires:  gstreamer >= 0.10, gstreamer-plugins-base >= 0.10
Requires:  openssl curl
# libX11 libExt libXv

%description common
Common files Shared between Gnash and Klash, Gnash/Klash is a GNU SWF movie
player that supports many SWF v7 features, with growing support for
swf v8, v9, and v10.

%package klash4
Summary:   Konqueror SWF player plugin for KDE 4
Group:     Applications/Multimedia
Requires:  gnash-common
Requires:  kdelibs >= 4, kdebase >= 4, qt >= 4, gnash

%description klash4
The gnash (klash) SWF player plugin for Konqueror in KDE4.

%package plugin
Summary:   Web-client SWF player plugin 
Group:     Applications/Internet
Requires:  gnash, gnash-common

%description plugin
The gnash SWF player plugin for firefox or mozilla.

%package cygnal
Summary:   Streaming media server
Group:     Applications/Multimedia
Requires:  gnash-common

%description cygnal
Cygnal is a streaming media server that's Flash aware.

%package devel
Summary:   Gnash header files
Group:     Applications/Multimedia
Requires:  gnash-common

%description devel
Gnash header files can be used to write external Gnash extensions.

%package widget
Summary:   Gnash widgets for Gtk and Python
Group:     Applications/Multimedia
Requires:  gnash, gnash-common

%description widget
The Gnash widgets can be used to embed Gnash into any Gtk or Python-Gtk
application.

%package fileio-extension
Summary:   Fileio extension for Gnash
Group:     Applications/Multimedia
Requires:  gnash-common

%description fileio-extension
This extension allows SWF files being played within Gnash to have direct access
to the file system. The API is similar to the C library one.

%package lirc-extension
Summary:   LIRC extension for Gnash
Group:     Applications/Multimedia
Requires:  gnash-common

%description lirc-extension
This extension allows SWF files being played within Gnash to have direct access
to a LIRC based remote control device. The API is similar to the standard
LIRC one.

%package dejagnu-extension
Summary:   DejaGnu extension for Gnash
Group:     Applications/Multimedia
Requires:  gnash-common

%description dejagnu-extension
This extension allows SWF files to have a simple unit testing API. The API
is similar to the DejaGnu unit testing one.

%package mysql-extension
Summary:   MySQL extension for Gnash
Group:     Applications/Multimedia
Requires:  gnash-common

%description mysql-extension
This extension allows SWF files being played within Gnash to have direct access
to a MySQL database. The API is similar to the standard MySQL one.

%prep
%setup -q

%build

# For QT3
# [ -n "$QTDIR" ] || . %{_sysconfdir}/profile.d/qt.sh

# handle cross building rpms. This gets messy when building for two
# archtectures with the same CPU type, like x86-Linux -> OLPC. We have
# to do this because an OLPC requires RPMs to install software, but
# doesn't have the resources to do native builds. So this hack lets us
# build RPM packages on one host for the OLPC, or other RPM based
# embedded distributions.
%if %{_target_cpu} != %{_build_arch}
%define cross_compile 1
%else
%define cross_compile 0
%endif
# if not defined, assume this is a native package.
%{?do_cross_compile:%define cross_compile 0}

# FIXME: this is a bad hack! Although all this does work correctly and
# build an RPM, it's set for an geode-olpc, so the actual hardware
# won't let us install it.
# %define cross_compile 0
# %define olpc 0

# Build rpms for an ARM based processor, in our case the Nokia 770/800/810
# tablet. 
%ifarch arm
RPM_TARGET=%{_target}
%endif

%if %{cross_compile}
# cross building an RPM. This works as long as you have a good cross
# compiler installed.
  CROSS_OPTS="--build=%{_host} --host=$RPM_TARGET --target=$RPM_TARGET"
  RENDERER="--enable-renderer=agg"		# could be opengl
  %ifarch arm
    SOUND="--enable-media=none --disable-nsapi --disable-kparts"
  %else
    SOUND="--enable-media=gst"			# could also be sdl
  %endif
%else
# Native RPM build
  CROSS_OPTS="" # "--enable-ghelp --enable-docbook"
  # these are actually the default values, but this way they get added
  # to the build so they appear in "gnash --version".
  GUI="--enable-gui=gtk,kde4"	# could be kde3, kde4, aqua, sdl
  SOUND="--enable-media=gst"	# could be ffmpeg
  OTHER="--enable-cygnal"
  RENDERER="--enable-renderer=all"		# could be opengl or cairo
  # These are not the defaults
  OPTIONAL="--enable-python"
%endif

%if %{distribution} != "ydl6"
  SOUND="--enable-media=gst" 
%endif

# we disable the testsuites by default, as when building packages we
# should have already been running the testsuites as part of the 
# normal build & test development cycle.

# The default options for the configure aren't suitable for
# cross configuring, so we force them to be what we know is correct.
# uncommenting these will produce huge volumes of debug info from the
# shell, but sometimes that's what you need to do.
# export CONFIG_SHELL="sh -x"
# sh -x ./configure \
%if %{cross_compile}
%configure --enable-static \
	$CROSS_OPTS \
	$SOUND $GUI \
	$RENDERER \
	$OTHER \
	$OPTIONAL \
	--disable-dependency-tracking \
	--disable-testsuite \
	--disable-rpath \
	--with-plugindir=%{_libdir}/mozilla/plugins

make MAKEFLAGS=$MAKEFLAGS dumpconfig all
%else
# uncommenting these will produce huge volumes of debug info from the
# shell, but sometimes that's what you need to do.
# export CONFIG_SHELL="sh -x"
# sh -x ./configure
sh ./configure \
	$CROSS_OPTS \
	$SOUND $GUI \
	$RENDERER \
	$OTHER \
	$OPTIONAL \
        --prefix=/usr \
	--libdir=%{_libdir} \
	--mandir=%{_prefix}/share/man \
	--infodir=%{_prefix}/share/info \
	--disable-dependency-tracking \
	--disable-testsuite \
	--disable-rpath \
	--enable-renderer=agg,cairo \
	--enable-cygnal \
	--enable-python \
	--with-plugins-install=system \
	--with-plugindir=%{_libdir}/mozilla/plugins \
	--enable-extensions=fileio,lirc,dejagnu,mysql

make MAKEFLAGS=$MAKEFLAGS dumpconfig all LDFLAGS="-Wl,--build-id"
%endif
# When testing the spec file, try setting MAKEFLAGS to
# "CXXFLAGS-O0 -j4" to speed up getting results. Note *don't*
# do that for release builds, as the performance will suffer.

%install
strip gui/.libs/*-gnash
strip utilities/.libs/g* utilities/.libs/soldumper utilities/.libs/flvdumper cygnal/.libs/cygnal
rm -rf $RPM_BUILD_ROOT
make MAKEFLAGS=$MAKEFLAGS install DESTDIR=$RPM_BUILD_ROOT LDFLAGS="-Wl,--build-id"
make MAKEFLAGS=$MAKEFLAGS install-plugins DESTDIR=$RPM_BUILD_ROOT LDFLAGS="-Wl,--build-id"
rm -f $RPM_BUILD_ROOT%{_libdir}/gnash/*.*a
%if !%{cross_compile}
rm -rf $RPM_BUILD_ROOT%{_localstatedir}/scrollkeeper
rm -f $RPM_BUILD_ROOT%{_infodir}/dir
%endif
/usr/lib/rpm/brp-compress

%clean
rm -rf $RPM_BUILD_ROOT

%post 
/sbin/ldconfig
%if !%{cross_compile}
scrollkeeper-update -q -o %{_datadir}/omf/%{name} || :
/sbin/install-info --entry="* Gnash: (gnash). GNU SWF Player" %{_infodir}/%{name}.info %{_infodir}/dir || :
%endif

%preun
if [ $1 = 0 ]; then
    /sbin/install-info --delete %{_infodir}/%{name}.info %{_infodir}/dir || :
fi

%postun
/sbin/ldconfig
%if !%{cross_compile}
scrollkeeper-update -q || :
%endif

%files
%defattr(-,root,root,-)
%{_bindir}/gnash-gtk-launcher
%{_bindir}/gtk-gnash
%{_mandir}/man1/gtk-gnash.1.gz

%files common
%defattr(-,root,root,-)
%dump
%doc README AUTHORS COPYING NEWS 
%{_bindir}/gnash
%{_mandir}/man1/gnash.1.gz
%{_bindir}/gprocessor
%{_bindir}/soldumper
%{_bindir}/flvdumper
%{_bindir}/findmicrophones
%{_bindir}/findwebcams
#%{_bindir}/dumpshm
%{_bindir}/rtmpget
%{_libdir}/gnash/*.so*
%{_prefix}/share/gnash/GnashG.png
%{_prefix}/share/gnash/gnash_128_96.ico
%{_mandir}/man1/gprocessor.1.gz
%{_mandir}/man1/soldumper.1.gz
%{_mandir}/man1/flvdumper.1.gz
%{_mandir}/man1/findmicrophones.1.gz
%{_mandir}/man1/findwebcams.1.gz
%{_mandir}/man1/rtmpget.1.gz
%{_mandir}/man1/gnash-gtk-launcher.1.gz
%{_datadir}/locale/*/LC_MESSAGES/gnash.mo
%{_prefix}/share/applications/gnash.desktop
%{_prefix}/share/icons/hicolor/32x32/apps/gnash.xpm
%{_prefix}/share/gnash/gnash-splash.swf
%if !%{cross_compile}
#%{_prefix}/share/info/*.info*
%{_prefix}/share/gnash/doc/gnash/C/gnash*.html
%{_prefix}/share/gnash/doc/gnash/C/images/*.png
%{_prefix}/etc/gnashrc
%{_prefix}/etc/gnashpluginrc
# %{_infodir}/*.info*
#%doc doc/C/gnash*.html
#%doc doc/C/images/*.png
#%doc doc/C/images/*.txt
%endif

%files plugin
%defattr(-,root,root,-)
%{_libdir}/mozilla/plugins/libgnashplugin.so

%files cygnal
%defattr(-,root,root,-)
%{_bindir}/cygnal
%{_prefix}/etc/cygnalrc
%{_libdir}/cygnal/plugins/*.so*
%{_mandir}/man1/cygnal.1.gz

%files devel
%defattr(-,root,root,-)
%{_prefix}/include/gnash/*.h*
%{_prefix}/include/gnash/vm/*.h
%{_prefix}/include/gnash/asobj/*.h
%{_prefix}/include/gnash/parser/*.h
%{_libdir}/pkgconfig/gnash.pc

%files widget
%defattr(-,root,root,-)
%{_prefix}/include/gnash/*.h
%{_prefix}/lib*/python*/site-packages/gtk-2.0/gnash.*

%files klash4
%defattr(-,root,root,-)
%{_bindir}/gnash-qt-launcher
%{_bindir}/kde4-gnash
%{_mandir}/man1/kde4-gnash.1.*
%{_mandir}/man1/gnash-qt-launcher.1.gz
%{_prefix}/lib*/kde4/libklashpart.*
%{_prefix}/share/kde4/apps/klash/klashpartui.rc
%{_prefix}/share/kde4/apps/klash/pluginsinfo
%{_prefix}/share/kde4/services/klash_part.desktop
%{_prefix}/share/applications/klash.desktop
%{_prefix}/share/icons/hicolor/32x32/apps/klash.xpm

%files fileio-extension
%defattr(-,root,root,-)
%{_libdir}/gnash/plugins/fileio.so

%files lirc-extension
%defattr(-,root,root,-)
%{_libdir}/gnash/plugins/lirc.so

%files dejagnu-extension
%defattr(-,root,root,-)
%{_libdir}/gnash/plugins/dejagnu.so

%files mysql-extension
%defattr(-,root,root,-)
%{_libdir}/gnash/plugins/mysql.so

%changelog
* Sat Mar 27 2010 Rob Savoye <rob@welcomehome.org> - %{version}-%{release}
- add gnash-common package for non GUI files so as not to contaminate
  the gtk or kde packages. 

* Sat Sep 07 2009 Rob Savoye <rob@welcomehome.org> - %{version}-%{release}
- add kde4 support for klash.

* Sat Jun 13 2009 Rob Savoye <rob@welcomehome.org> - trunk
- Add support for packaging the gtk & python widget

* Sat Feb 13 2009 Rob Savoye <rob@welcomehome.org> - trunk
- Split off klash into it's own spec file.

* Sat Oct 24 2008 Rob Savoye <rob@welcomehome.org> - trunk
- Adjust dependencies for current bzr trunk

* Sat Feb  16 2008 Rob Savoye <rob@welcomehome.org> - %{version}-%{release}
- Adjust dependencies for current cvs HEAD

* Sat Mar  6 2007 Rob Savoye <rob@welcomehome.org> - %{version}-%{release}
- merge in patch from John @ Redhat.

* Tue Mar 06 2007 John (J5) Palmieri <johnp@redhat.com> 0.7.2.cvs20070306-1
- update to new snapshot

* Thu Feb 28 2007 John (J5) Palmieri <johnp@redhat.com> 0.7.2.cvs20070226-3
- require xulrunner instead of webclient

* Wed Feb 28 2007 John (J5) Palmieri <johnp@redhat.com> 0.7.2.cvs20070226-2
- don't delete requires .so files

* Mon Feb 26 2007 John (J5) Palmieri <johnp@redhat.com> 0.7.2.cvs20070226-1
- cvs snapshot built for olpc

* Sat Nov  7 2006 Rob Savoye <rob@welcomehome.org> - 0.7.2-2
- update for 0.7.2 release.

* Sat Nov  6 2006 Patrice Dumas <pertusus@free.fr> 0.7.2-1
- update for 0.7.2 release.

* Thu Oct 05 2006 Christian Iseli <Christian.Iseli@licr.org> 0.7.1-9
 - rebuilt for unwind info generation, broken in gcc-4.1.1-21

* Sun Sep 24 2006 Patrice Dumas <pertusus@free.fr> 0.7.1-8
- plugin requires %%{_libdir}/mozilla/plugins. Fix (incompletly and 
  temporarily, but there is no better solution yet) #207613

* Sun Aug 27 2006 Patrice Dumas <pertusus@free.fr> - 0.7.1-7
- add defattr for klash
- add warnings in the description about stability

* Mon Aug 21 2006 Patrice Dumas <pertusus@free.fr> - 0.7.1-6
- remove superfluous buildrequires autoconf
- rename last patch to gnash-plugin-tempfile-dir.patch
- add README.fedora to plugin to explain tmpdirs

* Wed Aug 16 2006 Jens Petersen <petersen@redhat.com> - 0.7.1-5
- source qt.sh and configure --with-qtdir (Dominik Mierzejewski)
- add plugin-tempfile-dir.patch for plugin to use a safe tempdir

* Fri Jul 28 2006 Jens Petersen <petersen@redhat.com> - 0.7.1-4
- buildrequire autotools (Michael Knox)

* Fri Jun  2 2006 Patrice Dumas <pertusus@free.fr> - 0.7.1-3
- add gnash-continue_on_info_install_error.patch to avoid
- buildrequire libXmu-devel

* Wed May 17 2006 Jens Petersen <petersen@redhat.com> - 0.7.1-2
- configure with --disable-rpath
- buildrequire docbook2X
- remove devel files

* Sun May  7 2006 Jens Petersen <petersen@redhat.com> - 0.7.1-1
- update to 0.7.1 alpha release

* Sat Apr  22 2006 Rob Savoye <rob@welcomehome.org> - 0.7-1
- install the info file. Various tweaks for my system based on
Patrice's latest patch,

* Fri Feb  3 2006 Patrice Dumas <dumas@centre-cired.fr> - 0.7-0
- initial packaging

