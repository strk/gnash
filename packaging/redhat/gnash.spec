Name:           gnash
Version:        20090213
Release:        1
Distribution:	fc10
#Distribution:	ydl6
Summary:        GNU SWF player

Group:          Applications/Multimedia
Vendor:		Gnash Project
Packager:	Rob Savoye <rob@welcomehome.org>
License:        GPLv3
URL:            http://www.gnu.org/software/gnash/
Source0:        http://www.getgnash.org/packages/snapshots/fedora/%{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-%{_target_cpu}

BuildRequires:  libpng-devel libjpeg-devel libogg-devel
BuildRequires:  gtk2-devel glib2-devel
BuildRequires:  atk-devel pango-devel
BuildRequires:  agg-devel boost-devel curl-devel libXt-devel
BuildRequires:  SDL-devel
# YellowDog doesn't ship ffmpeg
%if %{distribution} != "ydl6"
BuildRequires:  ffmpeg-devel
%endif
# Mandriva uses differ names for the X11 library packages
%if %{distribution} != "mnb"
BuildRequires:  libx11_6-devel libxt_6-devel
%else
BuildRequires:  libX11-devel libXt-devel xorg-x11-proto-devel 
%endif

# Installation requirements
Requires: libpng libjpeg libogg gtk2 glib2 atk pango
# Mandriva uses differ names for the X11 library packages
%if %{distribution} != "mnb"
Requires: libX11 libXt 
%else
Requires: libx11_6 libxt_6
%endif
Requires: agg boost libcurl SDL
# YellowDog doesn't ship ffmpeg
%if %{distribution} != "ydl6"
Requires: ffmpeg
%endif

# BuildRequires:  scrollkeeper

#Requires(post): scrollkeeper
#Requires(postun): scrollkeeper
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
#Requires(post): /sbin/install-info
#Requires(preun): /sbin/install-info

%description
Gnash is a GNU SWF movie player that supports many SWF v7 features, with growing support for swf v8 and v9.

%package plugin
Summary:   Web-client SWF player plugin 
Requires:  %{name} = %{version}-%{release}
Requires: gstreamer >= 0.10 gnash
Group:     Applications/Internet

%description plugin
The gnash SWF player plugin for firefox or mozilla.

%package cygnal
Summary:   Streaming media server
Requires:  %{name} = %{version}-%{release}
Group:     Applications/Multimedia

%description cygnal
Cygnal is a streaming media server that's Flash aware.

%prep
%setup -q

%build

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

# FIXME: this ia a bad hack! Although all this does work correctly and
# build an RPM, it's set for an geode-olpc, so the actual hardware
# won't let us install it.
# %define cross_compile 0
# %define olpc 0

# Build rpms for an ARM based processor, in our case the Nokia 770/800
# tablet. 
%ifarch arm
RPM_TARGET=%{_target}
%endif

%if %{cross_compile}
# cross building an RPM. This works as long as you have a good cross
# compiler installed. We currently do want to cross compile the
# Mozilla plugin, but not the Konqueror one till we make KDE work
# better than it does now.
  CROSS_OPTS="--build=%{_host} --host=$RPM_TARGET --target=$RPM_TARGET"
  RENDERER="--enable-renderer=agg"		# could be opengl
  %ifarch arm
    SOUND="--enable-media=none --disable-nsapi --disable-kparts"
  %else
    SOUND="--enable-media=gst"			# could also be sdl
  %endif
# The OLPC is a weird case, it's basically an i386-linux toolchain
# targeted towards Fedora Core 6. The machine itself is too limited to
# build RPMs on, so we do it this way.
  %if olpc
    CROSS_OPTS="$CROSS_OPTS --disable-kparts --disable-menus"
    SOUND="--enable-media=gst --enable-jemalloc"
    GUI="--enable-gui=gtk"
    RENDERER="$RENDERER --with-pixelformat=RGB565"
  %endif
%else
# Native RPM build
  CROSS_OPTS="" # "--enable-ghelp --enable-docbook"
  GUI="--enable-gui=gtk"
  SOUND="--enable-media=ffmpeg --enable-jemalloc"
  RENDERER="" # --enable-render=ogl
%endif

%if %{distribution} != "ydl6"
  SOUND="--enable-media=gst" 
%endif

# we disable the testuites by default, as when building packages we
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
	--disable-dependency-tracking \
	--disable-testsuite \
	--disable-rpath \
	--with-plugindir=%{_libdir}/mozilla/plugins

make $(MAKEFLAGS) dumpconfig all
%else
./configure \
	$CROSS_OPTS \
	$SOUND $GUI \
	$RENDERER \
	--disable-dependency-tracking \
	--disable-rpath \
	--enable-cygnal \
	--disable-testsuite \
        --prefix=/usr \
	--mandir=%{_prefix}/share/man \
	--infodir=%{_prefix}/share/info \
        --with-npapi-install=system 
#	--with-npapi-plugindir=%{_libdir}/mozilla/plugins

make $(MAKEFLAGS) dumpconfig all
%endif
# When testing the spec file, try setting MAKEFLAGS to
# "CXXFLAGS-O0 -j4" to speed up getting results. Note *don't*
# do that for release buulds, as the performance will suffer.

%install
strip gui/.libs/*-gnash
strip utilities/.libs/dumpshm  utilities/.libs/g*  utilities/.libs/soldumper utilities/.libs/flvdumper cygnal/.libs/cygnal
rm -rf $RPM_BUILD_ROOT
make install install-plugins DESTDIR=$RPM_BUILD_ROOT
rm $RPM_BUILD_ROOT%{_libdir}/gnash/*.*a
%if !%{cross_compile}

rm -rf $RPM_BUILD_ROOT%{_localstatedir}/scrollkeeper
rm -f $RPM_BUILD_ROOT%{_infodir}/dir
%endif

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
%dump
%doc README AUTHORS COPYING NEWS 
%{_bindir}/gnash
%{_bindir}/gtk-gnash
%{_bindir}/gprocessor
%{_bindir}/soldumper
%{_bindir}/flvdumper
%{_bindir}/dumpshm
%{_bindir}/cygnal
%{_libdir}/gnash/*.so*
%{_libdir}/mozilla/plugins/*.so
%{_prefix}/share/gnash/GnashG.png
%{_prefix}/share/gnash/gnash_128_96.ico
%{_datadir}/man/man1/*.1*
%{_datadir}/locale/*/LC_MESSAGES/gnash.mo
%if !%{cross_compile}
#%{_prefix}/share/info/*.info*
%{_prefix}/share/doc/gnash/*.html
%{_prefix}/share/doc/gnash/images/*.png
%{_prefix}/etc/gnashrc
%{_prefix}/etc/gnashpluginrc
# %{_infodir}/*.info*
#%doc doc/C/gnash*.html 
#%doc doc/C/images/*.png
#%doc doc/C/images/*.txt
# %doc %{_prefix}/share/gnash/doc/gnash/C/images
# %doc %{_prefix}/share/gnash/doc/gnash/C/*.xml
%endif

%files plugin
%defattr(-,root,root,-)
%{_libdir}/mozilla/plugins/libgnashplugin.so

%files cygnal
%defattr(-,root,root,-)
%{_bindir}/cygnal

%changelog
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

