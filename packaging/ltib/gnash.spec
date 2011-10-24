#
# This spec file is specific to the ltib build system
#
%define pfx /opt/freescale/rootfs/%{_target_cpu}

Name:           gnash
Version:        20110912
Release:        0
Epoch: 		0
Distribution:	ltib
Summary:        GNU SWF player

Group:          Applications/Multimedia
Vendor:		Gnash Project
Packager:	Rob Savoye <rob@senecass.com>
License:        GPLv3
URL:            http://www.gnu.org/software/gnash/
Source:         gnash-%{version}.tar.bz2
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-%{_target_cpu}

# BuildRequires:  ffmpeg libpng libjpeg giflib boost agg curl freetype libstdc++ fontconfig
Prefix:         /usr

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description
Gnash is a GNU SWF movie player that supports many SWF v7 features,
with growing support for swf v8, v9, and v10.
Requires:  ffmpeg libpng libjpeg giflib boost curl freetype

%prep
%setup -q

%build

# Allow to change the sysroot path used by Gnash to find all headers and libraries
# using a GNASH_SYSROOT environment variable
if test x"${GNASH_SYSROOT}" = x; then
  # If no default sysroot is specified, look for one that should be at this location
  absroot=`cd ../../../rootfs/usr/ ; pwd`
  echo "GNASH_SYSROOT not set in the users environment, using \"${absroot}\" for sysroot"
  CROSS_OPTS="--host=arm-none-linux-gnueabi --with-sysroot=$absroot"
else
  CROSS_OPTS="--host=arm-none-linux-gnueabi --with-sysroot=${GNASH_SYSROOT}"
fi

# uncommenting these will produce huge volumes of debug info from the
# shell, but sometimes that's what you need to do.
# export CONFIG_SHELL="sh -x"
sh ./configure \
	$CROSS_OPTS \
        --prefix=/usr \
	--mandir=%{_prefix}/share/man \
	--infodir=%{_prefix}/share/info \
	--disable-dependency-tracking \
	--disable-testsuite \
	--disable-rpath \
        --disable-jemalloc \
        --enable-gui=fb \
	--disable-sound \
        --enable-media=ffmpeg \
        --enable-renderer=ovg \
        --enable-device=egl,rawfb  \
        CXXFLAGS='-g -O0'
# Only uncomment this for debugging
#        2>&1 | tee tmp/xxx

make $MAKEFLAGS all -w
make $MAKEFLAGS -C libdevice check -w
make $MAKEFLAGS -C librender check -w

# When testing the spec file, try setting MAKEFLAGS to
# "CXXFLAGS-O0 -j4" to speed up getting results. Note *don't*
# do that for release builds, as the performance will suffer.

%install
#strip gui/.libs/*-gnash
rm -rf $RPM_BUILD_ROOT
make $MAKEFLAGS install DESTDIR=$RPM_BUILD_ROOT
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

%files
%defattr(-,root,root,-)
%{_bindir}/fb-gnash
%defattr(-,root,root,-)
#%dump
%doc README AUTHORS COPYING NEWS 
%{_bindir}/gnash
%{_bindir}/gprocessor
#%{_bindir}/dumpshm
%{_bindir}/rtmpget
%{_libdir}/gnash/*.so*
%{_prefix}/share/gnash/GnashG.png
%{_prefix}/share/gnash/gnash_128_96.ico
%{_datadir}/locale/*/LC_MESSAGES/gnash.mo

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

