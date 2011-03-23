%define pfx /opt/freescale/rootfs/%{_target_cpu}

Name:           gnash
Version:        0.8.9dev
Release:        0
Epoch: 		1
Distribution:	ltib
Summary:        GNU SWF player

Group:          Applications/Multimedia
Vendor:		Gnash Project
Packager:	Rob Savoye <rob@senecass.com>
License:        GPLv3
URL:            http://www.gnu.org/software/gnash/
Source:         gnash-0.8.9dev.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-%{_target_cpu}

# BuildRequires:  ffmpeg libpng libjpeg giflib boost agg curl freetype libstdc++ fontconfig

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description
Gnash is a GNU SWF movie player that supports many SWF v7 features,
with growing support for swf v8, v9, and v10.

%package common
Summary:   Web-client SWF player plugin 
Group:     Applications/Multimedia
# Installation requirements
Requires:  ffmpeg libpng libjpeg giflib boost curl freetype

%description common
Common files Shared between Gnash and Klash, Gnash/Klash is a GNU SWF movie
player that supports many SWF v7 features, with growing support for
swf v8, v9, and v10.

%package devel
Summary:   Gnash header files
Group:     Applications/Multimedia
Requires:  gnash-common

%description devel
Gnash header files can be used to write external Gnash extensions.

%prep
%setup -q

%build

CROSS_OPTS="--host=arm-none-linux=gnueabi --with-sysroot=/opt/L2.6.31_10.07.11_ER/ltib/rootfs/usr"
# these are actually the default values, but this way they get added
# to the build so they appear in "gnash --version".
GUI="--enable-gui=fb"	# could be kde3, qt4, aqua, sdl
SOUND="--enable-media=ffmpeg"
OTHER="--disable-jemalloc"
RENDERER="--enable-renderer=agg"		# could be opengl or cairo
OPTIONAL=""

# we disable the testsuites by default, as when building packages we
# should have already been running the testsuites as part of the 
# normal build & test development cycle.

# The default options for the configure aren't suitable for
# cross configuring, so we force them to be what we know is correct.
# uncommenting these will produce huge volumes of debug info from the
# shell, but sometimes that's what you need to do.
# export CONFIG_SHELL="sh -x"
# sh -x ./configure \
sh ./configure \
	$CROSS_OPTS \
	$SOUND $GUI \
	$RENDERER \
	$OTHER \
	$OPTIONAL \
        --prefix=/usr \
	--mandir=%{_prefix}/share/man \
	--infodir=%{_prefix}/share/info \
	--disable-dependency-tracking \
	--disable-testsuite \
	--disable-rpath \
        --enable-sound=none
make $MAKEFLAGS dumpconfig all

# When testing the spec file, try setting MAKEFLAGS to
# "CXXFLAGS-O0 -j4" to speed up getting results. Note *don't*
# do that for release builds, as the performance will suffer.

%install
strip gui/.libs/*-gnash
strip utilities/.libs/g* utilities/.libs/soldumper utilities/.libs/flvdumper
rm -rf $RPM_BUILD_ROOT
make $MAKEFLAGS install DESTDIR=$RPM_BUILD_ROOT LDFLAGS="-Wl,--build-id"
make $MAKEFLAGS install-plugins DESTDIR=$RPM_BUILD_ROOT LDFLAGS="-Wl,--build-id"
rm $RPM_BUILD_ROOT%{_libdir}/gnash/*.*a
%clean
rm -rf $RPM_BUILD_ROOT

%post 
/sbin/ldconfig

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
%{_bindir}/fb-gnash
# %{_datadir}/man/man1/gtk-gnash.1.gz

%files common
%defattr(-,root,root,-)
%dump
%doc README AUTHORS COPYING NEWS 
%{_bindir}/gnash
%{_datadir}/man/man1/gnash.1.gz
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
%{_datadir}/man/man1/gprocessor.1.gz
%{_datadir}/man/man1/soldumper.1.gz
%{_datadir}/man/man1/flvdumper.1.gz
%{_datadir}/man/man1/findmicrophones.1.gz
%{_datadir}/man/man1/findwebcams.1.gz
%{_datadir}/man/man1/rtmpget.1.gz
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

%files devel
%{_prefix}/include/gnash/*.h*
%{_prefix}/include/gnash/vm/*.h
%{_prefix}/include/gnash/asobj/*.h
%{_prefix}/include/gnash/parser/*.h
%{_prefix}/lib/pkgconfig/gnash.pc

%changelog
* Sat Nov 30 2010 Rob Savoye <rob@welcomehome.org> - %{version}-%{release}
- Drop most of the packages for ltib builds.

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

