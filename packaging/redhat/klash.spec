Name:           gnash
Version:        20090213
Release:        1
Distribution:	fc10
Summary:        GNU SWF player

Group:          Applications/Multimedia
Vendor:		Gnash Project
Packager:	Rob Savoye <rob@welcomehome.org>
License:        GPLv3
URL:            http://www.gnu.org/software/gnash/
Source0:        http://www.getgnash.org/packages/snapshots/fedora/%{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-%{_target_cpu}


BuildRequires:  libpng-devel libjpeg-devel libogg-devel
BuildRequires:  libX11-devel libXt-devel
BuildRequires:  agg-devel boost-devel curl-devel libXt-devel
BuildRequires:  xorg-x11-proto-devel SDL-devel
BuildRequires:  kdelibs-devel kdebase-devel qt-devel
BuildRequires:  ffmpeg-devel

# Installation requirements
Requires: libpng libjpeg libogg
Requires: libX11 libXt
Requires: agg boost libcurl libXt SDL
Requires: ffmpeg

# BuildRequires:  scrollkeeper

#Requires(post): scrollkeeper
#Requires(postun): scrollkeeper
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
#Requires(post): /sbin/install-info
#Requires(preun): /sbin/install-info

%description
Gnash is a GNU SWF movie player that supports many SWF v7 features, with growing support for swf v8 and v9.

%package klash3
Summary:   Konqueror SWF player plugin for KDE 3
Requires:  %{name} = %{version}-%{release}
Requires:  kdelibs kdelibs qt gnash
Group:     Applications/Multimedia

%description klash3
The gnash SWF player plugin for Konqueror in KDE 3.

%package klash4
Summary:   Konqueror SWF player plugin for KDE 4
Requires:  %{name} = %{version}-%{release}
Requires:  kdelibs kdelibs qt gnash
Group:     Applications/Multimedia

%description klash4
The gnash SWF player plugin for Konqueror in KDE4.

%prep
%setup -q

%build

[ -n "$QTDIR" ] || . %{_sysconfdir}/profile.d/qt.sh

# Native RPM build
CROSS_OPTS="" # "--enable-ghelp --enable-docbook"
GUI="" # --enable-gui=gtk,kde3,qt4
SOUND="--enable-media=ffmpeg --enable-jemalloc"
RENDERER="" # --enable-render=ogl
# KDE3_OPTS="--with-kde3-plugindir=%{_libdir}/kde3/plugins \
#         --with-kde3-pluginprefix=%{_prefix} \
#         --with-kde3-servicesdir=%{_prefix}/share/services \
#         --with-kde3-appsdatadir=%{_prefix}/share/apps/klash \
#         --with-kde3-configdir=${_datadir}/config"
# KDE4_OPTS="--with-kde4-plugindir=%{_libdir}/kde4/plugins \
#         --with-kde4-pluginprefix=%{_prefix} \
#         --with-kde4-servicesdir=%{_prefix}/share/services \
#         --with-kde4-appsdatadir=%{_prefix}/share/apps/klash \
#         --with-kde4-configdir=${_datadir}/config"

# we disable the testuites by default, as when building packages we
# should have already been running the testsuites as part of the 
# normal build & test development cycle.
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
        --with-plugins-install=system

make $(MAKEFLAGS) dumpconfig all
# When testing the spec file, try setting MAKEFLAGS to
# "CXXFLAGS-O0 -j4" to speed up getting results. Note *don't*
# do that for release buulds, as the performance will suffer.

%install
strip gui/.libs/kde*-gnash
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
%{_bindir}/kde*-gnash
%{_bindir}/gprocessor
%{_bindir}/soldumper
%{_bindir}/flvdumper
%{_bindir}/dumpshm
%{_bindir}/cygnal
%{_libdir}/gnash/*.so*
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

%files klash3
%defattr(-,root,root,-)
%if !%{cross_compile}
%{_bindir}/kde3-gnash
%{_libdir}/kde3/libklashpart.*
%{_prefix}/share/apps/klash/klashpartui.rc
%{_prefix}/share/apps/klash/pluginsinfo
%{_prefix}/share/services/klash_part.desktop
%endif

%files klash4
%defattr(-,root,root,-)
%if !%{cross_compile}
%{_bindir}/kde4-gnash
%{_bindir}/gnash
%{_libdir}/kde4/libklashpart.*
%{_prefix}/share/apps/klash/klashpartui.rc
%{_prefix}/share/apps/klash/pluginsinfo
%{_prefix}/share/services/klash_part.desktop
%endif

%changelog
* Sat Feb 13 2009 Rob Savoye <rob@welcomehome.org> - trunk
- Split off from gnash.spec

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

