Name:           gnash
Version:        0.7.1
Release:        1%{?dist}
Summary:        GNU flash movie player

Group:          Applications/Multimedia
License:        GPL
URL:            http://www.gnu.org/software/gnash/
Source0:        http://www.gnu.org/software/gnash/releases/%{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  libxml2-devel libpng-devel libjpeg-devel libogg-devel
BuildRequires:  boost-devel curl-devel 
# the opengl devel packages are required by gtkglext-devel
# monolithic Xorg
#BuildRequires:  xorg-x11-devel
# modular Xorg 
#BuildRequires:  libGLU-devel libGL-devel
# SDL-devel is required by SDL_mixer-devel
#BuildRequires:  SDL-devel 
BuildRequires:  SDL_mixer-devel
BuildRequires:  kdelibs-devel
BuildRequires:  gtkglext-devel
BuildRequires:  docbook2X
# GStreamer isn't used yet, but will be in the near future.
BuildRequires:  gstreamer-devel
#BuildRequires:  gstreamer-devel >= 0.10
BuildRequires:  scrollkeeper

Requires(post): scrollkeeper
Requires(postun): scrollkeeper
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
Requires(post): /sbin/install-info
Requires(preun): /sbin/install-info

%description
Gnash is a GNU Flash movie player based on GameSWF,
and supports many SWF v7 features.

%package plugin
Summary:   Web-client flash movie player plugin 
Requires:  %{name} = %{version}-%{release}
Requires:  webclient
Group:     Applications/Internet

%description plugin
The gnash flash movie player plugin for firefox or mozilla.

%package klash
Summary:   Konqueror flash movie player plugin
Requires:  %{name} = %{version}-%{release}
Group:     Applications/Multimedia

%description klash
The gnash flash movie player plugin for Konqueror.

%prep
%setup -q

%build
[ -n "$QTDIR" ] || . %{_sysconfdir}/profile.d/qt.sh
%configure --disable-static --with-plugindir=%{_libdir}/mozilla/plugins \
  --enable-ghelp --enable-docbook --enable-klash --enable-plugin \
  --disable-dependency-tracking --disable-rpath --enable-sound=gst \
  --with-qtdir=$QTDIR
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
rm $RPM_BUILD_ROOT/%{_libdir}/*.la
rm $RPM_BUILD_ROOT/%{_libdir}/*.so
rm -rf $RPM_BUILD_ROOT/%{_localstatedir}/scrollkeeper
rm -f $RPM_BUILD_ROOT%{_infodir}/dir

%clean
rm -rf $RPM_BUILD_ROOT


%post 
/sbin/ldconfig
scrollkeeper-update -q -o %{_datadir}/omf/%{name} || :
/sbin/install-info --entry="* Gnash: (gnash). GNU Flash Player" %{_infodir}/%{name}.info %{_infodir}/dir || :

%preun
if [ $1 = 0 ]; then
    /sbin/install-info --delete %{_infodir}/%{name}.info %{_infodir}/dir || :
fi

%postun
/sbin/ldconfig
scrollkeeper-update -q || :

%files
%defattr(-,root,root,-)
%doc README AUTHORS COPYING NEWS 
%doc doc/C/gnash.html 
%doc doc/C/images
%{_bindir}/gnash
%{_bindir}/gparser
%{_bindir}/gprocessor
%{_libdir}/libgnash*.so.*
%{_mandir}/man1/gnash*
%{_infodir}/gnash*
%{_datadir}/gnash/
%{_datadir}/omf/gnash/

%files plugin
%defattr(-,root,root,-)
%{_libdir}/mozilla/plugins/libgnashplugin.so

%files klash
%defattr(-,root,root,-)
%{_bindir}/klash
%{_libdir}/kde3/libklashpart.*
%{_datadir}/apps/klash/
%{_datadir}/config/klashrc
%{_datadir}/services/klash_part.desktop

%changelog
* Sat Apr  22 2006 Rob Savoye <rob@welcomehome.org> - 0.7-1
- install the info file. Various tweaks for my system based on
Patrice's latest patch,

* Fri Feb  3 2006 Patrice Dumas <dumas@centre-cired.fr> - 0.7-0
- initial packaging

